// Copyright 2019 LexLiu. All Rights Reserved.

#include "Widgets/SCompoundWidget.h"
#include "DesktopPlatformModule.h"
#pragma once

#define LOCTEXT_NAMESPACE "AnchorPreviewWidget"
//Anchor
class SAnchorPreviewWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SAnchorPreviewWidget) {}
	SLATE_ATTRIBUTE(float, BasePadding)
	SLATE_ATTRIBUTE(UIAnchorHorizontalAlign, SelectedHAlign)
	SLATE_ATTRIBUTE(UIAnchorVerticalAlign, SelectedVAlign)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, 
		TArray<TWeakObjectPtr<UUIItem>> TargetScriptArray,
		UIAnchorHorizontalAlign HorizontalAlign,
		UIAnchorVerticalAlign VerticalAlign,
		FVector2D Size,
		FSimpleDelegate InOnClickAnchor)
	{
		OnClickAnchorDelegate = InOnClickAnchor;

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
		int anchorDotSize = 4;

		EHorizontalAlignment hAlign = EHorizontalAlignment::HAlign_Center;
		switch (HorizontalAlign)
		{
		case UIAnchorHorizontalAlign::None:
		{
			anchorDotSize = 0;
			anchorLineVerticalHeight = 0;
		}
		break;
		case UIAnchorHorizontalAlign::Left: 
		{
			hAlign = EHorizontalAlignment::HAlign_Left;
			anchorDotLeftTopHAlign = hAlign;
			anchorDotLeftBottomHAlign = hAlign;
			anchorDotRightTopHAlign = hAlign;
			anchorDotRightBottomHAlign = hAlign;
		}break;
		case UIAnchorHorizontalAlign::Center: 
		{ 
			hAlign = EHorizontalAlignment::HAlign_Center; 
			anchorDotLeftTopHAlign = hAlign;
			anchorDotLeftBottomHAlign = hAlign;
			anchorDotRightTopHAlign = hAlign;
			anchorDotRightBottomHAlign = hAlign;
		} break;
		case UIAnchorHorizontalAlign::Right: 
		{ 
			hAlign = EHorizontalAlignment::HAlign_Right; 
			anchorDotLeftTopHAlign = hAlign;
			anchorDotLeftBottomHAlign = hAlign;
			anchorDotRightTopHAlign = hAlign;
			anchorDotRightBottomHAlign = hAlign;
		} break;
		case UIAnchorHorizontalAlign::Stretch: 
		{ 
			hAlign = EHorizontalAlignment::HAlign_Fill;
			anchorLineVerticalHeight = 1; 
		} 
		break;
		}
		EVerticalAlignment vAlign = EVerticalAlignment::VAlign_Center;
		switch (VerticalAlign)
		{
		case UIAnchorVerticalAlign::None:
		{
			anchorDotSize = 0;
			anchorLineHorizontalWidth = 0;
		}
		break;
		case UIAnchorVerticalAlign::Top: 
		{
			vAlign = EVerticalAlignment::VAlign_Top;
			anchorDotLeftTopVAlign = vAlign;
			anchorDotLeftBottomVAlign = vAlign;
			anchorDotRightTopVAlign = vAlign;
			anchorDotRightBottomVAlign = vAlign;
		}
		break;
		case UIAnchorVerticalAlign::Middle: 
		{ 
			vAlign = EVerticalAlignment::VAlign_Center;
			anchorDotLeftTopVAlign = vAlign;
			anchorDotLeftBottomVAlign = vAlign;
			anchorDotRightTopVAlign = vAlign;
			anchorDotRightBottomVAlign = vAlign;
		} 
		break;
		case UIAnchorVerticalAlign::Bottom: 
		{
			vAlign = EVerticalAlignment::VAlign_Bottom; 
			anchorDotLeftTopVAlign = vAlign;
			anchorDotLeftBottomVAlign = vAlign;
			anchorDotRightTopVAlign = vAlign;
			anchorDotRightBottomVAlign = vAlign;
		} 
		break;
		case UIAnchorVerticalAlign::Stretch:
		{ 
			vAlign = EVerticalAlignment::VAlign_Fill;
			anchorLineHorizontalWidth = 1;
		}
		break;
		}

		auto anchorLineBrushHorizontal = FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot");
		auto anchorLineBrushVertical = FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot");
		if (HorizontalAlign == UIAnchorHorizontalAlign::Stretch && VerticalAlign == UIAnchorVerticalAlign::Stretch)
		{
			anchorLineBrushHorizontal = FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteFrame");
			anchorLineBrushVertical = FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteFrame");
		}
		else if (HorizontalAlign == UIAnchorHorizontalAlign::None && VerticalAlign == UIAnchorVerticalAlign::Stretch)
		{
			anchorLineBrushHorizontal = FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteFrameHorizontal");
			anchorLineHorizontalWidth = Size.X;
			anchorLineHorizontalHeight = Size.Y;
			anchorLineVerticalWidth = 0;
			anchorLineVerticalHeight = 0;
		}
		else if (HorizontalAlign == UIAnchorHorizontalAlign::Stretch && VerticalAlign == UIAnchorVerticalAlign::None)
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
					InArgs._SelectedHAlign.Get() == HorizontalAlign && InArgs._SelectedVAlign.Get() == VerticalAlign ?
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
			SNew(SBox)
			.Padding(InArgs._BasePadding.Get())
			[
				SNew(SButton)
				.ButtonStyle(FLGUIEditorStyle::Get(), "AnchorButton")
				.ButtonColorAndOpacity(FLinearColor(FColor(100, 100, 100)))
				.OnClicked(this, &SAnchorPreviewWidget::OnAnchorClicked, TargetScriptArray, HorizontalAlign, VerticalAlign)
				.ContentPadding(FMargin(0.0f, 0.0f))
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
					
						SNew(SBox)
						[
							SNew(SOverlay)
							//half size rect in the middle
							+SOverlay::Slot()
							[
								SNew(SBox)
								.WidthOverride(Size.X)
								.HeightOverride(Size.Y)
								.Padding(this, &SAnchorPreviewWidget::GetInnerRectMargin, Size, HorizontalAlign, VerticalAlign)
								.HAlign(EHorizontalAlignment::HAlign_Center)
								.VAlign(EVerticalAlignment::VAlign_Center)
								[
									SNew(SBox)
									.WidthOverride(this, &SAnchorPreviewWidget::GetInnerRectWidth, Size.X, HorizontalAlign)
									.HeightOverride(this, &SAnchorPreviewWidget::GetInnerRectHeight, Size.Y, VerticalAlign)
									[
										SNew(SImage)
										.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteFrame"))
										.ColorAndOpacity(FLinearColor(FColor(122, 122, 122, 255)))
									]
								]
							]
							//horizontal direction
							+SOverlay::Slot()
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
									]
								]
							]
							//vertical direction
							+SOverlay::Slot()
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
									]
								]
							]
							//left top anchor point
							+SOverlay::Slot()
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
										.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot"))
										.ColorAndOpacity(FLinearColor(FColor(201, 146, 7, 255)))
									]
								]
							]
							//right top anchor point
							+SOverlay::Slot()
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
										.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot"))
										.ColorAndOpacity(FLinearColor(FColor(201, 146, 7, 255)))
									]
								]
							]
							//left bottom anchor point
							+SOverlay::Slot()
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
										.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot"))
										.ColorAndOpacity(FLinearColor(FColor(201, 146, 7, 255)))
									]
								]
							]
							//right bottom anchor point
							+SOverlay::Slot()
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
										.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.WhiteDot"))
										.ColorAndOpacity(FLinearColor(FColor(201, 146, 7, 255)))
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

	FReply OnAnchorClicked(TArray<TWeakObjectPtr<UUIItem>> TargetScriptArray, UIAnchorHorizontalAlign HorizontalAlign,	UIAnchorVerticalAlign VerticalAlign)
	{
		bool snapAnchor = FSlateApplication::Get().GetModifierKeys().IsControlDown();
		if (TargetScriptArray.Num() > 0)
		{
			for (auto item : TargetScriptArray)
			{
				item->SetAnchorHAlign(HorizontalAlign);
				item->SetAnchorVAlign(VerticalAlign);
				if (snapAnchor)
				{
					if (HorizontalAlign == UIAnchorHorizontalAlign::Stretch)
					{
						item->SetStretchLeft(0);
						item->SetStretchRight(0);
					}
					else if (HorizontalAlign != UIAnchorHorizontalAlign::None)
					{
						item->SetAnchorOffsetX(0);
					}
					if (VerticalAlign == UIAnchorVerticalAlign::Stretch)
					{
						item->SetStretchBottom(0);
						item->SetStretchTop(0);
					}
					else if (VerticalAlign != UIAnchorVerticalAlign::None)
					{
						item->SetAnchorOffsetY(0);
					}
				}
				item->EditorForceUpdateImmediately();
			}
			
			// Close the menu
			FSlateApplication::Get().DismissAllMenus();
			OnClickAnchorDelegate.ExecuteIfBound();
		}
		return FReply::Handled();
	}
	FOptionalSize GetInnerRectWidth(float Size, UIAnchorHorizontalAlign hAlign) const
	{
		bool snapAnchor = FSlateApplication::Get().GetModifierKeys().IsControlDown();
		if (snapAnchor)
		{
			if (hAlign == UIAnchorHorizontalAlign::Stretch)
			{
				return Size * 0.9f;
			}
		}
		return Size * 0.5f;
	}
	FOptionalSize GetInnerRectHeight(float Size, UIAnchorVerticalAlign vAlign) const
	{
		bool snapAnchor = FSlateApplication::Get().GetModifierKeys().IsControlDown();
		if (snapAnchor)
		{
			if (vAlign == UIAnchorVerticalAlign::Stretch)
			{
				return Size * 0.9f;
			}
		}
		return Size * 0.5f;
	}
	FMargin GetInnerRectMargin(FVector2D Size, UIAnchorHorizontalAlign hAlign, UIAnchorVerticalAlign vAlign)const
	{
		FMargin result(0, 0, 0, 0);
		bool snapAnchor = FSlateApplication::Get().GetModifierKeys().IsControlDown();
		if (snapAnchor)
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

	FSimpleDelegate OnClickAnchorDelegate;
};
#undef LOCTEXT_NAMESPACE