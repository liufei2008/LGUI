// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ILGUILayoutInterface.generated.h"

USTRUCT(BlueprintType)
struct FLGUICanLayoutControlAnchor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "LGUI Layout") bool bCanControlHorizontalAnchor = false;
	UPROPERTY(EditAnywhere, Category = "LGUI Layout") bool bCanControlVerticalAnchor = false;
	UPROPERTY(EditAnywhere, Category = "LGUI Layout") bool bCanControlHorizontalAnchoredPosition = false;
	UPROPERTY(EditAnywhere, Category = "LGUI Layout") bool bCanControlVerticalAnchoredPosition = false;
	UPROPERTY(EditAnywhere, Category = "LGUI Layout") bool bCanControlHorizontalSizeDelta = false;
	UPROPERTY(EditAnywhere, Category = "LGUI Layout") bool bCanControlVerticalSizeDelta = false;

	bool HaveRepeatedControl(const FLGUICanLayoutControlAnchor& Other)
	{
		if (
			(bCanControlHorizontalAnchor && Other.bCanControlHorizontalAnchor)
			|| (bCanControlVerticalAnchor && Other.bCanControlVerticalAnchor)
			|| (bCanControlHorizontalAnchoredPosition && Other.bCanControlHorizontalAnchoredPosition)
			|| (bCanControlVerticalAnchoredPosition && Other.bCanControlVerticalAnchoredPosition)
			|| (bCanControlHorizontalSizeDelta && Other.bCanControlHorizontalSizeDelta)
			|| (bCanControlVerticalSizeDelta && Other.bCanControlVerticalSizeDelta)
			)
		{
			return true;
		}
		return false;
	}
	void Or(const FLGUICanLayoutControlAnchor& Other)
	{
		bCanControlHorizontalAnchor |= Other.bCanControlHorizontalAnchor;
		bCanControlVerticalAnchor |= Other.bCanControlVerticalAnchor;
		bCanControlHorizontalAnchoredPosition |= Other.bCanControlHorizontalAnchoredPosition;
		bCanControlVerticalAnchoredPosition |= Other.bCanControlVerticalAnchoredPosition;
		bCanControlHorizontalSizeDelta |= Other.bCanControlHorizontalSizeDelta;
		bCanControlVerticalSizeDelta |= Other.bCanControlVerticalSizeDelta;
	}
};

/**
 * Interface for handling LGUI's layout update.
 * Need to register UObject with RegisterLGUILayout, check UIBaseLayout for reference
 */
UINTERFACE(Blueprintable, MinimalAPI)
class ULGUILayoutInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LGUI's culture changed.
 * Need to register UObject with RegisterLGUILayout, check UIBaseLayout for reference
 */ 
class LGUI_API ILGUILayoutInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when need to update layout.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		void OnUpdateLayout();

	/**
	 * Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint
	 * @param	InUIItem	Target to check
	 * @param	OutResult	Result
	 * @return	true if the layout take care of the InUIItem.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool GetCanLayoutControlAnchor(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const;

	/**
	 * Mark this layout need to be rebuild.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		void MarkRebuildLayout();
};