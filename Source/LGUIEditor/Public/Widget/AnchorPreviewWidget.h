// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Widgets/SCompoundWidget.h"
#include "DesktopPlatformModule.h"
#include "LGUIEditorStyle.h"
#pragma once

#define LOCTEXT_NAMESPACE "AnchorPreviewWidget"

namespace LGUIAnchorPreviewWidget
{
	enum class UIAnchorHorizontalAlign :uint8
	{
		None, Left, Center, Right, Stretch
	};
	enum class UIAnchorVerticalAlign :uint8
	{
		None, Top, Middle, Bottom, Stretch
	};

	DECLARE_DELEGATE_TwoParams(FOnAnchorChange, UIAnchorHorizontalAlign, UIAnchorVerticalAlign);

	//Anchor
	class SAnchorPreviewWidget : public SCompoundWidget
	{
	public:

		SLATE_BEGIN_ARGS(SAnchorPreviewWidget) {}
		SLATE_ATTRIBUTE(float, BasePadding)
			SLATE_ATTRIBUTE(bool, ButtonEnable)
			SLATE_ATTRIBUTE(UIAnchorHorizontalAlign, SelectedHAlign)
			SLATE_ATTRIBUTE(UIAnchorVerticalAlign, SelectedVAlign)
			SLATE_ATTRIBUTE(UIAnchorHorizontalAlign, PersistentHAlign)
			SLATE_ATTRIBUTE(UIAnchorVerticalAlign, PersistentVAlign)
			SLATE_EVENT(FOnAnchorChange, OnAnchorChange)
			SLATE_END_ARGS()

