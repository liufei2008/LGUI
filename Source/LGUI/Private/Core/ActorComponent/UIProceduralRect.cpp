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


void UUIProceduralRect::FillData(uint8* Data, float width, float height)
{
	int DataOffset = 0;

	uint8 BoolAsByte = PackBoolToByte(bEnableBody, bSoftEdge, bEnableBodyGradient, bEnableBorder, bEnableBorderGradient, bEnableInnerShadow, bEnableRadialFill, false);
	Fill4BytesToData(Data
		, BoolAsByte
		, (uint8)BodyTextureScaleMode
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
	FillColorToData(Data, BodyColor, DataOffset);

	FillColorToData(Data, BodyGradientColor, DataOffset);
	FillVector2ToData(Data, GetValueWithUnitMode(BodyGradientCenter, BodyGradientCenterUnitMode, width, height), DataOffset);
	FillVector2ToData(Data, GetValueWithUnitMode(BodyGradientRadius, BodyGradientRadiusUnitMode, width, height), DataOffset);
	FillFloatToData(Data, BodyGradientRotation, DataOffset);

	FillFloatToData(Data, GetValueWithUnitMode(BorderWidth, BorderWidthUnitMode, width, height, 0.5f), DataOffset);
	FillColorToData(Data, BorderColor, DataOffset);
	FillColorToData(Data, BorderGradientColor, DataOffset);
	FillVector2ToData(Data, GetValueWithUnitMode(BorderGradientCenter, BorderGradientCenterUnitMode, width, height), DataOffset);
	FillVector2ToData(Data, GetValueWithUnitMode(BorderGradientRadius, BorderGradientRadiusUnitMode, width, height), DataOffset);
	FillFloatToData(Data, BorderGradientRotation, DataOffset);

	FillColorToData(Data, InnerShadowColor, DataOffset);
	FillFloatToData(Data, GetValueWithUnitMode(InnerShadowSize, InnerShadowSizeUnitMode, width, height, 0.5f), DataOffset);
	FillFloatToData(Data, GetValueWithUnitMode(InnerShadowBlur, InnerShadowBlurUnitMode, width, height, 1.0f), DataOffset);
	FillVector2ToData(Data, GetInnerShadowOffset(width, height), DataOffset);

	FillVector2ToData(Data, GetValueWithUnitMode(RadialFillCenter, RadialFillCenterUnitMode, width, height), DataOffset);
	FillFloatToData(Data, RadialFillRotation, DataOffset);
	FillFloatToData(Data, RadialFillAngle, DataOffset);

	FillColorToData(Data, OuterShadowColor, DataOffset);
	FillFloatToData(Data, GetValueWithUnitMode(OuterShadowSize, OuterShadowSizeUnitMode, width, height, 0.5f), DataOffset);
	FillFloatToData(Data, GetValueWithUnitMode(OuterShadowBlur, OuterShadowBlurUnitMode, width, height, 1.0f), DataOffset);
	FillVector2ToData(Data, GetOuterShadowOffset(width, height), DataOffset);
}

float UUIProceduralRect::GetValueWithUnitMode(float SourceValue, EUIProceduralRectUnitMode UnitMode, float RectWidth, float RectHeight, float AdditionalScale)
{
	return UnitMode == EUIProceduralRectUnitMode::Value ? SourceValue : (SourceValue * 0.01f * (RectWidth < RectHeight ? RectWidth : RectHeight) * AdditionalScale);
}
FVector2f UUIProceduralRect::GetValueWithUnitMode(const FVector2f& SourceValue, EUIProceduralRectUnitMode UnitMode, float RectWidth, float RectHeight)
{
	return UnitMode == EUIProceduralRectUnitMode::Value ? SourceValue : (SourceValue * 0.01f * FVector2f(RectWidth, RectHeight));
}

FVector2f UUIProceduralRect::GetInnerShadowOffset(float RectWidth, float RectHeight)
{
	float AngleRadian = FMath::DegreesToRadians(InnerShadowAngle + 90);
	float Sin = FMath::Sin(AngleRadian);
	float Cos = FMath::Cos(AngleRadian);
	float Distance = GetValueWithUnitMode(InnerShadowDistance, InnerShadowDistanceUnitMode, RectWidth, RectHeight, 0.5f);
	return FVector2f(Sin, Cos) * -Distance;
}
FVector2f UUIProceduralRect::GetOuterShadowOffset(float RectWidth, float RectHeight)
{
	float AngleRadian = FMath::DegreesToRadians(OuterShadowAngle + 90);
	float Sin = FMath::Sin(AngleRadian);
	float Cos = FMath::Cos(AngleRadian);
	float Distance = GetValueWithUnitMode(OuterShadowDistance, OuterShadowDistanceUnitMode, RectWidth, RectHeight, 0.5f);
	return FVector2f(Sin, Cos) * Distance;
}

constexpr int UUIProceduralRect::DataCountInBytes()
{
	const int result =
		4//bool and enum

		+ 8//quad size
		+ 16//corner radius
		+ 4//body color

		//gradient
		+ 4//color
		+ 8//center
		+ 8//radius
		+ 4//rotation

		//border
		+ 4//width
		+ 4//color
		//border gradient
		+ 4//color
		+ 8//center
		+ 8//radius
		+ 4//rotation

		//inner shadow
		+ 4//color
		+ 4//size
		+ 4//blur
		+ 8//offset, this is not angle & distance, we calculate offset result here

		//radial fill
		+ 8//center
		+ 4//rotation
		+ 4//angle

		//outer shadow
		+ 4//color
		+ 4//size
		+ 4//blur
		+ 8//offset, this is not angle & distance, we calculate offset result here
		;
	return result;
}

void UUIProceduralRect::FillColorToData(uint8* Data, const FColor& InValue, int& InOutDataOffset)
{
	auto ColorUint = InValue.ToPackedRGBA();
	int ByteCount = 4;
	FMemory::Memcpy(Data + InOutDataOffset, &ColorUint, ByteCount);
	InOutDataOffset += ByteCount;
}
uint8 UUIProceduralRect::PackBoolToByte(
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
void UUIProceduralRect::Fill4BytesToData(uint8* Data, uint8 InValue0, uint8 InValue1, uint8 InValue2, uint8 InValue3, int& InOutDataOffset)
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
void UUIProceduralRect::FillFloatToData(uint8* Data, const float& InValue, int& InOutDataOffset)
{
	int ByteCount = 4;
	FMemory::Memcpy(Data + InOutDataOffset, &InValue, ByteCount);
	InOutDataOffset += ByteCount;
}
void UUIProceduralRect::FillVector2ToData(uint8* Data, const FVector2f& InValue, int& InOutDataOffset)
{
	int ByteCount = 8;
	FMemory::Memcpy(Data + InOutDataOffset, &InValue, ByteCount);
	InOutDataOffset += ByteCount;
}
void UUIProceduralRect::FillVector4ToData(uint8* Data, const FVector4f& InValue, int& InOutDataOffset)
{
	int ByteCount = 16;
	FMemory::Memcpy(Data + InOutDataOffset, &InValue, ByteCount);
	InOutDataOffset += ByteCount;
}
void UUIProceduralRect::OnCornerRadiusUnitModeChanged(float width, float height)
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
	ProceduralRectData->Init(DataCountInBytes());
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
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIProceduralRect, Property##UnitMode))\
	{\
		this->On##Property##UnitModeChanged(this->GetWidth(), this->GetHeight());\
	}


	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		SetUnitChange(CornerRadius)
		else SetUnitChange(BodyGradientCenter)
		else SetUnitChange(BodyGradientRadius)

		else SetUnitChange(BorderWidth)
		else SetUnitChange(BorderGradientCenter)
		else SetUnitChange(BorderGradientRadius)

		else SetUnitChange(InnerShadowSize)
		else SetUnitChange(InnerShadowBlur)
		else SetUnitChange(InnerShadowDistance)

		else SetUnitChange(RadialFillCenter)

		else SetUnitChange(OuterShadowSize)
		else SetUnitChange(OuterShadowBlur)
		else SetUnitChange(OuterShadowDistance)
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
	if (!IsValid(this->BodyTexture))
	{
		this->BodyTexture = UUITextureBase::GetDefaultWhiteTexture();
	}
	return this->BodyTexture;
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
	UIGeometry::UpdateUIProceduralRectSimpleVertex(&InGeo
		, this->bEnableBody || this->bEnableBorder || this->bEnableInnerShadow
		, this->bEnableOuterShadow
		, this->GetOuterShadowOffset(this->GetWidth(), this->GetHeight())
		, this->GetValueWithUnitMode(OuterShadowSize, OuterShadowSizeUnitMode, this->GetWidth(), this->GetHeight(), 0.5f)
		, this->GetValueWithUnitMode(OuterShadowBlur, OuterShadowBlurUnitMode, this->GetWidth(), this->GetHeight(), 1)
		, this->bSoftEdge,
		this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), SimpleRectSpriteData, RenderCanvas.Get(), this, GetFinalColor(),
		InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
	);

	if (InTriangleChanged || InVertexPositionChanged || InVertexUVChanged || InVertexColorChanged)
	{
		auto& vertices = InGeo.vertices;
		if (this->bEnableOuterShadow)
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

		QuadSize.X = this->GetWidth();
		QuadSize.Y = this->GetHeight();

		auto BlockSize = ProceduralRectData->GetBlockSizeInByte();
		uint8* BlockBuffer = new uint8[BlockSize];
		FMemory::Memzero(BlockBuffer, BlockSize);
		FillData(BlockBuffer, this->GetWidth(), this->GetHeight());
		ProceduralRectData->UpdateBlock(DataStartPosition, BlockBuffer);
	}
}

void UUIProceduralRect::OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache)
{
	Super::OnAnchorChange(InPivotChange, InWidthChange, InHeightChange, InDiscardCache);
}

void UUIProceduralRect::SetCornerRadius(const FVector4& value)
{
	CornerRadius = (FVector4f)value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetEnableBody(bool value)
{
	this->bEnableBody = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}
void UUIProceduralRect::SetBodyColor(const FColor& value)
{
	this->BodyColor = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetBodyTexture(UTexture* value)
{
	this->BodyTexture = value;
	if (this->BodyTexture == nullptr)
	{
		this->BodyTexture = UUITextureBase::GetDefaultWhiteTexture();
	}
	MarkTextureDirty();
}
void UUIProceduralRect::SetSizeFromBodyTexture()
{
	if (IsValid(this->BodyTexture))
	{
		SetWidth(this->BodyTexture->GetSurfaceWidth());
		SetHeight(this->BodyTexture->GetSurfaceHeight());
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Texture is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void UUIProceduralRect::SetSoftEdge(bool value)
{
	this->bSoftEdge = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}
void UUIProceduralRect::SetBodyTextureScaleMode(EUIProceduralRectTextureScaleMode value)
{
	this->BodyTextureScaleMode = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}

void UUIProceduralRect::SetEnableBodyGradient(bool value)
{
	this->bEnableBodyGradient = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetBodyGradientColor(const FColor& value)
{
	this->BodyGradientColor = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetBodyGradientCenter(const FVector2D& value)
{
	this->BodyGradientCenter = (FVector2f)value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetBodyGradientRadius(const FVector2D& value)
{
	this->BodyGradientRadius = (FVector2f)value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetBodyGradientRotation(float value)
{
	this->BodyGradientRotation = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}

void UUIProceduralRect::SetEnableBorder(bool value)
{
	this->bEnableBorder = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}
void UUIProceduralRect::SetBorderWidth(float value)
{
	this->BorderWidth = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetBorderColor(const FColor& value)
{
	this->BorderColor = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetEnableBorderGradient(bool value)
{
	this->bEnableBorderGradient = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetBorderGradientColor(const FColor& value)
{
	this->BorderGradientColor = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetBorderGradientCenter(const FVector2D& value)
{
	this->BorderGradientCenter = (FVector2f)value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetBorderGradientRadius(const FVector2D& value)
{
	this->BorderGradientRadius = (FVector2f)value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetBorderGradientRotation(float value)
{
	this->BorderGradientRotation = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}

void UUIProceduralRect::SetEnableInnerShadow(bool value)
{
	this->bEnableInnerShadow = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}
void UUIProceduralRect::SetInnerShadowColor(const FColor& value)
{
	this->InnerShadowColor = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetInnerShadowSize(float value)
{
	this->InnerShadowSize = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetInnerShadowBlur(float value)
{
	this->InnerShadowBlur = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetInnerShadowAngle(float value)
{
	this->InnerShadowAngle = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}
void UUIProceduralRect::SetInnerShadowDistance(float value)
{
	this->InnerShadowDistance = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}

void UUIProceduralRect::SetEnableRadialFill(bool value)
{
	this->bEnableRadialFill = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetRadialFillCenter(const FVector2D& value)
{
	this->RadialFillCenter = (FVector2f)value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetRadialFillRotation(float value)
{
	this->RadialFillRotation = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}
void UUIProceduralRect::SetRadialFillAngle(float value)
{
	this->RadialFillAngle = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}

void UUIProceduralRect::SetEnableOuterShadow(bool value)
{
	this->bEnableOuterShadow = value;
	MarkAllDirty();
}
void UUIProceduralRect::SetOuterShadowColor(const FColor& value)
{
	this->OuterShadowColor = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetOuterShadowSize(float value)
{
	this->OuterShadowSize = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetOuterShadowBlur(float value)
{
	this->OuterShadowBlur = value;
	bNeedUpdateBlockData = true;
	MarkCanvasUpdate(false, false, false, false);
}
void UUIProceduralRect::SetOuterShadowAngle(float value)
{
	this->OuterShadowAngle = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}
void UUIProceduralRect::SetOuterShadowDistance(float value)
{
	this->OuterShadowDistance = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}

#define FunctionSetPropertyUnitMode(Property)\
void UUIProceduralRect::Set##Property##UnitMode(EUIProceduralRectUnitMode value)\
{\
	this->Property##UnitMode = value;\
	bNeedUpdateBlockData = true;\
	MarkCanvasUpdate(false, false, false, false);\
}

FunctionSetPropertyUnitMode(CornerRadius);
FunctionSetPropertyUnitMode(BodyGradientCenter);
FunctionSetPropertyUnitMode(BodyGradientRadius);
FunctionSetPropertyUnitMode(BorderWidth);
FunctionSetPropertyUnitMode(BorderGradientCenter);
FunctionSetPropertyUnitMode(BorderGradientRadius);
FunctionSetPropertyUnitMode(InnerShadowSize);
FunctionSetPropertyUnitMode(InnerShadowBlur);
FunctionSetPropertyUnitMode(InnerShadowDistance);
FunctionSetPropertyUnitMode(RadialFillCenter);
FunctionSetPropertyUnitMode(OuterShadowSize);
FunctionSetPropertyUnitMode(OuterShadowBlur);
FunctionSetPropertyUnitMode(OuterShadowDistance);


#pragma region TweenAnimation
#include "LTweenManager.h"
ULTweener* UUIProceduralRect::BodyColorTo(FColor endValue, float duration, float delay, LTweenEase ease)
{
	return ULTweenManager::To(this, FLTweenColorGetterFunction::CreateWeakLambda(this, [=] {
		return this->BodyColor;
		}), FLTweenColorSetterFunction::CreateWeakLambda(this, [=](FColor value) {
			this->SetBodyColor(value);
			}), endValue, duration)
		->SetDelay(delay)->SetEase(ease);
}
ULTweener* UUIProceduralRect::BodyAlphaTo(float endValue, float duration, float delay, LTweenEase ease)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateWeakLambda(this, [=] {
		return LGUIUtils::Color255To1_Table[this->BodyColor.A];
		}), FLTweenFloatSetterFunction::CreateWeakLambda(this, [=](float value) {
			auto PropertyValue = this->BodyColor;
            PropertyValue.A = (uint8)(value * 255.0f);
			this->SetBodyColor(PropertyValue);
			}), endValue, duration)
		->SetDelay(delay)->SetEase(ease);
}

#define FunctionPropertyAnimation(Property, EndValueType, GetterAndSetterType)\
ULTweener* UUIProceduralRect::Property##To(EndValueType endValue, float duration, float delay, LTweenEase ease)\
{\
	return ULTweenManager::To(this, FLTween##GetterAndSetterType##GetterFunction::CreateWeakLambda(this, [=] {\
		return (EndValueType)this->Property;\
		}), FLTween##GetterAndSetterType##SetterFunction::CreateWeakLambda(this, [=](EndValueType value) {\
			this->Set##Property(value);\
			}), endValue, duration)\
		->SetDelay(delay)->SetEase(ease);\
}

#define FunctionAlphaAnimation(Property, Function)\
ULTweener* UUIProceduralRect::Function##AlphaTo(float endValue, float duration, float delay, LTweenEase ease)\
{\
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateWeakLambda(this, [=] {\
		return LGUIUtils::Color255To1_Table[this->BodyColor.A];\
		}), FLTweenFloatSetterFunction::CreateWeakLambda(this, [=](float value) {\
			auto PropertyValue = this->Property;\
			PropertyValue.A = (uint8)(value * 255.0f);\
			this->Set##Property(PropertyValue);\
			}), endValue, duration)\
		->SetDelay(delay)->SetEase(ease);\
}

FunctionPropertyAnimation(CornerRadius, FVector4, Vector4);

FunctionPropertyAnimation(BodyGradientColor, FColor, Color);
FunctionAlphaAnimation(BodyGradientColor, BodyGradient);
FunctionPropertyAnimation(BodyGradientCenter, FVector2D, Vector2D);
FunctionPropertyAnimation(BodyGradientRadius, FVector2D, Vector2D);
FunctionPropertyAnimation(BodyGradientRotation, float, Float);

FunctionPropertyAnimation(BorderWidth, float, Float);
FunctionPropertyAnimation(BorderColor, FColor, Color);
FunctionAlphaAnimation(BorderColor, Border);
FunctionPropertyAnimation(BorderGradientColor, FColor, Color);
FunctionAlphaAnimation(BorderGradientColor, BorderGradient);
FunctionPropertyAnimation(BorderGradientCenter, FVector2D, Vector2D);
FunctionPropertyAnimation(BorderGradientRadius, FVector2D, Vector2D);
FunctionPropertyAnimation(BorderGradientRotation, float, Float);

FunctionPropertyAnimation(InnerShadowColor, FColor, Color);
FunctionAlphaAnimation(InnerShadowColor, InnerShadow);
FunctionPropertyAnimation(InnerShadowSize, float, Float);
FunctionPropertyAnimation(InnerShadowBlur, float, Float);
FunctionPropertyAnimation(InnerShadowAngle, float, Float);
FunctionPropertyAnimation(InnerShadowDistance, float, Float);

FunctionPropertyAnimation(RadialFillCenter, FVector2D, Vector2D);
FunctionPropertyAnimation(RadialFillRotation, float, Float);
FunctionPropertyAnimation(RadialFillAngle, float, Float);

FunctionPropertyAnimation(OuterShadowColor, FColor, Color);
FunctionAlphaAnimation(OuterShadowColor, OuterShadow);
FunctionPropertyAnimation(OuterShadowSize, float, Float);
FunctionPropertyAnimation(OuterShadowBlur, float, Float);
FunctionPropertyAnimation(OuterShadowAngle, float, Float);
FunctionPropertyAnimation(OuterShadowDistance, float, Float);

#pragma endregion

#undef LOCTEXT_NAMESPACE

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
