// Copyright 2025-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LexLayout.h"
#include "LexLayoutContainerFlexBox.generated.h"

class ULexLayoutSelfFlexBox;
class ULexWidget;

UENUM(BlueprintType, Category = LGUI)
enum class ELexLayoutFlexBoxDirectionType :uint8
{
	Horizontal,
	HorizontalReverse,
	Vertical,
	VerticalReverse,
};

UENUM(BlueprintType, Category = LGUI)
enum class ELexLayoutFlexBoxWrapType :uint8
{
	NoWrap,
	Wrap,
	WrapReverse,
};

UENUM(BlueprintType, Category = LGUI)
enum class ELexLayoutFlexBoxPrimaryAxisAlignment :uint8
{
	Start,
	Center,
	End,
	SpaceBetween,
	SpaceAround,
	SpaceEvenly,
};

UENUM(BlueprintType, Category = LGUI)
enum class ELexLayoutFlexBoxSecondaryAxisAlignment :uint8
{
	Start,
	Center,
	End,
	SpaceBetween,
	SpaceAround,
	SpaceEvenly,
	/**
	* Aligns a container's lines within the container when there is extra space in the secondary-axis, similar to how 'PrimaryAlignment' aligns individual items within the primary-axis.
	*/
	Stretch,
};

UENUM(BlueprintType, Category = LGUI)
enum class ELexLayoutFlexBoxSecondaryAxisLineAlignment :uint8
{
	Start,
	Center,
	End,
	/**
	 * Expand size to fill all area.
	 */
	Stretch,
};

/**
 * This layout act like html/css3-flex layout
 */
UCLASS(BlueprintType, DisplayName="LayoutContainer-FlexBox")
class LGUI_API ULexLayoutContainerFlexBox : public ULexLayoutContainer
{
	GENERATED_BODY()
private:
	/**
	 * Specifies how items are placed in the container, by setting the direction of the container's primary axis.
	 * Direction defines the primary axis, if Direction is Horizontal or HorizontalReversed then secondary axis would be vertical.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LayoutContainer", meta = (AllowPrivateAccess = true))
	ELexLayoutFlexBoxDirectionType Direction = ELexLayoutFlexBoxDirectionType::Horizontal;
	/**
	 * Controls whether the container is single-line or multi-line, and the direction of the secondary-axis, which determines the direction new lines are stacked in.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LayoutContainer", meta = (AllowPrivateAccess = true))
	ELexLayoutFlexBoxWrapType Wrap = ELexLayoutFlexBoxWrapType::NoWrap;
	/**
	 * Know as justify-content. Aligns items along the primary axis of the current line of the container.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LayoutContainer", meta = (AllowPrivateAccess = true))
	ELexLayoutFlexBoxPrimaryAxisAlignment PrimaryAlignment = ELexLayoutFlexBoxPrimaryAxisAlignment::Start;
	/**
	 * Know as align-content. Aligns a container's lines within the container when there is extra space in the secondary-axis, similar to how 'PrimaryAlignment' aligns individual items within the primary-axis.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LayoutContainer", meta = (AllowPrivateAccess = true))
	ELexLayoutFlexBoxSecondaryAxisAlignment SecondaryAlignment = ELexLayoutFlexBoxSecondaryAxisAlignment::Start;
	/**
	 * Know as align-items. Aligns items along the secondary-axis of the current line of the container.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LayoutContainer", meta = (AllowPrivateAccess = true))
	ELexLayoutFlexBoxSecondaryAxisLineAlignment SecondaryLineAlignment = ELexLayoutFlexBoxSecondaryAxisLineAlignment::Start;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LayoutContainer", meta = (AllowPrivateAccess = true, UIMin=0))
	float WidthGap = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LayoutContainer", meta = (AllowPrivateAccess = true, UIMin=0))
	float HeightGap = 0;
	/** Expands inwards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LayoutContainer", meta = (AllowPrivateAccess = true, UIMin=0))
	FMargin Padding;

	virtual void CalculateLayout() override;
	void DoCalculate(bool bApplyResult);
	void CalculatePreferredSize();
	
	struct FLineData
	{
		FLineData()
		{
			TotalMin = FVector2f::Zero();
			TotalMax = FVector2f::Zero();
			TotalPreferred = FVector2f::Zero();
			TotalGrow = FVector2f::Zero();
			TotalShrink = FVector2f::Zero();
		}
		TArray<ULexWidget*> Children;
		FVector2f TotalMin;
		FVector2f TotalMax;
		FVector2f TotalPreferred;
		FVector2f TotalGrow;
		FVector2f TotalShrink;
	};
	TArray<FLineData> LineDataArray;
	FVector2f TotalPreferredSize;

	virtual FLexLayoutControlAnchorData GetLayoutControlAnchor(const ULexWidget* TargetWidget)const override;
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual FVector2f GetLayoutPreferredSize() override;

	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	ELexLayoutFlexBoxDirectionType GetDirection()const{return Direction;}
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	ELexLayoutFlexBoxWrapType GetWrap()const{return Wrap;}
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	ELexLayoutFlexBoxPrimaryAxisAlignment GetPrimaryAlignment()const{return PrimaryAlignment;}
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	ELexLayoutFlexBoxSecondaryAxisAlignment GetSecondaryAlignment()const{return SecondaryAlignment;}
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	ELexLayoutFlexBoxSecondaryAxisLineAlignment GetSecondaryLineAlignment()const{return SecondaryLineAlignment;}
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	float GetWidthGap()const{return WidthGap;}
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	float GetHeightGap()const{return HeightGap;}
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	const FMargin& GetPadding()const{return Padding;}
	
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	void SetDirection(ELexLayoutFlexBoxDirectionType Value);
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	void SetWrap(ELexLayoutFlexBoxWrapType Value);
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	void SetPrimaryAlignment(ELexLayoutFlexBoxPrimaryAxisAlignment Value);
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	void SetSecondaryAlignment(ELexLayoutFlexBoxSecondaryAxisAlignment Value);
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	void SetSecondaryLineAlignment(ELexLayoutFlexBoxSecondaryAxisLineAlignment Value);
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	void SetWidthGap(float Value);
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	void SetHeightGap(float Value);
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	void SetPadding(const FMargin& Value);
};
