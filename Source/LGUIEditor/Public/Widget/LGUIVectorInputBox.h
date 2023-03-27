// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Margin.h"
#include "Fonts/SlateFontInfo.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "Styling/CoreStyle.h"
#include "Framework/SlateDelegates.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Widgets/Input/NumericTypeInterface.h"

#undef _X //@todo vogel: needs to be removed once Slate no longer uses _##Name

class FArrangedChildren;
class SHorizontalBox;

/**
 * Vector Slate control
 */
class SLGUIVectorInputBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGUIVectorInputBox)
		: _Font( FCoreStyle::Get().GetFontStyle("NormalFont") )
		, _AllowSpin(true)
		, _SpinDelta(1)
		, _bColorAxisLabels(false)
		, _ShowX(true)
		, _ShowY(false)
		, _ShowZ(false)
		, _ShowW(false)
		{}

		SLATE_ATTRIBUTE( TOptional<float>, X )
		SLATE_ATTRIBUTE( TOptional<float>, Y )
		SLATE_ATTRIBUTE( TOptional<float>, Z )
		SLATE_ATTRIBUTE( TOptional<float>, W )

		/** Font to use for the text in this box */
		SLATE_ATTRIBUTE( FSlateFontInfo, Font )

		/** Whether or not values can be spun or if they should be typed in */
		SLATE_ARGUMENT(bool, AllowSpin)

		/** The delta amount to apply, per pixel, when the spinner is dragged. */
		SLATE_ATTRIBUTE( float, SpinDelta )

		/** Should the axis labels be colored */
		SLATE_ARGUMENT( bool, bColorAxisLabels )	

		SLATE_ATTRIBUTE(bool, EnableX)
		SLATE_ATTRIBUTE(bool, EnableY)
		SLATE_ATTRIBUTE(bool, EnableZ)
		SLATE_ATTRIBUTE(bool, EnableW)	

		SLATE_ARGUMENT(bool, ShowX)
		SLATE_ARGUMENT(bool, ShowY)
		SLATE_ARGUMENT(bool, ShowZ)
		SLATE_ARGUMENT(bool, ShowW)

		SLATE_EVENT( FOnFloatValueChanged, OnXChanged )
		SLATE_EVENT( FOnFloatValueChanged, OnYChanged )
		SLATE_EVENT( FOnFloatValueChanged, OnZChanged )
		SLATE_EVENT( FOnFloatValueChanged, OnWChanged )

		SLATE_EVENT( FOnFloatValueCommitted, OnXCommitted )
		SLATE_EVENT( FOnFloatValueCommitted, OnYCommitted )
		SLATE_EVENT( FOnFloatValueCommitted, OnZCommitted )
		SLATE_EVENT( FOnFloatValueCommitted, OnWCommitted )

		/** Menu extender delegate for the X value */
		SLATE_EVENT( FMenuExtensionDelegate, ContextMenuExtenderX )

		/** Menu extender delegate for the Y value */
		SLATE_EVENT( FMenuExtensionDelegate, ContextMenuExtenderY )

		/** Menu extender delegate for the Z value */
		SLATE_EVENT( FMenuExtensionDelegate, ContextMenuExtenderZ )

		/** Menu extender delegate for the Z value */
		SLATE_EVENT( FMenuExtensionDelegate, ContextMenuExtenderW )

		/** Called right before the slider begins to move for any of the vector components */
		SLATE_EVENT( FSimpleDelegate, OnBeginSliderMovement )
		
		/** Called right after the slider handle is released by the user for any of the vector components */
		SLATE_EVENT( FOnFloatValueChanged, OnEndSliderMovement )

		/** Provide custom type functionality for the vector */
		SLATE_ARGUMENT( TSharedPtr< INumericTypeInterface<float> >, TypeInterface )

	SLATE_END_ARGS()

	/**
	 * Construct this AnchorData
	 *
	 * @param	InArgs	The declaration data for this AnchorData
	 */
	void Construct( const FArguments& InArgs );
private:
	/** Returns the index of the label widget to use (crushed or uncrushed) */	/** Creates a decorator label (potetially adding a switcher widget if this is cruhsable) */
	void ConstructX(const FArguments& InArgs, TSharedRef<SHorizontalBox> HorizontalBox);
	void ConstructY(const FArguments& InArgs, TSharedRef<SHorizontalBox> HorizontalBox);
	void ConstructZ(const FArguments& InArgs, TSharedRef<SHorizontalBox> HorizontalBox);
	void ConstructW(const FArguments& InArgs, TSharedRef<SHorizontalBox> HorizontalBox);
};
