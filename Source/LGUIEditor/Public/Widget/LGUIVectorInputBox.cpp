// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUIVectorInputBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "HAL/IConsoleManager.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

#define LOCTEXT_NAMESPACE "SLGUIVectorInputBox"

void SLGUIVectorInputBox::Construct( const FArguments& InArgs )
{
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
	TSharedRef<SWidget> LabelWidget = SNullWidget::NullWidget;
	if (InArgs._bColorAxisLabels)
	{
		const FLinearColor LabelColor = SNumericEntryBox<float>::RedLabelBackgroundColor;
		LabelWidget = SNumericEntryBox<float>::BuildNarrowColorLabel(LabelColor);
	}

	TAttribute<TOptional<float>> Value = InArgs._X;

	HorizontalBox->AddSlot()
	[
		SNew( SNumericEntryBox<float> )
		.AllowSpin(InArgs._AllowSpin)
		.Font( InArgs._Font )
		.Value( InArgs._X )
		.OnValueChanged( InArgs._OnXChanged )
		.OnValueCommitted( InArgs._OnXCommitted )		
		.IsEnabled(InArgs._EnableX)
		.ToolTipText(MakeAttributeLambda([=]
		{
			if (Value.Get().IsSet())
			{
				if (InArgs._EnableX.IsBound())
				{
					return
					InArgs._EnableX.Get()
					? FText::Format(LOCTEXT("X_ToolTip", "X: {0}"), Value.Get().GetValue())
					: LOCTEXT("X_ToolTip_Control", "X Value Controlled by LGUI's Anchors");
				}
				return FText::Format(LOCTEXT("X_ToolTip", "X: {0}"), Value.Get().GetValue());
			}
			return LOCTEXT("MultipleValues", "Multiple Values");
		}))
		.UndeterminedString(LOCTEXT("MultipleValues", "Multiple Values"))
		.ContextMenuExtender(InArgs._ContextMenuExtenderX)
		.TypeInterface(InArgs._TypeInterface)
		.MinValue(TOptional<float>())
		.MaxValue(TOptional<float>())
		.MinSliderValue(TOptional<float>())
		.MaxSliderValue(TOptional<float>())
		.LinearDeltaSensitivity(1)
		.Delta(InArgs._SpinDelta)
		.OnBeginSliderMovement(InArgs._OnBeginSliderMovement)
		.OnEndSliderMovement(InArgs._OnEndSliderMovement)
		.LabelPadding(FMargin(3))
		.LabelLocation(SNumericEntryBox<float>::ELabelLocation::Inside)
		.Label()
		[
			LabelWidget
		]
	];
	
}

void SLGUIVectorInputBox::ConstructY( const FArguments& InArgs, TSharedRef<SHorizontalBox> HorizontalBox )
{
	TSharedRef<SWidget> LabelWidget = SNullWidget::NullWidget;
	if (InArgs._bColorAxisLabels)
	{
		const FLinearColor LabelColor = SNumericEntryBox<float>::GreenLabelBackgroundColor;
		LabelWidget = SNumericEntryBox<float>::BuildNarrowColorLabel(LabelColor);
	}

	TAttribute<TOptional<float>> Value = InArgs._Y;

	HorizontalBox->AddSlot()
	[
		SNew( SNumericEntryBox<float> )
		.AllowSpin(InArgs._AllowSpin)
		.Font( InArgs._Font )
		.Value( InArgs._Y )
		.OnValueChanged( InArgs._OnYChanged )
		.OnValueCommitted( InArgs._OnYCommitted )
		.IsEnabled(InArgs._EnableY)
		.ToolTipText(MakeAttributeLambda([=]
		{
			if (Value.Get().IsSet())
			{
				if (InArgs._EnableY.IsBound())
				{
					return
					InArgs._EnableY.Get()
					? FText::Format(LOCTEXT("Y_ToolTip", "Y: {0}"), Value.Get().GetValue())
					: LOCTEXT("Y_ToolTip_Control", "Y Value Controlled by LGUI's Anchors");
				}
				return FText::Format(LOCTEXT("Y_ToolTip", "Y: {0}"), Value.Get().GetValue());
			}
			return LOCTEXT("MultipleValues", "Multiple Values");
		}))
		.UndeterminedString(LOCTEXT("MultipleValues", "Multiple Values"))
		.ContextMenuExtender(InArgs._ContextMenuExtenderY)
		.TypeInterface(InArgs._TypeInterface)
		.MinValue(TOptional<float>())
		.MaxValue(TOptional<float>())
		.MinSliderValue(TOptional<float>())
		.MaxSliderValue(TOptional<float>())
		.LinearDeltaSensitivity(1)
		.Delta(InArgs._SpinDelta)
		.OnBeginSliderMovement(InArgs._OnBeginSliderMovement)
		.OnEndSliderMovement(InArgs._OnEndSliderMovement)
		.LabelPadding(FMargin(3))
		.LabelLocation(SNumericEntryBox<float>::ELabelLocation::Inside)
		.Label()
		[
			LabelWidget
		]
	];

}

