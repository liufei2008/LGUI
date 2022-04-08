// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LGUIVectorInputBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "HAL/IConsoleManager.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

#define LOCTEXT_NAMESPACE "SLGUIVectorInputBox"

TAutoConsoleVariable<float> CVarCrushThem(TEXT("Slate.AllowNumericLabelCrush"), 1.0f, TEXT("Should we crush the vector input box?."));
TAutoConsoleVariable<float> CVarStopCrushWhenAbove(TEXT("Slate.NumericLabelWidthCrushStop"), 200.0f, TEXT("Stop crushing when the width is above."));
TAutoConsoleVariable<float> CVarStartCrushWhenBelow(TEXT("Slate.NumericLabelWidthCrushStart"), 190.0f, TEXT("Start crushing when the width is below."));

void SLGUIVectorInputBox::Construct( const FArguments& InArgs )
{
	bCanBeCrushed = InArgs._AllowResponsiveLayout;

	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);

	ChildSlot
	[
		HorizontalBox
	];

	if (InArgs._ShowX)ConstructX(InArgs, HorizontalBox);
	if (InArgs._ShowY)ConstructY(InArgs, HorizontalBox);
	if (InArgs._ShowZ)ConstructZ(InArgs, HorizontalBox);
	if (InArgs._ShowW)ConstructW(InArgs, HorizontalBox);
}

void SLGUIVectorInputBox::ConstructX( const FArguments& InArgs, TSharedRef<SHorizontalBox> HorizontalBox )
{	
	const FLinearColor LabelColor = InArgs._bColorAxisLabels ?  SNumericEntryBox<float>::RedLabelBackgroundColor : FLinearColor(0.0f,0.0f,0.0f,.5f);
	TSharedRef<SWidget> LabelWidget = BuildDecoratorLabel(LabelColor, FLinearColor::White, LOCTEXT("X_Label", "X"));
	TAttribute<FMargin> MarginAttribute;
	if (bCanBeCrushed)
	{
		MarginAttribute = TAttribute<FMargin>::Create(TAttribute<FMargin>::FGetter::CreateSP(this, &SLGUIVectorInputBox::GetTextMargin));
	}

	HorizontalBox->AddSlot()
	.VAlign( VAlign_Center )
	.FillWidth( 1.0f )
	.Padding( 0.0f, 1.0f, 2.0f, 1.0f )
	[
		SNew( SNumericEntryBox<float> )
		.AllowSpin(InArgs._AllowSpin)
		.Font( InArgs._Font )
		.Value( InArgs._X )
		.OnValueChanged( InArgs._OnXChanged )
		.OnValueCommitted( InArgs._OnXCommitted )		
		.UndeterminedString( LOCTEXT("MultipleValues", "Multiple Values") )
		.LabelPadding(0)
		.OverrideTextMargin(MarginAttribute)
		.TypeInterface(InArgs._TypeInterface)
		.IsEnabled(InArgs._EnableX)
		.ToolTipText_Lambda([=] {
			if (InArgs._EnableX.IsBound())
			{
				return
					InArgs._EnableX.Get()
					? LOCTEXT("X_ToolTip", "X Value")
					: LOCTEXT("X_ToolTip_Control", "X Value Controlled by LGUI's Anchors");
			}
			return LOCTEXT("X_ToolTip", "X Value");
		})
		.Label()
		[
			LabelWidget
		]
	];
	
}

