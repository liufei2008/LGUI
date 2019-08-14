// Copyright 2019 LexLiu. All Rights Reserved.

#include "DetailCustomization/UIPanelCustomization.h"

#define LOCTEXT_NAMESPACE "UIPanelComponentDetails"
FUIPanelCustomization::FUIPanelCustomization()
{
}

FUIPanelCustomization::~FUIPanelCustomization()
{
}

TSharedRef<IDetailCustomization> FUIPanelCustomization::MakeInstance()
{
	return MakeShareable(new FUIPanelCustomization);
}
void FUIPanelCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptArray.Empty();
	for (auto item : targetObjects)
	{
		if (auto validItem = Cast<UUIPanel>(item.Get()))
		{
			TargetScriptArray.Add(validItem);
			if (validItem->GetWorld() != nullptr && validItem->GetWorld()->WorldType == EWorldType::Editor)
			{
				validItem->EditorForceUpdateImmediately();
			}
		}
	}
	if (TargetScriptArray.Num() == 0)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[UIPanelCustomization]Get TargetScript is null"));
		return;
	}
	
	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");
	const FText widgetText = LOCTEXT("UIWidget", "UIWidget properties");

	auto clipTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIPanel, clipType));
	clipTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIPanelCustomization::ForceRefresh, &DetailBuilder));
	uint8 clipType;
	clipTypeHandle->GetValue(clipType);
	TArray<FName> needToHidePropertyNameForClipType;
	if (clipType == (uint8)(UIPanelClipType::None))
	{
		needToHidePropertyNameForClipType.Add(GET_MEMBER_NAME_CHECKED(UUIPanel, clipFeather));
		needToHidePropertyNameForClipType.Add(GET_MEMBER_NAME_CHECKED(UUIPanel, clipTexture));
		needToHidePropertyNameForClipType.Add(GET_MEMBER_NAME_CHECKED(UUIPanel, clipRectOffset));
		needToHidePropertyNameForClipType.Add(GET_MEMBER_NAME_CHECKED(UUIPanel, inheritRectClip));
	}
	else if (clipType == (uint8)UIPanelClipType::Rect)
	{
		needToHidePropertyNameForClipType.Add(GET_MEMBER_NAME_CHECKED(UUIPanel, clipTexture));
	}
	else if (clipType == (uint8)UIPanelClipType::Texture)
	{
		needToHidePropertyNameForClipType.Add(GET_MEMBER_NAME_CHECKED(UUIPanel, clipFeather));
		needToHidePropertyNameForClipType.Add(GET_MEMBER_NAME_CHECKED(UUIPanel, clipRectOffset));
		needToHidePropertyNameForClipType.Add(GET_MEMBER_NAME_CHECKED(UUIPanel, inheritRectClip));
	}

	for (auto item : needToHidePropertyNameForClipType)
	{
		DetailBuilder.HideProperty(item);
	}

	category.AddCustomRow(LOCTEXT("DrawcallInfo", "DrawcallInfo"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("DrawcallCount")))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Text(this, &FUIPanelCustomization::GetDrawcallInfo)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		;
}
void FUIPanelCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
FText FUIPanelCustomization::GetDrawcallInfo()const
{
	int drawcallCount = 0;
	if (TargetScriptArray[0] != nullptr)
	{
		drawcallCount = TargetScriptArray[0]->UIDrawcallList.Num();
	}
	return FText::FromString(FString::Printf(TEXT("%d"), drawcallCount));
}
#undef LOCTEXT_NAMESPACE