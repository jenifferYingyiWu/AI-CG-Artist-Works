// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "DetailCategoryBuilder.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "MetasoundFrontendDataTypeRegistry.h"
#include "MetasoundFrontendLiteral.h"
#include "MetasoundFrontendRegistries.h"
#include "Modules/ModuleInterface.h"
#include "PropertyHandle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Templates/Function.h"

// Forward Declarations
class IDetailLayoutBuilder;
class IDetailPropertyRow;
class UEdGraphPin;
class UMetasoundEditorGraph;
class UMetasoundEditorGraphMemberDefaultLiteral;

DECLARE_LOG_CATEGORY_EXTERN(LogMetasoundEditor, Log, All);


namespace Metasound
{
	namespace Editor
	{
		namespace Style
		{
			METASOUNDEDITOR_API FSlateIcon CreateSlateIcon(FName InName);
			METASOUNDEDITOR_API const FSlateBrush& GetSlateBrushSafe(FName InName);
		} // namespace Style

		// Status of initial asset scan when editor loads up.
		enum class EAssetScanStatus : uint8
		{
			NotRequested = 0,
			InProgress = 2,
			Complete = 3
		};

		// Primes status of MetaSound assets.  Priming an asset
		// effectively loading the asset asynchronously (if not already loaded)
		// & registers it with the MetaSound Class Registry.
		enum class EAssetPrimeStatus : uint8
		{
			NotRequested = 0,
			Requested = 1,
			InProgress = 2,
			Complete = 3
		};

		struct FGraphPinParams
		{
			FName PinCategory = {};
			FName PinSubcategory = {};
			const FLinearColor* PinColor = nullptr;
			const FSlateBrush* PinConnectedIcon = nullptr;
			const FSlateBrush* PinDisconnectedIcon = nullptr;
		};

		class METASOUNDEDITOR_API FMetasoundDefaultLiteralCustomizationBase
		{
		protected:
			IDetailCategoryBuilder* DefaultCategoryBuilder = nullptr;

		public:
			FMetasoundDefaultLiteralCustomizationBase(IDetailCategoryBuilder& InDefaultCategoryBuilder)
				: DefaultCategoryBuilder(&InDefaultCategoryBuilder)
			{
			}

			virtual ~FMetasoundDefaultLiteralCustomizationBase() = default;

			// Customizes the given literal for the provided DetailLayoutBuilder.
			// @return the DetailPropertyRow created for the default parameter set by this customization.
			virtual TArray<IDetailPropertyRow*> CustomizeLiteral(UMetasoundEditorGraphMemberDefaultLiteral& InLiteral, IDetailLayoutBuilder& InDetailLayout) { return { }; };
		};

		class METASOUNDEDITOR_API IMemberDefaultLiteralCustomizationFactory
		{
		public:
			virtual ~IMemberDefaultLiteralCustomizationFactory() = default;

			virtual TUniquePtr<FMetasoundDefaultLiteralCustomizationBase> CreateLiteralCustomization(IDetailCategoryBuilder& DefaultCategoryBuilder) const = 0;
		};

		class METASOUNDEDITOR_API IMetasoundEditorModule : public IModuleInterface
		{
		public:
			UE_DEPRECATED(5.3, "IsExplicitProxyClass is deprecated, use Metasound::Frontend::FDataTypeRegistryInfo::bIsExplicit")
			virtual bool IsExplicitProxyClass(const UClass& InClass) const = 0;

			UE_DEPRECATED(5.3, "RegisterExplicitProxyClass is deprecated, use Metasound::TIsExplicit<>")
			virtual void RegisterExplicitProxyClass(const UClass& InClass) = 0;

			UE_DEPRECATED(5.3, "IsMetaSoundAssetClass is deprecated, use IMetasoundUObjectRegistry::IsRegisteredClass")
			virtual bool IsMetaSoundAssetClass(const FTopLevelAssetPath& InClassName) const = 0;

			// Primes MetaSound assets, effectively loading the asset asynchronously (if not already
			// loaded) & registers them if not already registered with the MetaSound Class Registry.
			virtual void PrimeAssetRegistryAsync() = 0;
			virtual EAssetPrimeStatus GetAssetRegistryPrimeStatus() const = 0;

			virtual TUniquePtr<FMetasoundDefaultLiteralCustomizationBase> CreateMemberDefaultLiteralCustomization(UClass& InClass, IDetailCategoryBuilder& DefaultCategoryBuilder) const = 0;

			virtual const TSubclassOf<UMetasoundEditorGraphMemberDefaultLiteral> FindDefaultLiteralClass(EMetasoundFrontendLiteralType InLiteralType) const = 0;

			virtual const FEdGraphPinType* FindPinType(FName InDataTypeName) const = 0;

			virtual const FSlateBrush* GetIconBrush(FName InDataType, const bool bIsConstructorType) const = 0;

			virtual bool GetCustomPinIcons(UEdGraphPin* InPin, const FSlateBrush*& PinConnectedIcon, const FSlateBrush*& PinDisconnectedIcon) const = 0;
			virtual bool GetCustomPinIcons(FName InDataType, const FSlateBrush*& PinConnectedIcon, const FSlateBrush*& PinDisconnectedIcon) const = 0;
			
			virtual void RegisterPinType(FName InDataTypeName, FName InPinCategory = { }, FName InPinSubCategory = { },
				const FSlateBrush* InPinConnectedIcon = nullptr, const FSlateBrush* InPinDisconnectedIcon = nullptr) = 0;
			
			virtual void RegisterCustomPinType(FName InDataTypeName, const FGraphPinParams& Params) = 0;
		};
	} // namespace Editor
} // namespace Metasound
