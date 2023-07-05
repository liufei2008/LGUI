// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIProceduralRect.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/LGUISpriteInfo.h"
#include "Core/LGUIProceduralRectData.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Core/UIDrawcall.h"
#include "Core/ActorComponent/UITextureBase.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Utils/LGUIUtils.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

#define LOCTEXT_NAMESPACE "UIProceduralRect"


void FUIProceduralRectBlockData::FillData(uint8* Data, float width, float height)
{
	int DataOffset = 0;

	uint8 BoolAsByte = PackBoolToByte(bSoftEdge, bEnableGradient, bEnableBorder, bEnableBorderGradient, bEnableInnerShadow, bEnableRadialFill, false, false);
	Fill4BytesToData(Data
		, BoolAsByte
		, (uint8)TextureScaleMode
		, 0, 0
		, DataOffset);

	FillVector2ToData(Data, QuadSize, DataOffset);
	
#define GetFloatValueWithUnit(PropertyName, AdditionalScale) PropertyName##UnitMode == EUIProceduralRectUnitMode::Value ? PropertyName : (PropertyName * 0.01f * (width < height ? width : height) * AdditionalScale)

	auto GetVector2ValueWithUnit2 = [=](const FVector2f& value, EUIProceduralRectUnitMode unitMode) {
		if (unitMode == EUIProceduralRectUnitMode::Value)
			return value;
		else
			return value * 0.01f * (width < height ? width : height);
	};

	FillVector4ToData(Data, GetFloatValueWithUnit(CornerRadius, 0.5f), DataOffset);

	FillColorToData(Data, GradientColor, DataOffset);
	FillVector2ToData(Data, GetValueWithUnitMode(GradientCenter, GradientCenterUnitMode, width, height), DataOffset);
	FillVector2ToData(Data, GetValueWithUnitMode(GradientRadius, GradientRadiusUnitMode, width, height), DataOffset);
	FillFloatToData(Data, GradientRotation, DataOffset);

	FillFloatToData(Data, GetValueWithUnitMode(BorderWidth, BorderWidthUnitMode, width, height, 0.5f), DataOffset);
	FillColorToData(Data, BorderColor, DataOffset);
	FillColorToData(Data, BorderGradientColor, DataOffset);
	FillVector2ToData(Data, GetValueWithUnitMode(BorderGradientCenter, BorderGradientCenterUnitMode, width, height), DataOffset);
	FillVector2ToData(Data, GetValueWithUnitMode(BorderGradientRadius, BorderGradientRadiusUnitMode, width, height), DataOffset);
	FillFloatToData(Data, BorderGradientRotation, DataOffset);

	FillColorToData(Data, InnerShadowColor, DataOffset);
	FillFloatToData(Data, GetValueWithUnitMode(InnerShadowSize, InnerShadowSizeUnitMode, width, height, 0.5f), DataOffset);
	FillFloatToData(Data, GetValueWithUnitMode(InnerShadowBlur, InnerShadowBlurUnitMode, width, height, 0.5f), DataOffset);
	FillVector2ToData(Data, GetValueWithUnitMode(InnerShadowOffset, InnerShadowOffsetUnitMode, width, height), DataOffset);

	FillVector2ToData(Data, GetValueWithUnitMode(RadialFillCenter, RadialFillCenterUnitMode, width, height), DataOffset);
	FillFloatToData(Data, RadialFillRotation, DataOffset);
	FillFloatToData(Data, RadialFillAngle, DataOffset);

	FillColorToData(Data, OuterShadowColor, DataOffset);
	FillFloatToData(Data, GetValueWithUnitMode(OuterShadowSize, OuterShadowSizeUnitMode, width, height, 0.5f), DataOffset);
	FillFloatToData(Data, GetValueWithUnitMode(OuterShadowBlur, OuterShadowBlurUnitMode, width, height, 0.5f), DataOffset);
	FillVector2ToData(Data, GetValueWithUnitMode(OuterShadowOffset, OuterShadowOffsetUnitMode, width, height), DataOffset);
}