void SLGUIVectorInputBox::ConstructY( const FArguments& InArgs, TSharedRef<SHorizontalBox> HorizontalBox )
{
	const FLinearColor LabelColor = InArgs._bColorAxisLabels ?  SNumericEntryBox<float>::GreenLabelBackgroundColor : FLinearColor(0.0f,0.0f,0.0f,.5f);
	TSharedRef<SWidget> LabelWidget = BuildDecoratorLabel(LabelColor, FLinearColor::White, LOCTEXT("Y_Label", "Y"));
	TAttribute<FMargin> MarginAttribute;
	if (bCanBeCrushed)
	{
		MarginAttribute = TAttribute<FMargin>::Create(TAttribute<FMargin>::FGetter::CreateSP(this, &SLGUIVectorInputBox::GetTextMargin));
	}

	HorizontalBox->AddSlot()
	.VAlign( VAlign_Center )
	.FillWidth( 1.0f )
	.Padding( 0.0f, 1.0f, 2.0f, 1.0f )
	[
		SNew( SNumericEntryBox<float> )
		.AllowSpin(InArgs._AllowSpin)
		.Font( InArgs._Font )
		.Value( InArgs._Y )
		.OnValueChanged( InArgs._OnYChanged )
		.OnValueCommitted( InArgs._OnYCommitted )
		.UndeterminedString( LOCTEXT("MultipleValues", "Multiple Values") )
		.LabelPadding(0)
		.OverrideTextMargin(MarginAttribute)
		.TypeInterface(InArgs._TypeInterface)
		.IsEnabled(InArgs._EnableY)
		.ToolTipText_Lambda([=] {
			if (InArgs._EnableY.IsBound())
			{
				return
					InArgs._EnableY.Get()
					? LOCTEXT("Y_ToolTip", "Y Value")
					: LOCTEXT("Y_ToolTip_Control", "Y Value Controlled by LGUI's Anchors");
			}
			return LOCTEXT("Y_ToolTip", "Y Value");
		})
		.Label()
		[
			LabelWidget
		]
	];

}

void SLGUIVectorInputBox::ConstructZ( const FArguments& InArgs, TSharedRef<SHorizontalBox> HorizontalBox )
{
	const FLinearColor LabelColor = InArgs._bColorAxisLabels ?  SNumericEntryBox<float>::BlueLabelBackgroundColor : FLinearColor(0.0f,0.0f,0.0f,.5f);
	TSharedRef<SWidget> LabelWidget = BuildDecoratorLabel(LabelColor, FLinearColor::White, LOCTEXT("Z_Label", "Z"));
	TAttribute<FMargin> MarginAttribute;
	if (bCanBeCrushed)
	{
		MarginAttribute = TAttribute<FMargin>::Create(TAttribute<FMargin>::FGetter::CreateSP(this, &SLGUIVectorInputBox::GetTextMargin));
	}

	HorizontalBox->AddSlot()
	.VAlign( VAlign_Center )
	.FillWidth( 1.0f )
	.Padding( 0.0f, 1.0f, 2.0f, 1.0f )
	[
		SNew( SNumericEntryBox<float> )
		.AllowSpin(InArgs._AllowSpin)
		.Font( InArgs._Font )
		.Value( InArgs._Z )
		.OnValueChanged( InArgs._OnZChanged )
		.OnValueCommitted( InArgs._OnZCommitted )
		.UndeterminedString( LOCTEXT("MultipleValues", "Multiple Values") )
		.LabelPadding(0)
		.OverrideTextMargin(MarginAttribute)
		.TypeInterface(InArgs._TypeInterface)
		.IsEnabled(InArgs._EnableZ)
		.ToolTipText_Lambda([=] {
			if (InArgs._EnableZ.IsBound())
			{
				return
					InArgs._EnableZ.Get()
					? LOCTEXT("Z_ToolTip", "Z Value")
					: LOCTEXT("Z_ToolTip_Control", "Z Value Controlled by LGUI's Anchors");
			}
			return LOCTEXT("Z_ToolTip", "Z Value");
		})
		.Label()
		[
			LabelWidget
		]
	];
}

