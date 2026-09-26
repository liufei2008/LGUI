// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexVisualBatchMesh.h"
#include "Core/ILexUISpriteRenderInterface.h"
#include "Core/LexUIDataAsTexture.h"
#include "LexRectBlock.generated.h"


UCLASS(ClassGroup = (LGUI), BlueprintType)
class LGUI_API ULexRectBlockData :public ULexUIDataAsTexture
{
	GENERATED_BODY()
private:

	UPROPERTY(EditAnywhere, Category = "LexUI")
	TObjectPtr<UMaterialInterface> DefaultMaterial;
protected:
	virtual void PostInitProperties()override;
public:
	UMaterialInterface* GetMaterial();
};

UENUM(BlueprintType)
enum class ELexRectBlockTextureScaleMode: uint8
{
	Stretch,
	FitIn,
	Envelop,
};
UENUM(BlueprintType)
enum class ELexRectBlockUnitMode : uint8
{
	/** Direct value */
	Value			UMETA(DisplayName="V"),
	/** Percent with rect size from 0 to 100 */
	Percentage		UMETA(DisplayName="%"),
};
UENUM(BlueprintType)
enum class ELexRectBlockTextureMode : uint8
{
	Texture,
	Sprite,
};

class ULexUISpriteData_BaseObject;

/**
 * UV channel-
 *		UV0: full 0~1 UV coordinate for calculate sdf
 *		UV1: Default LexCanvas use, check LexCanvas
 *		UV2: Body texture's coordinate
 *		UV3: X- for RectBlock data coordinate
 */
