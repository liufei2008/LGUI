// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/UIFlexibleGridLayoutCustomization.h"
#include "LGUIEditorUtils.h"
#include "Layout/UIFlexibleGridLayout.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "UIFlexibleGridLayoutCustomization"

TSharedRef<IDetailCustomization> FUIFlexibleGridLayoutCustomization::MakeInstance()
{
	return MakeShareable(new FUIFlexibleGridLayoutCustomization);
}
void FUIFlexibleGridLayoutCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUIFlexibleGridLayout>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{

	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}
	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");
}
#undef LOCTEXT_NAMESPACE