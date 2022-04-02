// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/UICanvasScalerCustomization.h"
#include "Layout/LGUICanvasScaler.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSlider.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "LGUIEditorUtils.h"
#include "Core/ActorComponent/LGUICanvas.h"

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
			auto Msg = LOCTEXT("OnlyForRootLGUICanvas", "This component is only valid for root LGUICanvas");
			LGUIEditorUtils::ShowError(&lguiCategory, Msg);
		}
		else
		{
			if (canvas->GetRenderMode() == ELGUIRenderMode::WorldSpace || canvas->GetRenderMode() == ELGUIRenderMode::WorldSpace_LGUI)
			{
				lguiCategory.AddCustomRow(LOCTEXT("WorldSpaceUIInfo", "WorldSpaceUIInfo"))
					.WholeRowContent()
					.MinDesiredWidth(500)
					[
						SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(LOCTEXT("NothingHereForWorldSpaceUI", "Nothing here for WorldSpaceUI"))
						.AutoWrapText(true)
					];
			}
			else if (
				canvas->GetRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay
				|| canvas->GetRenderMode() == ELGUIRenderMode::RenderTarget
				)
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
						auto matchProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, MatchFromWidthToHeight));
						lguiCategory.AddCustomRow(LOCTEXT("MatchSlider", "MatchSlider"))
						.NameContent()
						[
							SNew(SBox)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Match", "Match"))
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
						]
						.ValueContent()
						.MinDesiredWidth(500)
						[
							SAssignNew(ValueBox, SHorizontalBox)
							+SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SBox)
								.WidthOverride(this, &FUICanvasScalerCustomization::GetValueWidth)
								[
									SNew(SVerticalBox)
									+SVerticalBox::Slot()
									[
										SNew(SSlider)
										.Value(this, &FUICanvasScalerCustomization::GetMatchValue, matchProperty)
										.OnValueChanged(this, &FUICanvasScalerCustomization::SetMatchValue, matchProperty)
									]
									+SVerticalBox::Slot()
									[
										SNew(SHorizontalBox)
										+SHorizontalBox::Slot()
										[
											SNew(STextBlock)
											.Text(LOCTEXT("Width", "Width"))
											.Font(IDetailLayoutBuilder::GetDetailFont())
										]
										+ SHorizontalBox::Slot()
										.HAlign(EHorizontalAlignment::HAlign_Right)
										[
											SNew(STextBlock)
											.Text(LOCTEXT("Height", "Height"))
											.Font(IDetailLayoutBuilder::GetDetailFont())
										]
									]
								]
							]
							+SHorizontalBox::Slot()
							.HAlign(EHorizontalAlignment::HAlign_Right)
							[
								SNew(SBox)
								.MinDesiredWidth(50)
								[
									matchProperty->CreatePropertyValueWidget()
								]
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

				lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, ProjectionType));
				lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, FOVAngle));
				lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, NearClipPlane));
				lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUICanvasScaler, FarClipPlane));
			}
		}
	}
	for (auto item : needToHidePropertyNameArray)
	{
		DetailBuilder.HideProperty(item);
	}
}
float FUICanvasScalerCustomization::GetMatchValue(TSharedRef<IPropertyHandle> Property)const
{
	float value = 0.0;
	Property->GetValue(value);
	return value;
}
void FUICanvasScalerCustomization::SetMatchValue(float value, TSharedRef<IPropertyHandle> Property)
{
	Property->SetValue(value);
}
FOptionalSize FUICanvasScalerCustomization::GetValueWidth()const
{
	return ValueBox->GetCachedGeometry().GetLocalSize().X - 60;
}
#undef LOCTEXT_NAMESPACE