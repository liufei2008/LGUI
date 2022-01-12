// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/UIScrollViewWithScrollBarCustomization.h"
#include "LGUIEditorUtils.h"
#include "Interaction/UIScrollViewWithScrollbarComponent.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

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
	TargetScriptPtr = Cast<UUIScrollViewWithScrollbarComponent>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[UITextCustomization]Get TargetScript is null"));
		return;
	}
	
	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI-ScrollViewWithScrollbar");
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbarComponent, Viewport));
	TArray<FName> needToHidePropertyName;
	auto ViewportHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbarComponent, Viewport));
	ViewportHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIScrollViewWithScrollBarCustomization::ForceRefresh, &DetailBuilder));
	UObject* ViewportObject = nullptr;
	ViewportHandle->GetValue(ViewportObject);
	AUIBaseActor* Viewport = nullptr;
	if (IsValid(ViewportObject))
	{
		Viewport = Cast<AUIBaseActor>(ViewportObject);
	}

	auto HorizontalScrollbarVisibilityHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbarComponent, HorizontalScrollbarVisibility));
	auto VerticalScrollbarVisibilityHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbarComponent, VerticalScrollbarVisibility));
	HorizontalScrollbarVisibilityHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIScrollViewWithScrollBarCustomization::ForceRefresh, &DetailBuilder));
	VerticalScrollbarVisibilityHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIScrollViewWithScrollBarCustomization::ForceRefresh, &DetailBuilder));
	uint8 HorizontalScrollbarVisibilityByte;
	uint8 VerticalScrollbarVisibilityByte;
	HorizontalScrollbarVisibilityHandle->GetValue(HorizontalScrollbarVisibilityByte);
	VerticalScrollbarVisibilityHandle->GetValue(VerticalScrollbarVisibilityByte);
	EScrollViewScrollbarVisibility HorizontalScrollbarVisibility = (EScrollViewScrollbarVisibility)HorizontalScrollbarVisibilityByte;
	EScrollViewScrollbarVisibility VerticalScrollbarVisibility = (EScrollViewScrollbarVisibility)VerticalScrollbarVisibilityByte;

	auto HorizontalScrollbarHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbarComponent, HorizontalScrollbar));
	HorizontalScrollbarHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIScrollViewWithScrollBarCustomization::ForceRefresh, &DetailBuilder));
	UObject* HorizontalScrollbarObject = nullptr;
	HorizontalScrollbarHandle->GetValue(HorizontalScrollbarObject);
	AUIBaseActor* HorizontalScrollbar = nullptr;
	if (IsValid(HorizontalScrollbarObject))
	{
		HorizontalScrollbar = Cast<AUIBaseActor>(HorizontalScrollbarObject);
	}

	auto VerticalScrollbarHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbarComponent, VerticalScrollbar));
	VerticalScrollbarHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUIScrollViewWithScrollBarCustomization::ForceRefresh, &DetailBuilder));
	UObject* VerticalScrollbarObject = nullptr;
	VerticalScrollbarHandle->GetValue(VerticalScrollbarObject);
	AUIBaseActor* VerticalScrollbar = nullptr;
	if (IsValid(VerticalScrollbarObject))
	{
		VerticalScrollbar = Cast<AUIBaseActor>(VerticalScrollbarObject);
	}

	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbarComponent, HorizontalScrollbar));
	IDetailPropertyRow& HorizontalScrollbarVisibilityProperty = category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbarComponent, HorizontalScrollbarVisibility));
	HorizontalScrollbarVisibilityProperty.IsEnabled(IsValid(HorizontalScrollbar));
	if (HorizontalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport)
	{
		bool showWarning = false;
		if (IsValid(Viewport))
		{
			if (Viewport->GetUIItem()->GetAttachParent() != TargetScriptPtr->GetRootUIComponent())
			{
				showWarning = true;
			}
		}
		else
		{
			showWarning = true;
		}
		if (IsValid(HorizontalScrollbar))
		{
			if (HorizontalScrollbar->GetUIItem()->GetAttachParent() != TargetScriptPtr->GetRootUIComponent())
			{
				showWarning = true;
			}
		}
		else
		{
			showWarning = false;
		}
		if (showWarning)
		{
			LGUIEditorUtils::ShowError(&category, LOCTEXT("ViewportOrScrollbarAttachError", "For this visibility mode, Viewport and HorizontalScrollbar must be a child of ScrollViewWithScrollBar"));
		}
	}

	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbarComponent, VerticalScrollbar));
	IDetailPropertyRow& VerticalScrollbarVisibilityProperty = category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbarComponent, VerticalScrollbarVisibility));
	VerticalScrollbarVisibilityProperty.IsEnabled(IsValid(VerticalScrollbar));
	if (VerticalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport)
	{
		bool showWarning = false;
		if (IsValid(Viewport))
		{
			if (Viewport->GetUIItem()->GetAttachParent() != TargetScriptPtr->GetRootUIComponent())
			{
				showWarning = true;
			}
		}
		else
		{
			showWarning = true;
		}
		if (IsValid(VerticalScrollbar))
		{
			if (VerticalScrollbar->GetUIItem()->GetAttachParent() != TargetScriptPtr->GetRootUIComponent())
			{
				showWarning = true;
			}
		}
		else
		{
			showWarning = false;
		}
		if (showWarning)
		{
			LGUIEditorUtils::ShowError(&category, LOCTEXT("ViewportOrScrollbarAttachError", "For this visibility mode, Viewport and HorizontalScrollbar must be a child of ScrollViewWithScrollBar"));
		}
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