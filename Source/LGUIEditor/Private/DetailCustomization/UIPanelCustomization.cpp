// Copyright 2019 LexLiu. All Rights Reserved.

#include "DetailCustomization/UIPanelCustomization.h"
#include "Window/LGUIEditorTools.h"
#include "Core/ActorComponent/UIPanel.h"
#include "LGUIEditorUtils.h"

#define LOCTEXT_NAMESPACE "UIPanelCustomization"

TSharedRef<IDetailCustomization> FUIPanelCustomization::MakeInstance()
{
	return MakeShareable(new FUIPanelCustomization);
}
void FUIPanelCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUIPanel>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{

	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}

	IDetailCategoryBuilder& lguiCategory = DetailBuilder.EditCategory("DeprecatedInfo", FText::GetEmpty(), ECategoryPriority::Variable);
	lguiCategory.AddCustomRow(LOCTEXT("Tips", "Tips"))
	.WholeRowContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString(FString(TEXT("This UIPanel component is deprecated, will be removed in future release. Follow these steps to update:\
			\n1. Add a LGUICanvas component to the actor\
			\n2. Select UIPanel component, click button from menu \"LGUI Tools/Copy Component Values\"\
			\n3. Select LGUICanvas component you just added, click button from menu \"LGUI Tools/Paste Component Values\"\
			\n4. Set LGUICanvas's SortOrder value with UIPanel's Depth value\
			\n5. Select the actor, click button from menu \"LGUI Tools/Replace this by.../UIContainer\""))))
		.ColorAndOpacity(FLinearColor(FColor::Red))
		.AutoWrapText(true)
	]
	;
}
#undef LOCTEXT_NAMESPACE