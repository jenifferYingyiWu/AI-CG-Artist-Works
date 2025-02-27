// Copyright Epic Games, Inc. All Rights Reserved.

#include "USDGroomTranslator.h"

#if USE_USD_SDK && WITH_EDITOR

#include "GroomAsset.h"
#include "GroomBuilder.h"
#include "GroomCache.h"
#include "GroomCacheImporter.h"
#include "GroomComponent.h"
#include "GroomImportOptions.h"
#include "HairDescription.h"
#include "HairStrandsImporter.h"
#include "Misc/ArchiveMD5.h"
#include "Templates/UniquePtr.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#include "UnrealUSDWrapper.h"
#include "USDAssetUserData.h"
#include "USDClassesModule.h"
#include "USDDrawModeComponent.h"
#include "USDGroomConversion.h"
#include "USDGroomTranslatorUtils.h"
#include "USDIntegrationUtils.h"
#include "USDLog.h"
#include "USDPrimConversion.h"

#include "USDIncludesStart.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/timeCode.h"
#include "pxr/usd/usdGeom/basisCurves.h"
#include "pxr/usd/usdGeom/imageable.h"
#include "USDIncludesEnd.h"

namespace UE::UsdGroomTranslator::Private
{
	UGroomImportOptions* CreateGroomImportOptions(
		const FHairDescriptionGroups& GroupsDescription,
		const TArray<FHairGroupsInterpolation>& BuildSettings
	)
	{
		// Create a new groom import options and populate the interpolation settings based on the group count
		UGroomImportOptions* ImportOptions = NewObject<UGroomImportOptions>();
		const uint32 GroupCount = GroupsDescription.HairGroups.Num();
		if (GroupCount != uint32(ImportOptions->InterpolationSettings.Num()))
		{
			ImportOptions->InterpolationSettings.Init(FHairGroupsInterpolation(), GroupCount);
		}

		TOptional<FHairGroupsInterpolation> LastBuildSettings;
		if (BuildSettings.Num() > 0)
		{
			LastBuildSettings = BuildSettings.Last();
		}

		// If there are less build settings than groups, use the last one that was specified by the user
		for (uint32 Index = 0; Index < GroupCount; ++Index)
		{
			if (BuildSettings.IsValidIndex(Index))
			{
				ImportOptions->InterpolationSettings[Index] = BuildSettings[Index];
			}
			else if (LastBuildSettings.IsSet())
			{
				ImportOptions->InterpolationSettings[Index] = LastBuildSettings.GetValue();
			}
		}
		return ImportOptions;
	}

	FSHAHash ComputeHairDescriptionHash(FHairDescription& HairDescription, TArray<FHairGroupsInterpolation>& BuildSettings)
	{
		// The computed hash takes the hair description and group build settings into account since the groom builder
		// will use those settings to build the groom asset from the description
		FArchiveMD5 ArMD5;
		HairDescription.Serialize(ArMD5);

		for (FHairGroupsInterpolation& GroupSettings : BuildSettings)
		{
			GroupSettings.BuildDDCKey(ArMD5);
		}

		FMD5Hash MD5Hash;
		ArMD5.GetHash(MD5Hash);

		FSHA1 SHA1;
		SHA1.Update(MD5Hash.GetBytes(), MD5Hash.GetSize());
		SHA1.Final();

		FSHAHash SHAHash;
		SHA1.GetHash(SHAHash.Hash);

		return SHAHash;
	}

	void ComputeFrameHairDescriptionHash(const pxr::UsdPrim& Prim, TArray<FHairGroupsInterpolation>& BuildSettings, int32 FrameNumber, FSHA1& Hash)
	{
		FHairDescription FrameHairDescription;
		if (UsdToUnreal::ConvertGroomHierarchy(Prim, pxr::UsdTimeCode(FrameNumber), FTransform::Identity, FrameHairDescription))
		{
			FHairDescriptionGroups GroupsDescription;
			FGroomBuilder::BuildHairDescriptionGroups(FrameHairDescription, GroupsDescription);

			FSHAHash SHAHash = ComputeHairDescriptionHash(FrameHairDescription, BuildSettings);
			Hash.Update((const uint8*)SHAHash.Hash, sizeof(SHAHash.Hash));
		}
	}
}	 // namespace UE::UsdGroomTranslator::Private

