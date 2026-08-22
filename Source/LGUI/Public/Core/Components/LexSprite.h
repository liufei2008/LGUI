// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexSpriteBase.h"
#include "Core/LexUISpriteInfo.h"
#include "LexSprite.generated.h"

UENUM(BlueprintType, Category = LGUI)
enum class ELexUISpriteDrawType :uint8
{
	Normal,
	Sliced,
	Tiled,
	Filled,
};

UENUM(BlueprintType, Category = LGUI)
enum class ELexUISpriteFillMethod:uint8
{
	Horizontal,
	Vertical,
	Radial90,
	Radial180,
	Radial360,
};

UENUM(BlueprintType, Category = LGUI)
enum class ELexUISpriteFillOriginType_Radial90 :uint8 
{
	BottomLeft,
	TopLeft,
	TopRight,
	BottomRight,
};

UENUM(BlueprintType, Category = LGUI)
enum class ELexUISpriteFillOriginType_Radial180 :uint8
{
	Bottom,
	Left,
	Top,
	Right,
};

UENUM(BlueprintType, Category = LGUI)
enum class ELexUISpriteFillOriginType_Radial360 :uint8
{
	Bottom,
	Right,
	Top,
	Left,
};

UCLASS(ClassGroup = (LGUI), NotBlueprintable)
class LGUI_API ULexSprite : public ULexSpriteBase
{
	GENERATED_BODY()

public:	
	ULexSprite(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	friend class FLexSpriteCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELexUISpriteDrawType DrawType = ELexUISpriteDrawType::Normal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		ELexUISpriteFlipMode FlipMode = ELexUISpriteFlipMode::Nothing;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ImageBrush", meta=(ClampMin = "0.01" ))
	float PixelsPerUnitMultiplier = 1;
	UPROPERTY(EditAnywhere, Category = "LGUI")
	bool bFillCenter = true;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELexUISpriteFillMethod FillMethod = ELexUISpriteFillMethod::Horizontal;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint8 FillOrigin = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool FillDirectionFlip = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float FillAmount = 1;
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")ELexUISpriteFillOriginType_Radial90 FillOriginType_Radial90;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")ELexUISpriteFillOriginType_Radial180 FillOriginType_Radial180;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")ELexUISpriteFillOriginType_Radial360 FillOriginType_Radial360;
#endif

	virtual void OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange)override;

	//width direction rectangle count, in tiled mode
	int32 Tiled_WidthRectCount = 0;
	//height direction rectangle count, in tiled mode
	int32 Tiled_HeightRectCount = 0;
	//width direction half rectangle size, in tiled mode
	float Tiled_WidthRemainedRectSize = 0;
	//height direction half rectangle size, in tiled mode
	float Tiled_HeightRemainedRectSize = 0;
	void CalculateTiledParams();

	virtual void OnUpdateGeometry(FLexUIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI") ELexUISpriteDrawType GetSpriteDrawType()const { return DrawType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") ELexUISpriteFlipMode GetFlipMode()const { return FlipMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") float GetPixelsPerUnitMultiplier() const { return PixelsPerUnitMultiplier; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") bool GetFillCenter() const { return bFillCenter; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	ELexUISpriteFillMethod GetFillMethod()const { return FillMethod; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	uint8 GetFillOrigin()const { return FillOrigin; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	bool GetFillDirectionFlip()const { return FillDirectionFlip; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	float GetFillAmount()const { return FillAmount; }

	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetDrawType(ELexUISpriteDrawType Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFlipMode(ELexUISpriteFlipMode Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetPixelsPerUnitMultiplier(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillCenter(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillMethod(ELexUISpriteFillMethod Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillOrigin(uint8 Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillDirectionFlip(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillAmount(float Value);
};
