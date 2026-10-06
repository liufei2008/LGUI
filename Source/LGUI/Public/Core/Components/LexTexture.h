// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Core/LexUISpriteData.h"
#include "LexTextureBase.h"
#include "LexSprite.h"
#include "LexTexture.generated.h"

UCLASS(ClassGroup = (LGUI), NotBlueprintable)
class LGUI_API ULexTexture : public ULexTextureBase
{
	GENERATED_BODY()

public:	
	ULexTexture(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	virtual void BeginPlay()override;
	virtual void OnRegister() override;
protected:
	friend class FLexTextureCustomization;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		ELexUISpriteDrawType DrawType = ELexUISpriteDrawType::Normal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI")
		ELexUISpriteFlipMode FlipMode = ELexUISpriteFlipMode::Nothing;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI", meta=(ClampMin = "0.01"))
	float PixelsPerUnitMultiplier = 1;
	UPROPERTY(EditAnywhere, Category = "LexUI")
	bool bFillCenter = true;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		FLexUISpriteInfo SpriteInfo;
	/** Texture UV offset and scale info. Only get good result when DrawType is Normal */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		FVector4f UVRect = FVector4f(0, 0, 1, 1);

	UPROPERTY(EditAnywhere, Category = "LexUI")
		ELexUISpriteFillMethod FillMethod = ELexUISpriteFillMethod::Horizontal;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		uint8 FillOrigin = 0;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool FillDirectionFlip = false;
	UPROPERTY(EditAnywhere, Category = "LexUI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float FillAmount = 1;
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, EditAnywhere, Category = "LexUI")ELexUISpriteFillOriginType_Radial90 FillOriginType_Radial90;
	UPROPERTY(Transient, EditAnywhere, Category = "LexUI")ELexUISpriteFillOriginType_Radial180 FillOriginType_Radial180;
	UPROPERTY(Transient, EditAnywhere, Category = "LexUI")ELexUISpriteFillOriginType_Radial360 FillOriginType_Radial360;
#endif

	void CheckSpriteInfo();

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
	UFUNCTION(BlueprintCallable, Category = "LexUI") ELexUISpriteDrawType GetDrawType()const { return DrawType; }
	UFUNCTION(BlueprintCallable, Category = "LexUI") ELexUISpriteFlipMode GetFlipMode()const { return FlipMode; }
	UFUNCTION(BlueprintCallable, Category = "LexUI") FLexUISpriteInfo GetSpriteInfo()const { return SpriteInfo; }
	UFUNCTION(BlueprintCallable, Category = "LexUI") FVector4f GetUVRect()const { return UVRect; }
	UFUNCTION(BlueprintCallable, Category = "LexUI") float GetPixelsPerUnitMultiplier() const { return PixelsPerUnitMultiplier; }
	UFUNCTION(BlueprintCallable, Category = "LexUI") bool GetFillCenter() const { return bFillCenter; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")	ELexUISpriteFillMethod GetFillMethod()const { return FillMethod; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")	uint8 GetFillOrigin()const { return FillOrigin; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")	bool GetFillDirectionFlip()const { return FillDirectionFlip; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")	float GetFillAmount()const { return FillAmount; }

	UFUNCTION(BlueprintCallable, Category = "LexUI") void SetDrawType(ELexUISpriteDrawType Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI") void SetFlipMode(ELexUISpriteFlipMode Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI") void SetSpriteInfo(FLexUISpriteInfo Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI") void SetUVRect(FVector4f Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI") void SetPixelsPerUnitMultiplier(float Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI") void SetFillCenter(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI") void SetFillMethod(ELexUISpriteFillMethod Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI") void SetFillOrigin(uint8 Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI") void SetFillDirectionFlip(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI") void SetFillAmount(float Value);

	virtual void SetTexture(UTexture* Value)override;
};
