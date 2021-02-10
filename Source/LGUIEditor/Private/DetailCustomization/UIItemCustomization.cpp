﻿// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DetailCustomization/UIItemCustomization.h"
#include "Core/ActorComponent/UIRenderable.h"
#include "Widgets/Layout/Anchors.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "IDetailGroup.h"
#include "LGUIEditorUtils.h"
#include "LGUIEditorStyle.h"
#include "LGUIEditorModule.h"
#include "Editor.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Widget/ComponentTransformDetails.h"
#include "Widget/AnchorPreviewWidget.h"

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
		item->EditorForceUpdateImmediately();
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
			if (validItem->GetWorld() != nullptr)
			{
				TargetScriptArray.Add(validItem);
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

	if (TargetScriptArray[0]->GetWorld()->WorldType == EWorldType::Editor)
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
						viewportClient->SetRealtime(true, false);
						UE_LOG(LGUIEditor, Warning, TEXT("[UIItemCustomization]Editor viewport set to realtime.(Something weird may happen if not realtime edit)"));
					}
				}
			}
		}
	}

	IDetailCategoryBuilder& lguiCategory = DetailBuilder.EditCategory("LGUI");

	//base
	{
		lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, bIsUIActive), nullptr, NAME_None, EPropertyLocation::Advanced);
		lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, bRaycastTarget), nullptr, NAME_None, EPropertyLocation::Advanced);
		lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, traceChannel), nullptr, NAME_None, EPropertyLocation::Advanced);
		auto uiActiveHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, bIsUIActive));
		uiActiveHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] {
			ForceUpdateUI();
		}));
	}
	//HierarchyIndex
	{
		auto hierarchyIndexHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, hierarchyIndex));
		DetailBuilder.HideProperty(hierarchyIndexHandle);
		hierarchyIndexHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] {
			ForceUpdateUI();
			FLGUIEditorModule::Instance->RefreshSceneOutliner();
		}));
		auto hierarchyIndexWidget =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.FillWidth(5)
			[
				hierarchyIndexHandle->CreatePropertyValueWidget()
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
					.OnClicked_Lambda([&]()
					{
						for (auto item : TargetScriptArray)
						{
							item->SetHierarchyIndex(item->hierarchyIndex + 1);
							ForceUpdateUI();
						}
						FLGUIEditorModule::Instance->RefreshSceneOutliner();
						return FReply::Handled(); 
					})
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
					.OnClicked_Lambda([&]()
					{
						for (auto item : TargetScriptArray)
						{
							item->SetHierarchyIndex(item->hierarchyIndex - 1);
							ForceUpdateUI();
						}
						FLGUIEditorModule::Instance->RefreshSceneOutliner();
						return FReply::Handled();
					})
				]
			];

		lguiCategory.AddCustomRow(LOCTEXT("HierarchyIndexManager", "HierarchyIndexManager"), true)
		.NameContent()
		[
			hierarchyIndexHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			hierarchyIndexWidget
		];
	}
	DetailBuilder.HideCategory("TransformCommon");
	IDetailCategoryBuilder& transformCategory = DetailBuilder.EditCategory("LGUITransform", LOCTEXT("LGUI Transform", "LGUI Transform"), ECategoryPriority::Transform);

	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget));
	auto uiType = TargetScriptArray[0]->GetUIItemType();

	//depth
	{
		auto depthHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.depth));
		DetailBuilder.HideProperty(depthHandle);
		depthHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] {
			ForceUpdateUI();
		}));
		auto depthWidget =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.FillWidth(5)
			[
				depthHandle->CreatePropertyValueWidget()
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.FillWidth(2)
			[
				SNew(SBox)
				.HeightOverride(18)
				[
					SNew(SButton)
					.Text(LOCTEXT("MoveUp", "+"))
					.HAlign(EHorizontalAlignment::HAlign_Center)
					.OnClicked_Lambda([&]()
					{
						for (auto Script : TargetScriptArray)
						{
							Script->widget.depth++;
						}
						ForceUpdateUI();
						return FReply::Handled(); 
					})
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
					.Text(LOCTEXT("MoveDown", "-"))
					.HAlign(EHorizontalAlignment::HAlign_Center)
					.OnClicked_Lambda([&]()
					{
						for (auto Script : TargetScriptArray)
						{
							Script->widget.depth--;
						}
						ForceUpdateUI();
						return FReply::Handled();
					})
				]
			];

			lguiCategory.AddCustomRow(LOCTEXT("DepthManager", "DepthManager"))
			.NameContent()
			[
				depthHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				depthWidget
			];


		//depth info
		{
			lguiCategory.AddCustomRow(LOCTEXT("DepthInfo", "DepthInfo"))
				.WholeRowContent()
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(this, &FUIItemCustomization::GetDepthInfo, TargetScriptArray[0])
					.AutoWrapText(true)
					.ToolTipText(FText::FromString(FString(TEXT("The same depth count shared by UI elements in same canvas"))))
				];
		}
	}
	//color
	auto inheritAlphaHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, inheritAlpha));
	auto colorHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.color));
	lguiCategory.AddProperty(colorHandle);
	lguiCategory.AddProperty(inheritAlphaHandle);
	//anchor, width, height
	{
		auto widthHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.width));
		auto heightHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.height));
		widthHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] {
			ForceUpdateUI();
		}));
		heightHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] {
			ForceUpdateUI();
		}));

		bool anchorControlledByParentLayout = false;
		bool widthControlledByParentLayout = false;
		bool heightControlledByParentLayout = false;
		bool anchorHControlledBySelfLayout = false;
		bool anchorVControlledBySelfLayout = false;
		bool widthControlledBySelfLayout = false;
		bool heightControlledBySelfLayout = false;
		bool stretchLeftControlledBySelfLayout = false;
		bool stretchRightControlledBySelfLayout = false;
		bool stretchTopControlledBySelfLayout = false;
		bool stretchBottomControlledBySelfLayout = false;
		if (auto thisActor = TargetScriptArray[0]->GetOwner())
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
					}
				}
			}
			if (auto thisLayout = thisActor->FindComponentByClass<UUILayoutBase>())
			{
				anchorHControlledBySelfLayout = thisLayout->CanControlSelfHorizontalAnchor();
				anchorVControlledBySelfLayout = thisLayout->CanControlSelfVerticalAnchor();
				widthControlledBySelfLayout = thisLayout->CanControlSelfWidth();
				heightControlledBySelfLayout = thisLayout->CanControlSelfHeight();
				stretchLeftControlledBySelfLayout = thisLayout->CanControlSelfStrengthLeft();
				stretchRightControlledBySelfLayout = thisLayout->CanControlSelfStrengthRight();
				stretchTopControlledBySelfLayout = thisLayout->CanControlSelfStrengthTop();
				stretchBottomControlledBySelfLayout = thisLayout->CanControlSelfStrengthBottom();
			}
		}

		
		auto refreshDelegate = FSimpleDelegate::CreateSP(this, &FUIItemCustomization::ForceRefresh, &DetailBuilder);

		auto anchorHAlignHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.anchorHAlign));
		uint8 anchorHAlignUInt8;
		anchorHAlignHandle->GetValue(anchorHAlignUInt8);
		anchorHAlignHandle->SetOnPropertyValueChanged(refreshDelegate);
		UIAnchorHorizontalAlign anchorHAlign = (UIAnchorHorizontalAlign)anchorHAlignUInt8;

		auto anchorVAlignHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.anchorVAlign));
		uint8 anchorVAlignUInt8;
		anchorVAlignHandle->GetValue(anchorVAlignUInt8);
		anchorVAlignHandle->SetOnPropertyValueChanged(refreshDelegate);
		UIAnchorVerticalAlign anchorVAlign = (UIAnchorVerticalAlign)anchorVAlignUInt8;

		//anchors preset menu
		FVector2D anchorItemSize(48, 48);
		float itemBasePadding = 8;
		IDetailGroup& anchorGroup = transformCategory.AddGroup(FName("AnchorGroup"), LOCTEXT("AnchorGroup", "AnchorGroup"));
		anchorGroup.HeaderRow()
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("Anchors", "Anchors"))
			]
			.ValueContent()
			[
				SNew(SComboButton)
				.HasDownArrow(true)
				.IsEnabled(this, &FUIItemCustomization::GetIsAnchorsEnabled)
				.ToolTipText(this, &FUIItemCustomization::GetAnchorsTooltipText)
				.ButtonStyle(FLGUIEditorStyle::Get(), "AnchorButton")
				.ButtonContent()
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SAnchorPreviewWidget, TargetScriptArray, anchorHAlign, anchorVAlign, FVector2D(28, 28), refreshDelegate)
							.BasePadding(0)
						]
					]
				]
				.MenuContent()
				[
					SNew(SBorder)
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
									+SUniformGridPanel::Slot(0, 0)
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::None, UIAnchorVerticalAlign::None, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(1, 0)
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Left, UIAnchorVerticalAlign::None, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(2, 0) 
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Center, UIAnchorVerticalAlign::None, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(3, 0) 
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Right, UIAnchorVerticalAlign::None, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(4, 0) 
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Stretch, UIAnchorVerticalAlign::None, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									//Top
									+SUniformGridPanel::Slot(0, 1)
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::None, UIAnchorVerticalAlign::Top, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(1, 1)
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Left, UIAnchorVerticalAlign::Top, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(2, 1) 
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Center, UIAnchorVerticalAlign::Top, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(3, 1) 
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Right, UIAnchorVerticalAlign::Top, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(4, 1) 
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Stretch, UIAnchorVerticalAlign::Top, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									//Center
									+SUniformGridPanel::Slot(0, 2)
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::None, UIAnchorVerticalAlign::Middle, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(1, 2)
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Left, UIAnchorVerticalAlign::Middle, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(2, 2) 
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Center, UIAnchorVerticalAlign::Middle, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(3, 2) 
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Right, UIAnchorVerticalAlign::Middle, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(4, 2) 
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Stretch, UIAnchorVerticalAlign::Middle, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									//Bottom
									+SUniformGridPanel::Slot(0, 3)
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::None, UIAnchorVerticalAlign::Bottom, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(1, 3)
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Left, UIAnchorVerticalAlign::Bottom, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(2, 3) 
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Center, UIAnchorVerticalAlign::Bottom, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(3, 3) 
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Right, UIAnchorVerticalAlign::Bottom, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(4, 3) 
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Stretch, UIAnchorVerticalAlign::Bottom, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									//Bottom stretch
									+SUniformGridPanel::Slot(0, 4)
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::None, UIAnchorVerticalAlign::Stretch, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(1, 4)
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Left, UIAnchorVerticalAlign::Stretch, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(2, 4) 
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Center, UIAnchorVerticalAlign::Stretch, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(3, 4) 
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Right, UIAnchorVerticalAlign::Stretch, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
									]
									+SUniformGridPanel::Slot(4, 4) 
									[
										SNew(SAnchorPreviewWidget, TargetScriptArray, UIAnchorHorizontalAlign::Stretch, UIAnchorVerticalAlign::Stretch, anchorItemSize, refreshDelegate)
										.BasePadding(itemBasePadding)
										.SelectedHAlign(anchorHAlign)
										.SelectedVAlign(anchorVAlign)
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
			;

		IDetailPropertyRow& anchorHAlignDetailProperty = anchorGroup.AddPropertyRow(anchorHAlignHandle);
		if (anchorControlledByParentLayout)
		{
			LGUIEditorUtils::SetControlledByParentLayout(anchorHAlignDetailProperty, anchorControlledByParentLayout);
		}
		else
		{
			if (anchorHControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(anchorHAlignDetailProperty, anchorHControlledBySelfLayout);
		}

		IDetailPropertyRow& anchorVAlignDetailProperty = anchorGroup.AddPropertyRow(anchorVAlignHandle);
		if (anchorControlledByParentLayout)
		{
			LGUIEditorUtils::SetControlledByParentLayout(anchorVAlignDetailProperty, anchorControlledByParentLayout);
		}
		else
		{
			if (anchorVControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(anchorVAlignDetailProperty, anchorVControlledBySelfLayout);
		}
		//stretch, anchorx, anchory
		if (anchorHAlign == UIAnchorHorizontalAlign::None)
		{
			transformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.width));
		}
		else if (anchorHAlign == UIAnchorHorizontalAlign::Stretch)
		{
			IDetailPropertyRow& stretchLeftProperty = transformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.stretchLeft));
			if (stretchLeftControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(stretchLeftProperty, true);
			IDetailPropertyRow& stretchRightProperty = transformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.stretchRight));
			if (stretchRightControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(stretchRightProperty, true);
		}
		else
		{
			IDetailPropertyRow& anchorOffsetXDetailProperty = transformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.anchorOffsetX));
			LGUIEditorUtils::SetControlledByParentLayout(anchorOffsetXDetailProperty, anchorControlledByParentLayout);
			IDetailPropertyRow& widthDetailProperty = transformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.width));
			if (widthControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(widthDetailProperty, widthControlledBySelfLayout);
			else
				if (widthControlledByParentLayout)
					LGUIEditorUtils::SetControlledByParentLayout(widthDetailProperty, widthControlledByParentLayout);
		}

		if (anchorVAlign == UIAnchorVerticalAlign::None)
		{
			transformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.height));
		}
		else if (anchorVAlign == UIAnchorVerticalAlign::Stretch)
		{
			IDetailPropertyRow& stretchTopProperty = transformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.stretchTop));
			if (stretchTopControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(stretchTopProperty, true);
			IDetailPropertyRow& stretchBottomProperty = transformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.stretchBottom));
			if (stretchBottomControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(stretchBottomProperty, true);
		}
		else
		{
			IDetailPropertyRow& anchorOffsetYDetailProperty = transformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.anchorOffsetY));
			LGUIEditorUtils::SetControlledByParentLayout(anchorOffsetYDetailProperty, anchorControlledByParentLayout);
			IDetailPropertyRow& heightDetailProperty = transformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.height));
			if (heightControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(heightDetailProperty, heightControlledBySelfLayout);
			else
				if (heightControlledByParentLayout)
					LGUIEditorUtils::SetControlledByParentLayout(heightDetailProperty, heightControlledByParentLayout);
		}
	}
	//pivot
	transformCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.pivot));

	//location rotation scale
	const FSelectedActorInfo& selectedActorInfo = DetailBuilder.GetDetailsView()->GetSelectedActorInfo();
	TSharedRef<FComponentTransformDetails> transformDetails = MakeShareable(new FComponentTransformDetails(TargetScriptArray, selectedActorInfo, DetailBuilder));
	transformCategory.AddCustomBuilder(transformDetails);
		
	//TSharedPtr<IPropertyHandle> widgetHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(FUIWidget, depth));
}
FText FUIItemCustomization::GetDepthInfo(TWeakObjectPtr<class UUIItem> TargetScript)const
{
	if (TargetScript.IsValid())
	{
		if (auto world = TargetScript->GetWorld())
		{
			TArray<UUIItem*> itemList;
			if (world->IsGameWorld())
			{
				if (auto instance = ALGUIManagerActor::GetLGUIManagerActorInstance(world))
				{
					itemList = instance->GetAllUIItem();
				}
			}
			else
			{
				if (ULGUIEditorManagerObject::Instance != nullptr)
				{
					itemList = ULGUIEditorManagerObject::Instance->GetAllUIItem();
				}
			}

			auto uiType = TargetScript->GetUIItemType();
			if (uiType != UIItemType::None)
			{
				auto renderCanvas = TargetScript.Get()->GetRenderCanvas();
				if (renderCanvas != nullptr)
				{
					int renderDepthCount = 0;
					int raycastDepthCount = 0;
					for (auto item : itemList)
					{
						if (IsValid(item))
						{
							if (item->GetRenderCanvas() == renderCanvas)
							{
								if (TargetScript->GetUIItemType() == UIItemType::UIRenderable && item->GetUIItemType() == UIItemType::UIRenderable)
								{
									if (item->widget.depth == TargetScript->widget.depth)
										renderDepthCount++;
								}
								if (TargetScript->IsRaycastTarget() && item->IsRaycastTarget())
								{
									if (item->widget.depth == TargetScript->widget.depth)
										raycastDepthCount++;
								}
							}
						}
					}
					auto depthInfo = FString::Printf(TEXT("Shared Renderable depth count:%d\nShared RaycastTarget depth count:%d"), renderDepthCount, raycastDepthCount);
					return FText::FromString(depthInfo);
				}
				else
				{
					return FText::FromString(FString::Printf(TEXT("(Need LGUI Canvas!)")));
				}
			}
		}
	}
	return LOCTEXT("", "");
}
void FUIItemCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
bool FUIItemCustomization::GetIsAnchorsEnabled()const
{
	bool anchorControlledByParentLayout = false;
	bool anchorHControlledBySelfLayout = false;
	bool anchorVControlledBySelfLayout = false;
	if (auto thisActor = TargetScriptArray[0]->GetOwner())
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

	if (anchorControlledByParentLayout)return false;
	if (anchorHControlledBySelfLayout)return false;
	if (anchorVControlledBySelfLayout)return false;
	return true;
}
FText FUIItemCustomization::GetAnchorsTooltipText()const
{
	return FText::FromString(FString(GetIsAnchorsEnabled() ? TEXT("Change anchor") : TEXT("Anchor is controlled by layout")));
}
#undef LOCTEXT_NAMESPACE