// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ILGUILayoutElementInterface.generated.h"

UENUM(BlueprintType, Category = LGUI)
enum class ELayoutElementType :uint8
{
	/** AutoSize, after ConstantSize and RatioSize are allocated, the left size is for AutoSize. euqal to without LayoutElement. */
	AutoSize,
	/** Ignore parent Layout. */
	IgnoreLayout,
	/** Constant size, need GetConstantSize() */
	ConstantSize,
	/** RatioSize, set size by ratio, total size is 1.0, need GetRatioSize() */
	RatioSize,
};

UENUM(BlueprintType, Category = LGUI)
enum class ELayoutElementSizeType : uint8
{
	/** Horizontal size, width */
	Horizontal,
	/** Vertical size, height */
	Vertical,
};

/**
 * Interface to define how this element position and sized in parent layout
 */
UINTERFACE(Blueprintable, MinimalAPI)
class ULGUILayoutElementInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface to define how this element position and sized in parent layout
 */ 
class LGUI_API ILGUILayoutElementInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout Element")
		ELayoutElementType GetLayoutType()const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout Element")
		float GetConstantSize(ELayoutElementSizeType type)const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout Element")
		float GetRatioSize(ELayoutElementSizeType type)const;
};