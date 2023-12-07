// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/UIItemCustomization.h"
#include "Widgets/Layout/Anchors.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "IDetailGroup.h"
#include "LGUIEditorStyle.h"
#include "Editor.h"
#include "Widget/ComponentTransformDetails.h"
#include "Widget/AnchorPreviewWidget.h"
#include "PropertyCustomizationHelpers.h"
#include "HAL/PlatformApplicationMisc.h"
#include "LevelEditorViewport.h"
#include "LGUIEditorUtils.h"
#include "LGUIEditorTools.h"
#include "Layout/UILayoutBase.h"
#include "Layout/UILayoutElement.h"
#include "LGUIHeaders.h"
#include "PrefabEditor/LGUIPrefabEditor.h"
#include "PrefabSystem/LGUIPrefabManager.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "UIItemComponentDetails"

UE_DISABLE_OPTIMIZATION

FUIItemCustomization::FUIItemCustomization()
{
	
}
FUIItemCustomization::~FUIItemCustomization()
{
	
}

TSharedRef<IDetailCustomization> FUIItemCustomization::MakeInstance()
{
	return MakeShareable(new FUIItemCustomization);
}
void FUIItemCustomization::ForceUpdateUI()
{
	for (auto item : TargetScriptArray)
	{
		if (item.IsValid())
		{
			item->EditorForceUpdate();
		}
	}
}
void FUIItemCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptArray.Empty();
	for (auto item : targetObjects)
	{
		if (auto validItem = Cast<UUIItem>(item.Get()))
		{
			TargetScriptArray.Add(validItem);
			if (validItem->GetWorld() != nullptr)
			{
				if (validItem->GetWorld()->WorldType == EWorldType::Editor)
				{
					validItem->EditorForceUpdate();
				}
			}
		}
	}
	if (TargetScriptArray.Num() == 0)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[UIItemCustomization]Get TargetScript is null"));
		return;
	}

	LGUIEditorUtils::ShowError_MultiComponentNotAllowed(&DetailBuilder, TargetScriptArray[0].Get());

	IDetailCategoryBuilder& LGUICategory = DetailBuilder.EditCategory("LGUI");
	DetailBuilder.HideCategory("TransformCommon");
	IDetailCategoryBuilder& TransformCategory = DetailBuilder.EditCategory("LGUITransform", LOCTEXT("LGUI-Transform", "LGUI-Transform"), ECategoryPriority::Transform);

	//base
	{
		auto uiActiveHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, bIsUIActive));
		uiActiveHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([this] {
			ForceUpdateUI();
		}));
	}

	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData));

	LGUICategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, bIsUIActive));

	//anchor, width, height
	{
		TMap<EAnchorControlledByLayoutType, TArray<UObject*>> MultipleLayoutControlMap;
		if (IsAnchorControlledByMultipleLayout(MultipleLayoutControlMap))
		{
			FString ControlTypeString;
			for (auto& KeyValue : MultipleLayoutControlMap)
			{
				if (KeyValue.Value.Num() <= 1)continue;
				FString LayoutControlTypeString;
				switch (KeyValue.Key)
				{
				case EAnchorControlledByLayoutType::HorizontalAnchor:
					LayoutControlTypeString = "HorizontalAnchor";
					break;
				case EAnchorControlledByLayoutType::HorizontalAnchoredPosition:
					LayoutControlTypeString = "HorizontalAnchoredPosition";
					break;
				case EAnchorControlledByLayoutType::HorizontalSizeDelta:
					LayoutControlTypeString = "HorizontalSizeDelta";
					break;
				case EAnchorControlledByLayoutType::VerticalAnchor:
					LayoutControlTypeString = "VerticalAnchor";
					break;
				case EAnchorControlledByLayoutType::VerticalAnchoredPosition:
					LayoutControlTypeString = "VerticalAnchoredPosition";
					break;
				case EAnchorControlledByLayoutType::VerticalSizeDelta:
					LayoutControlTypeString = "VerticalSizeDelta";
					break;
				}
				ControlTypeString += FString::Printf(TEXT("Anchor '%s' is controlled by layout object:\n"), *LayoutControlTypeString);
				for (auto& ObjectItem : KeyValue.Value)
				{
					FString LayoutObjectNameString;
					if (auto Actor = Cast<AActor>(ObjectItem))
					{
						LayoutObjectNameString = Actor->GetActorLabel();
					}
					else if (auto Component = Cast<UActorComponent>(ObjectItem))
					{
						LayoutObjectNameString = FString::Printf(TEXT("%s.%s"), *Component->GetOwner()->GetActorLabel(), *Component->GetName());
					}
					else
					{
						LayoutObjectNameString = Component->GetName();
					}
					ControlTypeString += FString::Printf(TEXT("		'%s'\n"), *LayoutObjectNameString);
				}
			}
			auto MessageText = FText::Format(LOCTEXT("MultiLayoutControlAnchorInfoText", "[{0}].{1} Detect multiple layout control this UI's anchor, this may cause issue!\n{2}")
				, FText::FromString(ANSI_TO_TCHAR(__FUNCTION__)), __LINE__, FText::FromString(ControlTypeString));
			TransformCategory.AddCustomRow(LOCTEXT("MultiLayoutControlAnchorError", "MultiLayoutControlAnchorError"))
				.WholeRowContent()
				[
					SNew(STextBlock)
					.Text(MessageText)
					.ColorAndOpacity(FLinearColor(FColor::Yellow))
					.AutoWrapText(true)
				]
			;
			LGUIUtils::EditorNotification(MessageText, 8.0f);
		}
		auto AnchorHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData));
		auto AnchorMinHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.AnchorMin));
		auto AnchorMaxHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.AnchorMax));
		auto AnchoredPositionHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.AnchoredPosition));
		auto SizeDeltaHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.SizeDelta));
		FVector2D AnchorMin, AnchorMax;
		AnchorMinHandle->GetValue(AnchorMin);
		AnchorMaxHandle->GetValue(AnchorMax);

		//anchors preset menu
		FVector2D anchorItemSize(42, 42);
		float itemBasePadding = 8;
		FMargin AnchorLabelMargin = FMargin(4, 2);
		FMargin AnchorValueMargin = FMargin(2, 2);

		auto MakeAnchorLabelWidget = [&](int AnchorLabelIndex) {
			return
				SNew(SBox)
				.Padding(AnchorLabelMargin)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(STextBlock)
					.Text(this, &FUIItemCustomization::GetAnchorLabelText, AnchorMinHandle, AnchorMaxHandle, AnchorLabelIndex)
					.ToolTipText(this, &FUIItemCustomization::GetAnchorLabelTooltipText, AnchorMinHandle, AnchorMaxHandle, AnchorLabelIndex)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			;
		};
		auto MakeAnchorValueWidget = [=, this](int AnchorValueIndex) {
			return
				SNew(SBox)
				.Padding(AnchorValueMargin)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(SNumericEntryBox<float>)
					.AllowSpin(true)
					.MinSliderValue(this, &FUIItemCustomization::GetMinMaxSliderValue, AnchorHandle, AnchorValueIndex, true)
					.MaxSliderValue(this, &FUIItemCustomization::GetMinMaxSliderValue, AnchorHandle, AnchorValueIndex, false)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.UndeterminedString( NSLOCTEXT( "PropertyEditor", "MultipleValues", "Multiple Values") )
					.Value(this, &FUIItemCustomization::GetAnchorValue, AnchorHandle, AnchorValueIndex)
					.OnValueChanged(this, &FUIItemCustomization::OnAnchorValueChanged, AnchorHandle, AnchorValueIndex)
					.OnValueCommitted(this, &FUIItemCustomization::OnAnchorValueCommitted, AnchorHandle, AnchorValueIndex)
					.OnBeginSliderMovement(this, &FUIItemCustomization::OnAnchorValueSliderMovementBegin)
					.OnEndSliderMovement(this, &FUIItemCustomization::OnAnchorValueSliderMovementEnd, AnchorHandle, AnchorValueIndex)
					.IsEnabled(this, &FUIItemCustomization::IsAnchorValueEnable, AnchorHandle, AnchorValueIndex)
				]
			;
		};
		auto DetailBuilderPoiter = &DetailBuilder;
		auto MakeAnchorPreviewWidget = [=, this](LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign HAlign, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign VAlign) {
			return
				SNew(LGUIAnchorPreviewWidget::SAnchorPreviewWidget, anchorItemSize)
				.BasePadding(itemBasePadding)
				.SelectedHAlign(this, &FUIItemCustomization::GetAnchorHAlign, AnchorMinHandle, AnchorMaxHandle)
				.SelectedVAlign(this, &FUIItemCustomization::GetAnchorVAlign, AnchorMinHandle, AnchorMaxHandle)
				.PersistentHAlign(HAlign)
				.PersistentVAlign(VAlign)
				.ButtonEnable(true)
				.OnAnchorChange(this, &FUIItemCustomization::OnSelectAnchor, DetailBuilderPoiter)
			;
		};//@todo: auto refresh SAnchorPreviewWidget when change from AnchorMinMax

		auto SplitLineColor = FLinearColor(0.5f, 0.5f, 0.5f);
		TransformCategory.AddCustomRow(LOCTEXT("Anchor","Anchor"))
		.CopyAction(FUIAction
		(
			FExecuteAction::CreateSP(this, &FUIItemCustomization::OnCopyAnchor),
			FCanExecuteAction::CreateSP(this, &FUIItemCustomization::OnCanCopyAnchor)
		))
		.PasteAction(FUIAction
		(
			FExecuteAction::CreateSP(this, &FUIItemCustomization::OnPasteAnchor, &DetailBuilder),
			FCanExecuteAction::CreateSP(this, &FUIItemCustomization::OnCanPasteAnchor)
		))
		.ValueContent()
		.MinDesiredWidth(500)
		[
			SNew(SBox)
			.IsEnabled(this, &FUIItemCustomization::IsAnchorEditable)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						MakeAnchorLabelWidget(0)
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						MakeAnchorLabelWidget(1)
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						MakeAnchorValueWidget(0)
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						MakeAnchorValueWidget(1)
					]
				]

				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						MakeAnchorLabelWidget(2)
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						MakeAnchorLabelWidget(3)
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						MakeAnchorValueWidget(2)
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					[
						MakeAnchorValueWidget(3)
					]
				]
			]
		]
		.NameContent()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SBox)
				.Padding(2)
				.Visibility(this, &FUIItemCustomization::GetAnchorPresetButtonVisibility)
				[
					SNew(SComboButton)
					.HasDownArrow(false)
					.ToolTipText(this, &FUIItemCustomization::GetAnchorsTooltipText)
					.ButtonStyle(FLGUIEditorStyle::Get(), "AnchorButton")
					.ButtonContent()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.Padding(FMargin(0, 0))
							[
								SNew(SBox)
								.Padding(FMargin(0, 0))
								.HAlign(EHorizontalAlignment::HAlign_Center)
								[
									SNew(STextBlock)
									.Text(this, &FUIItemCustomization::GetHAlignText, AnchorMinHandle, AnchorMaxHandle)
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
							]
							+SVerticalBox::Slot()
							.Padding(FMargin(0, 0))
							.AutoHeight()
							[
								TargetScriptArray[0]->GetParentUIItem() != nullptr
								?
								SNew(SBox)
								[
									SNew(LGUIAnchorPreviewWidget::SAnchorPreviewWidget, FVector2D(40, 40))
									.BasePadding(0)
									.ButtonEnable(false)
									.PersistentHAlign(this, &FUIItemCustomization::GetAnchorHAlign, AnchorMinHandle, AnchorMaxHandle)
									.PersistentVAlign(this, &FUIItemCustomization::GetAnchorVAlign, AnchorMinHandle, AnchorMaxHandle)
									//.SelectedHAlign(this, &FUIItemCustomization::GetAnchorHAlign, AnchorMinHandle, AnchorMaxHandle)
									//.SelectedVAlign(this, &FUIItemCustomization::GetAnchorVAlign, AnchorMinHandle, AnchorMaxHandle)
								]
								:
								SNew(SBox)
							]
						]
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SBox)
							.Padding(FMargin(0, 0))
							.HAlign(EHorizontalAlignment::HAlign_Center)
							[
								SNew(STextBlock)
								.Text(this, &FUIItemCustomization::GetVAlignText, AnchorMinHandle, AnchorMaxHandle)
								.Font(IDetailLayoutBuilder::GetDetailFont())
								.Justification(ETextJustify::Center)
								.RenderTransformPivot(FVector2D(0, 0.5f))
								.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(90)), FVector2D(-12, -10)))
							]
						]
					]
					.MenuContent()
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SBorder)
							.Padding(4)
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot()
								[
									SNew(STextBlock)
									.Text(LOCTEXT("AnchorPresets", "Anchor Presets"))
								]
								+SVerticalBox::Slot()
								[
									SNew(STextBlock)
									.Text(LOCTEXT("AnchorPresetsHelperKeys", "Shift: Also set pivot		Alt: Also set position"))
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
							]
						]
						+SVerticalBox::Slot()
						.AutoHeight()
						.Padding(4)
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SOverlay)
									+SOverlay::Slot()
									[
										SNew(SUniformGridPanel)
										+SUniformGridPanel::Slot(1, 0)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::None)
										]
										+SUniformGridPanel::Slot(2, 0) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::None)
										]
										+SUniformGridPanel::Slot(3, 0) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::None)
										]
										+SUniformGridPanel::Slot(4, 0) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::None)
										]
										//Top
										+SUniformGridPanel::Slot(0, 1)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::None, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top)
										]
										+SUniformGridPanel::Slot(1, 1)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top)
										]
										+SUniformGridPanel::Slot(2, 1) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top)
										]
										+SUniformGridPanel::Slot(3, 1) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top)
										]
										+SUniformGridPanel::Slot(4, 1) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top)
										]
										//Center
										+SUniformGridPanel::Slot(0, 2)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::None, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle)
										]
										+SUniformGridPanel::Slot(1, 2)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle)
										]
										+SUniformGridPanel::Slot(2, 2) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle)
										]
										+SUniformGridPanel::Slot(3, 2) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle)
										]
										+SUniformGridPanel::Slot(4, 2) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle)
										]
										//Bottom
										+SUniformGridPanel::Slot(0, 3)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::None, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom)
										]
										+SUniformGridPanel::Slot(1, 3)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom)
										]
										+SUniformGridPanel::Slot(2, 3) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom)
										]
										+SUniformGridPanel::Slot(3, 3) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom)
										]
										+SUniformGridPanel::Slot(4, 3) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom)
										]
										//Bottom stretch
										+SUniformGridPanel::Slot(0, 4)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::None, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch)
										]
										+SUniformGridPanel::Slot(1, 4)
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch)
										]
										+SUniformGridPanel::Slot(2, 4) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch)
										]
										+SUniformGridPanel::Slot(3, 4) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch)
										]
										+SUniformGridPanel::Slot(4, 4) 
										[
											MakeAnchorPreviewWidget(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch)
										]
									]
									//splite line
									+ SOverlay::Slot()
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.HAlign(EHorizontalAlignment::HAlign_Left)
										[
											SNew(SBox)
											.WidthOverride(anchorItemSize.X + 16)
											[
												SNew(SBox)
												.HAlign(EHorizontalAlignment::HAlign_Right)
												.WidthOverride(1)
												[
													SNew(SImage)
													.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot"))
													.ColorAndOpacity(SplitLineColor)
												]
											]
										]
									]
									+ SOverlay::Slot()
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.HAlign(EHorizontalAlignment::HAlign_Right)
										[
											SNew(SBox)
											.WidthOverride(anchorItemSize.X + 16)
											[
												SNew(SBox)
												.HAlign(EHorizontalAlignment::HAlign_Left)
												.WidthOverride(1)
												[
													SNew(SImage)
													.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot"))
													.ColorAndOpacity(SplitLineColor)
												]
											]
										]
									]
									+ SOverlay::Slot()
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.VAlign(EVerticalAlignment::VAlign_Top)
										[
											SNew(SBox)
											.HeightOverride(anchorItemSize.X + 16)
											[
												SNew(SBox)
												.VAlign(EVerticalAlignment::VAlign_Bottom)
												.HeightOverride(1)
												[
													SNew(SImage)
													.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot"))
													.ColorAndOpacity(SplitLineColor)
												]
											]
										]
									]
									+ SOverlay::Slot()
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.VAlign(EVerticalAlignment::VAlign_Bottom)
										[
											SNew(SBox)
											.HeightOverride(anchorItemSize.X + 16)
											[
												SNew(SBox)
												.VAlign(EVerticalAlignment::VAlign_Top)
												.HeightOverride(1)
												[
													SNew(SImage)
													.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot"))
													.ColorAndOpacity(SplitLineColor)
												]
											]
										]
									]
								]
							]
						]
					]
				]
			]
		]
		;

		IDetailGroup& AnchorGroup = TransformCategory.AddGroup(FName("Anchors"), LOCTEXT("AnchorsGroup", "Anchors"));

		auto AnchorControlledByLayout = GetLayoutControlHorizontalAnchor() || GetLayoutControlVerticalAnchor();
		IDetailPropertyRow& AnchorMinProperty = AnchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.AnchorMin)));
		if (!this->IsAnchorEditable())
		{
			AnchorMinProperty.IsEnabled(false);
			AnchorMinProperty.ToolTip(LOCTEXT("ControlledByLayoutTip", "This property is controlled by layout"));
		}

		IDetailPropertyRow& AnchorMaxProperty = AnchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.AnchorMax)));
		if (!this->IsAnchorEditable())
		{
			AnchorMaxProperty.IsEnabled(false);
			AnchorMaxProperty.ToolTip(LOCTEXT("ControlledByLayoutTip", "This property is controlled by layout"));
		}

		auto& AnchorRawDataGroup = TransformCategory.AddGroup(FName("AnchorsRawData"), LOCTEXT("AnchorsRawData", "AnchorsRawData"), true);
		AnchorRawDataGroup.AddWidgetRow()
		.WholeRowContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("AnchorRawDataWarning", "Normally do not edit these!"))
			.ColorAndOpacity(FLinearColor(FColor::Yellow))
			.AutoWrapText(true)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		;
		auto& AnchoredPositionProperty = AnchorRawDataGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.AnchoredPosition)));
		auto& SizeDeltaProperty = AnchorRawDataGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.SizeDelta)));
		AnchoredPositionProperty.IsEnabled(this->IsAnchorEditable());
		SizeDeltaProperty.IsEnabled(this->IsAnchorEditable());
	}
	//pivot
	auto PivotHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.Pivot));
	auto& PivotProperty = TransformCategory.AddProperty(PivotHandle);
	PivotProperty.IsEnabled(this->IsAnchorEditable());
	PivotHandle->SetOnPropertyValuePreChange(FSimpleDelegate::CreateLambda([=, this] {
		this->OnPrePivotChange();
		}));
	PivotHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=, this] {
		this->OnPivotChanged();
		}));
	PivotHandle->SetOnChildPropertyValuePreChange(FSimpleDelegate::CreateLambda([=, this] {
		this->OnPrePivotChange();
		}));
	PivotHandle->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateLambda([=, this] {
		this->OnPivotChanged();
		}));

	//location rotation scale
	const FSelectedActorInfo& selectedActorInfo = DetailBuilder.GetDetailsView()->GetSelectedActorInfo();
	TSharedRef<FComponentTransformDetails> transformDetails = MakeShareable(new FComponentTransformDetails(TargetScriptArray, selectedActorInfo, DetailBuilder));
	TransformCategory.AddCustomBuilder(transformDetails);

	//HierarchyIndex
	{
		auto HierarchyIndexHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, hierarchyIndex));
		DetailBuilder.HideProperty(HierarchyIndexHandle);
		HierarchyIndexHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=, this] {
			ForceUpdateUI();
			ULGUIPrefabManagerObject::MarkBroadcastLevelActorListChanged();
			}));
		auto hierarchyIndexWidget =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			[
				HierarchyIndexHandle->CreatePropertyValueWidget()
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.AutoWidth()
			[
				SNew(SButton)
				.ToolTipText(LOCTEXT("IncreaseHierarchyOrder_Tooltip", "Move order up"))
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.IsEnabled_Static(LGUIEditorUtils::IsEnabledOnProperty, HierarchyIndexHandle)
				.OnClicked(this, &FUIItemCustomization::OnClickIncreaseOrDecreaseHierarchyIndex, true, HierarchyIndexHandle)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("IncreaseHierarchyOrder", "+"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.AutoWidth()
			[
				SNew(SButton)
				.ToolTipText(LOCTEXT("DecreaseHierarchyOrder_Tooltip", "Move order down"))
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.IsEnabled_Static(LGUIEditorUtils::IsEnabledOnProperty, HierarchyIndexHandle)
				.OnClicked(this, &FUIItemCustomization::OnClickIncreaseOrDecreaseHierarchyIndex, false, HierarchyIndexHandle)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("DecreaseHierarchyOrder", "-"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			];

		TransformCategory.AddCustomRow(LOCTEXT("HierarchyIndexManager", "HierarchyIndexManager"))
		.CopyAction(FUIAction(
			FExecuteAction::CreateSP(this, &FUIItemCustomization::OnCopyHierarchyIndex)
		))
		.PasteAction(FUIAction(
			FExecuteAction::CreateSP(this, &FUIItemCustomization::OnPasteHierarchyIndex, HierarchyIndexHandle)
		))
		.NameContent()
		[
			HierarchyIndexHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			hierarchyIndexWidget
		]
		;

		TransformCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, flattenHierarchyIndex)), EPropertyLocation::Advanced);
	}
		
	//displayName
	auto displayNamePropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, displayName));
	DetailBuilder.HideProperty(displayNamePropertyHandle);
	LGUICategory.AddCustomRow(LOCTEXT("DisplayName", "Display Name"), true)
		.NameContent()
		[
			displayNamePropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(500)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				displayNamePropertyHandle->CreatePropertyValueWidget(true)
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("FixDisplayName", "Fix it"))
				.OnClicked(this, &FUIItemCustomization::OnClickFixDisplayNameButton, true, displayNamePropertyHandle)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.Visibility(this, &FUIItemCustomization::GetDisplayNameWarningVisibility)
				.ToolTipText(LOCTEXT("FixDisplayName_Tooltip", "DisplayName not equal to ActorLabel."))
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("FixDisplayNameOnHierarchy", "Fix all hierarchy"))
				.OnClicked(this, &FUIItemCustomization::OnClickFixDisplayNameButton, false, displayNamePropertyHandle)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.Visibility(this, &FUIItemCustomization::GetDisplayNameWarningVisibility)
				.ToolTipText(LOCTEXT("FixDisplayNameOnHierarchy_Tooltip", "DisplayName not equal to ActorLabel."))
			]
		]
		;
}

EVisibility FUIItemCustomization::GetAnchorPresetButtonVisibility()const
{
	if (TargetScriptArray.Num() > 0 && TargetScriptArray[0].IsValid())
	{
		return TargetScriptArray[0]->GetParentUIItem() != nullptr ? EVisibility::Visible : EVisibility::Hidden;
	}
	return EVisibility::Hidden;
}

bool FUIItemCustomization::IsAnchorEditable()const
{
	if (TargetScriptArray.Num() > 0 && TargetScriptArray[0].IsValid())
	{
		auto UIItem = TargetScriptArray[0];
		if (FLGUIPrefabEditor::ActorIsRootAgent(UIItem->GetOwner()))return true;//special for PrefabEditor's agent root actor
		if (UIItem->GetParentUIItem() != nullptr)return true;//not root
		if (UIItem->IsCanvasUIItem() && UIItem->GetRenderCanvas() != nullptr && UIItem->GetRenderCanvas()->IsRenderToScreenSpace())//is root canvas, and is render to screen space
		{
			return false;
		}
	}
	return true;
}

void FUIItemCustomization::OnPrePivotChange()
{
	AnchorAsMarginArray.Empty();
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return;
	for (auto& Item : TargetScriptArray)
	{
		FMargin AnchorAsMargin;
		if (Item.IsValid())
		{
			AnchorAsMargin.Left = Item->GetAnchorLeft();
			AnchorAsMargin.Right = Item->GetAnchorRight();
			AnchorAsMargin.Bottom = Item->GetAnchorBottom();
			AnchorAsMargin.Top = Item->GetAnchorTop();
		}
		AnchorAsMarginArray.Add(AnchorAsMargin);
	}
}
void FUIItemCustomization::OnPivotChanged()
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return;
	for (int i = 0; i < TargetScriptArray.Num(); i++)
	{
		auto& Item = TargetScriptArray[i];
		auto& AnchorAsMargin = AnchorAsMarginArray[i];
		if (Item.IsValid())
		{
			Item->MarkAllDirtyRecursive();
			//set anchors to make it stay as relative to parent
			Item->SetAnchorLeft(AnchorAsMargin.Left);
			Item->SetAnchorRight(AnchorAsMargin.Right);
			Item->SetAnchorBottom(AnchorAsMargin.Bottom);
			Item->SetAnchorTop(AnchorAsMargin.Top);
		}
	}
}

EVisibility FUIItemCustomization::GetDisplayNameWarningVisibility()const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return EVisibility::Hidden;

	if (auto actor = TargetScriptArray[0]->GetOwner())
	{
		if (TargetScriptArray[0] == actor->GetRootComponent())
		{
			auto actorLabel = actor->GetActorLabel();
			if (actorLabel.StartsWith("//"))
			{
				actorLabel = actorLabel.Right(actorLabel.Len() - 2);
			}
			if (TargetScriptArray[0]->GetDisplayName() == actorLabel)
			{
				return EVisibility::Hidden;
			}
			else
			{
				return EVisibility::Visible;
			}
		}
		else
		{
			if (TargetScriptArray[0]->GetName() == TargetScriptArray[0]->GetDisplayName())
			{
				return EVisibility::Hidden;
			}
			else
			{
				return EVisibility::Visible;
			}
		}
	}
	else
	{
		auto name = TargetScriptArray[0]->GetName();
		auto genVarSuffix = FString(TEXT("_GEN_VARIABLE"));
		if (name.EndsWith(genVarSuffix))
		{
			name.RemoveAt(name.Len() - genVarSuffix.Len(), genVarSuffix.Len());
		}
		if (TargetScriptArray[0]->GetDisplayName() == name)
		{
			return EVisibility::Hidden;
		}
		else
		{
			return EVisibility::Visible;
		}
	}
}

FReply FUIItemCustomization::OnClickIncreaseOrDecreaseHierarchyIndex(bool IncreaseOrDecrease, TSharedRef<IPropertyHandle> HierarchyIndexHandle)
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return FReply::Handled();

	//hierarchy index could affect other items
	GEditor->BeginTransaction(LOCTEXT("ChangeHierarchyIndex_Transaction", "Change LGUI Hierarchy Index"));
	for (auto& Item : TargetScriptArray)
	{
		Item->Modify();
		if (auto Parent = Item->GetParentUIItem())
		{
			for (auto Child : Parent->UIChildren)
			{
				Child->Modify();
			}
		}
	}

	for (auto& Item : TargetScriptArray)
	{
		HierarchyIndexHandle->SetValue(Item->hierarchyIndex + (IncreaseOrDecrease ? 1 : -1));
		//notify others
		if (auto Parent = Item->GetParentUIItem())
		{
			for (auto Child : Parent->UIChildren)
			{
				auto HierarchyIndexProperty = FindFProperty<FIntProperty>(UUIItem::StaticClass(), GET_MEMBER_NAME_CHECKED(UUIItem, hierarchyIndex));
				check(HierarchyIndexProperty != nullptr);
				LGUIUtils::NotifyPropertyChanged(Child, HierarchyIndexProperty);
			}
		}
	}
	GEditor->EndTransaction();

	ULGUIPrefabManagerObject::MarkBroadcastLevelActorListChanged();
	return FReply::Handled();
}

FText FUIItemCustomization::GetHAlignText(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return FText();

	FVector2D AnchorMinValue;
	AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	AnchorMaxHandle->GetValue(AnchorMaxValue);

	if (AnchorMinValue.X == AnchorMaxValue.X)
	{
		if (AnchorMinValue.X == 0)
		{
			return LOCTEXT("AnchorLeft", "Left");
		}
		else if (AnchorMinValue.X == 0.5f)
		{
			return LOCTEXT("AnchorCenter", "Center");
		}
		else if (AnchorMinValue.X == 1.0f)
		{
			return LOCTEXT("AnchorRight", "Right");
		}
		else
		{
			return LOCTEXT("AnchorCustom", "Custom");
		}
	}
	else if (AnchorMinValue.X == 0.0f && AnchorMaxValue.X == 1.0f)
	{
		return LOCTEXT("AnchorStretch", "Stretch");
	}
	else
	{
		return LOCTEXT("AnchorCustom", "Custom");
	}
}
FText FUIItemCustomization::GetVAlignText(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return FText();

	FVector2D AnchorMinValue;
	AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	AnchorMaxHandle->GetValue(AnchorMaxValue);

	if (AnchorMinValue.Y == AnchorMaxValue.Y)
	{
		if (AnchorMinValue.Y == 0)
		{
			return LOCTEXT("AnchorBottom", "Bottom");
		}
		else if (AnchorMinValue.Y == 0.5f)
		{
			return LOCTEXT("AnchorMiddle", "Middle");
		}
		else if (AnchorMinValue.Y == 1.0f)
		{
			return LOCTEXT("AnchorTop", "Top");
		}
		else
		{
			return LOCTEXT("AnchorCustom", "Custom");
		}
	}
	else if (AnchorMinValue.Y == 0.0f && AnchorMaxValue.Y == 1.0f)
	{
		return LOCTEXT("AnchorStretch", "Stretch");
	}
	else
	{
		return LOCTEXT("AnchorCustom", "Custom");
	}
}

FText FUIItemCustomization::GetAnchorLabelText(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle, int LabelIndex)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return FText();

	FVector2D AnchorMinValue;
	AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	AnchorMaxHandle->GetValue(AnchorMaxValue);

	switch (LabelIndex)
	{
	case 0://anchored position y, stretch left
	{
		if (AnchorMinValue.X == AnchorMaxValue.X)
		{
			return LOCTEXT("AnchoredPositionX", "PosY");
		}
		else
		{
			return LOCTEXT("AnchoredLeft", "Left");
		}
	}
	break;
	case 1://anchored position z, stretch top
	{
		if (AnchorMinValue.Y == AnchorMaxValue.Y)
		{
			return LOCTEXT("AnchoredPositionY", "PosZ");
		}
		else
		{
			return LOCTEXT("AnchoredTop", "Top");
		}
	}
	break;
	case 2://width, stretch right
	{
		if (AnchorMinValue.X == AnchorMaxValue.X)
		{
			return LOCTEXT("Width", "Width");
		}
		else
		{
			return LOCTEXT("AnchoredRight", "Right");
		}
	}
	break;
	case 3://height, stretch bottom
	{
		if (AnchorMinValue.Y == AnchorMaxValue.Y)
		{
			return LOCTEXT("Height", "Height");
		}
		else
		{
			return LOCTEXT("AnchoredBottom", "Bottom");
		}
	}
	break;
	}
	return LOCTEXT("AnchorEroor", "Error");
}

FText FUIItemCustomization::GetAnchorLabelTooltipText(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle, int LabelTooltipIndex)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return FText();

	FVector2D AnchorMinValue;
	AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	AnchorMaxHandle->GetValue(AnchorMaxValue);

	switch (LabelTooltipIndex)
	{
	default:
	case 0://anchored position x, stretch left
	{
		if (AnchorMinValue.X == AnchorMaxValue.X)
		{
			return FText::Format(LOCTEXT("AnchoredPositionX_Tooltip", "Horizontal anchored position. Related function: {0} / {1}."), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, GetAnchoredPosition)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, SetAnchoredPosition)));
		}
		else
		{
			return FText::Format(LOCTEXT("AnchoredLeft_Tooltip", "Calculated distance to parent's left anchor point. Related function: {0} / {1}."), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, GetAnchorLeft)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, SetAnchorLeft)));
		}
	}
	break;
	case 1://anchored position y, stretch top
	{
		if (AnchorMinValue.Y == AnchorMaxValue.Y)
		{
			return FText::Format(LOCTEXT("AnchoredPositionY_Tooltip", "Vertical anchored position. Related function: {0} / {1}."), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, GetAnchoredPosition)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, SetAnchoredPosition)));
		}
		else
		{
			return FText::Format(LOCTEXT("AnchoredTop_Tooltip", "Calculated distance to parent's top anchor point. Related function: {0} / {1}."), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, GetAnchorLeft)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, SetAnchorLeft)));
		}
	}
	break;
	case 2://width, stretch right
	{
		if (AnchorMinValue.X == AnchorMaxValue.X)
		{
			return FText::Format(LOCTEXT("Width_Tooltip", "Horizontal size. Related function: {0} / {1}."), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, GetWidth)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, SetWidth)));
		}
		else
		{
			return FText::Format(LOCTEXT("AnchoredRight_Tooltip", "Calculated distance to parent's right anchor point. Related function: {0} / {1}."), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, GetAnchorLeft)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, SetAnchorLeft)));
		}
	}
	break;
	case 3://height, stretch bottom
	{
		if (AnchorMinValue.Y == AnchorMaxValue.Y)
		{
			return FText::Format(LOCTEXT("Height_Tooltip", "Vertical size. Related function: {0} / {1}"), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, GetHeight)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, SetHeight)));
		}
		else
		{
			return FText::Format(LOCTEXT("AnchoredBottom_Tooltip", "Calculated distance to parent's bottom anchor point. Related function: {0} / {0}."), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, GetAnchorLeft)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, SetAnchorLeft)));
		}
	}
	break;
	}
	return LOCTEXT("AnchorError", "Error");
}

