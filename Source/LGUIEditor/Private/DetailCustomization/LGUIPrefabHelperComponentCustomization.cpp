// Copyright 2019 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIPrefabHelperComponentCustomization.h"
#include "LGUIEditorUtils.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabHelperComponentCustomization"

TSharedRef<IDetailCustomization> FLGUIPrefabHelperComponentCustomization::MakeInstance()
{
	return MakeShareable(new FLGUIPrefabHelperComponentCustomization);
}
void FLGUIPrefabHelperComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULGUIPrefabHelperComponent>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{
		
	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}
	TargetScriptPtr->LoadPrefab();

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");
	auto contentWidget = SNew(SHorizontalBox);
	category.AddCustomRow(LOCTEXT("PrefabEdit", "PrefabEdit"))
		.WholeRowContent()
		[
			contentWidget
		];

	contentWidget->AddSlot()
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Text(LOCTEXT("Save", "Save"))
		.ToolTipText(LOCTEXT("SaveButtonToolTip","Serialize the prefab, if anything changed then the prefab is marked dirty"))
		.OnClicked_Lambda([&]() { TargetScriptPtr->SavePrefab(); return FReply::Handled(); })
		];
	if (TargetScriptPtr->LoadedRootActor != nullptr)
	{
		contentWidget->AddSlot()
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Text(LOCTEXT("Revert", "Revert"))
			.OnClicked_Lambda([&]() { TargetScriptPtr->RevertPrefab(); return FReply::Handled(); })
			];
	}
	else
	{
		contentWidget->AddSlot()
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Text(LOCTEXT("Load", "Load"))
			.OnClicked_Lambda([&]() { TargetScriptPtr->LoadPrefab(); return FReply::Handled(); })
			];
	}
	contentWidget->AddSlot()
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Text(LOCTEXT("Delete", "Delete"))
		.OnClicked_Lambda([&]() { TargetScriptPtr->DeleteThisInstance(); return FReply::Handled(); })
		];
}
void FLGUIPrefabHelperComponentCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (auto Script = TargetScriptPtr.Get())
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE