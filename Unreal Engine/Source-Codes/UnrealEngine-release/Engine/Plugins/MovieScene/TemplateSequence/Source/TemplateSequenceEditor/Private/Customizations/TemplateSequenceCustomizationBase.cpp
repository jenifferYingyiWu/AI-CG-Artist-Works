// Copyright Epic Games, Inc. All Rights Reserved.

#include "TemplateSequenceCustomizationBase.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "GameFramework/Actor.h"
#include "ISequencer.h"
#include "LevelSequenceFBXInterop.h"
#include "Misc/TemplateSequenceEditorUtil.h"
#include "ScopedTransaction.h"
#include "TemplateSequence.h"

#define LOCTEXT_NAMESPACE "TemplateSequenceCustomizationBase"

void FTemplateSequenceCustomizationBase::RegisterSequencerCustomization(FSequencerCustomizationBuilder& Builder)
{
	Sequencer = &Builder.GetSequencer();
	TemplateSequence = Cast<UTemplateSequence>(&Builder.GetFocusedSequence());

	Sequencer->OnCloseEvent().AddRaw(this, &FTemplateSequenceCustomizationBase::OnSequencerClosed);
	Sequencer->OnMovieSceneDataChanged().AddRaw(this, &FTemplateSequenceCustomizationBase::OnMovieSceneDataChanged);

	FSequencerCustomizationInfo BaseInfo;
	BaseInfo.OnPaste.BindRaw(this, &FTemplateSequenceCustomizationBase::OnPaste);
	BaseInfo.OnBuildObjectBindingContextMenu = FOnGetSequencerMenuExtender::CreateRaw(this, &FTemplateSequenceCustomizationBase::CreateObjectBindingContextMenuExtender);
	Builder.AddCustomization(BaseInfo);
}

void FTemplateSequenceCustomizationBase::UnregisterSequencerCustomization()
{
	if (Sequencer)
	{
		Sequencer->OnCloseEvent().RemoveAll(this);
		Sequencer->OnMovieSceneDataChanged().RemoveAll(this);
	}

	Sequencer = nullptr;
	TemplateSequence = nullptr;
}

UClass* FTemplateSequenceCustomizationBase::GetBoundActorClass() const
{
	return TemplateSequence ? TemplateSequence->BoundActorClass.Get() : nullptr;
}

FText FTemplateSequenceCustomizationBase::GetBoundActorClassName() const
{
	const UClass* BoundActorClass = GetBoundActorClass();
	return BoundActorClass ? BoundActorClass->GetDisplayNameText() : FText::FromName(NAME_None);
}

void FTemplateSequenceCustomizationBase::OnBoundActorClassPicked(UClass* ChosenClass)
{
	FSlateApplication::Get().DismissAllMenus();

	if (TemplateSequence != nullptr)
	{
		ChangeActorBinding(ChosenClass);

		Sequencer->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
	}
}

void FTemplateSequenceCustomizationBase::ChangeActorBinding(UObject* Object, UActorFactory* ActorFactory, bool bSetupDefaults)
{
	const FScopedTransaction Transaction(LOCTEXT("ChangeActorBinding", "Change Template Sequence Actor Binding"));

	FTemplateSequenceEditorUtil Util(TemplateSequence, *Sequencer);
	Util.ChangeActorBinding(Object, ActorFactory, bSetupDefaults);
}

ESequencerPasteSupport FTemplateSequenceCustomizationBase::OnPaste()
{
	// We don't support pasting folders or new object bindings.
	return ESequencerPasteSupport::Tracks | ESequencerPasteSupport::Sections;
}

TSharedPtr<FExtender> FTemplateSequenceCustomizationBase::CreateObjectBindingContextMenuExtender(UE::Sequencer::FViewModelPtr InViewModel)
{
	using namespace UE::Sequencer;

	TSharedRef<FExtender> Extender = MakeShared<FExtender>();
	TSharedPtr<FObjectBindingModel> ObjectBindingModel = InViewModel->CastThisShared<FObjectBindingModel>();
	Extender->AddMenuExtension(
			"ObjectBindingActions", EExtensionHook::Before, nullptr,
			FMenuExtensionDelegate::CreateRaw(this, &FTemplateSequenceCustomizationBase::ExtendObjectBindingContextMenu, ObjectBindingModel));
	return Extender.ToSharedPtr();
}

void FTemplateSequenceCustomizationBase::ExtendObjectBindingContextMenu(FMenuBuilder& MenuBuilder, TSharedPtr<UE::Sequencer::FObjectBindingModel> ObjectBindingModel)
{
	using namespace UE::Sequencer;

	MenuBuilder.BeginSection("Import/Export", LOCTEXT("ImportExportMenuSectionName", "Import/Export"));

	TSharedPtr<ISequencer> SequencerPtr = Sequencer->AsShared();

	MenuBuilder.AddMenuEntry(
		LOCTEXT("ImportFBX", "Import..."),
		LOCTEXT("ImportFBXTooltip", "Import FBX animation to this object"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([=] {
					FLevelSequenceFBXInterop Interop(SequencerPtr);
					Interop.ImportFBXOntoSelectedNodes();
				})
		));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("ExportFBX", "Export..."),
		LOCTEXT("ExportFBXTooltip", "Export FBX animation from this object"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([=] {
					FLevelSequenceFBXInterop Interop(SequencerPtr);
					Interop.ExportFBX();
				})
		));

	MenuBuilder.EndSection();
}

void FTemplateSequenceCustomizationBase::OnMovieSceneDataChanged(EMovieSceneDataChangeType ChangeType)
{
	if (ChangeType == EMovieSceneDataChangeType::TrackValueChanged || ChangeType == EMovieSceneDataChangeType::TrackValueChangedRefreshImmediately)
	{
		return;
	}

	// Ensure the BoundActorClass is up to date with whatever change might have occured.
	// This is mostly only needed when the user deletes or cuts the root object binding and we are suddenly left
	// with an empty template sequence (in which case we need to set BoundActorClass to null).
	const UObject* RootSpawnableTemplate = TemplateSequence->GetRootObjectSpawnableTemplate();	
	const UClass* RootObjectClass = Cast<const UClass>(RootSpawnableTemplate);
	if (RootSpawnableTemplate && !RootObjectClass)
	{
		RootObjectClass = RootSpawnableTemplate->GetClass();
	}

	if (TemplateSequence->BoundActorClass != RootObjectClass)
	{
		FProperty* BoundActorClassProperty = FindFProperty<FProperty>(UTemplateSequence::StaticClass(), GET_MEMBER_NAME_CHECKED(UTemplateSequence, BoundActorClass));
		TemplateSequence->PreEditChange(BoundActorClassProperty);

		TemplateSequence->BoundActorClass = RootObjectClass;

		FPropertyChangedEvent PropertyEvent(BoundActorClassProperty, EPropertyChangeType::ValueSet);
		TemplateSequence->PostEditChangeProperty(PropertyEvent);
	}
}

void FTemplateSequenceCustomizationBase::OnSequencerClosed(TSharedRef<ISequencer> InSequencer)
{
	if (&InSequencer.Get() == Sequencer)
	{
		Sequencer->OnCloseEvent().RemoveAll(this);
		Sequencer->OnMovieSceneDataChanged().RemoveAll(this);

		Sequencer = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE

