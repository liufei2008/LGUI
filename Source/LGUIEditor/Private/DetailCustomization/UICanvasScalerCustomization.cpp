// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "DetailCustomization/UICanvasScalerCustomization.h"
#include "Window/LGUIEditorTools.h"
#include "Layout/LGUICanvasScaler.h"
#include "LGUIEditorUtils.h"

#define LOCTEXT_NAMESPACE "UICanvasScalarCustomization"

TSharedRef<IDetailCustomization> FUICanvasScalerCustomization::MakeInstance()
{
	return MakeShareable(new FUICanvasScalerCustomization);
}
void FUICanvasScalerCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULGUICanvasScaler>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{

	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}
	LGUIEditorUtils::ShowError_RequireComponent(&DetailBuilder, TargetScriptPtr.Get(), ULGUICanvas::StaticClass());
	LGUIEditorUtils::ShowError_MultiComponentNotAllowed(&DetailBuilder, TargetScriptPtr.Get());

	TargetScriptPtr->OnViewportParameterChanged();

	IDetailCategoryBuilder& lguiCategory = DetailBuilder.EditCategory("LGUI");
	TArray<FName> needToHidePropertyNameArray;
	//add all property
	needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, UIScaleMode));
	needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, PreferredHeight));
	needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, PreferredWidth));
	needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, ProjectionType));
	needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, FOVAngle));
	needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, NearClipPlane));
	needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, FarClipPlane));

	if (TargetScriptPtr->CheckCanvas())
	{
		auto canvas = TargetScriptPtr->Canvas;
		if (!canvas->IsRootCanvas())
		{
			lguiCategory.AddCustomRow(LOCTEXT("NotRootCanvasWarning", "NotRootCanvasWarning"))
			.WholeRowContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString(TEXT("Only valid for root LGUICanvas"))))
				.ColorAndOpacity(FSlateColor(FLinearColor::Red))
				.AutoWrapText(true)
			];
		}
		else
		{
			if (canvas->GetRenderMode() == ELGUIRenderMode::WorldSpace)
			{
				lguiCategory.AddCustomRow(LOCTEXT("NothingDisplay", "NothingDisplay"))
				.WholeRowContent()
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString(TEXT("Nothing for WorldSpace canvas"))))
					.ColorAndOpacity(FSlateColor(FLinearColor::Green))
					.AutoWrapText(true)
				];
			}
			else if (canvas->GetRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay)
			{
				needToHidePropertyNameArray.Reset();
				auto scaleModeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, UIScaleMode));
				scaleModeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
				uint8 scaleMode;
				scaleModeHandle->GetValue(scaleMode);
				if (scaleMode == (uint8)(LGUIScaleMode::ScaleWithScreenWidth))
				{
					needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, PreferredHeight));
				}
				else if (scaleMode == (uint8)(LGUIScaleMode::ScaleWithScreenHeight))
				{
					needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, PreferredWidth));
				}
				else if (scaleMode == (uint8)(LGUIScaleMode::ConstantPixelSize))
				{
					needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, PreferredHeight));
					needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, PreferredWidth));
				}

				auto projectionTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, ProjectionType));
				projectionTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
				uint8 projectionType; projectionTypeHandle->GetValue(projectionType);
				if (projectionType == (uint8)ECameraProjectionMode::Orthographic)
				{
					needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, FOVAngle));
				}
			}
		}
	}
	for (auto item : needToHidePropertyNameArray)
	{
		DetailBuilder.HideProperty(item);
	}
}
#undef LOCTEXT_NAMESPACE