UCLASS(ClassGroup = (LGUI), BlueprintType, Blueprintable)
class LGUI_API ULexRectBlock : public ULexVisualBatchMesh
	, public ILexUISpriteRenderInterface
{
	GENERATED_BODY()

public:
	ULexRectBlock(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
protected:
	virtual void OnPreChangeSpriteProperty();
	virtual void OnPostChangeSpriteProperty();
#endif
protected:
	virtual void BeginPlay()override;
	virtual void EndPlay()override;

	virtual void OnRegister()override;
	virtual void OnUnregister()override;
protected:
	friend class FLexRectBlockCustomization;

#pragma region BlockData
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		FVector4f CornerRadius = FVector4f(0.1f, 0.1f, 0.1f, 0.1f);
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		ELexRectBlockUnitMode CornerRadiusUnitMode = ELexRectBlockUnitMode::Percentage;
	/** Prevent edge aliasing, useful when in 3d. */
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect", AdvancedDisplay)
		bool bSoftEdge = true;

	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		bool bEnableBody = true;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		FColor BodyColor = FColor::White;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		ELexRectBlockTextureMode BodyTextureMode = ELexRectBlockTextureMode::Sprite;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect", meta = (DisplayThumbnail = "false"))
		TObjectPtr<class UTexture> BodyTexture = nullptr;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect", meta = (DisplayThumbnail = "false"))
		TObjectPtr<ULexUISpriteData_BaseObject> BodySpriteTexture = nullptr;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect", meta = (EditCondition = "BodyTexture"))
		ELexRectBlockTextureScaleMode BodyTextureScaleMode = ELexRectBlockTextureScaleMode::Stretch;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		bool bEnableBodyGradient = false;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		FColor BodyGradientColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		FVector2f BodyGradientCenter = FVector2f(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		ELexRectBlockUnitMode BodyGradientCenterUnitMode = ELexRectBlockUnitMode::Percentage;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		FVector2f BodyGradientRadius = FVector2f(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		ELexRectBlockUnitMode BodyGradientRadiusUnitMode = ELexRectBlockUnitMode::Percentage;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect", meta = (ClampMin = "0.0", ClampMax = "360.0"))
		float BodyGradientRotation = 0;

	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		bool bEnableBorder = false;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		float BorderWidth = 2;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		ELexRectBlockUnitMode BorderWidthUnitMode = ELexRectBlockUnitMode::Value;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		FColor BorderColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		bool bEnableBorderGradient = false;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		FColor BorderGradientColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		FVector2f BorderGradientCenter = FVector2f(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		ELexRectBlockUnitMode BorderGradientCenterUnitMode = ELexRectBlockUnitMode::Percentage;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		FVector2f BorderGradientRadius = FVector2f(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		ELexRectBlockUnitMode BorderGradientRadiusUnitMode = ELexRectBlockUnitMode::Percentage;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect", meta = (ClampMin = "0.0", ClampMax = "360.0"))
		float BorderGradientRotation = 0;

	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		bool bEnableInnerShadow = false;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		FColor InnerShadowColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		float InnerShadowSize = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		ELexRectBlockUnitMode InnerShadowSizeUnitMode = ELexRectBlockUnitMode::Value;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		float InnerShadowBlur = 4;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		ELexRectBlockUnitMode InnerShadowBlurUnitMode = ELexRectBlockUnitMode::Value;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect", meta = (ClampMin = "0.0", ClampMax = "360.0"))
		float InnerShadowAngle = 45;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		float InnerShadowDistance = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		ELexRectBlockUnitMode InnerShadowDistanceUnitMode = ELexRectBlockUnitMode::Value;

	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		bool bEnableRadialFill = false;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		FVector2f RadialFillCenter = FVector2f(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		ELexRectBlockUnitMode RadialFillCenterUnitMode = ELexRectBlockUnitMode::Percentage;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		float RadialFillRotation = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect", meta = (ClampMin = "0.0", ClampMax = "360.0"))
		float RadialFillAngle = 270;

	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		bool bEnableOuterShadow = false;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		FColor OuterShadowColor = FColor(0, 0, 0, 128);
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		float OuterShadowSize = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		ELexRectBlockUnitMode OuterShadowSizeUnitMode = ELexRectBlockUnitMode::Value;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect", meta = (ClampMin = "0.0"))
		float OuterShadowBlur = 4;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		ELexRectBlockUnitMode OuterShadowBlurUnitMode = ELexRectBlockUnitMode::Value;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect", meta = (ClampMin = "0.0", ClampMax = "360.0"))
		float OuterShadowAngle = 45;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		float OuterShadowDistance = 4;
	UPROPERTY(EditAnywhere, Category = "LGUI-ProceduralRect")
		ELexRectBlockUnitMode OuterShadowDistanceUnitMode = ELexRectBlockUnitMode::Value;

	void FillData(uint8* Data, float width, float height);
	float GetValueWithUnitMode(float SourceValue, ELexRectBlockUnitMode UnitMode, float RectWidth, float RectHeight, float AdditionalScale)const;
	FVector4f GetValueWithUnitMode(const FVector4f& SourceValue, ELexRectBlockUnitMode UnitMode, float RectWidth, float RectHeight, float AdditionalScale)const;
	FVector2f GetValueWithUnitMode(const FVector2f& SourceValue, ELexRectBlockUnitMode UnitMode, float RectWidth, float RectHeight)const;
	FVector2f GetInnerShadowOffset(float RectWidth, float RectHeight);
	FVector2f GetOuterShadowOffset(float RectWidth, float RectHeight);
	static constexpr int DataCountInBytes();

	void FillColorToData(uint8* Data, const FColor& InValue, int& InOutDataOffset);
	uint8 PackBoolToByte(
		bool v0
		, bool v1
		, bool v2
		, bool v3
		, bool v4
		, bool v5
		, bool v6
		, bool v7
	);
	void Fill8BytesToData(uint8* Data, uint8 InValue0, uint8 InValue1, uint8 InValue2, uint8 InValue3, int& InOutDataOffset);
	void FillFloatToData(uint8* Data, const float& InValue, int& InOutDataOffset);
	void FillVector2ToData(uint8* Data, const FVector2f& InValue, int& InOutDataOffset);
	void FillVector4ToData(uint8* Data, const FVector4f& InValue, int& InOutDataOffset);


#define OnFloatUnitModeChanged(Property, AdditionalScale)\
	void On##Property##UnitModeChanged(float width, float height)\
	{\
		if (Property##UnitMode == ELexRectBlockUnitMode::Value)\
		{\
			Property = Property * (width < height ? width : height) * AdditionalScale;\
		}\
		else\
		{\
			Property = Property / (width < height ? width : height) / AdditionalScale;\
		}\
	}

#define OnVector2UnitModeChanged(Property)\
	void On##Property##UnitModeChanged(float width, float height)\
	{\
		if (Property##UnitMode == ELexRectBlockUnitMode::Value)\
		{\
			Property.X = Property.X * width;\
			Property.Y = Property.Y * height;\
		}\
		else\
		{\
			Property.X = Property.X / width;\
			Property.Y = Property.Y / height;\
		}\
	}

	void OnCornerRadiusUnitModeChanged(float width, float height);
	OnVector2UnitModeChanged(BodyGradientCenter);
	OnVector2UnitModeChanged(BodyGradientRadius);

	OnFloatUnitModeChanged(BorderWidth, 0.5f);
	OnVector2UnitModeChanged(BorderGradientCenter);
	OnVector2UnitModeChanged(BorderGradientRadius);

	OnFloatUnitModeChanged(InnerShadowSize, 0.5f);
	OnFloatUnitModeChanged(InnerShadowBlur, 1.0f);
	OnFloatUnitModeChanged(InnerShadowDistance, 0.5f);

	OnVector2UnitModeChanged(RadialFillCenter);

	OnFloatUnitModeChanged(OuterShadowSize, 0.5f);
	OnFloatUnitModeChanged(OuterShadowBlur, 1.0f);
	OnFloatUnitModeChanged(OuterShadowDistance, 0.5f);

#pragma endregion

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool bUniformSetCornerRadius = true;
#endif

	UPROPERTY(VisibleAnywhere, Category = "LexUI", AdvancedDisplay)
		TObjectPtr<class ULexRectBlockData> RectBlockData = nullptr;
	/** When do raycast interaction, will the CornerRadius be considered? Only support RaycastType.Rect. */
	UPROPERTY(EditAnywhere, Category = "LexUI-Raycast", meta=(EditCondition=bRaycastTarget))
		bool bRaycastSupportCornerRadius = true;

	int DataStartPosition = 0;
	static FName DataTextureParameterName;

	virtual void OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange) override;

	virtual void OnBeforeCreateOrUpdateGeometry()override;
	virtual UTexture* GetTextureToCreateGeometry()override;
	virtual UMaterialInterface* GetMaterialToCreateGeometry()override;
	virtual void OnMaterialInstanceDynamicCreated(class UMaterialInstanceDynamic* mat) override;

	//virtual void OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache = true)override;
	virtual void OnUpdateGeometry(FLexUIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
	virtual void MarkAllDirty()override;
	virtual bool GetAnythingDirty() const override;

	void MarkNeedUpdateBlockData();
	void OnDataTextureChanged(class UTexture* Texture);
	FDelegateHandle OnDataTextureChangedDelegateHandle;
	uint8 bNeedUpdateBlockData : 1;
	uint8 bHasAddToSprite : 1;
protected:
	bool LineTraceUI_CheckCornerRadius(const FVector2D& InLocalHitPoint)const;
	virtual bool LineTraceUIRect(FLexUIHitResult& OutHit, const FVector& Start, const FVector& End)const override;
public:
#pragma region ILexUISpriteRenderInterface
	virtual ULexUISpriteData_BaseObject* SpriteRenderGetSprite_Implementation()const override { return BodySpriteTexture; }
	virtual void ApplyAtlasTextureChange_Implementation()override;
#pragma endregion

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FVector4f& GetCornerRadius()const { return CornerRadius; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELexRectBlockUnitMode GetCornerRadiusUnitMode()const { return CornerRadiusUnitMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetEnableBody()const { return bEnableBody; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FColor& GetBodyColor()const { return BodyColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UTexture* GetBodyTexture()const { return BodyTexture; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ULexUISpriteData_BaseObject* GetBodySpriteTexture()const { return BodySpriteTexture; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELexRectBlockTextureMode GetBodyTextureMode()const { return BodyTextureMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELexRectBlockTextureScaleMode GetBodyTextureScaleMode()const { return BodyTextureScaleMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetSoftEdge()const { return bSoftEdge; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetEnableBodyGradient()const { return bEnableBodyGradient; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FColor& GetBodyGradientColor()const { return BodyGradientColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FVector2f& GetBodyGradientCenter()const { return BodyGradientCenter; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELexRectBlockUnitMode GetBodyGradientCenterUnitMode()const { return BodyGradientCenterUnitMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FVector2f& GetBodyGradientRadius()const { return BodyGradientRadius; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELexRectBlockUnitMode GetBodyGradientRadiusUnitMode()const { return BodyGradientRadiusUnitMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetBodyGradientRotation()const { return BodyGradientRotation; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetEnableBorder()const { return bEnableBorder; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetBorderWidth()const { return BorderWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELexRectBlockUnitMode GetBorderWidthUnitMode()const { return BorderWidthUnitMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FColor& GetBorderColor()const { return BorderColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetEnableBorderGradient()const { return bEnableBorderGradient; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FColor& GetBorderGradientColor()const { return BorderGradientColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FVector2f& GetBorderGradientCenter()const { return BorderGradientCenter; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELexRectBlockUnitMode GetBorderGradientCenterUnitMode()const { return CornerRadiusUnitMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FVector2f& GetBorderGradientRadius()const { return BorderGradientRadius; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELexRectBlockUnitMode GetBorderGradientRadiusUnitMode()const { return BorderGradientRadiusUnitMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetBorderGradientRotation()const { return BorderGradientRotation; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetEnableInnerShadow()const { return bEnableInnerShadow; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FColor& GetInnerShadowColor()const { return InnerShadowColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetInnerShadowSize()const { return InnerShadowSize; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELexRectBlockUnitMode GetInnerShadowSizeUnitMode()const { return InnerShadowSizeUnitMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetInnerShadowBlur()const { return InnerShadowBlur; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELexRectBlockUnitMode GetInnerShadowBlurUnitMode()const { return InnerShadowBlurUnitMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetInnerShadowAngle()const { return InnerShadowAngle; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetInnerShadowDistance()const { return InnerShadowDistance; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELexRectBlockUnitMode GetInnerShadowDistanceUnitMode()const { return InnerShadowDistanceUnitMode; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetEnableRadialFill()const { return bEnableRadialFill; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FVector2f& GetRadialFillCenter()const { return RadialFillCenter; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELexRectBlockUnitMode GetRadialFillCenterUnitMode()const { return RadialFillCenterUnitMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetRadialFillRotation()const { return RadialFillRotation; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetRadialFillAngle()const { return RadialFillAngle; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetEnableOuterShadow()const { return bEnableOuterShadow; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FColor& GetOuterShadowColor()const { return OuterShadowColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetOuterShadowSize()const { return OuterShadowSize; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELexRectBlockUnitMode GetOuterShadowSizeUnitMode()const { return OuterShadowSizeUnitMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetOuterShadowBlur()const { return OuterShadowBlur; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELexRectBlockUnitMode GetOuterShadowBlurUnitMode()const { return OuterShadowBlurUnitMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetOuterShadowAngle()const { return OuterShadowAngle; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetOuterShadowDistance()const { return OuterShadowDistance; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELexRectBlockUnitMode GetOuterShadowDistanceUnitMode()const { return OuterShadowDistanceUnitMode; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetRaycastSupportCornerRadius()const { return bRaycastSupportCornerRadius; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetCornerRadius(const FVector4& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetCornerRadiusUnitMode(ELexRectBlockUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableBody(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBodyColor(const FColor& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBodyTexture(UTexture* value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBodySpriteTexture(ULexUISpriteData_BaseObject* value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBodyTextureMode(ELexRectBlockTextureMode value);
	/** Set size from current body texture or Sprite */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSizeFromBodyTexture();
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBodyTextureScaleMode(ELexRectBlockTextureScaleMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSoftEdge(bool value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableBodyGradient(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBodyGradientColor(const FColor& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBodyGradientCenter(const FVector2D& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBodyGradientCenterUnitMode(ELexRectBlockUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBodyGradientRadius(const FVector2D& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBodyGradientRadiusUnitMode(ELexRectBlockUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBodyGradientRotation(float value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableBorder(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderWidth(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderWidthUnitMode(ELexRectBlockUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderColor(const FColor& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableBorderGradient(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientColor(const FColor& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientCenter(const FVector2D& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientCenterUnitMode(ELexRectBlockUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientRadius(const FVector2D& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientRadiusUnitMode(ELexRectBlockUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientRotation(float value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableInnerShadow(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowColor(const FColor& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowSize(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowSizeUnitMode(ELexRectBlockUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowBlur(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowBlurUnitMode(ELexRectBlockUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowAngle(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowDistance(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowDistanceUnitMode(ELexRectBlockUnitMode value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableRadialFill(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRadialFillCenter(const FVector2D& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRadialFillCenterUnitMode(ELexRectBlockUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRadialFillRotation(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRadialFillAngle(float value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableOuterShadow(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOuterShadowColor(const FColor& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOuterShadowSize(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOuterShadowSizeUnitMode(ELexRectBlockUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOuterShadowBlur(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOuterShadowBlurUnitMode(ELexRectBlockUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOuterShadowAngle(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOuterShadowDistance(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOuterShadowDistanceUnitMode(ELexRectBlockUnitMode value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRaycastSupportCornerRadius(bool value);

#pragma region TweenAnimation
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* CornerRadiusTo(FVector4 endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BodyColorTo(FColor endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BodyAlphaTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BodyGradientColorTo(FColor endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BodyGradientAlphaTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BodyGradientCenterTo(FVector2D endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BodyGradientRadiusTo(FVector2D endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BodyGradientRotationTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderWidthTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderColorTo(FColor endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderAlphaTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderGradientColorTo(FColor endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderGradientAlphaTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderGradientCenterTo(FVector2D endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderGradientRadiusTo(FVector2D endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderGradientRotationTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* InnerShadowColorTo(FColor endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* InnerShadowAlphaTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* InnerShadowSizeTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* InnerShadowBlurTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* InnerShadowAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* InnerShadowDistanceTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* RadialFillCenterTo(FVector2D endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* RadialFillRotationTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* RadialFillAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* OuterShadowColorTo(FColor endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* OuterShadowAlphaTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* OuterShadowSizeTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* OuterShadowBlurTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* OuterShadowAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* OuterShadowDistanceTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
#pragma endregion
};
