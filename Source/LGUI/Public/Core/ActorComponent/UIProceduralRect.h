// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UIBatchGeometryRenderable.h"
#include "UIProceduralRect.generated.h"

UENUM(BlueprintType)
enum class EUIProceduralRectTextureScaleMode: uint8
{
	Stretch,
	Fit,
	Envelop,
};
UENUM(BlueprintType)
enum class EUIProceduralRectUnitMode : uint8
{
	/** Absolute value */
	Value			UMETA(DisplayName="V"),
	/** Percent with rect's size from 0 to 100 */
	Percentage		UMETA(DisplayName="%"),
};
USTRUCT(BlueprintType)
struct FUIProceduralRectBlockData
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
		FVector2f QuadSize = FVector2f::Zero();
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector4f CornerRadius = FVector4f::One();
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIProceduralRectUnitMode CornerRadiusUnitMode = EUIProceduralRectUnitMode::Percentage;
	/** Prevent edge aliasing, useful when in 3d. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bSoftEdge = true;

	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Body"))
		bool bEnableBody = true;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Color"))
		FColor BodyColor = FColor::White;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Texture", DisplayThumbnail = "false"))
		TObjectPtr<UTexture> BodyTexture = nullptr;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName="ScaleMode", EditCondition="Texture"))
		EUIProceduralRectTextureScaleMode BodyTextureScaleMode = EUIProceduralRectTextureScaleMode::Stretch;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Gradient"))
		bool bEnableGradient = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta=(DisplayName = "Color"))
		FColor GradientColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Center"))
		FVector2f GradientCenter = FVector2f(50, 50);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIProceduralRectUnitMode GradientCenterUnitMode = EUIProceduralRectUnitMode::Percentage;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Radius"))
		FVector2f GradientRadius = FVector2f(50, 50);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIProceduralRectUnitMode GradientRadiusUnitMode = EUIProceduralRectUnitMode::Percentage;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Rotation", ClampMin = "0.0", ClampMax = "360.0"))
		float GradientRotation = 0;

	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Border"))
		bool bEnableBorder = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Width"))
		float BorderWidth = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIProceduralRectUnitMode BorderWidthUnitMode = EUIProceduralRectUnitMode::Percentage;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Color"))
		FColor BorderColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Gradient"))
		bool bEnableBorderGradient = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Color"))
		FColor BorderGradientColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Center"))
		FVector2f BorderGradientCenter = FVector2f(50, 50);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIProceduralRectUnitMode BorderGradientCenterUnitMode = EUIProceduralRectUnitMode::Percentage;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Radius"))
		FVector2f BorderGradientRadius = FVector2f(50, 50);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIProceduralRectUnitMode BorderGradientRadiusUnitMode = EUIProceduralRectUnitMode::Percentage;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Rotation", ClampMin = "0.0", ClampMax = "360.0"))
		float BorderGradientRotation = 0;
		
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "InnerShadow"))
		bool bEnableInnerShadow = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Color"))
		FColor InnerShadowColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Size"))
		float InnerShadowSize = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIProceduralRectUnitMode InnerShadowSizeUnitMode = EUIProceduralRectUnitMode::Percentage;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Blur"))
		float InnerShadowBlur = 4;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIProceduralRectUnitMode InnerShadowBlurUnitMode = EUIProceduralRectUnitMode::Percentage;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Angle", ClampMin = "0.0", ClampMax = "360.0"))
		float InnerShadowAngle = 45;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Distance"))
		float InnerShadowDistance = 4;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIProceduralRectUnitMode InnerShadowDistanceUnitMode = EUIProceduralRectUnitMode::Value;

	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "RadialFill"))
		bool bEnableRadialFill = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Center"))
		FVector2f RadialFillCenter = FVector2f(50, 50);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIProceduralRectUnitMode RadialFillCenterUnitMode = EUIProceduralRectUnitMode::Percentage;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Rotation"))
		float RadialFillRotation = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Angle", ClampMin = "0.0", ClampMax = "360.0"))
		float RadialFillAngle = 270;

	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "OuterShadow"))
		bool bEnableOuterShadow = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Color"))
		FColor OuterShadowColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Size"))
		float OuterShadowSize = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIProceduralRectUnitMode OuterShadowSizeUnitMode = EUIProceduralRectUnitMode::Percentage;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Blur", ClampMin = "0.0"))
		float OuterShadowBlur = 4;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIProceduralRectUnitMode OuterShadowBlurUnitMode = EUIProceduralRectUnitMode::Percentage;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Angle", ClampMin = "0.0", ClampMax = "360.0"))
		float OuterShadowAngle = 135;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Distance"))
		float OuterShadowDistance = 4;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIProceduralRectUnitMode OuterShadowDistanceUnitMode = EUIProceduralRectUnitMode::Value;

	void FillData(uint8* Data, float width, float height);
	float GetValueWithUnitMode(float SourceValue, EUIProceduralRectUnitMode UnitMode, float RectWidth, float RectHeight, float AdditionalScale);
	FVector2f GetValueWithUnitMode(const FVector2f& SourceValue, EUIProceduralRectUnitMode UnitMode, float RectWidth, float RectHeight);
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
	void Fill4BytesToData(uint8* Data, uint8 InValue0, uint8 InValue1, uint8 InValue2, uint8 InValue3, int& InOutDataOffset);
	void FillFloatToData(uint8* Data, const float& InValue, int& InOutDataOffset);
	void FillVector2ToData(uint8* Data, const FVector2f& InValue, int& InOutDataOffset);
	void FillVector4ToData(uint8* Data, const FVector4f& InValue, int& InOutDataOffset);


#define OnFloatUnitModeChanged(Property, AdditionalScale)\
	void On##Property##UnitModeChanged(float width, float height)\
	{\
		if (Property##UnitMode == EUIProceduralRectUnitMode::Value)\
		{\
			Property = Property * 0.01f * (width < height ? width : height) * AdditionalScale;\
		}\
		else\
		{\
			Property = Property * 100.0f / (width < height ? width : height) / AdditionalScale;\
		}\
	}

#define OnVector2UnitModeChanged(Property)\
	void On##Property##UnitModeChanged(float width, float height)\
	{\
		if (Property##UnitMode == EUIProceduralRectUnitMode::Value)\
		{\
			Property.X = Property.X * 0.01f * width;\
			Property.Y = Property.Y * 0.01f * height;\
		}\
		else\
		{\
			Property.X = Property.X * 100.0f / width;\
			Property.Y = Property.Y * 100.0f / height;\
		}\
	}

	void OnCornerRadiusUnitModeChanged(float width, float height);
	OnVector2UnitModeChanged(GradientCenter);
	OnVector2UnitModeChanged(GradientRadius);

	OnFloatUnitModeChanged(BorderWidth, 0.5f);
	OnVector2UnitModeChanged(BorderGradientCenter);
	OnVector2UnitModeChanged(BorderGradientRadius);

	OnFloatUnitModeChanged(InnerShadowSize, 0.5f);
	OnFloatUnitModeChanged(InnerShadowBlur, 0.5f);
	OnFloatUnitModeChanged(InnerShadowDistance, 0.5f);

	OnVector2UnitModeChanged(RadialFillCenter);

	OnFloatUnitModeChanged(OuterShadowSize, 0.5f);
	OnFloatUnitModeChanged(OuterShadowBlur, 0.5f);
	OnFloatUnitModeChanged(OuterShadowDistance, 0.5f);
};

UCLASS(ClassGroup = (LGUI), NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIProceduralRect : public UUIBatchGeometryRenderable
{
	GENERATED_BODY()

public:
	UUIProceduralRect(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void EditorForceUpdate() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	virtual void BeginPlay()override;

	virtual void OnRegister()override;
	virtual void OnUnregister()override;
protected:
	friend class FUIProceduralRectCustomization;

	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		TObjectPtr<class ULGUIProceduralRectData> ProceduralRectData = nullptr;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FUIProceduralRectBlockData BlockData;
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bUniformSetCornerRadius = true;
#endif

	FIntVector2 DataStartPosition = FIntVector2(0, 0);
	static FName DataTextureParameterName;

	virtual UTexture* GetTextureToCreateGeometry()override;
	virtual UMaterialInterface* GetMaterialToCreateGeometry()override;
	virtual void UpdateMaterialClipType()override;
	virtual void OnMaterialInstanceDynamicCreated(class UMaterialInstanceDynamic* mat) override;

	//virtual void OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache = true)override;
	virtual void OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
	virtual void OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache = true)override;
	virtual void MarkAllDirty()override;

	void CheckAdditionalShaderChannels();
	void OnDataTextureChanged(class UTexture2D* Texture);
	FDelegateHandle OnDataTextureChangedDelegateHandle;
	uint8 bNeedUpdateBlockData : 1;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const FUIProceduralRectBlockData& GetBlockData()const { return BlockData; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBlockData(const FUIProceduralRectBlockData& value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetCornerRadius(const FVector4& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetCornerRadiusUnitMode(EUIProceduralRectUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableBody(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBodyColor(const FColor& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBodyTexture(UTexture* value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSizeFromBodyTexture();
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBodyTextureScaleMode(EUIProceduralRectTextureScaleMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSoftEdge(bool value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableGradient(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetGradientColor(const FColor& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetGradientCenter(const FVector2D& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetGradientCenterUnitMode(EUIProceduralRectUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetGradientRadius(const FVector2D& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetGradientRadiusUnitMode(EUIProceduralRectUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetGradientRotation(float value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableBorder(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderWidth(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderWidthUnitMode(EUIProceduralRectUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderColor(const FColor& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableBorderGradient(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientColor(const FColor& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientCenter(const FVector2D& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientCenterUnitMode(EUIProceduralRectUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientRadius(const FVector2D& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientRadiusUnitMode(EUIProceduralRectUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientRotation(float value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableInnerShadow(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowColor(const FColor& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowSize(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowSizeUnitMode(EUIProceduralRectUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowBlur(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowBlurUnitMode(EUIProceduralRectUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowAngle(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowDistance(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowDistanceUnitMode(EUIProceduralRectUnitMode value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableRadialFill(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRadialFillCenter(const FVector2D& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRadialFillCenterUnitMode(EUIProceduralRectUnitMode value);
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
		void SetOuterShadowSizeUnitMode(EUIProceduralRectUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOuterShadowBlur(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOuterShadowBlurUnitMode(EUIProceduralRectUnitMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOuterShadowAngle(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOuterShadowDistance(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOuterShadowDistanceUnitMode(EUIProceduralRectUnitMode value);

#pragma region TweenAnimation
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BodyColorTo(FColor endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BodyAlphaTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* GradientColorTo(FColor endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* GradientAlphaTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* GradientCenterTo(FVector2D endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* GradientRadiusTo(FVector2D endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* GradientRotationTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderWidthTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderColorTo(FColor endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderAlphaTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderGradientColorTo(FColor endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderGradientAlphaTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderGradientCenterTo(FVector2D endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderGradientRadiusTo(FVector2D endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* BorderGradientRotationTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* InnerShadowColorTo(FColor endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* InnerShadowAlphaTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* InnerShadowSizeTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* InnerShadowBlurTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* InnerShadowAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* InnerShadowDistanceTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* RadialFillCenterTo(FVector2D endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* RadialFillRotationTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* RadialFillAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* OuterShadowColorTo(FColor endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* OuterShadowAlphaTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* OuterShadowSizeTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* OuterShadowBlurTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* OuterShadowAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* OuterShadowDistanceTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
#pragma endregion
};
