// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LGUIPrefabSequenceComponentCustomization.h"

#include "PrefabAnimation/LGUIPrefabSequence.h"
#include "PrefabAnimation/LGUIPrefabSequenceComponent.h"
#include "EditorStyleSet.h"
#include "GameFramework/Actor.h"
#include "IDetailsView.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Docking/SDockTab.h"
#include "SSCSEditor.h"
#include "BlueprintEditorTabs.h"
#include "ScopedTransaction.h"
#include "ISequencerModule.h"
#include "Editor.h"
#include "IPropertyUtilities.h"
#include "Widgets/Input/SButton.h"
#include "LGUIEditorModule.h"
#include "LGUIPrefabSequenceEditor.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabSequenceComponentCustomization"


TSharedRef<IDetailCustomization> FLGUIPrefabSequenceComponentCustomization::MakeInstance()
{
	return MakeShared<FLGUIPrefabSequenceComponentCustomization>();
}

void FLGUIPrefabSequenceComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	PropertyUtilities = DetailBuilder.GetPropertyUtilities();

	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() != 1)
	{
		return;
	}

	WeakSequenceComponent = Cast<ULGUIPrefabSequenceComponent>(Objects[0].Get());
	if (!WeakSequenceComponent.Get())
	{
		return;
	}

	const IDetailsView* DetailsView = DetailBuilder.GetDetailsView();
	TSharedPtr<FTabManager> HostTabManager = FGlobalTabmanager::Get();

	DetailBuilder.HideProperty("Sequence");

	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Sequence", FText(), ECategoryPriority::Important);

	bool bIsExternalTabAlreadyOpened = false;

	TSharedPtr<SDockTab> ExistingTab = HostTabManager->FindExistingLiveTab(FLGUIEditorModule::LGUIPrefabSequenceTabName);
	if (ExistingTab.IsValid())
	{
		auto SequencerWidget = StaticCastSharedRef<SLGUIPrefabSequenceEditor>(ExistingTab->GetContent());
		bIsExternalTabAlreadyOpened = WeakSequenceComponent.IsValid() && SequencerWidget->GetSequenceComponent() == WeakSequenceComponent.Get();
	}
	Category.AddCustomRow(FText())
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SequenceValueText", "Sequence"))
			.Font(DetailBuilder.GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SButton)
			.OnClicked(this, &FLGUIPrefabSequenceComponentCustomization::InvokeSequencer)
			[
				SNew(STextBlock)
				.Text(bIsExternalTabAlreadyOpened ? LOCTEXT("FocusSequenceTabButtonText", "Focus Tab") : LOCTEXT("OpenSequenceTabButtonText", "Open in Tab"))
				.Font(DetailBuilder.GetDetailFont())
			]
		];

}

FReply FLGUIPrefabSequenceComponentCustomization::InvokeSequencer()
{
	if (TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->TryInvokeTab(FLGUIEditorModule::LGUIPrefabSequenceTabName))
	{
		{
			// Set up a delegate that forces a refresh of this panel when the tab is closed to ensure we see the inline widget
			TWeakPtr<IPropertyUtilities> WeakUtilities = PropertyUtilities;
			auto OnClosed = [WeakUtilities](TSharedRef<SDockTab>)
			{
				TSharedPtr<IPropertyUtilities> PinnedPropertyUtilities = WeakUtilities.Pin();
				if (PinnedPropertyUtilities.IsValid())
				{
					PinnedPropertyUtilities->EnqueueDeferredAction(FSimpleDelegate::CreateSP(PinnedPropertyUtilities.ToSharedRef(), &IPropertyUtilities::ForceRefresh));
				}
			};

			Tab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateLambda(OnClosed));
		}

		StaticCastSharedRef<SLGUIPrefabSequenceEditor>(Tab->GetContent())->AssignLGUIPrefabSequenceComponent(WeakSequenceComponent);
	}


	//FGlobalTabmanager::Get()->TryInvokeTab(FLGUIEditorModule::LGUIPrefabSequenceTabName);

	PropertyUtilities->ForceRefresh();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