TArray<float> FUIItemCustomization::ValueRangeArray = {
		1.0f, 10.0f, 100.0f, 1000.0f, 10000.0f
};
TOptional<float> FUIItemCustomization::GetMinMaxSliderValue(TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex, bool MinOrMax)const
{
	auto Value = GetAnchorValue(AnchorHandle, AnchorValueIndex).Get(0.0f);
	Value = FMath::Abs(Value);
	float MaxRangeValue = ValueRangeArray[ValueRangeArray.Num() - 1];
	float RangeValue = MaxRangeValue;
	for (int i = ValueRangeArray.Num() - 1; i >= 0; i--)
	{
		auto RangeValueItem = ValueRangeArray[i];
		if (Value > RangeValueItem)
		{
			break;
		}
		else
		{
			RangeValue = RangeValueItem;
		}
	}
	return RangeValue * 
		(RangeValue >= MaxRangeValue ? 1.0f : (FMath::Abs(Value - RangeValue) < KINDA_SMALL_NUMBER ? 2.0f : 1.0f))
		* (MinOrMax ? -1.0f : 1.0f);
}

TOptional<float> FUIItemCustomization::GetAnchorValue(TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return TOptional<float>();

	auto AnchorMinHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIAnchorData, AnchorMin));
	auto AnchorMaxHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIAnchorData, AnchorMax));
	auto AnchoredPositionHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIAnchorData, AnchoredPosition));
	auto SizeDeltaHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIAnchorData, SizeDelta));

	FVector2D AnchorMinValue;
	auto AnchorMinValueAccessResult = AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	auto AnchorMaxValueAccessResult = AnchorMaxHandle->GetValue(AnchorMaxValue);
	FVector2D AnchoredPosition;
	auto AnchoredPositionAccessResult = AnchoredPositionHandle->GetValue(AnchoredPosition);
	FVector2D SizeDelta;
	auto SizeDeltaAccessResult = SizeDeltaHandle->GetValue(SizeDelta);

	switch (AnchorValueIndex)
	{
	default:
	case 0://anchored position x, stretch left
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success)
		{
			auto GetValue = [=](TWeakObjectPtr<UUIItem> Item)->float {
				if (AnchorMinValue.X == AnchorMaxValue.X)
				{
					return Item->GetHorizontalAnchoredPosition();
				}
				else
				{
					return Item->GetAnchorLeft();
				}
			};
			if (AnchoredPositionAccessResult == FPropertyAccess::Result::Success)
			{
				return GetValue(TargetScriptArray[0]);
			}
			else if (AnchoredPositionAccessResult == FPropertyAccess::Result::MultipleValues)
			{
				bool bIsSameValue = true;
				float Value = 0;
				bool bIsFirst = true;
				for (auto& Item : TargetScriptArray)
				{
					if (bIsFirst)
					{
						Value = GetValue(Item);
						bIsFirst = false;
					}
					else
					{
						if (FMath::Abs(GetValue(Item) - Value) > KINDA_SMALL_NUMBER)
						{
							bIsSameValue = false;
							break;
						}
					}
				}
				if (bIsSameValue)
				{
					return Value;
				}
			}
		}
		return TOptional<float>();
	}
	break;
	case 1://anchored position y, stretch top
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success)
		{
			auto GetValue = [=](TWeakObjectPtr<UUIItem> Item)->float {
				if (AnchorMinValue.Y == AnchorMaxValue.Y)
				{
					return Item->GetVerticalAnchoredPosition();
				}
				else
				{
					return Item->GetAnchorTop();
				}
			};
			if (AnchoredPositionAccessResult == FPropertyAccess::Result::Success)
			{
				return GetValue(TargetScriptArray[0]);
			}
			else if (AnchoredPositionAccessResult == FPropertyAccess::Result::MultipleValues)
			{
				bool bIsSameValue = true;
				float Value = 0;
				bool bIsFirst = true;
				for (auto& Item : TargetScriptArray)
				{
					if (bIsFirst)
					{
						Value = GetValue(Item);
						bIsFirst = false;
					}
					else
					{
						if (FMath::Abs(GetValue(Item) - Value) > KINDA_SMALL_NUMBER)
						{
							bIsSameValue = false;
							break;
						}
					}
				}
				if (bIsSameValue)
				{
					return Value;
				}
			}
		}
		return TOptional<float>();
	}
	break;
	case 2://width, stretch right
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success)
		{
			auto GetValue = [=](TWeakObjectPtr<UUIItem> Item)->float {
				if (AnchorMinValue.X == AnchorMaxValue.X)
				{
					return Item->GetSizeDelta().X;
				}
				else
				{
					return Item->GetAnchorRight();
				}
			};
			if (SizeDeltaAccessResult == FPropertyAccess::Result::Success)
			{
				return GetValue(TargetScriptArray[0]);
			}
			else if (SizeDeltaAccessResult == FPropertyAccess::Result::MultipleValues)
			{
				bool bIsSameValue = true;
				float Value = 0;
				bool bIsFirst = true;
				for (auto& Item : TargetScriptArray)
				{
					if (bIsFirst)
					{
						Value = GetValue(Item);
						bIsFirst = false;
					}
					else
					{
						if (FMath::Abs(GetValue(Item) - Value) > KINDA_SMALL_NUMBER)
						{
							bIsSameValue = false;
							break;
						}
					}
				}
				if (bIsSameValue)
				{
					return Value;
				}
			}
		}
		return TOptional<float>();
	}
	break;
	case 3://height, stretch bottom
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success)
		{
			auto GetValue = [=](TWeakObjectPtr<UUIItem> Item)->float {
				if (AnchorMinValue.Y == AnchorMaxValue.Y)
				{
					return Item->GetSizeDelta().Y;
				}
				else
				{
					return Item->GetAnchorBottom();
				}
			};
			if (SizeDeltaAccessResult == FPropertyAccess::Result::Success)
			{
				return GetValue(TargetScriptArray[0]);
			}
			else if (SizeDeltaAccessResult == FPropertyAccess::Result::MultipleValues)
			{
				bool bIsSameValue = true;
				float Value = 0;
				bool bIsFirst = true;
				for (auto& Item : TargetScriptArray)
				{
					if (bIsFirst)
					{
						Value = GetValue(Item);
						bIsFirst = false;
					}
					else
					{
						if (FMath::Abs(GetValue(Item) - Value) > KINDA_SMALL_NUMBER)
						{
							bIsSameValue = false;
							break;
						}
					}
				}
				if (bIsSameValue)
				{
					return Value;
				}
			}
		}
		return TOptional<float>();
	}
	break;
	}
}
void FUIItemCustomization::ApplyValueChanged(float Value, TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex)
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return;

	auto AnchorMinHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIAnchorData, AnchorMin));
	auto AnchorMaxHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIAnchorData, AnchorMax));
	auto AnchoredPositionHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIAnchorData, AnchoredPosition));
	auto SizeDeltaHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIAnchorData, SizeDelta));

	FVector2D AnchorMinValue;
	AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	AnchorMaxHandle->GetValue(AnchorMaxValue);

	switch (AnchorValueIndex)
	{
	case 0://anchored position x, stretch left
	{
		if (AnchorMinValue.X == AnchorMaxValue.X)
		{
			for (auto& Item : TargetScriptArray)
			{
				Item->SetHorizontalAnchoredPosition(Value);
			}
		}
		else
		{
			for (auto& Item : TargetScriptArray)
			{
				Item->SetAnchorLeft(Value);
			}
		}
	}
	break;
	case 1://anchored position y, stretch top
	{
		if (AnchorMinValue.Y == AnchorMaxValue.Y)
		{
			for (auto& Item : TargetScriptArray)
			{
				Item->SetVerticalAnchoredPosition(Value);
			}
		}
		else
		{
			for (auto& Item : TargetScriptArray)
			{
				Item->SetAnchorTop(Value);
			}
		}
	}
	break;
	case 2://width, stretch right
	{
		if (AnchorMinValue.X == AnchorMaxValue.X)
		{
			for (auto& Item : TargetScriptArray)
			{
				Item->SetWidth(Value);
			}
		}
		else
		{
			for (auto& Item : TargetScriptArray)
			{
				Item->SetAnchorRight(Value);
			}
		}
	}
	break;
	case 3://height, stretch bottom
	{
		if (AnchorMinValue.Y == AnchorMaxValue.Y)
		{
			for (auto& Item : TargetScriptArray)
			{
				Item->SetHeight(Value);
			}
		}
		else
		{
			for (auto& Item : TargetScriptArray)
			{
				Item->SetAnchorBottom(Value);
			}
		}
	}
	break;
	}

	auto AnchorProperty = FindFProperty<FProperty>(UUIItem::StaticClass(), GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData));
	auto RelativeLocationProperty = FindFProperty<FProperty>(USceneComponent::StaticClass(), FName(TEXT("RelativeLocation")));
	for (auto& Item : TargetScriptArray)
	{
		LGUIUtils::NotifyPropertyChanged(Item.Get(), AnchorProperty);
		LGUIUtils::NotifyPropertyChanged(Item.Get(), RelativeLocationProperty);
	}
}
void FUIItemCustomization::OnAnchorValueChanged(float Value, TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex)
{
	GEditor->BeginTransaction(LOCTEXT("ChangeAnchorValue_Transaction", "Change LGUI Anchor Value"));
	for (auto& Item : TargetScriptArray)
	{
		Item->Modify();
	}
	ApplyValueChanged(Value, AnchorHandle, AnchorValueIndex);
	GEditor->EndTransaction();
}
void FUIItemCustomization::OnAnchorValueCommitted(float Value, ETextCommit::Type commitType, TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex)
{
	GEditor->BeginTransaction(LOCTEXT("CommitAnchorValue_Transaction", "Commit LGUI Anchor Value"));
	for (auto& Item : TargetScriptArray)
	{
		Item->Modify();
	}
	ApplyValueChanged(Value, AnchorHandle, AnchorValueIndex);
	GEditor->EndTransaction();
}

void FUIItemCustomization::OnAnchorValueSliderMovementBegin()
{
	GEditor->BeginTransaction(LOCTEXT("SlideAnchorValue_Transaction", "Slide LGUI Anchor Value"));
	for (auto& Item : TargetScriptArray)
	{
		Item->Modify();
	}
}

void FUIItemCustomization::OnAnchorValueSliderMovementEnd(float Value, TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex)
{
	ApplyValueChanged(Value, AnchorHandle, AnchorValueIndex);
	GEditor->EndTransaction();
}

LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign FUIItemCustomization::GetAnchorHAlign(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::None;

	FVector2D AnchorMinValue;
	AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	AnchorMaxHandle->GetValue(AnchorMaxValue);

	LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign AnchorHAlign = LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::None;
	if (AnchorMinValue.X == AnchorMaxValue.X)
	{
		if (AnchorMinValue.X == 0)
		{
			AnchorHAlign = LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left;
		}
		else if (AnchorMinValue.X == 0.5f)
		{
			AnchorHAlign = LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center;
		}
		else if (AnchorMinValue.X == 1.0f)
		{
			AnchorHAlign = LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right;
		}
	}
	else if (AnchorMinValue.X == 0.0f && AnchorMaxValue.X == 1.0f)
	{
		AnchorHAlign = LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch;
	}
	return AnchorHAlign;
}
LGUIAnchorPreviewWidget::UIAnchorVerticalAlign FUIItemCustomization::GetAnchorVAlign(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::None;

	FVector2D AnchorMinValue;
	AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	AnchorMaxHandle->GetValue(AnchorMaxValue);

	LGUIAnchorPreviewWidget::UIAnchorVerticalAlign AnchorVAlign = LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::None;
	if (AnchorMinValue.Y == AnchorMaxValue.Y)
	{
		if (AnchorMinValue.Y == 0)
		{
			AnchorVAlign = LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom;
		}
		else if (AnchorMinValue.Y == 0.5f)
		{
			AnchorVAlign = LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle;
		}
		else if (AnchorMinValue.Y == 1.0f)
		{
			AnchorVAlign = LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top;
		}
	}
	else if (AnchorMinValue.Y == 0.0f && AnchorMaxValue.Y == 1.0f)
	{
		AnchorVAlign = LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch;
	}
	return AnchorVAlign;
}

void FUIItemCustomization::OnSelectAnchor(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign HorizontalAlign, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign VerticalAlign, IDetailLayoutBuilder* DetailBuilder)
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return;

	bool ShiftPressed = FSlateApplication::Get().GetModifierKeys().IsShiftDown();
	bool AltPressed = FSlateApplication::Get().GetModifierKeys().IsAltDown();

	GEditor->BeginTransaction(LOCTEXT("ChangeAnchor_Transaction", "Change LGUI Anchor"));
	for (auto& UIItem : TargetScriptArray)
	{
		UIItem->Modify();
	}

	for (auto& UIItem : TargetScriptArray)
	{
		FVector2D DesiredPivot = UIItem->GetPivot();
		auto AnchorMin = UIItem->GetAnchorMin();
		auto AnchorMax = UIItem->GetAnchorMax();
		switch (HorizontalAlign)
		{
		case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::None:
			break;
		case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left:
		{
			DesiredPivot.X = 0;
			AnchorMin.X = AnchorMax.X = 0;
		}
			break;
		case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center:
		{
			DesiredPivot.X = 0.5f;
			AnchorMin.X = AnchorMax.X = 0.5f;
		}
			break;
		case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right:
		{
			DesiredPivot.X = 1.0f;
			AnchorMin.X = AnchorMax.X = 1.0f;
		}
			break;
		case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch:
		{
			DesiredPivot.X = 0.5f;
			AnchorMin.X = 0;
			AnchorMax.X = 1.0f;
		}
		break;
		}
		switch (VerticalAlign)
		{
		case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::None:
			break;
		case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top:
		{
			DesiredPivot.Y = 1.0f;
			AnchorMin.Y = AnchorMax.Y = 1;
		}
			break;
		case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle:
		{
			DesiredPivot.Y = 0.5f;
			AnchorMin.Y = AnchorMax.Y = 0.5f;
		}
			break;
		case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom:
		{
			DesiredPivot.Y = 0.0f;
			AnchorMin.Y = AnchorMax.Y = 0.0f;
		}
			break;
		case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch:
		{
			DesiredPivot.Y = 0.5f;
			AnchorMin.Y = 0;
			AnchorMax.Y = 1.0f;
		}
		break;
		}
		auto PrevRelativeLocation = UIItem->GetRelativeLocation();
		auto PrevWidth = UIItem->GetWidth();
		auto PrevHeight = UIItem->GetHeight();
		UIItem->SetAnchorMin(AnchorMin);
		UIItem->SetAnchorMax(AnchorMax);
		UIItem->MarkAllDirtyRecursive();
		UIItem->SetWidth(PrevWidth);
		UIItem->SetHeight(PrevHeight);
		UIItem->SetRelativeLocation(PrevRelativeLocation);
		if (AltPressed)
		{
			switch (HorizontalAlign)
			{
			case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left:
			{
				UIItem->SetHorizontalAnchoredPosition(-UIItem->GetLocalSpaceLeft());
			}
				break;
			case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center:
			{
				UIItem->SetHorizontalAnchoredPosition(UIItem->GetWidth() * (UIItem->GetPivot().X - 0.5f));
			}
				break;
			case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right:
			{
				UIItem->SetHorizontalAnchoredPosition(-UIItem->GetLocalSpaceRight());
			}
				break;
			case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch:
			{
				UIItem->SetAnchorLeft(0);
				UIItem->SetAnchorRight(0);
			}
				break;
			}
			switch (VerticalAlign)
			{
			case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Top:
			{
				UIItem->SetVerticalAnchoredPosition(-UIItem->GetLocalSpaceTop());
			}
				break;
			case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle:
			{
				UIItem->SetVerticalAnchoredPosition(UIItem->GetHeight() * (UIItem->GetPivot().Y - 0.5f));
			}
				break;
			case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom:
			{
				UIItem->SetVerticalAnchoredPosition(-UIItem->GetLocalSpaceBottom());
			}
				break;
			case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch:
			{
				UIItem->SetAnchorBottom(0);
				UIItem->SetAnchorTop(0);
			}
				break;
			}
		}
		if (ShiftPressed)
		{
			FMargin PrevAnchorAsMargin(UIItem->GetAnchorLeft(), UIItem->GetAnchorTop(), UIItem->GetAnchorRight(), UIItem->GetAnchorBottom());
			UIItem->SetPivot(DesiredPivot);
			UIItem->SetAnchorLeft(PrevAnchorAsMargin.Left);
			UIItem->SetAnchorRight(PrevAnchorAsMargin.Right);
			UIItem->SetAnchorBottom(PrevAnchorAsMargin.Bottom);
			UIItem->SetAnchorTop(PrevAnchorAsMargin.Top);
		}

		LGUIUtils::NotifyPropertyChanged(UIItem.Get(), GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData));
	}
	TargetScriptArray[0]->EditorForceUpdate();
	ForceRefreshEditor(DetailBuilder);
	GEditor->EndTransaction();
}