class FUsdGroomCreateAssetsTaskChain : public FUsdSchemaTranslatorTaskChain
{
public:
	explicit FUsdGroomCreateAssetsTaskChain(const TSharedRef<FUsdSchemaTranslationContext>& InContext, const UE::FSdfPath& InPrimPath)
		: PrimPath(InPrimPath)
		, Context(InContext)
	{
		AnimInfo.Attributes = EGroomCacheAttributes::None;
		AnimInfo.StartFrame = TNumericLimits<int32>::Max();
		AnimInfo.EndFrame = TNumericLimits<int32>::Min();

		SetupTasks();
	}

protected:

	UE::FSdfPath PrimPath;
	TSharedRef<FUsdSchemaTranslationContext> Context;

	FHairDescription HairDescription;

	TStrongObjectPtr<UGroomImportOptions> ImportOptions;
	TUniquePtr<FGroomCacheProcessor> GroomCacheProcessor;
	FGroomAnimationInfo AnimInfo;
	FString PrefixedGroomCacheHash;

protected:
	UE::FUsdPrim GetPrim() const
	{
		return Context->Stage.GetPrimAtPath(PrimPath);
	}

protected:
	void SetupTasks()
	{
		FScopedUnrealAllocs UnrealAllocs;

		// Create hair description (Async)
		Do(ESchemaTranslationLaunchPolicy::Async,
		   [this]() -> bool
		   {
			   const bool bSuccess = UsdToUnreal::ConvertGroomHierarchy(
				   GetPrim(),
				   pxr::UsdTimeCode::EarliestTime(),
				   FTransform::Identity,
				   HairDescription,
				   &AnimInfo
			   );

			   const double StageTimeCodesPerSecond = Context->Stage.GetTimeCodesPerSecond();
			   AnimInfo.SecondsPerFrame = float(1.0f / StageTimeCodesPerSecond);

			   if (bSuccess && AnimInfo.IsValid())
			   {
				   AnimInfo.Duration = AnimInfo.NumFrames * AnimInfo.SecondsPerFrame;
				   AnimInfo.StartTime = AnimInfo.StartFrame * AnimInfo.SecondsPerFrame;
				   AnimInfo.EndTime = AnimInfo.EndFrame * AnimInfo.SecondsPerFrame;
			   }

			   return bSuccess && HairDescription.IsValid();
		   });
		// Build groom asset from hair description (Sync)
		Then(
			ESchemaTranslationLaunchPolicy::Sync,
			[this]()
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(FUsdGroomCreateAssetsTaskChain::Build);

				// Extract the groom groups info from the hair description to get the number of groups
				FHairDescriptionGroups GroupsDescription;
				FGroomBuilder::BuildHairDescriptionGroups(HairDescription, GroupsDescription);

				ImportOptions.Reset(UE::UsdGroomTranslator::Private::CreateGroomImportOptions(GroupsDescription, Context->GroomInterpolationSettings)
				);

				FSHAHash SHAHash = UE::UsdGroomTranslator::Private::ComputeHairDescriptionHash(HairDescription, ImportOptions->InterpolationSettings);

				FString PrefixedAssetHash = UsdUtils::GetAssetHashPrefix(GetPrim(), Context->bReuseIdenticalAssets) + SHAHash.ToString();

				const FString PrimPathString = PrimPath.GetString();
				UGroomAsset* GroomAsset = Cast<UGroomAsset>(Context->AssetCache->GetCachedAsset(PrefixedAssetHash));
				if (!GroomAsset)
				{
					FName AssetName = MakeUniqueObjectName(
						GetTransientPackage(),
						UGroomAsset::StaticClass(),
						*IUsdClassesModule::SanitizeObjectName(FPaths::GetBaseFilename(PrimPathString))
					);

					FHairImportContext HairImportContext(
						ImportOptions.Get(),
						GetTransientPackage(),
						UGroomAsset::StaticClass(),
						AssetName,
						Context->ObjectFlags | RF_Public | RF_Transient
					);
					UGroomAsset* ExistingAsset = nullptr;
					GroomAsset = FHairStrandsImporter::ImportHair(HairImportContext, HairDescription, ExistingAsset);
					if (GroomAsset)
					{
						Context->AssetCache->CacheAsset(PrefixedAssetHash, GroomAsset);

						if (UUsdAssetUserData* UserData = UsdUtils::GetOrCreateAssetUserData(GroomAsset))
						{
							UserData->PrimPaths.AddUnique(PrimPathString);

							if (Context->MetadataOptions.bCollectMetadata)
							{
								UsdToUnreal::ConvertMetadata(
									GetPrim(),
									UserData,
									Context->MetadataOptions.BlockedPrefixFilters,
									Context->MetadataOptions.bInvertFilters,
									Context->MetadataOptions.bCollectFromEntireSubtrees
								);
							}
							else
							{
								// Strip the metadata from this prim, so that if we uncheck "Collect Metadata" it actually disappears on the
								// AssetUserData
								UserData->StageIdentifierToMetadata.Remove(GetPrim().GetStage().GetRootLayer().GetIdentifier());
							}
						}
					}
				}

				if (GroomAsset && Context->InfoCache)
				{
					Context->InfoCache->LinkAssetToPrim(PrimPath, GroomAsset);
				}

				// Next step is to parse the GroomCache data if it was determined that the groom has animated attributes
				return GroomAsset != nullptr && AnimInfo.IsValid();
			}
		);
		// Parse GroomCache data into processor (Async)
		Then(
			ESchemaTranslationLaunchPolicy::Async,
			[this]() -> bool
			{
				if (!Context->InfoCache)
				{
					return false;
				}

				UGroomAsset* GroomAsset = Context->InfoCache->GetSingleAssetForPrim<UGroomAsset>(PrimPath);
				if (!GroomAsset)
				{
					return false;
				}

				// Compute GroomCache hash from the first and last frame HairDescription...
				FSHA1 SHA1;
				UE::UsdGroomTranslator::Private::ComputeFrameHairDescriptionHash(
					GetPrim(),
					ImportOptions->InterpolationSettings,
					AnimInfo.StartFrame,
					SHA1
				);
				UE::UsdGroomTranslator::Private::ComputeFrameHairDescriptionHash(
					GetPrim(),
					ImportOptions->InterpolationSettings,
					AnimInfo.EndFrame,
					SHA1
				);

				// Along with relevant AnimInfo data
				SHA1.Update((const uint8*)&AnimInfo.NumFrames, sizeof(AnimInfo.NumFrames));
				SHA1.Update((const uint8*)&AnimInfo.Attributes, sizeof(AnimInfo.Attributes));
				SHA1.Final();

				FSHAHash Hash;
				SHA1.GetHash(Hash.Hash);

				PrefixedGroomCacheHash = UsdUtils::GetAssetHashPrefix(GetPrim(), Context->bReuseIdenticalAssets) + Hash.ToString();

				bool bSuccess = true;
				UGroomCache* GroomCache = Cast<UGroomCache>(Context->AssetCache->GetCachedAsset(PrefixedGroomCacheHash));
				if (!GroomCache)
				{
					GroomCacheProcessor = MakeUnique<FGroomCacheProcessor>(EGroomCacheType::Strands, AnimInfo.Attributes);

					// ref. FGroomCacheImporter::ImportGroomCache
					const TArray<FHairGroupPlatformData>& GroomHairGroupsData = GroomAsset->GetHairGroupsPlatformData();

					// Each frame is translated into a HairDescription and processed into HairGroupData
					// Sample one extra frame so that we can interpolate between EndFrame - 1 and EndFrame
					for (int32 FrameIndex = AnimInfo.StartFrame; FrameIndex < AnimInfo.EndFrame + 1; ++FrameIndex)
					{
						FHairDescription FrameHairDescription;
						bSuccess = UsdToUnreal::ConvertGroomHierarchy(
							GetPrim(),
							pxr::UsdTimeCode(FrameIndex),
							FTransform::Identity,
							FrameHairDescription
						);

						if (!bSuccess)
						{
							break;
						}

						FHairDescriptionGroups HairDescriptionGroups;
						if (!FGroomBuilder::BuildHairDescriptionGroups(FrameHairDescription, HairDescriptionGroups))
						{
							bSuccess = false;
							break;
						}

						const uint32 GroupCount = HairDescriptionGroups.HairGroups.Num();

						TArray<FHairGroupInfoWithVisibility> HairGroupsInfo = GroomAsset->GetHairGroupsInfo();
						TArray<FHairDescriptionGroup> HairGroupsData;
						HairGroupsData.SetNum(GroupCount);
						for (uint32 GroupIndex = 0; GroupIndex < GroupCount; ++GroupIndex)
						{
							const FHairDescriptionGroup& HairGroup = HairDescriptionGroups.HairGroups[GroupIndex];
							FHairDescriptionGroup& HairGroupData = HairGroupsData[GroupIndex];
							FGroomBuilder::BuildData(
								HairGroup,
								GroomAsset->GetHairGroupsInterpolation()[GroupIndex],
								HairGroupsInfo[GroupIndex],
								HairGroupData.Strands,
								HairGroupData.Guides
							);
						}

						// Validate that the GroomCache has the same topology as the static groom
						if (HairGroupsData.Num() == GroomHairGroupsData.Num())
						{
							for (uint32 GroupIndex = 0; GroupIndex < GroupCount; ++GroupIndex)
							{
								if (HairGroupsData[GroupIndex].Strands.GetNumPoints()
									!= GroomHairGroupsData[GroupIndex].Strands.BulkData.GetNumPoints())
								{
									bSuccess = false;
									UE_LOG(
										LogUsd,
										Warning,
										TEXT("GroomCache frame %d does not have the same number of vertices as the static groom (%u instead of %u). "
											 "Aborting GroomCache import."),
										FrameIndex,
										HairGroupsData[GroupIndex].Strands.GetNumPoints(),
										GroomHairGroupsData[GroupIndex].Strands.BulkData.GetNumPoints()
									);
									break;
								}
							}
						}
						else
						{
							bSuccess = false;
							UE_LOG(
								LogUsd,
								Warning,
								TEXT("GroomCache does not have the same number of groups as the static groom (%d instead of %d). Aborting GroomCache "
									 "import."),
								HairGroupsData.Num(),
								GroomHairGroupsData.Num()
							);
						}

						if (!bSuccess)
						{
							break;
						}

						// The HairGroupData is converted into animated groom data by the GroomCacheProcessor
						GroomCacheProcessor->AddGroomSample(MoveTemp(HairGroupsData));
					}
				}

				if (Context->InfoCache && GroomCache)
				{
					Context->InfoCache->LinkAssetToPrim(PrimPath, GroomCache);
				}

				if (!bSuccess)
				{
					GroomCacheProcessor.Reset();
				}

				// Go to the next step only if it needs to create a GroomCache asset
				return GroomCache == nullptr && GroomCacheProcessor;
			}
		);
		// Create GroomCache asset from processor (Sync)
		Then(
			ESchemaTranslationLaunchPolicy::Sync,
			[this]() -> bool
			{
				// TEMP: This is a small trick to prevent two concurrent task chains from running into a hash collision in the asset cache.
				// This is enough of a workaround because this is a Sync task, so we can guarantee only one of the competing task chains will be
				// run at a time. Whichever wins gets to create the GroomCache, and the other will exit through this branch.
				// It will likely be properly fixed before 5.4 is out, but check UE-201011 for more details.
				UGroomCache* ExistingGroomCache = Cast<UGroomCache>(Context->AssetCache->GetCachedAsset(PrefixedGroomCacheHash));
				if (ExistingGroomCache)
				{
					Context->InfoCache->LinkAssetToPrim(PrimPath, ExistingGroomCache);
					return false;
				}

				const FString StrandsGroomCachePrimPath = UsdGroomTranslatorUtils::GetStrandsGroomCachePrimPath(PrimPath);
				FHairImportContext
					HairImportContext(nullptr, GetTransientPackage(), nullptr, FName(), Context->ObjectFlags | RF_Public | RF_Transient);
				FName UniqueName = MakeUniqueObjectName(
					GetTransientPackage(),
					UGroomCache::StaticClass(),
					*IUsdClassesModule::SanitizeObjectName(FPaths::GetBaseFilename(StrandsGroomCachePrimPath))
				);

				// Once the processing has completed successfully, the data is transferred to the GroomCache
				UGroomCache*
					GroomCache = FGroomCacheImporter::ProcessToGroomCache(*GroomCacheProcessor, AnimInfo, HairImportContext, UniqueName.ToString());
				if (GroomCache)
				{
					if (UUsdAssetUserData* UserData = UsdUtils::GetOrCreateAssetUserData(GroomCache))
					{
						UserData->PrimPaths.AddUnique(PrimPath.GetString());

						if (Context->MetadataOptions.bCollectMetadata)
						{
							UsdToUnreal::ConvertMetadata(
								GetPrim(),
								UserData,
								Context->MetadataOptions.BlockedPrefixFilters,
								Context->MetadataOptions.bInvertFilters,
								Context->MetadataOptions.bCollectFromEntireSubtrees
							);
						}
						else
						{
							// Strip the metadata from this prim, so that if we uncheck "Collect Metadata" it actually disappears on the AssetUserData
							UserData->StageIdentifierToMetadata.Remove(GetPrim().GetStage().GetRootLayer().GetIdentifier());
						}
					}

					Context->AssetCache->CacheAsset(PrefixedGroomCacheHash, GroomCache);
					Context->InfoCache->LinkAssetToPrim(PrimPath, GroomCache);
				}

				return GroomCache != nullptr;
			}
		);
	}
};

bool FUsdGroomTranslator::IsGroomPrim() const
{
	return UsdUtils::PrimHasSchema(GetPrim(), UnrealIdentifiers::GroomAPI);
}

void FUsdGroomTranslator::CreateAssets()
{
	if (!Context->bAllowParsingGroomAssets || !IsGroomPrim())
	{
		return Super::CreateAssets();
	}

	// Don't bother generating assets if we're going to just draw some bounds for this prim instead
	EUsdDrawMode DrawMode = UsdUtils::GetAppliedDrawMode(GetPrim());
	if (DrawMode != EUsdDrawMode::Default)
	{
		CreateAlternativeDrawModeAssets(DrawMode);
		return;
	}

	Context->TranslatorTasks.Add(MakeShared<FUsdGroomCreateAssetsTaskChain>(Context, PrimPath));
}

USceneComponent* FUsdGroomTranslator::CreateComponents()
{
	if (!Context->bAllowParsingGroomAssets || !IsGroomPrim())
	{
		return Super::CreateComponents();
	}

	// Display the groom as a standalone actor only if the stage loads the matching purpose.
	// The groom asset is processed regardless of the purpose so that it can be bound to mesh prims.
	if (!EnumHasAllFlags(Context->PurposesToLoad, IUsdPrim::GetPurpose(GetPrim())))
	{
		return nullptr;
	}

	USceneComponent* Component = nullptr;

	EUsdDrawMode DrawMode = UsdUtils::GetAppliedDrawMode(GetPrim());
	if (DrawMode == EUsdDrawMode::Default)
	{
		const bool bNeedsActor = true;
		Component = CreateComponentsEx({UGroomComponent::StaticClass()}, bNeedsActor);
	}
	else
	{
		Component = CreateAlternativeDrawModeComponents(DrawMode);
	}

	UpdateComponents(Component);

	return Component;
}

