// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/UIScrollViewWithScrollBarCustomization.h"
#include "LexUIEditorUtils.h"
#include "Interaction/UIScrollViewWithScrollbar.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Interaction/UIScrollbar.h"

#define LOCTEXT_NAMESPACE "UIScrollViewWithScrollBarComponentDetails"
FUIScrollViewWithScrollBarCustomization::FUIScrollViewWithScrollBarCustomization()
{
}

FUIScrollViewWithScrollBarCustomization::~FUIScrollViewWithScrollBarCustomization()
{
}

TSharedRef<IDetailCustomization> FUIScrollViewWithScrollBarCustomization::MakeInstance()
{
	return MakeShareable(new FUIScrollViewWithScrollBarCustomization);
}
void FUIScrollViewWithScrollBarCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUIScrollViewWithScrollbar>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[%s].%d Get TargetScript is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}
	
	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LexUI-ScrollViewWithScrollbar");
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, Viewport));
	TArray<FName> needToHidePropertyName;
	auto ViewportHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, Viewport));
	ViewportHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIScrollViewWithScrollBarCustomization::ForceRefresh, &DetailBuilder));
	ULexWidget* Viewport = nullptr;
	ViewportHandle->GetValue(*(UObject**)&Viewport);

	auto HorizontalScrollbarVisibilityHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, HorizontalScrollbarVisibility));
	auto VerticalScrollbarVisibilityHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, VerticalScrollbarVisibility));
	HorizontalScrollbarVisibilityHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIScrollViewWithScrollBarCustomization::ForceRefresh, &DetailBuilder));
	VerticalScrollbarVisibilityHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIScrollViewWithScrollBarCustomization::ForceRefresh, &DetailBuilder));
	uint8 HorizontalScrollbarVisibilityByte;
	uint8 VerticalScrollbarVisibilityByte;
	HorizontalScrollbarVisibilityHandle->GetValue(HorizontalScrollbarVisibilityByte);
	VerticalScrollbarVisibilityHandle->GetValue(VerticalScrollbarVisibilityByte);
	ELexUIScrollViewScrollbarVisibility HorizontalScrollbarVisibility = (ELexUIScrollViewScrollbarVisibility)HorizontalScrollbarVisibilityByte;
	ELexUIScrollViewScrollbarVisibility VerticalScrollbarVisibility = (ELexUIScrollViewScrollbarVisibility)VerticalScrollbarVisibilityByte;

	auto HorizontalScrollbarHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, HorizontalScrollbar));
	HorizontalScrollbarHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIScrollViewWithScrollBarCustomization::ForceRefresh, &DetailBuilder));
	UUIScrollbar* HorizontalScrollbar = nullptr;
	HorizontalScrollbarHandle->GetValue(*(UObject**)&HorizontalScrollbar);

	auto VerticalScrollbarHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, VerticalScrollbar));
	VerticalScrollbarHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIScrollViewWithScrollBarCustomization::ForceRefresh, &DetailBuilder));
	UUIScrollbar* VerticalScrollbar = nullptr;
	VerticalScrollbarHandle->GetValue(*(UObject**)&VerticalScrollbar);

	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, HorizontalScrollbarWidget));
	IDetailPropertyRow& HorizontalScrollbarVisibilityProperty = category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, HorizontalScrollbarVisibility));
	HorizontalScrollbarVisibilityProperty.IsEnabled(IsValid(HorizontalScrollbar));

	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, VerticalScrollbarWidget));
	IDetailPropertyRow& VerticalScrollbarVisibilityProperty = category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, VerticalScrollbarVisibility));
	VerticalScrollbarVisibilityProperty.IsEnabled(IsValid(VerticalScrollbar));

	auto KeepProgressHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, KeepProgress));
	KeepProgressHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIScrollViewWithScrollBarCustomization::ForceRefresh, &DetailBuilder));
	bool KeepProgress;
	KeepProgressHandle->GetValue(KeepProgress);
	if (!KeepProgress)
	{
		needToHidePropertyName.Add(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, Progress));
	}

	for (auto item : needToHidePropertyName)
	{
		DetailBuilder.HideProperty(item);
	}
}
void FUIScrollViewWithScrollBarCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (TargetScriptPtr.IsValid())
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE