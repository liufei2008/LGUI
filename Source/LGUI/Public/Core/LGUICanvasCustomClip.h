// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUICanvasCustomClip.generated.h"

/** Provide ability to do custom clip for LGUICanvas */
UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API ULGUICanvasCustomClip : public UObject
{
	GENERATED_BODY()
public:
	ULGUICanvasCustomClip();
	/** Called when LGUICanvas process BeginPlay. */
	virtual void Init();
	/** Called by LGUICanvas to get the clip material. */
	virtual UMaterialInterface* GetReplaceMaterial(UMaterialInterface* InMaterial);

	/** Called when set material's clip parameter. */
	virtual void ApplyMaterialParameter(UMaterialInstanceDynamic* InMaterial, class ULGUICanvas* InCanvas, class UUIItem* InUIItem);
	/** Called when event system do raycast hit test. */
	virtual bool CheckPointVisible(const FVector& InWorldPoint, class ULGUICanvas* InCanvas, class UUIItem* InUIItem);
protected:

	/** use this to tell if the class is compiled from blueprint, only blueprint can execute ReceiveXXX. */
	bool bCanExecuteBlueprintEvent = false;
	/** Called when LGUICanvas process BeginPlay. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Init"), Category = "LGUI")
		void ReceiveInit();
	/** Called when set material's clip parameter. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ApplyMaterialParameter"), Category = "LGUI")
		void ReceiveApplyMaterialParameter(UMaterialInstanceDynamic* InMaterial, class ULGUICanvas* InCanvas, class UUIItem* InUIItem);
	/** Called when event system do raycast hit test. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "CheckPointVisible"), Category = "LGUI")
		bool ReceiveCheckPointVisible(const FVector& InWorldPoint, class ULGUICanvas* InCanvas, class UUIItem* InUIItem);

	/** Map from standard none-clip material to clip material */
	UPROPERTY(EditAnywhere, Category = LGUI)
		TMap<UMaterialInterface*, UMaterialInterface*> replaceMaterialMap;
};

UCLASS(ClassGroup = LGUI, Blueprintable)
class LGUI_API ULGUICanvasCustomClip_Circle: public ULGUICanvasCustomClip
{
	GENERATED_BODY()
public:
	ULGUICanvasCustomClip_Circle();
	virtual void ApplyMaterialParameter(UMaterialInstanceDynamic* InMaterial, class ULGUICanvas* InCanvas, class UUIItem* InUIItem)override;
	virtual bool CheckPointVisible(const FVector& InWorldPoint, class ULGUICanvas* InCanvas, class UUIItem* InUIItem)override;
private:
	/** 1- full radius size, 0- zero radius */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (UIMin=0.0, UIMax = 1.0))
		float sizeMultiply = 1.0f;
	TWeakObjectPtr<UMaterialInstanceDynamic> cacheMaterial = nullptr;
	FLinearColor cacheParam_CenterAndSize;
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetSizeMultiply()const { return sizeMultiply; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSizeMultiply(float value);
};