void FUsdGroomTranslator::UpdateComponents(USceneComponent* SceneComponent)
{
	if (Context->bAllowParsingGroomAssets && IsGroomPrim())
	{
		if (UGroomComponent* GroomComponent = Cast<UGroomComponent>(SceneComponent))
		{
			GroomComponent->Modify();

			UGroomAsset* Groom = nullptr;
			if (Context->InfoCache)
			{
				Groom = Context->InfoCache->GetSingleAssetForPrim<UGroomAsset>(PrimPath);
			}

			bool bShouldRegister = false;
			if (Groom != GroomComponent->GroomAsset.Get())
			{
				bShouldRegister = true;

				if (GroomComponent->IsRegistered())
				{
					GroomComponent->UnregisterComponent();
				}

				GroomComponent->SetGroomAsset(Groom);

				if (Groom)
				{
					UGroomCache* GroomCache = Context->InfoCache->GetSingleAssetForPrim<UGroomCache>(PrimPath);
					if (GroomCache != GroomComponent->GroomCache.Get())
					{
						GroomComponent->SetGroomCache(GroomCache);
					}
				}
			}

			// Use the prim purpose in conjunction with the prim's computed visibility to toggle the visibility of the groom component
			// since the component itself cannot be removed if the groom shouldn't be displayed
			const bool bShouldRender = UsdUtils::IsVisible(GetPrim()) && EnumHasAllFlags(Context->PurposesToLoad, IUsdPrim::GetPurpose(GetPrim()));
			GroomComponent->SetVisibility(bShouldRender);

			if (bShouldRegister && !GroomComponent->IsRegistered())
			{
				GroomComponent->RegisterComponent();
			}

			return;
		}
	}

	Super::UpdateComponents(SceneComponent);
}

