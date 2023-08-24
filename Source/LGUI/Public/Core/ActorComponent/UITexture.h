// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Core/LGUISpriteData.h"
#include "UITextureBase.h"
#include "UISprite.h"
#include "UITexture.generated.h"

UENUM(BlueprintType, Category = LGUI)
enum class EUITextureType :uint8
{
	Normal,
	Sliced,
	SlicedFrame,
	Tiled,
	Filled,
};
#ifndef UITextureType
#define UITextureType UE_DEPRECATED_MACRO(5.0, "UITextureType has been renamed to EUITextureType") EUITextureType
#endif

UENUM(BlueprintType, Category = LGUI)
enum class EUITextureUVRectControlMode :uint8
{
	None,
	KeepAspectRatio_FitIn,
	KeepAspectRatio_Envelope,
};
UCLASS(ClassGroup = (LGUI), NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUITexture : public UUITextureBase
{
	GENERATED_BODY()

public:	
	UUITexture(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void EditorForceUpdate() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	virtual void BeginPlay()override;
protected:
	friend class FUITextureCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUITextureType type = EUITextureType::Normal;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FLGUISpriteInfo spriteData;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUITextureUVRectControlMode UVRectControlMode = EUITextureUVRectControlMode::None;
	/** Texture UV offset and scale info. Only get good result when Type is Normal */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta=(EditCondition="UVRectControlMode==EUITextureUVRectControlMode::None"))
		FVector4 uvRect = FVector4(0, 0, 1, 1);

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

	void CheckSpriteData();
	void ApplyUVRect();

	virtual void OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache = true)override;

	virtual void OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI") EUITextureType GetTextureType()const { return type; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") FLGUISpriteInfo GetSpriteData()const { return spriteData; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") FVector4 GetUVRect()const { return uvRect; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	EUISpriteFillMethod GetFillMethod()const { return fillMethod; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	uint8 GetFillOrigin()const { return fillOrigin; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	bool GetFillDirectionFlip()const { return fillDirectionFlip; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	float GetFillAmount()const { return fillAmount; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")	EUITextureUVRectControlMode GetUVRectControlMode()const { return UVRectControlMode; }

	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetTextureType(EUITextureType newType);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetSpriteData(FLGUISpriteInfo newSpriteData);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetUVRect(FVector4 newUVRect);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillMethod(EUISpriteFillMethod newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillOrigin(uint8 newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillDirectionFlip(bool newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetFillAmount(float newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI") void SetUVRectControlMode(EUITextureUVRectControlMode newValue);

	virtual void SetTexture(UTexture* newTexture)override;
};
