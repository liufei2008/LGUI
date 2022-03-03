// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "Core/UIGeometry.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "Components/ActorComponent.h"
#include "UIGeometryModifierBase.generated.h"

class UUIBatchGeometryRenderable;
class UUIText;

UENUM(BlueprintType)
enum class ELGUIGeometryModifierHelper_UITextModifyPositionType:uint8
{
	//Relative to character's origin position
	Relative,
	//Direct set character's position, relative to UIText's pivot position
	Absolute,
};
/** a helper class for UIGeometryModifierBase to easily modify ui geometry */
UCLASS(BlueprintType)
class LGUI_API ULGUIGeometryModifierHelper : public ULGUIGeometryHelper
{
	GENERATED_BODY()
public:
	/** Get character's center position in UIText's rect range, and convert to 0-1 range (left is 0 and right is 1) */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float UITextHelperFunction_GetCharHorizontalPositionRatio01(UUIText* InUIText, int InCharIndex)const;
	/**
	 * Modify character's position & rotation & scale
	 * @param	InPositionType		Set position type, relative to origin position or absolute position
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void UITextHelperFunction_ModifyCharGeometry_Transform(UUIText* InUIText, int InCharIndex
			, ELGUIGeometryModifierHelper_UITextModifyPositionType InPositionType
			, const FVector& InPosition
			, const FRotator& InRotator = FRotator::ZeroRotator
			, const FVector& InScale = FVector(1,1,1)
		);
	/**
	 * Get character's pivot position relative to UIText's pivot position
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void UITextHelperFunction_GetCharGeometry_AbsolutePosition(UUIText* InUIText, int InCharIndex, FVector& OutPosition)const;
	/**
	 * Modify character's position
	 * @param	InPositionType		Set position type, relative to origin position or absolute position
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void UITextHelperFunction_ModifyCharGeometry_Position(UUIText* InUIText, int InCharIndex
			, const FVector& InPosition, ELGUIGeometryModifierHelper_UITextModifyPositionType InPositionType
		);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void UITextHelperFunction_ModifyCharGeometry_Rotate(UUIText* InUIText, int InCharIndex, const FRotator& InRotator = FRotator::ZeroRotator);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void UITextHelperFunction_ModifyCharGeometry_Scale(UUIText* InUIText, int InCharIndex, const FVector& InScale = FVector(1, 1, 1));
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void UITextHelperFunction_ModifyCharGeometry_Color(UUIText* InUIText, int InCharIndex, const FColor& InColor);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void UITextHelperFunction_ModifyCharGeometry_Alpha(UUIText* InUIText, int InCharIndex, const float& InAlpha);
};

/** 
 * For modify ui geometry, act like a filter.
 * Need UIBatchGeometryRenderable component.
 */
UCLASS(Abstract, Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIGeometryModifierBase : public UActorComponent
{
	GENERATED_BODY()

public:	
	UUIGeometryModifierBase();

protected:
	virtual void BeginPlay()override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnRegister()override;
	virtual void OnUnregister()override;

	/** Enable this geometry modifier */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bEnable = true;
	/** Execute order of this effect in actor. Smaller executeOrder will execute eailer */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int executeOrder = 0;

private:
	mutable TWeakObjectPtr<UUIBatchGeometryRenderable> UIRenderable;
	void RemoveFromUIBatchGeometry();
	void AddToUIBatchGeometry();
public:
	UE_DEPRECATED(4.24, "Use GetUIRenderable instead.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use GetUIRenderable instead."))
		UUIBatchGeometryRenderable* GetRenderableUIItem()const { return GetUIRenderable(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIBatchGeometryRenderable* GetUIRenderable()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetEnable()const { return bEnable; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetExecuteOrder()const { return executeOrder; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnable(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetExecuteOrder();
	/**
	 * Modify UI geometry's vertex and triangle.
	 * @param	InTriangleChanged		triangle changed
	 * @param	InUVChanged			vertex uv changed
	 * @param	InColorChanged			vertex color changed
	 * @param	InVertexPositionChanged			vertex position changed
	 * @param	InTransformChanged			object's transform changed
	 */
	virtual void ModifyUIGeometry(
		UIGeometry& InGeometry
		, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
	);
	/**
	 * Will this modifier affect these geometry data? Save some calculation if not affect.
	 * For blueprint just make all to true, for easier use.
	 */
	virtual void ModifierWillChangeVertexData(bool& OutTriangleIndices, bool& OutVertexPosition, bool& OutUV, bool& OutColor)
	{
		OutTriangleIndices = true;
		OutVertexPosition = true;
		OutUV = true;
		OutColor = true;
	}
protected:
	UPROPERTY(Transient) ULGUIGeometryModifierHelper* GeometryModifierHelper = nullptr;
	/**
	 * Modify UI geometry's vertex and triangle.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "ModifyUIGeometry"))
		void ReceiveModifyUIGeometry(ULGUIGeometryModifierHelper* InGeometryModifierHelper);
};