bool FUsdGroomTranslator::CollapsesChildren(ECollapsingType CollapsingType) const
{
	if (!Context->bAllowParsingGroomAssets || !IsGroomPrim())
	{
		return Super::CollapsesChildren(CollapsingType);
	}

	return true;
}

bool FUsdGroomTranslator::CanBeCollapsed(ECollapsingType CollapsingType) const
{
	if (!Context->bAllowParsingGroomAssets || !IsGroomPrim())
	{
		return Super::CanBeCollapsed(CollapsingType);
	}

	return true;
}

TSet<UE::FSdfPath> FUsdGroomTranslator::CollectAuxiliaryPrims() const
{
	if (!Context->bAllowParsingGroomAssets || !IsGroomPrim())
	{
		return Super::CollectAuxiliaryPrims();
	}

	if (!Context->bIsBuildingInfoCache)
	{
		return Context->InfoCache->GetAuxiliaryPrims(PrimPath);
	}

	if (!Context->InfoCache->DoesPathCollapseChildren(PrimPath, ECollapsingType::Assets))
	{
		return {};
	}

	TSet<UE::FSdfPath> Result;
	{
		FScopedUsdAllocs UsdAllocs;

		TFunction<void(const pxr::UsdPrim&)> RecursivelyRegisterPrims;
		RecursivelyRegisterPrims = [&](const pxr::UsdPrim& UsdPrim)
		{
			if (pxr::UsdGeomBasisCurves Curves = pxr::UsdGeomBasisCurves(UsdPrim))
			{
				Result.Add(UE::FSdfPath{UsdPrim.GetPrimPath()});
			}
			else if (pxr::UsdGeomImageable(UsdPrim))
			{
				Result.Add(UE::FSdfPath{UsdPrim.GetPrimPath()});

				for (const pxr::UsdPrim& Child : UsdPrim.GetChildren())
				{
					RecursivelyRegisterPrims(Child);
				}
			}
		};

		pxr::UsdPrim Prim = GetPrim();
		RecursivelyRegisterPrims(Prim);
	}
	return Result;
}

#endif	  // #if USE_USD_SDK