void SLGUIVectorInputBox::ConstructW(const FArguments& InArgs, TSharedRef<SHorizontalBox> HorizontalBox)
{
	const FLinearColor LabelColor = InArgs._bColorAxisLabels ? FLinearColor(0.594f, 0.3959f, 0.0f) : FLinearColor(0.0f, 0.0f, 0.0f, .5f);
	TSharedRef<SWidget> LabelWidget = BuildDecoratorLabel(LabelColor, FLinearColor::White, LOCTEXT("W_Label", "W"));
	TAttribute<FMargin> MarginAttribute;
	if (bCanBeCrushed)
	{
		MarginAttribute = TAttribute<FMargin>::Create(TAttribute<FMargin>::FGetter::CreateSP(this, &SLGUIVectorInputBox::GetTextMargin));
	}

	HorizontalBox->AddSlot()
		.VAlign(VAlign_Center)
		.FillWidth(1.0f)
		.Padding(0.0f, 1.0f, 0.0f, 1.0f)
		[
			SNew(SNumericEntryBox<float>)
			.AllowSpin(InArgs._AllowSpin)
			.Font(InArgs._Font)
			.Value(InArgs._W)
			.OnValueChanged(InArgs._OnWChanged)
			.OnValueCommitted(InArgs._OnWCommitted)
			.UndeterminedString(LOCTEXT("MultipleValues", "Multiple Values"))
			.LabelPadding(0)
			.OverrideTextMargin(MarginAttribute)
			.TypeInterface(InArgs._TypeInterface)
			.IsEnabled(InArgs._EnableW)
			.ToolTipText_Lambda([=] {
			if (InArgs._EnableW.IsBound())
			{
				return
					InArgs._EnableW.Get()
					? LOCTEXT("W_ToolTip", "W Value")
					: LOCTEXT("W_ToolTip_Control", "W Value Controlled by LGUI's Anchors");
			}
			return LOCTEXT("W_ToolTip", "W Value");
			})
			.Label()
				[
					LabelWidget
				]
		];
}

TSharedRef<SWidget> SLGUIVectorInputBox::BuildDecoratorLabel(FLinearColor BackgroundColor, FLinearColor InForegroundColor, FText Label)
{
	TSharedRef<SWidget> LabelWidget = SNumericEntryBox<float>::BuildLabel(Label, InForegroundColor, BackgroundColor);

	TSharedRef<SWidget> ResultWidget = LabelWidget;
	
	if (bCanBeCrushed)
	{
		ResultWidget =
			SNew(SWidgetSwitcher)
			.WidgetIndex(this, &SLGUIVectorInputBox::GetLabelActiveSlot)
			+SWidgetSwitcher::Slot()
			[
				LabelWidget
			]
			+SWidgetSwitcher::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NumericEntrySpinBox.NarrowDecorator"))
				.BorderBackgroundColor(BackgroundColor)
				.ForegroundColor(InForegroundColor)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				.Padding(FMargin(5.0f, 0.0f, 0.0f, 0.0f))
			];
	}

	return ResultWidget;
}

int32 SLGUIVectorInputBox::GetLabelActiveSlot() const
{
	return bIsBeingCrushed ? 1 : 0;
}

FMargin SLGUIVectorInputBox::GetTextMargin() const
{
	return bIsBeingCrushed ? FMargin(1.0f, 2.0f) : FMargin(4.0f, 2.0f);
}

void SLGUIVectorInputBox::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	bool bFoop = bCanBeCrushed && (CVarCrushThem.GetValueOnAnyThread() > 0.0f);

	if (bFoop)
	{
		const float AlottedWidth = AllottedGeometry.GetLocalSize().X;

		const float CrushBelow = CVarStartCrushWhenBelow.GetValueOnAnyThread();
		const float StopCrushing = CVarStopCrushWhenAbove.GetValueOnAnyThread();

		if (bIsBeingCrushed)
		{
			bIsBeingCrushed = AlottedWidth < StopCrushing;
		}
		else
		{
			bIsBeingCrushed = AlottedWidth < CrushBelow;
		}
	}
	else
	{
		bIsBeingCrushed = false;
	}

	SCompoundWidget::OnArrangeChildren(AllottedGeometry, ArrangedChildren);
}

#undef LOCTEXT_NAMESPACE