bool FUIItemCustomization::IsAnchorValueEnable(TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return false;

	auto AnchorMinHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIAnchorData, AnchorMin));
	auto AnchorMaxHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIAnchorData, AnchorMax));
	auto AnchoredPositionHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIAnchorData, AnchoredPosition));
	auto SizeDeltaHandle = AnchorHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIAnchorData, SizeDelta));

	FVector2D AnchorMinValue;
	auto AnchorMinValueAccessResult = AnchorMinHandle->GetValue(AnchorMinValue);
	FVector2D AnchorMaxValue;
	auto AnchorMaxValueAccessResult = AnchorMaxHandle->GetValue(AnchorMaxValue);
	FVector2D AnchoredPosition;
	auto AnchoredPositionAccessResult = AnchoredPositionHandle->GetValue(AnchoredPosition);
	FVector2D SizeDelta;
	auto SizeDeltaAccessResult = SizeDeltaHandle->GetValue(SizeDelta);

	switch (AnchorValueIndex)
	{
	default:
	case 0://anchored position x, stretch left
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success
			&& AnchoredPositionAccessResult == FPropertyAccess::Result::Success
			)
		{
			if (AnchorMinValue.X == AnchorMaxValue.X)
			{
				return !GetLayoutControlHorizontalAnchoredPosition();
			}
			else
			{
				return !GetLayoutControlHorizontalAnchoredPosition() && !GetLayoutControlHorizontalSizeDelta();
			}
		}
		else
		{
			return true;
		}
	}
	break;
	case 1://anchored position y, stretch top
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success
			&& AnchoredPositionAccessResult == FPropertyAccess::Result::Success
			)
		{
			if (AnchorMinValue.Y == AnchorMaxValue.Y)
			{
				return !GetLayoutControlVerticalAnchoredPosition();
			}
			else
			{
				return !GetLayoutControlVerticalAnchoredPosition() && !GetLayoutControlVerticalSizeDelta();
			}
		}
		else
		{
			return true;
		}
	}
	break;
	case 2://width, stretch right
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success
			&& SizeDeltaAccessResult == FPropertyAccess::Result::Success
			)
		{
			if (AnchorMinValue.X == AnchorMaxValue.X)
			{
				return !GetLayoutControlHorizontalSizeDelta();
			}
			else
			{
				return !GetLayoutControlHorizontalAnchoredPosition() && !GetLayoutControlHorizontalSizeDelta();
			}
		}
		else
		{
			return true;
		}
	}
	break;
	case 3://height, stretch bottom
	{
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success
			&& SizeDeltaAccessResult == FPropertyAccess::Result::Success
			)
		{
			if (AnchorMinValue.Y == AnchorMaxValue.Y)
			{
				return !GetLayoutControlVerticalSizeDelta();
			}
			else
			{
				return !GetLayoutControlVerticalAnchoredPosition() && !GetLayoutControlVerticalSizeDelta();
			}
		}
		else
		{
			return true;
		}
	}
	break;
	}
}

FReply FUIItemCustomization::OnClickFixDisplayNameButton(bool singleOrAll, TSharedRef<IPropertyHandle> DisplayNameHandle)
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return FReply::Handled();

	TArray<TWeakObjectPtr<UUIItem>> UIItems;
	if (singleOrAll)
	{
		UIItems = TargetScriptArray;
	}
	else
	{
		TArray<AActor*> SelectedActors;
		for (auto& UIItem : TargetScriptArray)
		{
			if (!SelectedActors.Contains(UIItem->GetOwner()))
			{
				SelectedActors.Add(UIItem->GetOwner());
			}
		}
		auto SelectedRootActors = LGUIEditorTools::GetRootActorListFromSelection(SelectedActors);
		for (auto& RootActor : SelectedRootActors)
		{
			TArray<AActor*> ChildrenActors;
			LGUIUtils::CollectChildrenActors(RootActor, ChildrenActors, true);
			for (auto& Actor : ChildrenActors)
			{
				if (auto UIItem = Cast<UUIItem>(Actor->GetRootComponent()))
				{
					UIItems.Add(UIItem);
				}
			}
		}
	}

	GEditor->BeginTransaction(LOCTEXT("FixDisplayName_Transaction", "Fix DisplayName"));
	for (auto& UIItem : UIItems)
	{
		UIItem->Modify();
	}

	for (auto& UIItem : TargetScriptArray)
	{
		FString DisplayName;
		if (auto actor = UIItem->GetOwner())
		{
			if (UIItem == actor->GetRootComponent())
			{
				auto actorLabel = UIItem->GetOwner()->GetActorLabel();
				if (actorLabel.StartsWith("//"))
				{
					actorLabel = actorLabel.Right(actorLabel.Len() - 2);
				}
				DisplayName = actorLabel;
			}
			else
			{
				DisplayName = UIItem->GetName();
			}
		}
		else
		{
			auto name = UIItem->GetName();
			auto genVarSuffix = FString(TEXT("_GEN_VARIABLE"));
			if (name.EndsWith(genVarSuffix))
			{
				name.RemoveAt(name.Len() - genVarSuffix.Len(), genVarSuffix.Len());
			}
			DisplayName = name;
		}
		DisplayNameHandle->SetValue(DisplayName);

		LGUIUtils::NotifyPropertyChanged(UIItem.Get(), GET_MEMBER_NAME_CHECKED(UUIItem, displayName));
	}
	GEditor->EndTransaction();

	return FReply::Handled();
}