float FUIProceduralRectBlockData::GetValueWithUnitMode(float SourceValue, EUIProceduralRectUnitMode UnitMode, float RectWidth, float RectHeight, float AdditionalScale)
{
	return UnitMode == EUIProceduralRectUnitMode::Value ? SourceValue : (SourceValue * 0.01f * (RectWidth < RectHeight ? RectWidth : RectHeight) * AdditionalScale);
}
FVector2f FUIProceduralRectBlockData::GetValueWithUnitMode(const FVector2f& SourceValue, EUIProceduralRectUnitMode UnitMode, float RectWidth, float RectHeight)
{
	return UnitMode == EUIProceduralRectUnitMode::Value ? SourceValue : (SourceValue * 0.01f * FVector2f(RectWidth, RectHeight));
}

constexpr int FUIProceduralRectBlockData::DataCountInBytes()
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

void FUIProceduralRectBlockData::FillColorToData(uint8* Data, const FColor& InValue, int& InOutDataOffset)
{
	auto ColorUint = InValue.ToPackedRGBA();
	int ByteCount = 4;
	FMemory::Memcpy(Data + InOutDataOffset, &ColorUint, ByteCount);
	InOutDataOffset += ByteCount;
}
uint8 FUIProceduralRectBlockData::PackBoolToByte(
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
void FUIProceduralRectBlockData::Fill4BytesToData(uint8* Data, uint8 InValue0, uint8 InValue1, uint8 InValue2, uint8 InValue3, int& InOutDataOffset)
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
void FUIProceduralRectBlockData::FillFloatToData(uint8* Data, const float& InValue, int& InOutDataOffset)
{
	int ByteCount = 4;
	FMemory::Memcpy(Data + InOutDataOffset, &InValue, ByteCount);
	InOutDataOffset += ByteCount;
}
void FUIProceduralRectBlockData::FillVector2ToData(uint8* Data, const FVector2f& InValue, int& InOutDataOffset)
{
	int ByteCount = 8;
	FMemory::Memcpy(Data + InOutDataOffset, &InValue, ByteCount);
	InOutDataOffset += ByteCount;
}
void FUIProceduralRectBlockData::FillVector4ToData(uint8* Data, const FVector4f& InValue, int& InOutDataOffset)
{
	int ByteCount = 16;
	FMemory::Memcpy(Data + InOutDataOffset, &InValue, ByteCount);
	InOutDataOffset += ByteCount;
}
void FUIProceduralRectBlockData::OnCornerRadiusUnitModeChanged(float width, float height)
{
	if (CornerRadiusUnitMode == EUIProceduralRectUnitMode::Value)//from percentage to value
	{
		CornerRadius = CornerRadius * 0.01f * (width < height ? width : height) * 0.5f;
	}
	else//from value to percentage
	{
		CornerRadius = CornerRadius * 100.0f / (width < height ? width : height) * 2.0f;
	}
}

FName UUIProceduralRect::DataTextureParameterName = TEXT("DataTexture");

UUIProceduralRect::UUIProceduralRect(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	bNeedUpdateBlockData = true;
}

void UUIProceduralRect::BeginPlay()
{
	Super::BeginPlay();
}

void UUIProceduralRect::OnRegister()
{
	Super::OnRegister();
	if (ProceduralRectData == nullptr)
	{
		ProceduralRectData = LoadObject<ULGUIProceduralRectData>(NULL, TEXT("/LGUI/DefaultProceduralRectData"));
		check(ProceduralRectData != nullptr);
	}
	ProceduralRectData->Init(FUIProceduralRectBlockData::DataCountInBytes());
	DataStartPosition = ProceduralRectData->RegisterBuffer();
	OnDataTextureChangedDelegateHandle = ProceduralRectData->OnDataTextureChange.AddUObject(this, &UUIProceduralRect::OnDataTextureChanged);
}
void UUIProceduralRect::OnUnregister()
{
	Super::OnUnregister();
	ProceduralRectData->UnregisterBuffer(DataStartPosition);
	if (OnDataTextureChangedDelegateHandle.IsValid())
	{
		ProceduralRectData->OnDataTextureChange.Remove(OnDataTextureChangedDelegateHandle);
		OnDataTextureChangedDelegateHandle.Reset();
	}
}

#if WITH_EDITOR
void UUIProceduralRect::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{

#define SetUnitChange(Property)\
	if (PropertyName == GET_MEMBER_NAME_CHECKED(FUIProceduralRectBlockData, Property##UnitMode))\
	{\
		this->BlockData.On##Property##UnitModeChanged(this->GetWidth(), this->GetHeight());\
	}


	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		SetUnitChange(CornerRadius)
		else SetUnitChange(GradientCenter)
		else SetUnitChange(GradientRadius)

		else SetUnitChange(BorderWidth)
		else SetUnitChange(BorderGradientCenter)
		else SetUnitChange(BorderGradientRadius)

		else SetUnitChange(InnerShadowSize)
		else SetUnitChange(InnerShadowBlur)
		else SetUnitChange(InnerShadowOffset)

		else SetUnitChange(RadialFillCenter)

		else SetUnitChange(OuterShadowSize)
		else SetUnitChange(OuterShadowBlur)
		else SetUnitChange(OuterShadowOffset)
	}
}
void UUIProceduralRect::EditorForceUpdate()
{
	Super::EditorForceUpdate();
}
#endif

UTexture* UUIProceduralRect::GetTextureToCreateGeometry()
{
	CheckAdditionalShaderChannels();
	if (!IsValid(BlockData.Texture))
	{
		BlockData.Texture = UUITextureBase::GetDefaultWhiteTexture();
	}
	return BlockData.Texture;
}
UMaterialInterface* UUIProceduralRect::GetMaterialToCreateGeometry()
{
	if (auto Result = Super::GetMaterialToCreateGeometry())
	{
		return Result;
	}
	else
	{
		check(ProceduralRectData);
		CheckAdditionalShaderChannels();
		auto CanvasClipType = ELGUICanvasClipType::None;
		if (this->GetRenderCanvas() != nullptr)
		{
			CanvasClipType = this->GetRenderCanvas()->GetActualClipType();
			if (CanvasClipType == ELGUICanvasClipType::Custom)
			{
				CanvasClipType = ELGUICanvasClipType::None;
			}
		}
		return ProceduralRectData->GetMaterial(CanvasClipType);
	}
}
void UUIProceduralRect::UpdateMaterialClipType()
{
	geometry->material = GetMaterialToCreateGeometry();
	if (drawcall.IsValid())
	{
		drawcall->bMaterialChanged = true;
		drawcall->bMaterialNeedToReassign = true;
		drawcall->bNeedToUpdateVertex = true;
	}
}
void UUIProceduralRect::OnMaterialInstanceDynamicCreated(class UMaterialInstanceDynamic* mat) 
{
	mat->SetTextureParameterValue(DataTextureParameterName, ProceduralRectData->GetDataTexture());
}

void UUIProceduralRect::MarkAllDirty()
{
	Super::MarkAllDirty();
	bNeedUpdateBlockData = true;
}

void UUIProceduralRect::CheckAdditionalShaderChannels()
{
	if (RenderCanvas.IsValid())
	{
		auto flags =
			//UV1: data position 2d
			//UV2.x: bIsOuterShadow
			//UV3: outer shadow's full uv
			(1 << (int)ELGUICanvasAdditionalChannelType::UV1)
			| (1 << (int)ELGUICanvasAdditionalChannelType::UV2)
			| (1 << (int)ELGUICanvasAdditionalChannelType::UV3)
			;
#if WITH_EDITOR
		auto originFlags = RenderCanvas->GetActualAdditionalShaderChannelFlags();
		if (flags != 0 && (originFlags & flags) == 0)
		{
			auto MsgText = FText::Format(LOCTEXT("UIProceduralRectShaderChannels"
				, "{0} Automatically change 'AdditionalShaderChannels' property for LGUICanvas because UIProceduralRect need it")
				, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)));
			//LGUIUtils::EditorNotification(MsgText);
			UE_LOG(LGUI, Log, TEXT("%s"), *MsgText.ToString());
		}
#endif
		RenderCanvas->SetActualRequireAdditionalShaderChannels(flags);
	}
}

void UUIProceduralRect::OnDataTextureChanged(class UTexture2D* Texture)
{
	geometry->texture = GetTextureToCreateGeometry();
	if (drawcall.IsValid())
	{
		drawcall->Texture = geometry->texture;
		drawcall->bTextureChanged = true;
		drawcall->bNeedToUpdateVertex = true;
	}
	MarkVerticesDirty(false, true, true, false);
	MarkCanvasUpdate(true, true, false);
}

void UUIProceduralRect::OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	static FLGUISpriteInfo SimpleRectSpriteData;
	UIGeometry::UpdateUIProceduralRectSimpleVertex(&InGeo,
		this->BlockData.bEnableOuterShadow
		, this->BlockData.GetValueWithUnitMode(BlockData.OuterShadowOffset, BlockData.OuterShadowOffsetUnitMode, this->GetWidth(), this->GetHeight())
		, this->BlockData.GetValueWithUnitMode(BlockData.OuterShadowSize, BlockData.OuterShadowSizeUnitMode, this->GetWidth(), this->GetHeight(), 0.5f)
		, this->BlockData.GetValueWithUnitMode(BlockData.OuterShadowBlur, BlockData.OuterShadowBlurUnitMode, this->GetWidth(), this->GetHeight(), 0.5f)
		, this->BlockData.bSoftEdge,
		this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), SimpleRectSpriteData, RenderCanvas.Get(), this, GetFinalColor(),
		InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
	);

	if (InTriangleChanged || InVertexPositionChanged || InVertexUVChanged || InVertexColorChanged)
	{
		auto& vertices = InGeo.vertices;
		if (this->BlockData.bEnableOuterShadow)
		{
			for (int i = 0; i < 8; i++)
			{
				vertices[i].TextureCoordinate[1] = FVector2f(DataStartPosition.X, DataStartPosition.Y);
			}
			for (int i = 0; i < 4; i++)
			{
				vertices[i].TextureCoordinate[2] = FVector2f(1, 0);
			}
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				vertices[i].TextureCoordinate[1] = FVector2f(DataStartPosition.X, DataStartPosition.Y);
				vertices[i].TextureCoordinate[2] = FVector2f(0, 0);
			}
		}
		bNeedUpdateBlockData = true;
	}

	if (bNeedUpdateBlockData)
	{
		bNeedUpdateBlockData = false;

		BlockData.QuadSize.X = this->GetWidth();
		BlockData.QuadSize.Y = this->GetHeight();

		auto BlockSize = ProceduralRectData->GetBlockSizeInByte();
		uint8* BlockBuffer = new uint8[BlockSize];
		FMemory::Memzero(BlockBuffer, BlockSize);
		BlockData.FillData(BlockBuffer, this->GetWidth(), this->GetHeight());
		ProceduralRectData->UpdateBlock(DataStartPosition, BlockBuffer);
	}
}

void UUIProceduralRect::OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache)
{
	Super::OnAnchorChange(InPivotChange, InWidthChange, InHeightChange, InDiscardCache);
}

void UUIProceduralRect::SetBlockData(const FUIProceduralRectBlockData& value)
{
	BlockData = value;
	MarkAllDirty();
}

void UUIProceduralRect::SetCornerRadius(const FVector4& value)
{
	BlockData.CornerRadius = (FVector4f)value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetTexture(UTexture* value)
{
	BlockData.Texture = value;
	if (BlockData.Texture == nullptr)
	{
		BlockData.Texture = UUITextureBase::GetDefaultWhiteTexture();
	}
	MarkTextureDirty();
}
void UUIProceduralRect::SetSizeFromTexture()
{
	if (IsValid(BlockData.Texture))
	{
		SetWidth(BlockData.Texture->GetSurfaceWidth());
		SetHeight(BlockData.Texture->GetSurfaceHeight());
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Texture is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void UUIProceduralRect::SetSoftEdge(bool value)
{
	BlockData.bSoftEdge = value;
	bNeedUpdateBlockData = true;
	MarkVerticesDirty(false, true, false, false);
}
void UUIProceduralRect::SetTextureScaleMode(EUIProceduralRectTextureScaleMode value)
{
	BlockData.TextureScaleMode = value;
	bNeedUpdateBlockData = true;
}

void UUIProceduralRect::SetEnableGradient(bool value)
{
	BlockData.bEnableGradient = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetGradientColor(const FColor& value)
{
	BlockData.GradientColor = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetGradientCenter(const FVector2D& value)
{
	BlockData.GradientCenter = (FVector2f)value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetGradientRadius(const FVector2D& value)
{
	BlockData.GradientRadius = (FVector2f)value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetGradientRotation(float value)
{
	BlockData.GradientRotation = value;
	bNeedUpdateBlockData = true;
}

void UUIProceduralRect::SetEnableBorder(bool value)
{
	BlockData.bEnableBorder = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetBorderWidth(float value)
{
	BlockData.BorderWidth = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetBorderColor(const FColor& value)
{
	BlockData.BorderColor = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetEnableBorderGradient(bool value)
{
	BlockData.bEnableBorderGradient = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetBorderGradientColor(const FColor& value)
{
	BlockData.BorderGradientColor = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetBorderGradientCenter(const FVector2D& value)
{
	BlockData.BorderGradientCenter = (FVector2f)value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetBorderGradientRadius(const FVector2D& value)
{
	BlockData.BorderGradientRadius = (FVector2f)value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetBorderGradientRotation(float value)
{
	BlockData.BorderGradientRotation = value;
	bNeedUpdateBlockData = true;
}

void UUIProceduralRect::SetEnableInnerShadow(bool value)
{
	BlockData.bEnableInnerShadow = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetInnerShadowColor(const FColor& value)
{
	BlockData.InnerShadowColor = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetInnerShadowSize(float value)
{
	BlockData.InnerShadowSize = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetInnerShadowBlur(float value)
{
	BlockData.InnerShadowBlur = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetInnerShadowOffset(const FVector2D& value)
{
	BlockData.InnerShadowOffset = (FVector2f)value;
	bNeedUpdateBlockData = true;
}

void UUIProceduralRect::SetEnableRadialFill(bool value)
{
	BlockData.bEnableRadialFill = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetRadialFillCenter(const FVector2D& value)
{
	BlockData.RadialFillCenter = (FVector2f)value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetRadialFillRotation(float value)
{
	BlockData.RadialFillRotation = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetRadialFillAngle(float value)
{
	BlockData.RadialFillAngle = value;
	bNeedUpdateBlockData = true;
}

void UUIProceduralRect::SetEnableOuterShadow(bool value)
{
	BlockData.bEnableOuterShadow = value;
	MarkAllDirty();
}
void UUIProceduralRect::SetOuterShadowColor(const FColor& value)
{
	BlockData.OuterShadowColor = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetOuterShadowSize(float value)
{
	BlockData.OuterShadowSize = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetOuterShadowBlur(float value)
{
	BlockData.OuterShadowBlur = value;
	bNeedUpdateBlockData = true;
}
void UUIProceduralRect::SetOuterShadowOffset(const FVector2D& value)
{
	BlockData.OuterShadowOffset = (FVector2f)value;
	bNeedUpdateBlockData = true;
}

#undef LOCTEXT_NAMESPACE

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
