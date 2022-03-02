// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UISpriteBase.h"
#include "UISprite.generated.h"

UENUM(BlueprintType, Category = LGUI)
enum class UISpriteType :uint8
{
	Normal,
	Sliced,
	SlicedFrame,
	Tiled,
	Filled,
};
UENUM(BlueprintType, Category = LGUI)
enum class UISpriteFillMethod:uint8
{
	Horizontal,
	Vertical,
	Radial90,
	Radial180,
	Radial360,
};
UENUM(BlueprintType, Category = LGUI)
enum class UISpriteFillOriginType_Radial90 :uint8 
{
	BottomLeft,
	TopLeft,
	TopRight,
	BottomRight,
};
UENUM(BlueprintType, Category = LGUI)
enum class UISpriteFillOriginType_Radial180 :uint8
{
	Bottom,
	Left,
	Top,
	Right,
};
UENUM(BlueprintType, Category = LGUI)
enum class UISpriteFillOriginType_Radial360 :uint8
{
	Bottom,
	Right,
	Top,
	Left,
};
UCLASS(ClassGroup = (LGUI), NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUISprite : public UUISpriteBase
{
	GENERATED_BODY()

public:	
	UUISprite(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	friend class FUISpriteCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UISpriteType type = UISpriteType::Normal;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UISpriteFillMethod fillMethod = UISpriteFillMethod::Horizontal;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint8 fillOrigin = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool fillDirectionFlip = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float fillAmount = 1;
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")UISpriteFillOriginType_Radial90 fillOriginType_Radial90;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")UISpriteFillOriginType_Radial180 fillOriginType_Radial180;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")UISpriteFillOriginType_Radial360 fillOriginType_Radial360;
#endif

	virtual void OnAnchorChange(bool InPivotChange, bool InSizeChange, bool InDiscardCache = true)override;

	//width direction rectangel count, in tiled mode
	int32 Tiled_WidthRectCount = 0;
	//height direction rectangel count, in tiled mode
	int32 Tiled_HeightRectCount = 0;
	//width direction half rectangel size, in tiled mode
	float Tiled_WidthRemainedRectSize = 0;
	//height direction half rectangel size, in tiled mode
	float Tiled_HeightRemainedRectSize = 0;
	void CalculateTiledWidth();
	void CalculateTiledHeight();

	virtual void OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI") UISpriteType GetSpriteType()const { return type; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	UISpriteFillMethod GetFillMethod()const { return fillMethod; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	uint8 GetFillOrigin()const { return fillOrigin; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	bool GetFillDirectionFlip()const { return fillDirectionFlip; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	float GetFillAmount()const { return fillAmount; }

	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetSpriteType(UISpriteType newType);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillMethod(UISpriteFillMethod newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillOrigin(uint8 newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillDirectionFlip(bool newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillAmount(float newValue);
};