void FUIItemCustomization::ForceRefreshEditor(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
bool FUIItemCustomization::GetIsAnchorsEnabled()const
{
	return !GetLayoutControlHorizontalAnchor() || !GetLayoutControlVerticalAnchor();
}
FText FUIItemCustomization::GetAnchorsTooltipText()const
{
	return GetIsAnchorsEnabled() ? LOCTEXT("ChangeAnchor_Tooltip", "Change anchor") : LOCTEXT("AnchorIsControlledByLayout", "Anchor is controlled by layout");
}
bool FUIItemCustomization::OnCanCopyAnchor()const
{
	return TargetScriptArray.Num() == 1;
}
#define BEGIN_LGUI_AnchorData_CLIPBOARD TEXT("Begin LGUI AnchorData")
bool FUIItemCustomization::OnCanPasteAnchor()const
{
	FString PastedText;
	FPlatformApplicationMisc::ClipboardPaste(PastedText);
	return PastedText.StartsWith(BEGIN_LGUI_AnchorData_CLIPBOARD);
}
void FUIItemCustomization::OnCopyAnchor()
{
	if (TargetScriptArray.Num() == 1)
	{
		auto script = TargetScriptArray[0];
		if (script.IsValid())
		{
			auto AnchorData = script->GetAnchorData();
			auto CopiedText = FString::Printf(TEXT("%s, PivotX=%f, PivotY=%f\
, AnchorMinX=%f, AnchorMinY=%f, AnchorMaxX=%f, AnchorMaxY=%f\
, AnchoredPositionX=%f, AnchoredPositionY=%f\
, SizeDeltaX=%f, SizeDeltaY=%f")
, BEGIN_LGUI_AnchorData_CLIPBOARD
, AnchorData.Pivot.X
, AnchorData.Pivot.Y
, AnchorData.AnchorMin.X
, AnchorData.AnchorMin.Y
, AnchorData.AnchorMax.X
, AnchorData.AnchorMax.Y
, AnchorData.AnchoredPosition.X
, AnchorData.AnchoredPosition.Y
, AnchorData.SizeDelta.X
, AnchorData.SizeDelta.Y
);
			FPlatformApplicationMisc::ClipboardCopy(*CopiedText);
		}
	}
}
void FUIItemCustomization::OnPasteAnchor(IDetailLayoutBuilder* DetailBuilder)
{
	FString PastedText;
	FPlatformApplicationMisc::ClipboardPaste(PastedText);
	if (PastedText.StartsWith(BEGIN_LGUI_AnchorData_CLIPBOARD))
	{
		FUIAnchorData AnchorData;
		FParse::Value(*PastedText, TEXT("PivotX="), AnchorData.Pivot.X);
		FParse::Value(*PastedText, TEXT("PivotY="), AnchorData.Pivot.Y);
		FParse::Value(*PastedText, TEXT("AnchorMinX="), AnchorData.AnchorMin.X);
		FParse::Value(*PastedText, TEXT("AnchorMinY="), AnchorData.AnchorMin.Y);
		FParse::Value(*PastedText, TEXT("AnchorMaxX="), AnchorData.AnchorMax.X);
		FParse::Value(*PastedText, TEXT("AnchorMaxY="), AnchorData.AnchorMax.Y);
		FParse::Value(*PastedText, TEXT("AnchoredPositionX="), AnchorData.AnchoredPosition.X);
		FParse::Value(*PastedText, TEXT("AnchoredPositionY="), AnchorData.AnchoredPosition.Y);
		FParse::Value(*PastedText, TEXT("SizeDeltaX="), AnchorData.SizeDelta.X);
		FParse::Value(*PastedText, TEXT("SizeDeltaY="), AnchorData.SizeDelta.Y);
		for (auto item : TargetScriptArray)
		{
			if (item.IsValid())
			{
				auto itemWidget = item->GetAnchorData();
				item->SetAnchorData(AnchorData);
				LGUIUtils::NotifyPropertyChanged(item.Get(), GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData));
				item->MarkPackageDirty();
			}
		}
		ForceUpdateUI();
		ForceRefreshEditor(DetailBuilder);
	}
}
void FUIItemCustomization::OnCopyHierarchyIndex()
{
	if (TargetScriptArray.Num() > 0)
	{
		if (TargetScriptArray[0].IsValid())
		{
			FPlatformApplicationMisc::ClipboardCopy(*FString::Printf(TEXT("%d"), TargetScriptArray[0]->GetHierarchyIndex()));
		}
	}
}
void FUIItemCustomization::OnPasteHierarchyIndex(TSharedRef<IPropertyHandle> PropertyHandle)
{
	FString PastedText;
	FPlatformApplicationMisc::ClipboardPaste(PastedText);
	if (PastedText.IsNumeric())
	{
		int value = FCString::Atoi(*PastedText);
		PropertyHandle->SetValue(value);
	}
}

FLGUICanLayoutControlAnchor FUIItemCustomization::GetLayoutControlAnchorValue()const
{
	FLGUICanLayoutControlAnchor Result;
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return Result;

	auto Actor = TargetScriptArray[0]->GetOwner();
	auto World = TargetScriptArray[0]->GetWorld();
	if (Actor && World)
	{
		if (auto Manager = ULGUIManagerWorldSubsystem::GetInstance(World))
		{
			auto& AllLayoutArray = Manager->GetAllLayoutArray();

			if (AllLayoutArray.Num() > 0)
			{
				for (auto& Item : AllLayoutArray)
				{
					FLGUICanLayoutControlAnchor ItemResult;
					if (ILGUILayoutInterface::Execute_GetCanLayoutControlAnchor(Item.Get(), TargetScriptArray[0].Get(), ItemResult))
					{
						Result.Or(ItemResult);
					}
				}
			}
		}
	}
	return Result;
}

bool FUIItemCustomization::IsAnchorControlledByMultipleLayout(TMap<EAnchorControlledByLayoutType, TArray<UObject*>>& Result)const
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return false;

	auto Actor = TargetScriptArray[0]->GetOwner();
	auto World = TargetScriptArray[0]->GetWorld();
	if (Actor && World)
	{
		if (auto Manager = ULGUIManagerWorldSubsystem::GetInstance(World))
		{
			auto AllLayoutArray = Manager->GetAllLayoutArray();
			if (AllLayoutArray.Num() > 0)
			{
				for (auto& Item : AllLayoutArray)
				{
					FLGUICanLayoutControlAnchor ItemLayoutControl;
					if (ILGUILayoutInterface::Execute_GetCanLayoutControlAnchor(Item.Get(), TargetScriptArray[0].Get(), ItemLayoutControl))
					{
						if (ItemLayoutControl.bCanControlHorizontalAnchor)
						{
							Result.FindOrAdd(EAnchorControlledByLayoutType::HorizontalAnchor).Add(Item.Get());
						}
						if (ItemLayoutControl.bCanControlHorizontalAnchoredPosition)
						{
							Result.FindOrAdd(EAnchorControlledByLayoutType::HorizontalAnchoredPosition).Add(Item.Get());
						}
						if (ItemLayoutControl.bCanControlHorizontalSizeDelta)
						{
							Result.FindOrAdd(EAnchorControlledByLayoutType::HorizontalSizeDelta).Add(Item.Get());
						}
						if (ItemLayoutControl.bCanControlVerticalAnchor)
						{
							Result.FindOrAdd(EAnchorControlledByLayoutType::VerticalAnchor).Add(Item.Get());
						}
						if (ItemLayoutControl.bCanControlVerticalAnchoredPosition)
						{
							Result.FindOrAdd(EAnchorControlledByLayoutType::VerticalAnchoredPosition).Add(Item.Get());
						}
						if (ItemLayoutControl.bCanControlVerticalSizeDelta)
						{
							Result.FindOrAdd(EAnchorControlledByLayoutType::VerticalSizeDelta).Add(Item.Get());
						}
					}
				}
			}
		}
	}
	for (auto& KeyValue : Result)
	{
		if (KeyValue.Value.Num() > 1)return true;
	}
	return false;
}

bool FUIItemCustomization::GetLayoutControlHorizontalAnchor()const
{
	return GetLayoutControlAnchorValue().bCanControlHorizontalAnchor;
}
bool FUIItemCustomization::GetLayoutControlVerticalAnchor()const
{
	return GetLayoutControlAnchorValue().bCanControlVerticalAnchor;
}
bool FUIItemCustomization::GetLayoutControlHorizontalAnchoredPosition()const
{
	return GetLayoutControlAnchorValue().bCanControlHorizontalAnchoredPosition;
}
bool FUIItemCustomization::GetLayoutControlVerticalAnchoredPosition()const
{
	return GetLayoutControlAnchorValue().bCanControlVerticalAnchoredPosition;
}
bool FUIItemCustomization::GetLayoutControlHorizontalSizeDelta()const
{
	return GetLayoutControlAnchorValue().bCanControlHorizontalSizeDelta;
}
bool FUIItemCustomization::GetLayoutControlVerticalSizeDelta()const
{
	return GetLayoutControlAnchorValue().bCanControlVerticalSizeDelta;
}

UE_ENABLE_OPTIMIZATION
#undef LOCTEXT_NAMESPACE