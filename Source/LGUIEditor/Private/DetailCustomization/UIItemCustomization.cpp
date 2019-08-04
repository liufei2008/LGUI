// Copyright 2019 LexLiu. All Rights Reserved.

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

#define LOCTEXT_NAMESPACE "UIItemComponentDetails"



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
			SetHierarchyIndexInfo(TargetScriptArray[0]);
			ForceUpdateUI();
		}));
		auto hierarchyIndexWidget =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(2, 4)
			.FillWidth(5)
			[
				hierarchyIndexHandle->CreatePropertyValueWidget()
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 6)
			.FillWidth(2)
			[
				SNew(SButton)
				.Text(LOCTEXT("Increase", "+"))
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.OnClicked_Lambda([&]()
				{
				for (auto item : TargetScriptArray)
				{
					item->SetHierarchyIndex(item->hierarchyIndex + 1);
					SetHierarchyIndexInfo(item);
					ForceUpdateUI();
				}
				return FReply::Handled(); 
				})
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 6)
			.FillWidth(2)
			[
				SNew(SButton)
				.Text(LOCTEXT("Decrease", "-"))
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.OnClicked_Lambda([&]()
				{
				for (auto item : TargetScriptArray)
				{
					item->SetHierarchyIndex(item->hierarchyIndex - 1);
					SetHierarchyIndexInfo(item);
					ForceUpdateUI();
				}
				return FReply::Handled();
				})
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

		lguiCategory.AddCustomRow(LOCTEXT("HierarchyIndexInfo", "HierarchyIndexInfo"), true)
			.ValueContent()
			[
				SAssignNew(HierarchyIndexTextBlock, STextBlock)
				.Text(FText::FromString("SharedCount:0"))
			];
		SetHierarchyIndexInfo(TargetScriptArray[0]);
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI-Widget");
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget));
	auto uiType = TargetScriptArray[0]->GetUIItemType();

	//depth
	{
		auto depthHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.depth));
		DetailBuilder.HideProperty(depthHandle);
		depthHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] {
			SetDepthInfo(TargetScriptArray[0]);
			ForceUpdateUI();
		}));
		auto depthWidget =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(2, 4)
			.FillWidth(5)
			[
				depthHandle->CreatePropertyValueWidget()
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 6)
			.FillWidth(2)
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
				SetDepthInfo(TargetScriptArray[0]);
				ForceUpdateUI();
					return FReply::Handled(); 
				})
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 6)
			.FillWidth(2)
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
				SetDepthInfo(TargetScriptArray[0]);
				ForceUpdateUI();
				return FReply::Handled();
				})
			];

		category.AddCustomRow(LOCTEXT("DepthManager", "DepthManager"))
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
			if (uiType == UIItemType::UIRenderable || uiType == UIItemType::UIItem)
			{
				category.AddCustomRow(LOCTEXT("DepthInfo", "DepthInfo"))
					.ValueContent()
					.MinDesiredWidth(500)
					[
						SAssignNew(DepthInfoTextBlock, STextBlock)
						.Text(FText::FromString("0"))
					];
			}
			else if (uiType == UIItemType::UIPanel)
			{
				category.AddCustomRow(LOCTEXT("DepthInfo", "DepthInfo"))
					.ValueContent()
					.MinDesiredWidth(500)
					[
						SAssignNew(DepthInfoTextBlock, STextBlock)
						.Text(FText::FromString("0"))
					];
			}
			SetDepthInfo(TargetScriptArray[0]);
		}
	}
	//color
	auto inheritAlphaHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, inheritAlpha));
	auto colorHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.color));
	category.AddProperty(colorHandle);
	category.AddProperty(inheritAlphaHandle);
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
		IDetailGroup& anchorGroup = category.AddGroup(FName("AnchorGroup"), LOCTEXT("AnchorGroup", "AnchorGroup"));
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
				.IsEnabled(!anchorControlledByParentLayout)
				.ToolTipText(FText::FromString(FString(anchorControlledByParentLayout ? TEXT("Anchor is controlled by parent layout") : TEXT("Change anchor"))))
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
			category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.width));
		}
		else if (anchorHAlign == UIAnchorHorizontalAlign::Stretch)
		{
			category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.stretchLeft));
			category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.stretchRight));
		}
		else
		{
			IDetailPropertyRow& anchorOffsetXDetailProperty = category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.anchorOffsetX));
			LGUIEditorUtils::SetControlledByParentLayout(anchorOffsetXDetailProperty, anchorControlledByParentLayout);
			IDetailPropertyRow& widthDetailProperty = category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.width));
			if (widthControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(widthDetailProperty, widthControlledBySelfLayout);
			else
				if (widthControlledByParentLayout)
					LGUIEditorUtils::SetControlledByParentLayout(widthDetailProperty, widthControlledByParentLayout);
		}

		if (anchorVAlign == UIAnchorVerticalAlign::None)
		{
			category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.height));
		}
		else if (anchorVAlign == UIAnchorVerticalAlign::Stretch)
		{
			category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.stretchTop));
			category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.stretchBottom));
		}
		else
		{
			IDetailPropertyRow& anchorOffsetYDetailProperty = category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.anchorOffsetY));
			LGUIEditorUtils::SetControlledByParentLayout(anchorOffsetYDetailProperty, anchorControlledByParentLayout);
			IDetailPropertyRow& heightDetailProperty = category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.height));
			if (heightControlledBySelfLayout)
				LGUIEditorUtils::SetControlledBySelfLayout(heightDetailProperty, heightControlledBySelfLayout);
			else
				if (heightControlledByParentLayout)
					LGUIEditorUtils::SetControlledByParentLayout(heightDetailProperty, heightControlledByParentLayout);
		}
	}
	//pivot
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIItem, widget.pivot));
		
	//TSharedPtr<IPropertyHandle> widgetHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(FUIWidget, depth));
}
void FUIItemCustomization::SetDepthInfo(TWeakObjectPtr<class UUIItem> TargetScript)
{
	if (DepthInfoTextBlock.Get() != nullptr)
	{
		auto uiType = TargetScript->GetUIItemType();
		if (uiType == UIItemType::UIRenderable || uiType == UIItemType::UIItem)
		{
			auto renderUIPanel = (UUIPanel*)((UUIRenderable*)TargetScript.Get())->GetRenderUIPanel();
			if (renderUIPanel != nullptr)
			{
				auto itemList = renderUIPanel->UIRenderableItemList;
				int depthCount = 0;
				for (auto item : itemList)
				{
					if (item->widget.depth == TargetScript->widget.depth)
						depthCount++;
				}
				DepthInfoTextBlock->SetText(FString::Printf(TEXT("SharedDepthCount:%d"), depthCount));
			}
			else
			{
				DepthInfoTextBlock->SetText(FString::Printf(TEXT("(MustAttachToUIPanel)")));
			}
		}
		else if (uiType == UIItemType::UIPanel)
		{
			int depthCount = 0;
			if (LGUIManager::IsManagerValid(GWorld))
			{
				auto& panelList = LGUIManager::GetAllUIPanel(GWorld);
				for (auto item : panelList)
				{
					if (item->widget.depth == TargetScript->widget.depth)
						depthCount++;
				}
			}
			DepthInfoTextBlock->SetText(FString::Printf(TEXT("PanelSharedDepthCount:%d"), depthCount));
		}
	}
}
void FUIItemCustomization::SetHierarchyIndexInfo(TWeakObjectPtr<class UUIItem> InTargetScript)
{
	int sameIndexCount = 0;
	if (auto parentSceneComp = InTargetScript->GetAttachParent())
	{
		auto hierarchyIndex = InTargetScript->hierarchyIndex;
		const auto& childrenList = parentSceneComp->GetAttachChildren();
		for (auto childSceneComp : childrenList)
		{
			if (auto uiChild = Cast<UUIItem>(childSceneComp))
			{
				if (uiChild->hierarchyIndex == hierarchyIndex)
				{
					sameIndexCount++;
				}
			}
		}
		
	}
	HierarchyIndexTextBlock->SetText(FString::Printf(TEXT("SharedCount:%d"), sameIndexCount));
}
void FUIItemCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (DetailBuilder)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE