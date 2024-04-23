// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/UIPanelLayoutFlexibleGridSlotCustomization.h"
#include "LGUIEditorUtils.h"
#include "Layout/UIPanelLayout_FlexibleGrid.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "PanelLayout/HorizontalAlignmentCustomization.h"
#include "PanelLayout/VerticalAlignmentCustomization.h"
#include "PanelLayout/SlateChildSizeCustomization.h"

#define LOCTEXT_NAMESPACE "UIPanelLayoutFlexibleGridSlotCustomization"

TSharedRef<IDetailCustomization> FUIPanelLayoutFlexibleGridSlotCustomization::MakeInstance()
{
	return MakeShareable(new FUIPanelLayoutFlexibleGridSlotCustomization);
}
void FUIPanelLayoutFlexibleGridSlotCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptArray.Empty();
	for (auto item : targetObjects)
	{
		if (auto validItem = Cast<UUIPanelLayout_FlexibleGrid_Slot>(item.Get()))
		{
			TargetScriptArray.Add(validItem);
		}
	}
	if (TargetScriptArray.Num() == 0)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[UIPanelLayoutFlexibleGridSlotCustomization]Get TargetScript is null"));
		return;
	}
	DetailBuilder.RegisterInstancedCustomPropertyTypeLayout(TEXT("EHorizontalAlignment"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FHorizontalAlignmentCustomization::MakeInstance));
	DetailBuilder.RegisterInstancedCustomPropertyTypeLayout(TEXT("EVerticalAlignment"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FVerticalAlignmentCustomization::MakeInstance));

	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Panel Layout Slot");
	Category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIPanelLayout_FlexibleGrid_Slot, Padding));
}
#undef LOCTEXT_NAMESPACE