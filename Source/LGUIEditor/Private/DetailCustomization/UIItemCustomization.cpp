// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DetailCustomization/UIItemCustomization.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Widgets/Layout/Anchors.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "IDetailGroup.h"
#include "LGUIEditorStyle.h"
#include "Editor.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Widget/ComponentTransformDetails.h"
#include "Widget/AnchorPreviewWidget.h"
#include "PropertyCustomizationHelpers.h"
#include "HAL/PlatformApplicationMisc.h"
#include "EditorViewportClient.h"
#include "Core/ActorComponent/UIItem.h"
#include "LGUIEditorUtils.h"
#include "LGUIEditorTools.h"
#include "Layout/UILayoutBase.h"
#include "Layout/UILayoutElement.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "UIItemComponentDetails"


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
			item->EditorForceUpdateImmediately();
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
					validItem->EditorForceUpdateImmediately();
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

	if (auto world = TargetScriptArray[0]->GetWorld())
	{
		if (world->WorldType == EWorldType::Editor)
		{
			//something weird may happen if not realtime edit
			if (GEditor)
			{
				if (auto viewport = GEditor->GetActiveViewport())
				{
					if (auto viewportClient = (FEditorViewportClient*)viewport->GetClient())
					{
						if (!viewportClient->IsRealtime())
						{
							viewportClient->SetRealtime(true);
							UE_LOG(LGUIEditor, Warning, TEXT("[UIItemCustomization]Editor viewport set to realtime.(Something weird may happen if not realtime edit)"));
						}
					}
				}
			}
		}
	}

	IDetailCategoryBuilder& lguiCategory = DetailBuilder.EditCategory("LGUI");
	DetailBuilder.HideCategory("TransformCommon");
	IDetailCategoryBuilder& TransformCategory = DetailBuilder.EditCategory("LGUITransform", LOCTEXT("LGUI-Transform", "LGUI-Transform"), ECategoryPriority::Transform);

	//base
	{
		auto uiActiveHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, bIsUIActive));
		uiActiveHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] {
			ForceUpdateUI();
		}));
	}

	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData));

	lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, bIsUIActive));

	//anchor, width, height
	{
		bool anchorControlledByParentLayout = false;
		bool anchorOffsetXControlledByParentLayout = false;
		bool anchorOffsetYControlledByParentLayout = false;
		bool widthControlledByParentLayout = false;
		bool heightControlledByParentLayout = false;
		bool anchorHControlledBySelfLayout = false;
		bool anchorVControlledBySelfLayout = false;
		bool anchorOffsetXControlledBySelfLayout = false;
		bool anchorOffsetYControlledBySelfLayout = false;
		bool widthControlledBySelfLayout = false;
		bool heightControlledBySelfLayout = false;
		bool stretchLeftControlledBySelfLayout = false;
		bool stretchRightControlledBySelfLayout = false;
		bool stretchTopControlledBySelfLayout = false;
		bool stretchBottomControlledBySelfLayout = false;
		if (auto thisActor = TargetScriptArray[0]->GetOwner())
		{
			if (thisActor->GetRootComponent() == TargetScriptArray[0].Get())
			{
				if (auto parentActor = thisActor->GetAttachParentActor())
				{
					bool ignoreParentLayout = false;
					if (auto thisLayoutElement = thisActor->FindComponentByClass<UUILayoutElement>())
					{
						ignoreParentLayout = thisLayoutElement->GetIgnoreLayout();
					}
					if (!ignoreParentLayout)
					{
						if (auto parentLayout = parentActor->FindComponentByClass<UUILayoutBase>())
						{
							anchorControlledByParentLayout = parentLayout->CanControlChildAnchor();
							widthControlledByParentLayout = parentLayout->CanControlChildWidth();
							heightControlledByParentLayout = parentLayout->CanControlChildHeight();
							anchorOffsetXControlledByParentLayout = parentLayout->CanControlSelfAnchorOffsetX();
							anchorOffsetYControlledByParentLayout = parentLayout->CanControlSelfAnchorOffsetY();
						}
					}
				}
				if (auto thisLayout = thisActor->FindComponentByClass<UUILayoutBase>())
				{
					anchorHControlledBySelfLayout = thisLayout->CanControlSelfHorizontalAnchor();
					anchorVControlledBySelfLayout = thisLayout->CanControlSelfVerticalAnchor();
					anchorOffsetXControlledBySelfLayout = thisLayout->CanControlSelfAnchorOffsetX();
					anchorOffsetYControlledBySelfLayout = thisLayout->CanControlSelfAnchorOffsetY();
					widthControlledBySelfLayout = thisLayout->CanControlSelfWidth();
					heightControlledBySelfLayout = thisLayout->CanControlSelfHeight();
					stretchLeftControlledBySelfLayout = thisLayout->CanControlSelfStrengthLeft();
					stretchRightControlledBySelfLayout = thisLayout->CanControlSelfStrengthRight();
					stretchTopControlledBySelfLayout = thisLayout->CanControlSelfStrengthTop();
					stretchBottomControlledBySelfLayout = thisLayout->CanControlSelfStrengthBottom();
				}
			}
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
		FMargin AnchorLabelMargin = FMargin(2, 2);
		FMargin AnchorValueMargin = FMargin(2, 2);

		auto MakeAnchorLabelWidget = [&](int AnchorLabelIndex) {
			return
				SNew(SBox)
				.Padding(AnchorLabelMargin)
				[
					SNew(STextBlock)
					.Text(this, &FUIItemCustomization::GetAnchorLabelText, AnchorMinHandle, AnchorMaxHandle, AnchorLabelIndex)
					.ToolTipText(this, &FUIItemCustomization::GetAnchorLabelTooltipText, AnchorMinHandle, AnchorMaxHandle, AnchorLabelIndex)
					.ColorAndOpacity(FLinearColor(FColor(255, 255, 255, 255)))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			;
		};
		auto MakeAnchorValueWidget = [=](int AnchorValueIndex) {
			return
				SNew(SBox)
				.Padding(AnchorValueMargin)
				[
					SNew(SNumericEntryBox<float>)
					.AllowSpin(true)
					.MinSliderValue(-1000)
					.MaxSliderValue(1000)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.UndeterminedString( NSLOCTEXT( "PropertyEditor", "MultipleValues", "Multiple Values") )
					.Value(this, &FUIItemCustomization::GetAnchorValue, AnchorHandle, AnchorValueIndex)
					.OnValueChanged(this, &FUIItemCustomization::OnAnchorValueChanged, AnchorHandle, AnchorValueIndex)
					.OnValueCommitted(this, &FUIItemCustomization::OnAnchorValueCommitted, AnchorHandle, AnchorValueIndex)
					.OnBeginSliderMovement(this, &FUIItemCustomization::OnAnchorSliderSliderMovementBegin)
				]
			;
		};
		auto DetailBuilderPoiter = &DetailBuilder;
		auto MakeAnchorPreviewWidget = [=](LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign HAlign, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign VAlign) {
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
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
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
		.NameContent()
		[
			SNew(SVerticalBox)
			//+SVerticalBox::Slot()
			//[
			//	SNew(SBox)
			//	.Padding(FMargin(0, 0))
			//	[
			//		SNew(STextBlock)
			//		.Font(IDetailLayoutBuilder::GetDetailFont())
			//		.Text(LOCTEXT("AnchorPreset", "AnchorPreset"))
			//		.Font(IDetailLayoutBuilder::GetDetailFont())
			//	]
			//]
			+SVerticalBox::Slot()
			[
				SNew(SBox)
				.Padding(2)
				.Visibility(this, &FUIItemCustomization::GetAnchorPresetButtonVisibility)
				[
					SNew(SComboButton)
					.HasDownArrow(false)
					.IsEnabled(this, &FUIItemCustomization::GetIsAnchorsEnabled)
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
									.ColorAndOpacity(FLinearColor(FColor(255, 255, 255, 255)))
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
								.ColorAndOpacity(FLinearColor(FColor(255, 255, 255, 255)))
								.Font(IDetailLayoutBuilder::GetDetailFont())
								.Justification(ETextJustify::Center)
								.RenderTransformPivot(FVector2D(0, 0.5f))
								.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(90)), FVector2D(-12, -10)))
							]
						]
					]
					.MenuContent()
					[
						SNew(SBox)
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
										//None
										//+SUniformGridPanel::Slot(0, 0)
										//[
										//	SNew(SAnchorPreviewWidget, anchorItemSize)
										//	.BasePadding(itemBasePadding)
										//	.SelectedHAlign(this, &FUIItemCustomization::GetAnchorHAlign, AnchorMinHandle, AnchorMaxHandle)
										//	.SelectedVAlign(this, &FUIItemCustomization::GetAnchorVAlign, AnchorMinHandle, AnchorMaxHandle)
										//	.PersistentHAlign(UIAnchorHorizontalAlign::None)
										//	.PersistentVAlign(UIAnchorVerticalAlign::None)
										//	.ButtonEnable(false)
										//]
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
													.ColorAndOpacity(FLinearColor(FColor(0, 255, 255, 255)))
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
													.ColorAndOpacity(FLinearColor(FColor(0, 255, 255, 255)))
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
													.ColorAndOpacity(FLinearColor(FColor(0, 255, 255, 255)))
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
													.ColorAndOpacity(FLinearColor(FColor(0, 255, 255, 255)))
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

		IDetailGroup& anchorGroup = TransformCategory.AddGroup(FName("Anchors"), LOCTEXT("AnchorsGroup", "Anchors"));

		IDetailPropertyRow& anchorHAlignDetailProperty = anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.AnchorMin)));
		if (anchorControlledByParentLayout)
		{
			LGUIEditorUtils::SetControlledByParentLayout(anchorHAlignDetailProperty, anchorControlledByParentLayout);
		}
		else
		{
			if (anchorHControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(anchorHAlignDetailProperty, anchorHControlledBySelfLayout);
		}

		IDetailPropertyRow& anchorVAlignDetailProperty = anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.AnchorMax)));
		if (anchorControlledByParentLayout)
		{
			LGUIEditorUtils::SetControlledByParentLayout(anchorVAlignDetailProperty, anchorControlledByParentLayout);
		}
		else
		{
			if (anchorVControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(anchorVAlignDetailProperty, anchorVControlledBySelfLayout);
		}

		auto& AnchorRawDataGroup = TransformCategory.AddGroup(FName("AnchorsRawData"), LOCTEXT("AnchorsRawData", "AnchorsRawData"), true);
		AnchorRawDataGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.AnchoredPosition)));
		AnchorRawDataGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.SizeDelta)));
		
		//stretch, anchorx, anchory
		/*if (anchorHAlign == UIAnchorHorizontalAlign::None)
		{
			TransformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.width));

			anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.anchorOffsetX))).IsEnabled(false);
			anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.stretchLeft))).IsEnabled(false);
			anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.stretchRight))).IsEnabled(false);
		}
		else if (anchorHAlign == UIAnchorHorizontalAlign::Stretch)
		{
			IDetailPropertyRow& stretchLeftProperty = TransformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.stretchLeft));
			if (stretchLeftControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(stretchLeftProperty, true);
			IDetailPropertyRow& stretchRightProperty = TransformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.stretchRight));
			if (stretchRightControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(stretchRightProperty, true);

			anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.anchorOffsetX))).IsEnabled(false);
			anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.width))).IsEnabled(false);
		}
		else
		{
			IDetailPropertyRow& anchorOffsetXDetailProperty = TransformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.anchorOffsetX));
			LGUIEditorUtils::SetControlledByParentLayout(anchorOffsetXDetailProperty, anchorControlledByParentLayout);
			IDetailPropertyRow& widthDetailProperty = TransformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.width));
			if (widthControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(widthDetailProperty, widthControlledBySelfLayout);
			else
				if (widthControlledByParentLayout)
					LGUIEditorUtils::SetControlledByParentLayout(widthDetailProperty, widthControlledByParentLayout);

			anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.stretchLeft))).IsEnabled(false);
			anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.stretchRight))).IsEnabled(false);
		}

		if (anchorVAlign == UIAnchorVerticalAlign::None)
		{
			TransformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.height));

			anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.anchorOffsetY))).IsEnabled(false);
			anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.stretchTop))).IsEnabled(false);
			anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.stretchBottom))).IsEnabled(false);
		}
		else if (anchorVAlign == UIAnchorVerticalAlign::Stretch)
		{
			IDetailPropertyRow& stretchTopProperty = TransformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.stretchTop));
			if (stretchTopControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(stretchTopProperty, true);
			IDetailPropertyRow& stretchBottomProperty = TransformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.stretchBottom));
			if (stretchBottomControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(stretchBottomProperty, true);

			anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.anchorOffsetY))).IsEnabled(false);
			anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.height))).IsEnabled(false);
		}
		else
		{
			IDetailPropertyRow& anchorOffsetYDetailProperty = TransformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.anchorOffsetY));
			LGUIEditorUtils::SetControlledByParentLayout(anchorOffsetYDetailProperty, anchorControlledByParentLayout);
			IDetailPropertyRow& heightDetailProperty = TransformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.height));
			if (heightControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(heightDetailProperty, heightControlledBySelfLayout);
			else
				if (heightControlledByParentLayout)
					LGUIEditorUtils::SetControlledByParentLayout(heightDetailProperty, heightControlledByParentLayout);

			anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.stretchTop))).IsEnabled(false);
			anchorGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.stretchBottom))).IsEnabled(false);
		}

		if (anchorControlledByParentLayout && (anchorHControlledBySelfLayout || anchorVControlledBySelfLayout))
		{
			LGUIEditorUtils::ShowWarning(&DetailBuilder, FString("Anchor is controlled by more than one UILayout component! This may cause issue!"));
		}
		if (anchorOffsetXControlledByParentLayout && anchorOffsetXControlledBySelfLayout)
		{
			LGUIEditorUtils::ShowWarning(&DetailBuilder, FString("AnchorOffsetX is controlled by more than one UILayout component! This may cause issue!"));
		}
		if (anchorOffsetYControlledByParentLayout && anchorOffsetYControlledBySelfLayout)
		{
			LGUIEditorUtils::ShowWarning(&DetailBuilder, FString("AnchorOffsetY is controlled by more than one UILayout component! This may cause issue!"));
		}
		if (widthControlledByParentLayout && widthControlledBySelfLayout)
		{
			LGUIEditorUtils::ShowWarning(&DetailBuilder, FString("Width is controlled by more than one UILayout component! This may cause issue!"));
		}
		if (heightControlledByParentLayout && heightControlledBySelfLayout)
		{
			LGUIEditorUtils::ShowWarning(&DetailBuilder, FString("Height is controlled by more than one UILayout component! This may cause issue!"));
		}*/
	}
	//pivot
	auto PivotHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData.Pivot));
	TransformCategory.AddProperty(PivotHandle);

	//location rotation scale
	const FSelectedActorInfo& selectedActorInfo = DetailBuilder.GetDetailsView()->GetSelectedActorInfo();
	TSharedRef<FComponentTransformDetails> transformDetails = MakeShareable(new FComponentTransformDetails(TargetScriptArray, selectedActorInfo, DetailBuilder));
	TransformCategory.AddCustomBuilder(transformDetails);

	//HierarchyIndex
	{
		auto HierarchyIndexHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, hierarchyIndex));
		DetailBuilder.HideProperty(HierarchyIndexHandle);
		HierarchyIndexHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] {
			ForceUpdateUI();
			LGUIEditorTools::RefreshSceneOutliner();
		}));
		auto hierarchyIndexWidget =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.FillWidth(5)
			[
				HierarchyIndexHandle->CreatePropertyValueWidget()
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.FillWidth(2)
			[
				SNew(SBox)
				.HeightOverride(18)
				[
					SNew(SButton)
					.Text(LOCTEXT("Increase", "+"))
					.HAlign(EHorizontalAlignment::HAlign_Center)
					.OnClicked(this, &FUIItemCustomization::OnClickIncreaseOrDecreaseHierarchyIndex, true)
				]
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.FillWidth(2)
			[
				SNew(SBox)
				.HeightOverride(18)
				[
					SNew(SButton)
					.Text(LOCTEXT("Decrease", "-"))
					.HAlign(EHorizontalAlignment::HAlign_Center)
					.OnClicked(this, &FUIItemCustomization::OnClickIncreaseOrDecreaseHierarchyIndex, false)
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
	lguiCategory.AddCustomRow(LOCTEXT("DisplayName", "Display Name"), true)
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
			.MaxWidth(80)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Fix it")))
				.OnClicked(this, &FUIItemCustomization::OnClickFixDisplayNameButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.Visibility(this, &FUIItemCustomization::GetDisplayNameWarningVisibility)
				.ToolTipText(FText::FromString(FString(TEXT("DisplayName not equal to ActorLabel."))))
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

FReply FUIItemCustomization::OnClickIncreaseOrDecreaseHierarchyIndex(bool IncreaseOrDecrease)
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return FReply::Handled();

	GEditor->BeginTransaction(LOCTEXT("ChangeAnchorValue", "Change LGUI Hierarchy Index"));
	for (auto& Item : TargetScriptArray)
	{
		Item->Modify();
	}
	GEditor->EndTransaction();

	for (auto& Item : TargetScriptArray)
	{
		Item->SetHierarchyIndex(Item->hierarchyIndex + (IncreaseOrDecrease ? 1 : -1));
	}

	LGUIEditorTools::RefreshSceneOutliner();
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
			return LOCTEXT("AnchorLeft", "Center");
		}
		else if (AnchorMinValue.X == 1.0f)
		{
			return LOCTEXT("AnchorLeft", "Right");
		}
		else
		{
			return LOCTEXT("AnchorCustom", "Custom");
		}
	}
	else if (AnchorMinValue.X == 0.0f && AnchorMaxValue.X == 1.0f)
	{
		return LOCTEXT("AnchorCustom", "Stretch");
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
			return LOCTEXT("AnchorCustom", "Bottom");
		}
		else if (AnchorMinValue.Y == 0.5f)
		{
			return LOCTEXT("AnchorCustom", "Middle");
		}
		else if (AnchorMinValue.Y == 1.0f)
		{
			return LOCTEXT("AnchorCustom", "Top");
		}
		else
		{
			return LOCTEXT("AnchorCustom", "Custom");
		}
	}
	else if (AnchorMinValue.Y == 0.0f && AnchorMaxValue.Y == 1.0f)
	{
		return LOCTEXT("AnchorCustom", "Stretch");
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
			return LOCTEXT("AnchoredPositionX", "Width");
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
			return LOCTEXT("AnchoredPositionX", "Height");
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
			return FText::Format(LOCTEXT("AnchoredPositionX_Tooltip", "Horizontal size. Related function: {0} / {1}."), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, GetWidth)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, SetWidth)));
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
			return FText::Format(LOCTEXT("AnchoredPositionX_Tooltip", "Vertical size. Related function: {0} / {1}"), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, GetHeight)), FText::FromString(GET_FUNCTION_NAME_STRING_CHECKED(UUIItem, SetHeight)));
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
		if (AnchorMinValueAccessResult == FPropertyAccess::Result::Success && AnchorMaxValueAccessResult == FPropertyAccess::Result::Success
			&& AnchoredPositionAccessResult == FPropertyAccess::Result::Success
			)
		{
			if (AnchorMinValue.X == AnchorMaxValue.X)
			{
				return AnchoredPosition.X;
			}
			else
			{
				return TargetScriptArray[0]->GetAnchorLeft();
			}
		}
		else
		{
			return TOptional<float>();
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
				return AnchoredPosition.Y;
			}
			else
			{
				return TargetScriptArray[0]->GetAnchorTop();
			}
		}
		else
		{
			return TOptional<float>();
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
				return SizeDelta.X;
			}
			else
			{
				return TargetScriptArray[0]->GetAnchorRight();
			}
		}
		else
		{
			return TOptional<float>();
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
				return SizeDelta.Y;
			}
			else
			{
				return TargetScriptArray[0]->GetAnchorBottom();
			}
		}
		else
		{
			return TOptional<float>();
		}
	}
	break;
	}
}
void FUIItemCustomization::OnAnchorValueChanged(float Value, TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex)
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
	FVector2D AnchoredPosition;
	AnchoredPositionHandle->GetValue(AnchoredPosition);
	FVector2D SizeDelta;
	SizeDeltaHandle->GetValue(SizeDelta);
	
	switch (AnchorValueIndex)
	{
	case 0://anchored position x, stretch left
	{
		if (AnchorMinValue.X == AnchorMaxValue.X)
		{
			AnchoredPosition.X = Value;
			for (auto& Item : TargetScriptArray)
			{
				Item->SetAnchoredPosition(AnchoredPosition);
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
			AnchoredPosition.Y = Value;
			for (auto& Item : TargetScriptArray)
			{
				Item->SetAnchoredPosition(AnchoredPosition);
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
}
void FUIItemCustomization::OnAnchorValueCommitted(float Value, ETextCommit::Type commitType, TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex)
{
	GEditor->BeginTransaction(LOCTEXT("ChangeAnchorValue", "Change LGUI Anchor Value"));
	for (auto& Item : TargetScriptArray)
	{
		Item->Modify();
	}
	GEditor->EndTransaction();
	OnAnchorValueChanged(Value, AnchorHandle, AnchorValueIndex);
}

void FUIItemCustomization::OnAnchorSliderSliderMovementBegin()
{
	GEditor->BeginTransaction(LOCTEXT("ChangeAnchorValue", "Change LGUI Anchor Value"));
	for (auto& Item : TargetScriptArray)
	{
		Item->Modify();
	}
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

	bool snapAnchor = FSlateApplication::Get().GetModifierKeys().IsAltDown();

	GEditor->BeginTransaction(LOCTEXT("ChangeAnchor", "Change LGUI Anchor"));
	for (auto& UIItem : TargetScriptArray)
	{
		UIItem->Modify();
	}
	GEditor->EndTransaction();

	for (auto& UIItem : TargetScriptArray)
	{
		auto AnchorMin = UIItem->GetAnchorMin();
		auto AnchorMax = UIItem->GetAnchorMax();
		switch (HorizontalAlign)
		{
		case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::None:
			break;
		case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Left:
			AnchorMin.X = AnchorMax.X = 0;
			break;
		case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Center:
			AnchorMin.X = AnchorMax.X = 0.5f;
			break;
		case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Right:
			AnchorMin.X = AnchorMax.X = 1.0f;
			break;
		case LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch:
		{
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
			AnchorMin.Y = AnchorMax.Y = 1;
			break;
		case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Middle:
			AnchorMin.Y = AnchorMax.Y = 0.5f;
			break;
		case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Bottom:
			AnchorMin.Y = AnchorMax.Y = 0.0f;
			break;
		case LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch:
		{
			AnchorMin.Y = 0;
			AnchorMax.Y = 1.0f;
		}
		break;
		}
		auto PrevWidth = UIItem->GetWidth();
		auto PrevHeight = UIItem->GetHeight();
		UIItem->SetAnchorMin(AnchorMin);
		UIItem->SetAnchorMax(AnchorMax);
		UIItem->SetWidth(PrevWidth);
		UIItem->SetHeight(PrevHeight);
		if (snapAnchor)
		{
			if (HorizontalAlign == LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign::Stretch && VerticalAlign == LGUIAnchorPreviewWidget::UIAnchorVerticalAlign::Stretch)
			{
				UIItem->SetAnchoredPosition(FVector2D::ZeroVector);
				UIItem->SetSizeDelta(FVector2D::ZeroVector);
			}
		}
	}
	TargetScriptArray[0]->EditorForceUpdateImmediately();
	ForceRefreshEditor(DetailBuilder);
}

FReply FUIItemCustomization::OnClickFixDisplayNameButton()
{
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return FReply::Handled();

	GEditor->BeginTransaction(LOCTEXT("FixDisplayName", "Fix DisplayName"));
	for (auto& UIItem : TargetScriptArray)
	{
		UIItem->Modify();
	}
	GEditor->EndTransaction();

	for (auto& UIItem : TargetScriptArray)
	{
		if (auto actor = UIItem->GetOwner())
		{
			if (UIItem == actor->GetRootComponent())
			{
				auto actorLabel = UIItem->GetOwner()->GetActorLabel();
				if (actorLabel.StartsWith("//"))
				{
					actorLabel = actorLabel.Right(actorLabel.Len() - 2);
				}
				UIItem->SetDisplayName(actorLabel);
			}
			else
			{
				UIItem->SetDisplayName(UIItem->GetName());
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
			UIItem->SetDisplayName(name);
		}
	}

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
	if (TargetScriptArray.Num() == 0 || !TargetScriptArray[0].IsValid())return false;

	bool anchorControlledByParentLayout = false;
	bool anchorHControlledBySelfLayout = false;
	bool anchorVControlledBySelfLayout = false;
	if (auto thisActor = TargetScriptArray[0]->GetOwner())
	{
		if (thisActor->GetRootComponent() == TargetScriptArray[0].Get())
		{
			if (auto parentActor = thisActor->GetAttachParentActor())
			{
				bool ignoreParentLayout = false;
				if (auto thisLayoutElement = thisActor->FindComponentByClass<UUILayoutElement>())
				{
					ignoreParentLayout = thisLayoutElement->GetIgnoreLayout();
				}
				if (!ignoreParentLayout)
				{
					if (auto parentLayout = parentActor->FindComponentByClass<UUILayoutBase>())
					{
						anchorControlledByParentLayout = parentLayout->CanControlChildAnchor();
					}
				}
			}
			if (auto thisLayout = thisActor->FindComponentByClass<UUILayoutBase>())
			{
				anchorHControlledBySelfLayout = thisLayout->CanControlSelfHorizontalAnchor();
				anchorVControlledBySelfLayout = thisLayout->CanControlSelfVerticalAnchor();
			}
		}
	}

	if (anchorControlledByParentLayout)return false;
	if (anchorHControlledBySelfLayout)return false;
	if (anchorVControlledBySelfLayout)return false;
	return true;
}
FText FUIItemCustomization::GetAnchorsTooltipText()const
{
	return FText::FromString(FString(GetIsAnchorsEnabled() ? TEXT("Change anchor") : TEXT("Anchor is controlled by layout")));
}
bool FUIItemCustomization::OnCanCopyAnchor()const
{
	return TargetScriptArray.Num() == 1;
}
#define BEGIN_UIWIDGET_CLIPBOARD TEXT("Begin LGUI UIWidget")
bool FUIItemCustomization::OnCanPasteAnchor()const
{
	FString PastedText;
	FPlatformApplicationMisc::ClipboardPaste(PastedText);
	return PastedText.StartsWith(BEGIN_UIWIDGET_CLIPBOARD);
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
, BEGIN_UIWIDGET_CLIPBOARD
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
	if (PastedText.StartsWith(BEGIN_UIWIDGET_CLIPBOARD))
	{
		FUIAnchorData AnchorData;
		FParse::Value(*PastedText, TEXT("PivotX="), AnchorData.Pivot.X);
		FParse::Value(*PastedText, TEXT("PivotY="), AnchorData.Pivot.Y);
		FParse::Value(*PastedText, TEXT("AnchorMinX="), AnchorData.AnchorMin.X);
		FParse::Value(*PastedText, TEXT("AnchorMinY="), AnchorData.AnchorMin.Y);
		FParse::Value(*PastedText, TEXT("anchorOffsetX="), AnchorData.AnchorMax.X);
		FParse::Value(*PastedText, TEXT("anchorOffsetY="), AnchorData.AnchorMax.Y);
		FParse::Value(*PastedText, TEXT("width="), AnchorData.AnchoredPosition.X);
		FParse::Value(*PastedText, TEXT("height="), AnchorData.AnchoredPosition.Y);
		FParse::Value(*PastedText, TEXT("stretchLeft="), AnchorData.SizeDelta.X);
		FParse::Value(*PastedText, TEXT("stretchRight="), AnchorData.SizeDelta.Y);
		for (auto item : TargetScriptArray)
		{
			if (item.IsValid())
			{
				auto itemWidget = item->GetAnchorData();
				item->SetAnchorData(AnchorData);
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

#undef LOCTEXT_NAMESPACE