void SLGUIVectorInputBox::ConstructZ( const FArguments& InArgs, TSharedRef<SHorizontalBox> HorizontalBox )
{
	TSharedRef<SWidget> LabelWidget = SNullWidget::NullWidget;
	if (InArgs._bColorAxisLabels)
	{
		const FLinearColor LabelColor = SNumericEntryBox<float>::BlueLabelBackgroundColor;
		LabelWidget = SNumericEntryBox<float>::BuildNarrowColorLabel(LabelColor);
	}

	TAttribute<TOptional<float>> Value = InArgs._Z;

	HorizontalBox->AddSlot()
	[
		SNew( SNumericEntryBox<float> )
		.AllowSpin(InArgs._AllowSpin)
		.Font( InArgs._Font )
		.Value( InArgs._Z )
		.OnValueChanged( InArgs._OnZChanged )
		.OnValueCommitted( InArgs._OnZCommitted )
		.IsEnabled(InArgs._EnableZ)
		.ToolTipText(MakeAttributeLambda([=]
		{
			if (Value.Get().IsSet())
			{
				if (InArgs._EnableZ.IsBound())
				{
					return
					InArgs._EnableZ.Get()
					? FText::Format(LOCTEXT("Z_ToolTip", "Z: {0}"), Value.Get().GetValue())
					: LOCTEXT("Z_ToolTip_Control", "Z Value Controlled by LGUI's Anchors");
				}
				return FText::Format(LOCTEXT("Z_ToolTip", "Z: {0}"), Value.Get().GetValue());
			}
			return LOCTEXT("MultipleValues", "Multiple Values");
		}))
		.UndeterminedString(LOCTEXT("MultipleValues", "Multiple Values"))
		.ContextMenuExtender(InArgs._ContextMenuExtenderZ)
		.TypeInterface(InArgs._TypeInterface)
		.MinValue(TOptional<float>())
		.MaxValue(TOptional<float>())
		.MinSliderValue(TOptional<float>())
		.MaxSliderValue(TOptional<float>())
		.LinearDeltaSensitivity(1)
		.Delta(InArgs._SpinDelta)
		.OnBeginSliderMovement(InArgs._OnBeginSliderMovement)
		.OnEndSliderMovement(InArgs._OnEndSliderMovement)
		.LabelPadding(FMargin(3))
		.LabelLocation(SNumericEntryBox<float>::ELabelLocation::Inside)
		.Label()
		[
			LabelWidget
		]
	];
}

void SLGUIVectorInputBox::ConstructW(const FArguments& InArgs, TSharedRef<SHorizontalBox> HorizontalBox)
{
	TSharedRef<SWidget> LabelWidget = SNullWidget::NullWidget;
	if (InArgs._bColorAxisLabels)
	{
		const FLinearColor LabelColor = FLinearColor::Yellow;
		LabelWidget = SNumericEntryBox<float>::BuildNarrowColorLabel(LabelColor);
	}

	TAttribute<TOptional<float>> Value = InArgs._W;

	HorizontalBox->AddSlot()
	[
		SNew( SNumericEntryBox<float> )
		.AllowSpin(InArgs._AllowSpin)
		.Font( InArgs._Font )
		.Value( InArgs._W )
		.OnValueChanged( InArgs._OnWChanged )
		.OnValueCommitted( InArgs._OnWCommitted )
		.IsEnabled(InArgs._EnableW)
		.ToolTipText(MakeAttributeLambda([=]
		{
			if (Value.Get().IsSet())
			{
				if (InArgs._EnableW.IsBound())
				{
					return
					InArgs._EnableW.Get()
					? FText::Format(LOCTEXT("W_ToolTip", "W: {0}"), Value.Get().GetValue())
					: LOCTEXT("W_ToolTip_Control", "W Value Controlled by LGUI's Anchors");
				}
				return FText::Format(LOCTEXT("W_ToolTip", "W: {0}"), Value.Get().GetValue());
			}
			return LOCTEXT("MultipleValues", "Multiple Values");
		}))
		.UndeterminedString(LOCTEXT("MultipleValues", "Multiple Values"))
		.ContextMenuExtender(InArgs._ContextMenuExtenderW)
		.TypeInterface(InArgs._TypeInterface)
		.MinValue(TOptional<float>())
		.MaxValue(TOptional<float>())
		.MinSliderValue(TOptional<float>())
		.MaxSliderValue(TOptional<float>())
		.LinearDeltaSensitivity(1)
		.Delta(InArgs._SpinDelta)
		.OnBeginSliderMovement(InArgs._OnBeginSliderMovement)
		.OnEndSliderMovement(InArgs._OnEndSliderMovement)
		.LabelPadding(FMargin(3))
		.LabelLocation(SNumericEntryBox<float>::ELabelLocation::Inside)
		.Label()
		[
			LabelWidget
		]
	];
}

#undef LOCTEXT_NAMESPACE
