// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UITextureBase.h"
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

	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIProceduralRectTextureScaleMode TextureScaleMode = EUIProceduralRectTextureScaleMode::Stretch;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bGradient = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor GradientColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2f GradientCenter = FVector2f(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2f GradientRadius = FVector2f(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float GradientRotation = 0;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bBorder = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float BorderWidth = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bBorderGradient = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor BorderColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor BorderGradientColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2f BorderGradientCenter = FVector2f(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2f BorderGradientRadius = FVector2f(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float BorderGradientRotation = 0;
		
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bInnerShadow = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor InnerShadowColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float InnerShadowSize = 4;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float InnerShadowBlur = 4;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2f InnerShadowOffset = FVector2f(1, 1);

	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bRadialFill = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2f RadialFillCenter = FVector2f(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float RadialFillRotation = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float RadialFillAngle = 270;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bOuterShadow = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor OuterShadowColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float OuterShadowSize = 4;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float OuterShadowBlur = 4;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2f OuterShadowOffset = FVector2f(0, 0);

	void FillData(uint8* Data)
	{
		int DataOffset = 0;

		uint8 BoolAsByte = PackBoolToByte(bGradient, bBorder, bBorderGradient, bInnerShadow, bRadialFill, false, false, false);
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
		//FillVector2ToData(Data, OuterShadowOffset, DataOffset);//OuterShadowOffset is stored in vertex position
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
			//+ 8
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
class LGUI_API UUIProceduralRect : public UUITextureBase
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
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		class ULGUIProceduralRectData* ProceduralRectData = nullptr;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FUIProceduralRectBlockData BlockData;

	FIntVector2 DataStartPosition = FIntVector2(0, 0);
	static FName DataTextureParameterName;

	virtual UTexture* GetTextureToCreateGeometry()override;
	virtual UMaterialInterface* GetMaterialToCreateGeometry()override;
	virtual void UpdateMaterialClipType()override;
	virtual void OnMaterialInstanceDynamicCreated(class UMaterialInstanceDynamic* mat) override;

	//virtual void OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache = true)override;
	virtual void OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
	virtual void OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache = true)override;

	void CheckAdditionalShaderChannels();
	void OnDataTextureChanged(class UTexture2D* Texture);
	FDelegateHandle OnDataTextureChangedDelegateHandle;
public:
	
};
