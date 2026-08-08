// Copyright 2025-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LexLayout.h"
#include "LexLayoutSelfFlexBox.generated.h"

enum class ELexLayoutFlexBoxSecondaryAxisLineAlignment : uint8;
class ULexWidget;

UENUM(BlueprintType)
enum class ELexLayoutSizeType : uint8
{
	/**
	 * Auto get size value with these roles:
	 * 1. Get value from this widget's LayoutContainer.
	 * 2. If not 1, then get value from this widget's Visual.
	 * 3. If not 2, then fallback to Fixed.
	 */
	Auto,
	/** Fixed pixel value */
	Fixed,
	/**
	 * Percentage relative it's parent size. If parent size is auto and fit to children, then this size will get 0
	 * If no parent then fallback to 0.
	 */
	Percent,
};

UENUM(BlueprintType, Category = LGUI)
enum class ELexLayoutFlexBoxSecondaryAxisSelfAlignment :uint8
{
	Auto,
	Start,
	Center,
	End,
	/**
	 * Expand size to fill all area.
	 */
	Stretch,
};

USTRUCT(BlueprintType)
struct FLexLayoutSize
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
	bool bEnable = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
	ELexLayoutSizeType Type = ELexLayoutSizeType::Auto;
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	float AutoValue = 0;
#endif
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", meta = (EditCondition = "Type == ELexLayoutSizeType::Fixed", UIMin=0))
	float FixedValue = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", meta = (EditCondition = "Type == ELexLayoutSizeType::Percent", UIMin=0, UIMax=1))
	float PercentValue = 0.5;

	FLexLayoutSize(){}
	FLexLayoutSize(ELexLayoutSizeType Type, float Value)
	{
		this->bEnable = true;
		this->Type = Type;
		switch (Type)
		{
		default:
		case ELexLayoutSizeType::Fixed:
			this->FixedValue = Value;
			break;
		case ELexLayoutSizeType::Percent:
			this->PercentValue = Value;
			break;
		}
	}
	FLexLayoutSize(float PixelValue)
	{
		this->bEnable = true;
		this->Type = ELexLayoutSizeType::Fixed;
		this->FixedValue = PixelValue;
	}
	bool operator==(const FLexLayoutSize& Other) const
	{
		return this->Type == Other.Type && this->FixedValue == Other.FixedValue && this->PercentValue == Other.PercentValue;
	}
	bool operator!=(const FLexLayoutSize& Other) const
	{
		return this->Type != Other.Type || this->FixedValue != Other.FixedValue || this->PercentValue != Other.PercentValue;
	}

	float Calculate(ULexWidget* Widget, bool IsVertical)const;
};

UENUM(BlueprintType)
enum class ELexLayoutMinMaxSizeType : uint8
{
	/** Fixed pixel value */
	Fixed,
	/**
	 * Percentage relative it's parent size.
	 * If no parent then fallback to Not-Enabled.
	 */
	Percent,
};

USTRUCT(BlueprintType)
struct FLexLayoutMinMaxSize
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
	bool bEnable = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
	ELexLayoutMinMaxSizeType Type = ELexLayoutMinMaxSizeType::Fixed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", meta = (EditCondition = "Type == ELexLayoutMinMaxSizeType::Fixed", UIMin=0))
	float FixedValue = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", meta = (EditCondition = "Type == ELexLayoutMinMaxSizeType::Percent", UIMin=0, UIMax=100))
	float PercentValue = 0.5f;

	FLexLayoutMinMaxSize(){}
	FLexLayoutMinMaxSize(ELexLayoutMinMaxSizeType Type, float Value)
	{
		this->bEnable = true;
		this->Type = Type;
		switch (Type)
		{
		default:
		case ELexLayoutMinMaxSizeType::Fixed:
			this->FixedValue = Value;
			break;
		case ELexLayoutMinMaxSizeType::Percent:
			this->PercentValue = Value;
			break;
		}
	}
	FLexLayoutMinMaxSize(float Value)
	{
		this->bEnable = true;
		this->Type = ELexLayoutMinMaxSizeType::Fixed;
		this->FixedValue = Value;
	}
	bool operator==(const FLexLayoutMinMaxSize& Other) const
	{
		return this->Type == Other.Type && this->FixedValue == Other.FixedValue && this->PercentValue == Other.PercentValue;
	}
	bool operator!=(const FLexLayoutMinMaxSize& Other) const
	{
		return this->Type != Other.Type || this->FixedValue != Other.FixedValue || this->PercentValue != Other.PercentValue;
	}

	float Calculate(ULexWidget* Widget, bool IsVertical, bool IsMinOrMax)const;
};

/**
 * Provide item properties for FlexBoxContainer, and we can also use it independently to easily control self size.
 */
