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
USTRUCT(BlueprintType)
struct FUIProceduralRectBlockData
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
		FVector2f QuadSize = FVector2f::Zero();
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector4f CornerRadius = FVector4f::One();
	/** Prevent edge aliasing, useful when in 3d. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bSoftEdge = true;

	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayThumbnail = "false"))
		TObjectPtr<UTexture> Texture = nullptr;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName="ScaleMode", EditCondition="Texture"))
		EUIProceduralRectTextureScaleMode TextureScaleMode = EUIProceduralRectTextureScaleMode::Stretch;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bEnableGradient = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta=(DisplayName = "Color"))
		FColor GradientColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Center"))
		FVector2f GradientCenter = FVector2f(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Radius"))
		FVector2f GradientRadius = FVector2f(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Rotation", ClampMin = "0.0", ClampMax = "360.0"))
		float GradientRotation = 0;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bEnableBorder = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Width"))
		float BorderWidth = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Color"))
		FColor BorderColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Gradient"))
		bool bEnableBorderGradient = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Color"))
		FColor BorderGradientColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Center"))
		FVector2f BorderGradientCenter = FVector2f(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Radius"))
		FVector2f BorderGradientRadius = FVector2f(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Rotation", ClampMin = "0.0", ClampMax = "360.0"))
		float BorderGradientRotation = 0;
		
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bEnableInnerShadow = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Color"))
		FColor InnerShadowColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Size"))
		float InnerShadowSize = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Blur"))
		float InnerShadowBlur = 4;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Offset"))
		FVector2f InnerShadowOffset = FVector2f(0, 0);

	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bEnableRadialFill = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Center"))
		FVector2f RadialFillCenter = FVector2f(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Rotation"))
		float RadialFillRotation = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Angle", ClampMin = "0.0", ClampMax = "360.0"))
		float RadialFillAngle = 270;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bEnableOuterShadow = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Color"))
		FColor OuterShadowColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Size"))
		float OuterShadowSize = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Blur", ClampMin = "0.0"))
		float OuterShadowBlur = 4;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Offset"))
		FVector2f OuterShadowOffset = FVector2f(0, 0);

	void FillData(uint8* Data)
	{
		int DataOffset = 0;

		uint8 BoolAsByte = PackBoolToByte(bSoftEdge, bEnableGradient, bEnableBorder, bEnableBorderGradient, bEnableInnerShadow, bEnableRadialFill, false, false);
		Fill4BytesToData(Data
			, BoolAsByte
			, (uint8)TextureScaleMode
			, 0, 0
			, DataOffset);

		FillVector2ToData(Data, QuadSize, DataOffset);
		FillVector4ToData(Data, CornerRadius, DataOffset);

		FillColorToData(Data, GradientColor, DataOffset);
		FillVector2ToData(Data, GradientCenter, DataOffset);
		FillVector2ToData(Data, GradientRadius, DataOffset);
		FillFloatToData(Data, GradientRotation, DataOffset);

		FillFloatToData(Data, BorderWidth, DataOffset);
		FillColorToData(Data, BorderColor, DataOffset);
		FillColorToData(Data, BorderGradientColor, DataOffset);
		FillVector2ToData(Data, BorderGradientCenter, DataOffset);
		FillVector2ToData(Data, BorderGradientRadius, DataOffset);
		FillFloatToData(Data, BorderGradientRotation, DataOffset);

		FillColorToData(Data, InnerShadowColor, DataOffset);
		FillFloatToData(Data, InnerShadowSize, DataOffset);
		FillFloatToData(Data, InnerShadowBlur, DataOffset);
		FillVector2ToData(Data, InnerShadowOffset, DataOffset);

		FillVector2ToData(Data, RadialFillCenter, DataOffset);
		FillFloatToData(Data, RadialFillRotation, DataOffset);
		FillFloatToData(Data, RadialFillAngle, DataOffset);

		FillColorToData(Data, OuterShadowColor, DataOffset);
		FillFloatToData(Data, OuterShadowSize, DataOffset);
		FillFloatToData(Data, OuterShadowBlur, DataOffset);
		FillVector2ToData(Data, OuterShadowOffset, DataOffset);
	}
	static constexpr int DataCountInBytes()
	{
		const int result =
			4

			+ 8
			+ 16

			+ 4
			+ 8
			+ 8
			+ 4

			+ 4
			+ 4
			+ 4
			+ 8
			+ 8
			+ 4

			+ 4
			+ 4
			+ 4
			+ 8

			+ 8
			+ 4
			+ 4

			+ 4
			+ 4
			+ 4
			+ 8
			;
		return result;
	}

	void FillColorToData(uint8* Data, const FColor& InValue, int& InOutDataOffset)
	{
		auto ColorUint = InValue.ToPackedRGBA();
		int ByteCount = 4;
		FMemory::Memcpy(Data + InOutDataOffset, &ColorUint, ByteCount);
		InOutDataOffset += ByteCount;
	}
	uint8 PackBoolToByte(
		bool v0
		, bool v1
		, bool v2
		, bool v3
		, bool v4
		, bool v5
		, bool v6
		, bool v7
	)
	{
		uint8 Result;
		Result =
			((v0 ? 1 : 0) << 7)
			| ((v1 ? 1 : 0) << 6)
			| ((v2 ? 1 : 0) << 5)
			| ((v3 ? 1 : 0) << 4)
			| ((v4 ? 1 : 0) << 3)
			| ((v5 ? 1 : 0) << 2)
			| ((v6 ? 1 : 0) << 1)
			| ((v7 ? 1 : 0) << 0)
			;
		return Result;
	}
	void Fill4BytesToData(uint8* Data, uint8 InValue0, uint8 InValue1, uint8 InValue2, uint8 InValue3, int& InOutDataOffset)
	{
		int ByteCount = 4;
		uint32 DataAsUint =
			(InValue0 << 24)
			| (InValue1 << 16)
			| (InValue2 << 8)
			| (InValue3 << 0)
			;
		FMemory::Memcpy(Data + InOutDataOffset, &DataAsUint, ByteCount);
		InOutDataOffset += ByteCount;
	}
	void FillFloatToData(uint8* Data, const float& InValue, int& InOutDataOffset)
	{
		int ByteCount = 4;
		FMemory::Memcpy(Data + InOutDataOffset, &InValue, ByteCount);
		InOutDataOffset += ByteCount;
	}
	void FillVector2ToData(uint8* Data, const FVector2f& InValue, int& InOutDataOffset)
	{
		int ByteCount = 8;
		FMemory::Memcpy(Data + InOutDataOffset, &InValue, ByteCount);
		InOutDataOffset += ByteCount;
	}
	void FillVector4ToData(uint8* Data, const FVector4f& InValue, int& InOutDataOffset)
	{
		int ByteCount = 16;
		FMemory::Memcpy(Data + InOutDataOffset, &InValue, ByteCount);
		InOutDataOffset += ByteCount;
	}
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
		void SetCornerRadius(const FVector4f& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetTexture(UTexture* value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSizeFromTexture();
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSoftEdge(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetTextureScaleMode(EUIProceduralRectTextureScaleMode value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableGradient(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetGradientColor(const FColor& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetGradientCenter(const FVector2f& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetGradientRadius(const FVector2f& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetGradientRotation(float value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableBorder(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderWidth(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderColor(const FColor& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableBorderGradient(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientColor(const FColor& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientCenter(const FVector2f& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientRadius(const FVector2f& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBorderGradientRotation(float value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableInnerShadow(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowColor(const FColor& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowSize(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowBlur(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetInnerShadowOffset(const FVector2f& value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEnableRadialFill(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRadialFillCenter(const FVector2f& value);
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
		void SetOuterShadowBlur(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOuterShadowOffset(const FVector2f& value);
};