			void Construct(const FArguments& InArgs,
				FVector2D Size
			)
		{
			auto anchorLineHorizontalWidth = Size.X;
			auto anchorLineHorizontalHeight = 1;
			auto anchorLineVerticalWidth = 1;
			auto anchorLineVerticalHeight = Size.Y;

			EHorizontalAlignment anchorDotLeftTopHAlign = EHorizontalAlignment::HAlign_Left;
			EVerticalAlignment anchorDotLeftTopVAlign = EVerticalAlignment::VAlign_Top;
			EHorizontalAlignment anchorDotRightTopHAlign = EHorizontalAlignment::HAlign_Right;
			EVerticalAlignment anchorDotRightTopVAlign = EVerticalAlignment::VAlign_Top;
			EHorizontalAlignment anchorDotLeftBottomHAlign = EHorizontalAlignment::HAlign_Left;
			EVerticalAlignment anchorDotLeftBottomVAlign = EVerticalAlignment::VAlign_Bottom;
			EHorizontalAlignment anchorDotRightBottomHAlign = EHorizontalAlignment::HAlign_Right;
			EVerticalAlignment anchorDotRightBottomVAlign = EVerticalAlignment::VAlign_Bottom;
			int anchorDotSize = 5;
			//EHorizontalAlignment pivotDotHAlign = EHorizontalAlignment::HAlign_Left;
			//EVerticalAlignment pivotDotVAlign = EVerticalAlignment::VAlign_Top;
			int pivotDotSize = 5;

			EHorizontalAlignment hAlign = EHorizontalAlignment::HAlign_Center;
			switch (InArgs._PersistentHAlign.Get())
			{
			case UIAnchorHorizontalAlign::None:
			{
				anchorDotSize = 0;
				anchorLineVerticalHeight = 0;
				//pivotDotHAlign = EHorizontalAlignment::HAlign_Center;
			}
			break;
			case UIAnchorHorizontalAlign::Left:
			{
				hAlign = EHorizontalAlignment::HAlign_Left;
				anchorDotLeftTopHAlign = hAlign;
				anchorDotLeftBottomHAlign = hAlign;
				anchorDotRightTopHAlign = hAlign;
				anchorDotRightBottomHAlign = hAlign;
				//pivotDotHAlign = hAlign;
			}break;
			case UIAnchorHorizontalAlign::Center:
			{
				hAlign = EHorizontalAlignment::HAlign_Center;
				anchorDotLeftTopHAlign = hAlign;
				anchorDotLeftBottomHAlign = hAlign;
				anchorDotRightTopHAlign = hAlign;
				anchorDotRightBottomHAlign = hAlign;
				//pivotDotHAlign = hAlign;
			} break;
			case UIAnchorHorizontalAlign::Right:
			{
				hAlign = EHorizontalAlignment::HAlign_Right;
				anchorDotLeftTopHAlign = hAlign;
				anchorDotLeftBottomHAlign = hAlign;
				anchorDotRightTopHAlign = hAlign;
				anchorDotRightBottomHAlign = hAlign;
				//pivotDotHAlign = hAlign;
			} break;
			case UIAnchorHorizontalAlign::Stretch:
			{
				hAlign = EHorizontalAlignment::HAlign_Fill;
				anchorLineVerticalHeight = 1;
				//pivotDotHAlign = EHorizontalAlignment::HAlign_Center;
			}
			break;
			}
			EVerticalAlignment vAlign = EVerticalAlignment::VAlign_Center;
			switch (InArgs._PersistentVAlign.Get())
			{
			case UIAnchorVerticalAlign::None:
			{
				anchorDotSize = 0;
				anchorLineHorizontalWidth = 0;
				//pivotDotVAlign = EVerticalAlignment::VAlign_Center;
			}
			break;
			case UIAnchorVerticalAlign::Top:
			{
				vAlign = EVerticalAlignment::VAlign_Top;
				anchorDotLeftTopVAlign = vAlign;
				anchorDotLeftBottomVAlign = vAlign;
				anchorDotRightTopVAlign = vAlign;
				anchorDotRightBottomVAlign = vAlign;
				//pivotDotVAlign = vAlign;
			}
			break;
			case UIAnchorVerticalAlign::Middle:
			{
				vAlign = EVerticalAlignment::VAlign_Center;
				anchorDotLeftTopVAlign = vAlign;
				anchorDotLeftBottomVAlign = vAlign;
				anchorDotRightTopVAlign = vAlign;
				anchorDotRightBottomVAlign = vAlign;
				//pivotDotVAlign = vAlign;
			}
			break;
			case UIAnchorVerticalAlign::Bottom:
			{
				vAlign = EVerticalAlignment::VAlign_Bottom;
				anchorDotLeftTopVAlign = vAlign;
				anchorDotLeftBottomVAlign = vAlign;
				anchorDotRightTopVAlign = vAlign;
				anchorDotRightBottomVAlign = vAlign;
				//pivotDotVAlign = vAlign;
			}
			break;
			case UIAnchorVerticalAlign::Stretch:
			{
				vAlign = EVerticalAlignment::VAlign_Fill;
				anchorLineHorizontalWidth = 1;
				//pivotDotVAlign = EVerticalAlignment::VAlign_Center;
			}
			break;
			}

			auto anchorLineBrushHorizontal = FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot");
			auto anchorLineBrushVertical = FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot");
			if (InArgs._PersistentHAlign.Get() == UIAnchorHorizontalAlign::Stretch && InArgs._PersistentVAlign.Get() == UIAnchorVerticalAlign::Stretch)
			{
				anchorLineBrushHorizontal = FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteFrame");
				anchorLineBrushVertical = FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteFrame");
			}
			else if (InArgs._PersistentHAlign.Get() == UIAnchorHorizontalAlign::None && InArgs._PersistentVAlign.Get() == UIAnchorVerticalAlign::Stretch)
			{
				anchorLineBrushHorizontal = FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteFrameHorizontal");
				anchorLineHorizontalWidth = Size.X;
				anchorLineHorizontalHeight = Size.Y;
				anchorLineVerticalWidth = 0;
				anchorLineVerticalHeight = 0;
			}
			else if (InArgs._PersistentHAlign.Get() == UIAnchorHorizontalAlign::Stretch && InArgs._PersistentVAlign.Get() == UIAnchorVerticalAlign::None)
			{
				anchorLineBrushVertical = FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteFrameVertical");
				anchorLineVerticalWidth = Size.X;
				anchorLineVerticalHeight = Size.Y;
				anchorLineHorizontalWidth = 0;
				anchorLineHorizontalHeight = 0;
			}

			ChildSlot
			[
				SNew(SOverlay)
				//highlight selected
				+ SOverlay::Slot()
				[
					SNew(SBox)
					.WidthOverride(Size.X)
					.HeightOverride(Size.Y)
					.HAlign(EHorizontalAlignment::HAlign_Center)
					.VAlign(EVerticalAlignment::VAlign_Center)
					[
						InArgs._SelectedHAlign.Get() == InArgs._PersistentHAlign.Get() && InArgs._SelectedVAlign.Get() == InArgs._PersistentVAlign.Get() ?
						(
						SNew(SBox)
						.WidthOverride(Size.X + 10)
						.HeightOverride(Size.Y + 10)
						[
							SNew(SImage)
							.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteFrame"))
						.ColorAndOpacity(FLinearColor(FColor(255, 255, 255, 255)))
						]
						)
						:
						(SNew(SBox))
					]
				]
				+ SOverlay::Slot()
				[
					SNew(SOverlay)
					//outer rect
					+ SOverlay::Slot()
					[
						SNew(SBox)
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(Size.X)
							.HeightOverride(Size.Y)
							[
								SNew(SImage)
								.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteFrame"))
								.ColorAndOpacity(FLinearColor(FColor(122, 122, 122, 122)))
							]
						]
					]
					+ SOverlay::Slot()
					[
						SNew(SBox)
						.Padding(InArgs._BasePadding.Get())
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								InArgs._ButtonEnable.Get() ?
								(
								SNew(SBox)
								[
									SNew(SButton)
									.ButtonStyle(FLGUIEditorStyle::Get(), "AnchorButton")
									.ButtonColorAndOpacity(FLinearColor(FColor(100, 100, 100)))
									.OnClicked_Lambda([=]() {
										InArgs._OnAnchorChange.ExecuteIfBound(InArgs._PersistentHAlign.Get(), InArgs._PersistentVAlign.Get());
										return FReply::Handled();
										})
									.ContentPadding(FMargin(0.0f, 0.0f))
								]
								)
								:
								(
								SNew(SBox)
								)
							]
							+ SOverlay::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SBox)
									[
										SNew(SOverlay)
										//half size rect in the middle
										+ SOverlay::Slot()
										[
											SNew(SBox)
											.WidthOverride(Size.X)
											.HeightOverride(Size.Y)
											.Padding(this, &SAnchorPreviewWidget::GetInnerRectMargin, Size, InArgs._PersistentHAlign.Get(), InArgs._PersistentVAlign.Get(), InArgs._ButtonEnable.Get())
											.HAlign(EHorizontalAlignment::HAlign_Center)
											.VAlign(EVerticalAlignment::VAlign_Center)
											[
												SNew(SBox)
												.WidthOverride(this, &SAnchorPreviewWidget::GetInnerRectWidth, Size.X, InArgs._PersistentHAlign.Get(), InArgs._ButtonEnable.Get())
												.HeightOverride(this, &SAnchorPreviewWidget::GetInnerRectHeight, Size.Y, InArgs._PersistentVAlign.Get(), InArgs._ButtonEnable.Get())
												[
													SNew(SImage)
													.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteFrame"))
													.ColorAndOpacity(FLinearColor(FColor(122, 122, 122, 255)))
													.Visibility(EVisibility::HitTestInvisible)
												]
											]
										]
										//horizontal direction
										+ SOverlay::Slot()
										[
											SNew(SBox)
											.WidthOverride(Size.X)
											.HeightOverride(Size.Y)
											.HAlign(hAlign)
											.VAlign(vAlign)
											[
												SNew(SBox)
												.WidthOverride(anchorLineHorizontalWidth)
												.HeightOverride(anchorLineHorizontalHeight)
												[
													SNew(SImage)
													.Image(anchorLineBrushHorizontal)
													.ColorAndOpacity(FLinearColor(FColor(170, 77, 77, 255)))
													.Visibility(EVisibility::HitTestInvisible)
												]
											]
										]
										//vertical direction
										+ SOverlay::Slot()
										[
											SNew(SBox)
											.WidthOverride(Size.X)
											.HeightOverride(Size.Y)
											.HAlign(hAlign)
											.VAlign(vAlign)
											[
												SNew(SBox)
												.WidthOverride(anchorLineVerticalWidth)
												.HeightOverride(anchorLineVerticalHeight)
												[
													SNew(SImage)
													.Image(anchorLineBrushVertical)
													.ColorAndOpacity(FLinearColor(FColor(170, 77, 77, 255)))
													.Visibility(EVisibility::HitTestInvisible)
												]
											]
										]
										//pivot point//@todo: why commet? because not the desired behaviour i want
										//+ SOverlay::Slot()
										//[
										//	SNew(SBox)
										//	.WidthOverride(Size.X)
										//	.HeightOverride(Size.Y)
										//	.Padding(this, &SAnchorPreviewWidget::GetInnerRectMargin, Size, InArgs._PersistentHAlign.Get(), InArgs._PersistentVAlign.Get(), InArgs._ButtonEnable.Get())
										//	.Visibility(this, &SAnchorPreviewWidget::GetPivotDotVisibility, InArgs._PersistentHAlign.Get(), InArgs._PersistentVAlign.Get(), InArgs._ButtonEnable.Get())
										//	[
										//		SNew(SBox)
										//		.WidthOverride(Size.X * 0.5f)
										//		.HeightOverride(Size.Y * 0.5f)
										//		[
										//			SNew(SBox)
										//			.HAlign(pivotDotHAlign)
										//			.VAlign(pivotDotVAlign)
										//			[
										//				SNew(SBox)
										//				.WidthOverride(pivotDotSize)
										//				.HeightOverride(pivotDotSize)
										//				[
										//					SNew(SImage)
										//					.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.AnchorData_Dot"))
										//					.ColorAndOpacity(FLinearColor(FColor(50, 255, 200, 128)))
										//					.Visibility(EVisibility::HitTestInvisible)
										//				]
										//			]
										//		]
										//	]
										//]
										//left top anchor point
										+ SOverlay::Slot()
										[
											SNew(SBox)
											.WidthOverride(Size.X)
											.HeightOverride(Size.Y)
											.HAlign(anchorDotLeftTopHAlign)
											.VAlign(anchorDotLeftTopVAlign)
											[
												SNew(SBox)
												.WidthOverride(anchorDotSize)
												.HeightOverride(anchorDotSize)
												[
													SNew(SImage)
													.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.AnchorData_Dot"))
													.ColorAndOpacity(FLinearColor(FColor(201, 146, 7, 255)))
													.Visibility(EVisibility::HitTestInvisible)
												]
											]
										]
										//right top anchor point
										+ SOverlay::Slot()
										[
											SNew(SBox)
											.WidthOverride(Size.X)
											.HeightOverride(Size.Y)
											.HAlign(anchorDotRightTopHAlign)
											.VAlign(anchorDotRightTopVAlign)
											[
												SNew(SBox)
												.WidthOverride(anchorDotSize)
												.HeightOverride(anchorDotSize)
												[
													SNew(SImage)
													.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.AnchorData_Dot"))
													.ColorAndOpacity(FLinearColor(FColor(201, 146, 7, 255)))
													.Visibility(EVisibility::HitTestInvisible)
												]
											]
										]
										//left bottom anchor point
										+ SOverlay::Slot()
										[
											SNew(SBox)
											.WidthOverride(Size.X)
											.HeightOverride(Size.Y)
											.HAlign(anchorDotLeftBottomHAlign)
											.VAlign(anchorDotLeftBottomVAlign)
											[
												SNew(SBox)
												.WidthOverride(anchorDotSize)
												.HeightOverride(anchorDotSize)
												[
													SNew(SImage)
													.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.AnchorData_Dot"))
													.ColorAndOpacity(FLinearColor(FColor(201, 146, 7, 255)))
													.Visibility(EVisibility::HitTestInvisible)
												]
											]
										]
										//right bottom anchor point
										+ SOverlay::Slot()
										[
											SNew(SBox)
											.WidthOverride(Size.X)
											.HeightOverride(Size.Y)
											.HAlign(anchorDotRightBottomHAlign)
											.VAlign(anchorDotRightBottomVAlign)
											[
												SNew(SBox)
												.WidthOverride(anchorDotSize)
												.HeightOverride(anchorDotSize)
												[
													SNew(SImage)
													.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.AnchorData_Dot"))
													.ColorAndOpacity(FLinearColor(FColor(201, 146, 7, 255)))
													.Visibility(EVisibility::HitTestInvisible)
												]
											]
										]
									]
								]
							]
						]
					]
				]
			];
		}

	private:

		EVisibility GetPivotDotVisibility(UIAnchorHorizontalAlign PivotHAlign, UIAnchorVerticalAlign PivotVAlign, bool IsInteractable)const
		{
			return (
				IsInteractable && FSlateApplication::Get().GetModifierKeys().IsShiftDown()
				&& PivotHAlign != UIAnchorHorizontalAlign::None && PivotVAlign != UIAnchorVerticalAlign::None
				) ? EVisibility::HitTestInvisible : EVisibility::Hidden;
		}
		FOptionalSize GetInnerRectWidth(float Size, UIAnchorHorizontalAlign hAlign, bool IsInteractable) const
		{
			bool snapAnchor = FSlateApplication::Get().GetModifierKeys().IsAltDown();
			if (snapAnchor && IsInteractable)
			{
				if (hAlign == UIAnchorHorizontalAlign::Stretch)
				{
					return Size * 0.9f;
				}
			}
			return Size * 0.5f;
		}
		FOptionalSize GetInnerRectHeight(float Size, UIAnchorVerticalAlign vAlign, bool IsInteractable) const
		{
			bool snapAnchor = FSlateApplication::Get().GetModifierKeys().IsAltDown();
			if (snapAnchor && IsInteractable)
			{
				if (vAlign == UIAnchorVerticalAlign::Stretch)
				{
					return Size * 0.9f;
				}
			}
			return Size * 0.5f;
		}
		FMargin GetInnerRectMargin(FVector2D Size, UIAnchorHorizontalAlign hAlign, UIAnchorVerticalAlign vAlign, bool IsInteractable)const
		{
			FMargin result(0, 0, 0, 0);
			bool snapAnchor = FSlateApplication::Get().GetModifierKeys().IsAltDown();
			if (snapAnchor && IsInteractable)
			{
				switch (hAlign)
				{
				case UIAnchorHorizontalAlign::Left:
					result.Right = Size.X * 0.25f;
					break;
				case UIAnchorHorizontalAlign::Right:
					result.Left = Size.X * 0.25f;
					break;
				}
				switch (vAlign)
				{
				case UIAnchorVerticalAlign::Top:
					result.Bottom = Size.X * 0.25f;
					break;
				case UIAnchorVerticalAlign::Bottom:
					result.Top = Size.X * 0.25f;
					break;
				}
			}
			return result;
		}
	};
}

#undef LOCTEXT_NAMESPACE