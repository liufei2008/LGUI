// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIPrefabOverrideParameterEditor.h"
#include "PrefabSystem/LGUIPrefabOverrideParameter.h"
#include "LGUIPrefabEditor.h"

#define LOCTEXT_NAMESPACE "SLGUIPrefabOverrideParameterEditor"

void SLGUIPrefabOverrideParameterEditor::Construct(const FArguments& InArgs, TSharedPtr<FLGUIPrefabEditor> InPrefabEditorPtr)
{
	PrefabEditorPtr = InPrefabEditorPtr;
	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	{
		DetailsViewArgs.bAllowSearch = false;
		DetailsViewArgs.bShowOptions = false;
		DetailsViewArgs.bAllowMultipleTopLevelObjects = false;
		DetailsViewArgs.bAllowFavoriteSystem = false;
		DetailsViewArgs.bShowActorLabel = false;
		DetailsViewArgs.bHideSelectionTip = true;
	}
	DescriptorDetailView = EditModule.CreateDetailView(DetailsViewArgs);
	ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.MaxHeight(26)
			[
				SNew(SBox)
				.Padding(FMargin(6, 6))
				[
					SAssignNew(TipText, STextBlock)
				]
			]
			+SVerticalBox::Slot()
			.MaxHeight(26)
			[
				SNew(SButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.Text(LOCTEXT("ApplyParameterButton", "ApplyParameterChange"))
				.OnClicked_Raw(this, &SLGUIPrefabOverrideParameterEditor::OnClickApplyParameterButton)
				.Visibility(this, &SLGUIPrefabOverrideParameterEditor::ApplyChangeButtonVisibility)
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				DescriptorDetailView.ToSharedRef()
			]
			
		];
}
void SLGUIPrefabOverrideParameterEditor::SetTargetObject(ULGUIPrefabOverrideParameterObject* InTargetObject, AActor* InActor)
{
	TargetObject = InTargetObject;
	DescriptorDetailView->SetObject(InTargetObject);
	PrefabRootActor = InActor;
}
void SLGUIPrefabOverrideParameterEditor::SetTipText(const FString& InTipText)
{
	TipText->SetText(FText::FromString(TEXT("Prefab Override Parameter for \"") + InTipText + TEXT("\"")));
}
FReply SLGUIPrefabOverrideParameterEditor::OnClickApplyParameterButton()
{
	PrefabEditorPtr.Pin()->ApplySubPrefabParameterChange(PrefabRootActor.Get());
	return FReply::Handled();
}
EVisibility SLGUIPrefabOverrideParameterEditor::ApplyChangeButtonVisibility()const
{
	if (TargetObject.IsValid())
	{
		return TargetObject->GetIsIsTemplate() ? EVisibility::Hidden : EVisibility::Visible;
	}
	return EVisibility::Hidden;
}

#undef LOCTEXT_NAMESPACE
