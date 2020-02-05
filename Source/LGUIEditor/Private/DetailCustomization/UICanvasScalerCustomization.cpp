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
	needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, ProjectionType));
	needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, FOVAngle));
	needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, NearClipPlane));
	needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, FarClipPlane));

	needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, UIScaleMode));
	needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, ReferenceResolution));
	needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, ScreenMatchMode));
	needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, MatchFromWidthToHeight));

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
				lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, UIScaleMode));

				DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, UIScaleMode))
					->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
				if (TargetScriptPtr->UIScaleMode == LGUIScaleMode::ScaleWithScreenSize)
				{
					lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, ReferenceResolution));
					DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, ScreenMatchMode))
						->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
					switch (TargetScriptPtr->ScreenMatchMode)
					{
					case LGUIScreenMatchMode::Expand:
					case LGUIScreenMatchMode::Shrink:
					{
						
					}
					break;
					case LGUIScreenMatchMode::MatchWidthOrHeight:
					{
						lguiCategory.AddCustomRow(LOCTEXT("MatchSlider", "MatchSlider"))
						.NameContent()
						[
							SNew(SBox)
							.HAlign(EHorizontalAlignment::HAlign_Right)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Match", "Match"))
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
						]
						.ValueContent()
						[
							SNew(SBox)
							[
								SNew(SSlider)
								.Value(this, &FUICanvasScalerCustomization::GetMatchValue)
								.OnValueChanged(this, &FUICanvasScalerCustomization::SetMatchValue, true)
							]
						]
						;
					}
					break;
					}
					lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, ScreenMatchMode));
				}
				else if (TargetScriptPtr->UIScaleMode == LGUIScaleMode::ConstantPixelSize)
				{
					needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, ReferenceResolution));
					needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, ScreenMatchMode));
					needToHidePropertyNameArray.Add(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, MatchFromWidthToHeight));
				}

				auto projectionTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, ProjectionType));
				projectionTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
				if (TargetScriptPtr->ProjectionType == ECameraProjectionMode::Orthographic)
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
float FUICanvasScalerCustomization::GetMatchValue()const
{
	if (TargetScriptPtr.IsValid())
	{
		return TargetScriptPtr->GetMatchFromWidthToHeight();
	}
	return 0.0f;
}
void FUICanvasScalerCustomization::SetMatchValue(float value, bool fromSlider)
{
	if (fromSlider)
	{
		if (TargetScriptPtr.IsValid())
		{
			TargetScriptPtr->SetMatchFromWidthToHeight(value);
		}
	}
}
#undef LOCTEXT_NAMESPACE