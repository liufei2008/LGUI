// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUICanvasCustomClip.generated.h"

class UMaterialInterface;

/** Provide ability to do custom clip for LGUICanvas */
UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API ULGUICanvasCustomClip : public UObject
{
	GENERATED_BODY()
public:
	ULGUICanvasCustomClip();
	/** Called by LGUICanvas to get the clip material. */
	virtual UMaterialInterface* GetReplaceMaterial(UMaterialInterface* InMaterial);
	/**
	 * Called when set material's clip parameter. This is the place to set your clip parameter to material.
	 * @param InMaterial	Clip material, set your clip parameter to this material.
	 * @param InCanvas		The LGUICanvas component object reference, which call this function.
	 * @param InUIItem		The UIItem where the LGUICanvas attach with.
	 */
	virtual void ApplyMaterialParameter(UMaterialInstanceDynamic* InMaterial, class ULGUICanvas* InCanvas, class UUIItem* InUIItem);
	/**
	 * Called when event system do raycast hit test. This is the function that check mouse/ray hit test for interaction.
	 * @param InWorldPoint	Line trace hit point on UI element's rect area.
	 * @param InCanvas		The LGUICanvas component object reference, which call this function.
	 * @param InUIItem		The UIItem where the LGUICanvas attach with.
	 * @return true if the point is at visible area, false otherwise.
	 */
	virtual bool CheckPointVisible(const FVector& InWorldPoint, class ULGUICanvas* InCanvas, class UUIItem* InUIItem);
	/**
	 * Called before LGUICanvas decide to create MaterialInstanceDynamic or not, depend on this return value.
	 * @param InMaterial The material to check.
	 * @return true- If the material contains this clip type's parameter.
	 */
	virtual bool MaterialContainsClipParameter(UMaterialInterface* InMaterial);
protected:

	/** use this to tell if the class is compiled from blueprint, only blueprint can execute ReceiveXXX. */
	bool bCanExecuteBlueprintEvent = false;
	/** Called when LGUICanvas process BeginPlay. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Init"), Category = "LGUI")
		void ReceiveInit();
	/**
	 * Called when set material's clip parameter. This is the place to set your clip parameter to material.
	 * @param InMaterial	Clip material, set your clip parameter to this material.
	 * @param InCanvas		The LGUICanvas component object reference, which call this function.
	 * @param InUIItem		The UIItem where the LGUICanvas attach with.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ApplyMaterialParameter"), Category = "LGUI")
		void ReceiveApplyMaterialParameter(UMaterialInstanceDynamic* InMaterial, class ULGUICanvas* InCanvas, class UUIItem* InUIItem);
	/**
	 * Called when event system do raycast hit test. This is the function that check mouse/ray hit test for interaction.
	 * @param InWorldPoint	Line trace hit point on UI element's rect area.
	 * @param InCanvas		The LGUICanvas component object reference, which call this function.
	 * @param InUIItem		The UIItem where the LGUICanvas attach with.
	 * @return true if the point is at visible area, false otherwise.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "CheckPointVisible"), Category = "LGUI")
		bool ReceiveCheckPointVisible(const FVector& InWorldPoint, class ULGUICanvas* InCanvas, class UUIItem* InUIItem);
	/**
	 * Called before LGUICanvas decide to create MaterialInstanceDynamic or not, depend on this return value.
	 * @param InMaterial The material to check.
	 * @return true- If the material contains this clip type's parameter.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "MaterialContainsClipParameter"), Category = "LGUI")
		bool ReceiveMaterialContainsClipParameter(UMaterialInterface* InMaterial);

	/** Map from standard none-clip material to clip material */
	UPROPERTY(EditAnywhere, Category = LGUI)
		TMap<TObjectPtr<UMaterialInterface>, TObjectPtr<UMaterialInterface>> replaceMaterialMap;
};

UCLASS(ClassGroup = LGUI, Blueprintable)
class LGUI_API ULGUICanvasCustomClip_Circle: public ULGUICanvasCustomClip
{
	GENERATED_BODY()
public:
	ULGUICanvasCustomClip_Circle();
	virtual void ApplyMaterialParameter(UMaterialInstanceDynamic* InMaterial, class ULGUICanvas* InCanvas, class UUIItem* InUIItem)override;
	virtual bool CheckPointVisible(const FVector& InWorldPoint, class ULGUICanvas* InCanvas, class UUIItem* InUIItem)override;
	virtual bool MaterialContainsClipParameter(UMaterialInterface* InMaterial)override;
private:
	/** 1- full radius size, 0- zero radius */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (UIMin=0.0, UIMax = 1.0))
		float sizeMultiply = 1.0f;
	static FName CenterAndSizeParameterName;
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetSizeMultiply()const { return sizeMultiply; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSizeMultiply(float value);
};

UENUM(BlueprintType)
enum class ELGUICanvasCustomClip_RoundedRect_UnitMode : uint8
{
	/** Direct value */
	Value			,
	/** Percent with rect's size from 0 to 100 */
	Percentage		,
};

UCLASS(ClassGroup = LGUI, Blueprintable)
class LGUI_API ULGUICanvasCustomClip_RoundedRect : public ULGUICanvasCustomClip
{
	GENERATED_BODY()
public:
	ULGUICanvasCustomClip_RoundedRect();
	virtual void ApplyMaterialParameter(UMaterialInstanceDynamic* InMaterial, class ULGUICanvas* InCanvas, class UUIItem* InUIItem)override;
	virtual bool CheckPointVisible(const FVector& InWorldPoint, class ULGUICanvas* InCanvas, class UUIItem* InUIItem)override;
	virtual bool MaterialContainsClipParameter(UMaterialInterface* InMaterial)override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty)const override;
#endif
private:
	friend class FLGUICanvasCustomClip_RoundedRect_Customization;
	UPROPERTY(EditAnywhere, Category = LGUI)
		FVector4f CornerRadius = FVector4f(1, 1, 1, 1);
	UPROPERTY(EditAnywhere, Category = LGUI)
		ELGUICanvasCustomClip_RoundedRect_UnitMode CornerRadiusUnitMode = ELGUICanvasCustomClip_RoundedRect_UnitMode::Percentage;
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bUniformSetCornerRadius = true;
#endif
	static FName CenterAndSizeParameterName;
	static FName CornerRadiusParameterName;
	void ApplyMaterialParameter(UMaterialInstanceDynamic* InMaterial, const FLinearColor& InCenterAndSize, const FLinearColor& InConerRadius);
	void ApplyMaterialParameterOnCanvas();
	FVector4f GetCornerRadiusWithUnitMode(float RectWidth, float RectHeight, float AdditionalScale);
	void OnCornerRadiusUnitModeChanged(float width, float height);
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FVector4 GetCornerRadius()const { return (FVector4)CornerRadius; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUICanvasCustomClip_RoundedRect_UnitMode GetCornerRadiusUnitMode()const { return CornerRadiusUnitMode; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetCornerRadius(const FVector4& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetCornerRadiusUnitMode(ELGUICanvasCustomClip_RoundedRect_UnitMode value);
};