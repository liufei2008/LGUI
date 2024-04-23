// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/UIPanelLayoutVerticalBoxSlotCustomization.h"
#include "LGUIEditorUtils.h"
#include "Layout/UIPanelLayout_VerticalBox.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "PanelLayout/HorizontalAlignmentCustomization.h"
#include "PanelLayout/VerticalAlignmentCustomization.h"
#include "PanelLayout/SlateChildSizeCustomization.h"

#define LOCTEXT_NAMESPACE "UIPanelLayoutVerticalBoxSlotCustomization"

TSharedRef<IDetailCustomization> FUIPanelLayoutVerticalBoxSlotCustomization::MakeInstance()
{
	return MakeShareable(new FUIPanelLayoutVerticalBoxSlotCustomization);
}
void FUIPanelLayoutVerticalBoxSlotCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptArray.Empty();
	for (auto item : targetObjects)
	{
		if (auto validItem = Cast<UUIPanelLayout_VerticalBox_Slot>(item.Get()))
		{
			TargetScriptArray.Add(validItem);
		}
	}
	if (TargetScriptArray.Num() == 0)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[UIPanelLayoutVerticalBoxSlotCustomization]Get TargetScript is null"));
		return;
	}
	DetailBuilder.RegisterInstancedCustomPropertyTypeLayout(TEXT("EHorizontalAlignment"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FHorizontalAlignmentCustomization::MakeInstance));
	DetailBuilder.RegisterInstancedCustomPropertyTypeLayout(TEXT("EVerticalAlignment"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FVerticalAlignmentCustomization::MakeInstance));
	DetailBuilder.RegisterInstancedCustomPropertyTypeLayout(TEXT("SlateChildSize"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSlateChildSizeCustomization::MakeInstance));

	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Panel Layout Slot");
	Category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIPanelLayout_VerticalBox_Slot, Padding));
}
#undef LOCTEXT_NAMESPACE