// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UISpriteBase.h"
#include "UISprite.generated.h"

UENUM(BlueprintType, Category = LGUI)
enum class EUISpriteType :uint8
{
	Normal,
	Sliced,
	SlicedFrame,
	Tiled,
	Filled,
};
#ifndef UISpriteType
#define UISpriteType DEPRECATED_MACRO(5.0, "UISpriteType has been renamed to EUISpriteType") EUISpriteType
#endif

UENUM(BlueprintType, Category = LGUI)
enum class EUISpriteFillMethod:uint8
{
	Horizontal,
	Vertical,
	Radial90,
	Radial180,
	Radial360,
};
#ifndef UISpriteFillMethod
#define UISpriteFillMethod DEPRECATED_MACRO(5.0, "UISpriteFillMethod has been renamed to EUISpriteFillMethod") EUISpriteFillMethod
#endif

UENUM(BlueprintType, Category = LGUI)
enum class EUISpriteFillOriginType_Radial90 :uint8 
{
	BottomLeft,
	TopLeft,
	TopRight,
	BottomRight,
};
#ifndef UISpriteFillOriginType_Radial90
#define UISpriteFillOriginType_Radial90 DEPRECATED_MACRO(5.0, "UISpriteFillOriginType_Radial90 has been renamed to EUISpriteFillOriginType_Radial90") EUISpriteFillOriginType_Radial90
#endif

UENUM(BlueprintType, Category = LGUI)
enum class EUISpriteFillOriginType_Radial180 :uint8
{
	Bottom,
	Left,
	Top,
	Right,
};
#ifndef UISpriteFillOriginType_Radial180
#define UISpriteFillOriginType_Radial180 DEPRECATED_MACRO(5.0, "UISpriteFillOriginType_Radial180 has been renamed to EUISpriteFillOriginType_Radial180") EUISpriteFillOriginType_Radial180
#endif

UENUM(BlueprintType, Category = LGUI)
enum class EUISpriteFillOriginType_Radial360 :uint8
{
	Bottom,
	Right,
	Top,
	Left,
};
#ifndef UISpriteFillOriginType_Radial360
#define UISpriteFillOriginType_Radial360 DEPRECATED_MACRO(5.0, "UISpriteFillOriginType_Radial360 has been renamed to EUISpriteFillOriginType_Radial360") EUISpriteFillOriginType_Radial360
#endif

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
		EUISpriteType type = EUISpriteType::Normal;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUISpriteFillMethod fillMethod = EUISpriteFillMethod::Horizontal;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint8 fillOrigin = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool fillDirectionFlip = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float fillAmount = 1;
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")EUISpriteFillOriginType_Radial90 fillOriginType_Radial90;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")EUISpriteFillOriginType_Radial180 fillOriginType_Radial180;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")EUISpriteFillOriginType_Radial360 fillOriginType_Radial360;
#endif

	virtual void OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache = true)override;

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
	UFUNCTION(BlueprintCallable, Category = "LGUI") EUISpriteType GetSpriteType()const { return type; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	EUISpriteFillMethod GetFillMethod()const { return fillMethod; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	uint8 GetFillOrigin()const { return fillOrigin; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	bool GetFillDirectionFlip()const { return fillDirectionFlip; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	float GetFillAmount()const { return fillAmount; }

	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetSpriteType(EUISpriteType newType);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillMethod(EUISpriteFillMethod newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillOrigin(uint8 newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillDirectionFlip(bool newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillAmount(float newValue);
};