UCLASS(BlueprintType, DisplayName="LayoutSelf-FlexBox")
class LGUI_API ULexLayoutSelfFlexBox : public ULexLayoutSelf
{
	GENERATED_BODY()
private:
	friend class FLexLayoutSelfFlexBoxCustomization;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LayoutSelf", Getter, Setter, meta = (AllowPrivateAccess = true))
	FLexLayoutSize PreferredWidth;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LayoutSelf", Getter, Setter, meta = (AllowPrivateAccess = true))
	FLexLayoutSize PreferredHeight;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LayoutSelf", Getter, Setter, meta = (AllowPrivateAccess = true))
	FLexLayoutMinMaxSize MinWidth;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LayoutSelf", Getter, Setter, meta = (AllowPrivateAccess = true))
	FLexLayoutMinMaxSize MinHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LayoutSelf", Getter, Setter, meta = (AllowPrivateAccess = true))
	FLexLayoutMinMaxSize MaxWidth;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LayoutSelf", Getter, Setter, meta = (AllowPrivateAccess = true))
	FLexLayoutMinMaxSize MaxHeight;

	/** Expands outwards. Only valid when parent have FlexBoxContainer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout", meta = (AllowPrivateAccess = true, UIMin=0))
	FMargin Margin;

	/**
	 * Grow size when FlexBoxContainer's primary-axis can provide extra space, not working with secondary axis.
	 * Only valid when PreferredWidth/Height is enabled.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LayoutSelf", Getter, Setter, meta = (AllowPrivateAccess = true, UIMin=0))
	float Grow = 0;
	/**
	 * Shrink size when FlexBoxContainer's primary-axis can't provide enough space, not working with secondary axis.
	 * Only valid when PreferredWidth/Height is enabled.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LayoutSelf", Getter, Setter, meta = (AllowPrivateAccess = true, UIMin=0))
	float Shrink = 0;

	/**
	 * Know as align-self. Aligns flex items along the secondary-axis of the current line of the flex container.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LayoutSelf", Getter, Setter, meta = (AllowPrivateAccess = true, UIMin=0))
	ELexLayoutFlexBoxSecondaryAxisSelfAlignment SelfAlignment = ELexLayoutFlexBoxSecondaryAxisSelfAlignment::Auto;

	float CalculatedMinWidth = 0;
	float CalculatedMinHeight = 0;
	float CalculatedMaxWidth = 0;
	float CalculatedMaxHeight = 0;
	float CalculatedPreferredWidth = 0;
	float CalculatedPreferredHeight = 0;
	float CalculatedFinalWidth = 0;
	float CalculatedFinalHeight = 0;
public:
	static FName GetPropertyName_PreferredWidth();
	static FName GetPropertyName_PreferredHeight();
	static FName GetPropertyName_MinWidth();
	static FName GetPropertyName_MinHeight();
	static FName GetPropertyName_MaxWidth();
	static FName GetPropertyName_MaxHeight();
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostInitProperties() override;
#endif
	virtual FLexLayoutControlAnchorData GetLayoutControlAnchor(const ULexWidget* Widget)const override;
	virtual FVector2f GetLayoutPreferredSize() override;
	virtual void CalculateSize() override;
	virtual void MarkLayoutDirty() override;
	void GetLayoutMinMax(FVector2f& OutMin, FVector2f& OutMax);
	void RebuildSelfLayout();
	
	float GetGrowForLayoutContainer(int Axis)const;
	float GetShrinkForLayoutContainer(int Axis)const;
	ELexLayoutFlexBoxSecondaryAxisLineAlignment GetAlignmentForLayoutContainer(ELexLayoutFlexBoxSecondaryAxisLineAlignment DefaultAlignment)const;
	bool GetSecondaryAxisSizeCanStretchByLayoutContainer(int SecondaryAxis)const;
	void SetFinalSizeByLayoutContainer(FVector2f Value);
	
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	const FLexLayoutSize& GetPreferredWidth()const{return PreferredWidth;}
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	const FLexLayoutSize& GetPreferredHeight()const{return PreferredHeight;}
	
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	const FLexLayoutMinMaxSize& GetMinWidth()const{return MinWidth;}
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	const FLexLayoutMinMaxSize& GetMinHeight()const{return MinHeight;}

	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	const FLexLayoutMinMaxSize& GetMaxWidth()const{return MaxWidth;}
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	const FLexLayoutMinMaxSize& GetMaxHeight()const{return MaxHeight;}
	
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	const FMargin& GetMargin()const{return Margin;}
	
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	float GetGrow()const{return Grow;}
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	float GetShrink()const{return Shrink;}
	
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	void SetPreferredWidth(const FLexLayoutSize& Value);
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	void SetPreferredHeight(const FLexLayoutSize& Value);
	
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	void SetMinWidth(const FLexLayoutMinMaxSize& Value);
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	void SetMinHeight(const FLexLayoutMinMaxSize& Value);

	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	void SetMaxWidth(const FLexLayoutMinMaxSize& Value);
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	void SetMaxHeight(const FLexLayoutMinMaxSize& Value);

	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	void SetMargin(const FMargin& Value);
	
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	void SetGrow(float Value);
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	void SetShrink(float Value);
	
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	ELexLayoutFlexBoxSecondaryAxisSelfAlignment GetSelfAlignment()const{return SelfAlignment;}
	UFUNCTION(BlueprintCallable, Category = "LayoutSelf")
	void SetSelfAlignment(ELexLayoutFlexBoxSecondaryAxisSelfAlignment Value);
};
