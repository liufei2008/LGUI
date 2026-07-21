// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexRectBlock.h"
#include "LGUI.h"
#include "Core/LexUIGeometry.h"
#include "Core/LexUISpriteInfo.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Core/Components/LexTextureBase.h"
#include "Utils/LexUIUtils.h"
#include "Core/LexUISpriteData.h"
#include "Core/LexUISpriteData_BaseObject.h"
#include "Core/Components/LexWidget.h"
#include "Event/LexPointerEventData.h"


#define LOCTEXT_NAMESPACE "LexRectBlock"


void ULexRectBlockData::PostInitProperties()
{
	Super::PostInitProperties();
}
UMaterialInterface* ULexRectBlockData::GetMaterial()
{
	if (!DefaultMaterial)
	{
		DefaultMaterial = LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/LexUI_RectBlock"));;
	}
	return DefaultMaterial;
}

void ULexRectBlock::FillData(uint8* Data, float width, float height)
{
	int DataOffset = 0;

	uint8 BoolAsByte = PackBoolToByte(bEnableBody, bEnableOuterShadow, bEnableBodyGradient, bEnableBorder, bEnableBorderGradient, bEnableInnerShadow, bEnableRadialFill, false);
	Fill8BytesToData(Data
		, BoolAsByte
		, static_cast<uint8>(BodyTextureScaleMode)
		, 0, 0
		, DataOffset);

	FillVector2ToData(Data, FVector2f(width, height), DataOffset);

	FillVector4ToData(Data, GetValueWithUnitMode(CornerRadius, CornerRadiusUnitMode, width, height, 0.5f), DataOffset);
	FillColorToData(Data, BodyColor, DataOffset);
	FillVector2ToData(Data
		, (BodyTextureMode == ELexRectBlockTextureMode::Sprite && IsValid(BodySpriteTexture)) ? FVector2f(BodySpriteTexture->GetSpriteInfo().GetUVCenter()) : FVector2f(0.5f, 0.5f)
		, DataOffset);

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

float ULexRectBlock::GetValueWithUnitMode(float SourceValue, ELexRectBlockUnitMode UnitMode, float RectWidth, float RectHeight, float AdditionalScale)const
{
	return UnitMode == ELexRectBlockUnitMode::Value ? SourceValue : (SourceValue * (RectWidth < RectHeight ? RectWidth : RectHeight) * AdditionalScale);
}
FVector4f ULexRectBlock::GetValueWithUnitMode(const FVector4f& SourceValue, ELexRectBlockUnitMode UnitMode, float RectWidth, float RectHeight, float AdditionalScale)const
{
	return UnitMode == ELexRectBlockUnitMode::Value ? SourceValue : (SourceValue * (RectWidth < RectHeight ? RectWidth : RectHeight) * AdditionalScale);
}
FVector2f ULexRectBlock::GetValueWithUnitMode(const FVector2f& SourceValue, ELexRectBlockUnitMode UnitMode, float RectWidth, float RectHeight)const
{
	return UnitMode == ELexRectBlockUnitMode::Value ? SourceValue : (SourceValue * FVector2f(RectWidth, RectHeight));
}

FVector2f ULexRectBlock::GetInnerShadowOffset(float RectWidth, float RectHeight)
{
	float AngleRadian = FMath::DegreesToRadians(InnerShadowAngle + 90);
	float Sin = FMath::Sin(AngleRadian);
	float Cos = FMath::Cos(AngleRadian);
	float Distance = GetValueWithUnitMode(InnerShadowDistance, InnerShadowDistanceUnitMode, RectWidth, RectHeight, 0.5f);
	return FVector2f(-Sin, Cos) * Distance;
}
FVector2f ULexRectBlock::GetOuterShadowOffset(float RectWidth, float RectHeight)
{
	float AngleRadian = FMath::DegreesToRadians(OuterShadowAngle + 90);
	float Sin = FMath::Sin(AngleRadian);
	float Cos = FMath::Cos(AngleRadian);
	float Distance = GetValueWithUnitMode(OuterShadowDistance, OuterShadowDistanceUnitMode, RectWidth, RectHeight, 0.5f);
	return FVector2f(-Sin, Cos) * Distance;
}

constexpr int ULexRectBlock::DataCountInBytes()
{
	const int result =
		4//bool and enum
		+ 4//for extra bool and enum

		+ 8//quad size
		+ 16//corner radius
		+ 4//body color
		+ 8//body texture's uv's center point

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

void ULexRectBlock::FillColorToData(uint8* Data, const FColor& InValue, int& InOutDataOffset)
{
	auto ColorUint = InValue.ToPackedRGBA();
	int ByteCount = 4;
	FMemory::Memcpy(Data + InOutDataOffset, &ColorUint, ByteCount);
	InOutDataOffset += ByteCount;
}
uint8 ULexRectBlock::PackBoolToByte(
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
	uint8 Result =
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
void ULexRectBlock::Fill8BytesToData(uint8* Data, uint8 InValue0, uint8 InValue1, uint8 InValue2, uint8 InValue3, int& InOutDataOffset)
{
	int ByteCount = 8;//actually data only cover 4 bytes, but we need extra 4 bytes to make decode easier (because data texture is 16bytes per pixel)
	uint32 DataAsUint =
		(InValue0 << 24)
		| (InValue1 << 16)
		| (InValue2 << 8)
		| (InValue3 << 0)
		;
	FMemory::Memcpy(Data + InOutDataOffset, &DataAsUint, 4);
	InOutDataOffset += ByteCount;
}
void ULexRectBlock::FillFloatToData(uint8* Data, const float& InValue, int& InOutDataOffset)
{
	int ByteCount = 4;
	FMemory::Memcpy(Data + InOutDataOffset, &InValue, ByteCount);
	InOutDataOffset += ByteCount;
}
void ULexRectBlock::FillVector2ToData(uint8* Data, const FVector2f& InValue, int& InOutDataOffset)
{
	int ByteCount = 8;
	FMemory::Memcpy(Data + InOutDataOffset, &InValue, ByteCount);
	InOutDataOffset += ByteCount;
}
void ULexRectBlock::FillVector4ToData(uint8* Data, const FVector4f& InValue, int& InOutDataOffset)
{
	int ByteCount = 16;
	FMemory::Memcpy(Data + InOutDataOffset, &InValue, ByteCount);
	InOutDataOffset += ByteCount;
}
void ULexRectBlock::OnCornerRadiusUnitModeChanged(float width, float height)
{
	if (CornerRadiusUnitMode == ELexRectBlockUnitMode::Value)//from percentage to value
	{
		CornerRadius = CornerRadius * (width < height ? width : height) * 0.5f;
	}
	else//from value to percentage
	{
		CornerRadius = CornerRadius / (width < height ? width : height) * 2.0f;
	}
}

FName ULexRectBlock::DataTextureParameterName = TEXT("LexUI_RectBlockDataTexture");

ULexRectBlock::ULexRectBlock(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	bNeedUpdateBlockData = true;
	BodyTexture = FLexUIUtils::GetDefaultWhiteTexture();
	BodySpriteTexture = ULexUISpriteData::GetDefaultWhiteSolid();
}

void ULexRectBlock::BeginPlay()
{
	Super::BeginPlay();
	if (!bHasAddToSprite)
	{
		if (IsValid(BodySpriteTexture))
		{
			BodySpriteTexture->AddUISprite(this);
			bHasAddToSprite = true;
		}
	}
}
void ULexRectBlock::EndPlay()
{
	if (bHasAddToSprite)
	{
		if (IsValid(BodySpriteTexture))
		{
			BodySpriteTexture->RemoveUISprite(this);
			bHasAddToSprite = false;
		}
	}
}

void ULexRectBlock::OnRegister()
{
	Super::OnRegister();
	if (RectBlockData == nullptr)
	{
		RectBlockData = LoadObject<ULexRectBlockData>(NULL, TEXT("/LGUI/DefaultRectBlockData"));
		check(RectBlockData != nullptr);
	}
	RectBlockData->Init(DataCountInBytes(), ELexUIDataAsTexturePixelFormat::R32G32B32A32, 32);
	DataStartPosition = RectBlockData->RegisterBuffer();
	OnDataTextureChangedDelegateHandle = RectBlockData->OnDataTextureChange.AddUObject(this, &ULexRectBlock::OnDataTextureChanged);
#if WITH_EDITOR
	if (this->GetWorld() && this->GetWorld()->WorldType == EWorldType::Editor)
	{
		if (!bHasAddToSprite)
		{
			if (IsValid(BodySpriteTexture))
			{
				BodySpriteTexture->AddUISprite(this);
				bHasAddToSprite = true;
			}
		}
	}
#endif
}
void ULexRectBlock::OnUnregister()
{
	Super::OnUnregister();
	RectBlockData->UnregisterBuffer(DataStartPosition);
	if (OnDataTextureChangedDelegateHandle.IsValid())
	{
		RectBlockData->OnDataTextureChange.Remove(OnDataTextureChangedDelegateHandle);
		OnDataTextureChangedDelegateHandle.Reset();
	}
#if WITH_EDITOR
	if (this->GetWorld() && this->GetWorld()->WorldType == EWorldType::Editor)
	{
		if (bHasAddToSprite)
		{
			if (IsValid(BodySpriteTexture))
			{
				BodySpriteTexture->RemoveUISprite(this);
				bHasAddToSprite = false;
			}
		}
	}
#endif
}

#if WITH_EDITOR
void ULexRectBlock::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{

#define SetUnitChange(Property)\
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexRectBlock, Property##UnitMode))\
	{\
		this->On##Property##UnitModeChanged(GetWidget()->GetWidth(), GetWidget()->GetHeight());\
	}


	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		if (!this->GetName().StartsWith("Default__"))
		{
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

		else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexRectBlock, BodyTextureMode))
		{
			if (!this->GetName().StartsWith("Default__"))
			{
				MarkTextureDirty();
				MarkVertexUVDirty();
			}
		}
		
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexRectBlock, bUniformSetCornerRadius))
		{
			if (bUniformSetCornerRadius)
			{
				CornerRadius.Y = CornerRadius.Z = CornerRadius.W = CornerRadius.X;
			}
		}
	}
}

bool ULexRectBlock::CanEditChange(const FProperty* InProperty) const
{
	auto PropertyName = InProperty->GetFName();
	static auto RaycastSupportCornerRadius_Name = GET_MEMBER_NAME_CHECKED(ULexRectBlock, bRaycastSupportCornerRadius);
	if (PropertyName == RaycastSupportCornerRadius_Name)
	{
		if (!this->GetName().StartsWith("Default__"))
		{
			if (!GetWidget()->GetRaycastableInHierarchy() || RaycastType != ELexVisualRaycastType::Rect)
			{
				return false;
			}
		}
	}
	return Super::CanEditChange(InProperty);
}

void ULexRectBlock::OnPreChangeSpriteProperty()
{
	if (IsValid(BodySpriteTexture))
	{
		BodySpriteTexture->RemoveUISprite(this);
		bHasAddToSprite = false;
	}
}
void ULexRectBlock::OnPostChangeSpriteProperty()
{
	if (IsValid(BodySpriteTexture))
	{
		BodySpriteTexture->AddUISprite(this);
		bHasAddToSprite = true;
	}
}
#endif

void ULexRectBlock::OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange)
{
	Super::OnDimensionChanged(InPivotChange, InWidthChange, InHeightChange);
}

void ULexRectBlock::OnBeforeCreateOrUpdateGeometry()
{
	
}

UTexture* ULexRectBlock::GetTextureToCreateGeometry()
{
	if (BodyTextureMode == ELexRectBlockTextureMode::Texture)
	{
		if (!IsValid(this->BodyTexture))
		{
			this->BodyTexture = FLexUIUtils::GetDefaultWhiteTexture();
		}
		return this->BodyTexture;
	}
	else
	{
		if (!IsValid(BodySpriteTexture))
		{
			BodySpriteTexture = ULexUISpriteData::GetDefaultWhiteSolid();
		}
		if (IsValid(BodySpriteTexture) && IsValid(BodySpriteTexture->GetAtlasTexture()))
		{
			return BodySpriteTexture->GetAtlasTexture();
		}
	}
	return nullptr;
}
UMaterialInterface* ULexRectBlock::GetMaterialToCreateGeometry()
{
	if (auto Result = Super::GetMaterialToCreateGeometry())
	{
		return Result;
	}
	else
	{
		check(RectBlockData);
		return RectBlockData->GetMaterial();
	}
}
void ULexRectBlock::OnMaterialInstanceDynamicCreated(class UMaterialInstanceDynamic* mat) 
{
	mat->SetTextureParameterValue(DataTextureParameterName, RectBlockData->GetDataTexture());
}

void ULexRectBlock::MarkAllDirty()
{
	Super::MarkAllDirty();
	bNeedUpdateBlockData = true;
}

bool ULexRectBlock::GetAnythingDirty() const
{
	return Super::GetAnythingDirty() || bNeedUpdateBlockData;
}

void ULexRectBlock::MarkNeedUpdateBlockData()
{
	bNeedUpdateBlockData = true;
	GetWidget()->MarkCanvasUpdate(false);
}

bool ULexRectBlock::LineTraceUI_CheckCornerRadius(const FVector2D& InLocalHitPoint)const
{
	auto Widget = GetWidget();
	auto TempCornerRadius = GetValueWithUnitMode(CornerRadius, CornerRadiusUnitMode, Widget->GetWidth(), Widget->GetHeight(), 0.5f);
	auto HalfWidth = Widget->GetWidth() * 0.5f;
	auto HalfHeight = Widget->GetHeight() * 0.5f;
	auto MinSize = FMath::Min(HalfWidth, HalfHeight);
	TempCornerRadius.X = FMath::Min(TempCornerRadius.X, MinSize);
	TempCornerRadius.Y = FMath::Min(TempCornerRadius.Y, MinSize);
	TempCornerRadius.Z = FMath::Min(TempCornerRadius.Z, MinSize);
	TempCornerRadius.W = FMath::Min(TempCornerRadius.W, MinSize);
	if (InLocalHitPoint.X > 0 && InLocalHitPoint.Y < 0)//right bottom area of rect
	{
		auto Radius = TempCornerRadius.X;
		auto CenterPos = FVector2D(Widget->GetLocalSpaceRight() - Radius, Widget->GetLocalSpaceBottom() + Radius);
		if (InLocalHitPoint.X > CenterPos.X && InLocalHitPoint.Y < CenterPos.Y)
		{
			if (FVector2D::DistSquared(InLocalHitPoint, CenterPos) > Radius * Radius)
			{
				return false;
			}
		}
		return true;
	}
	else if (InLocalHitPoint.X > 0 && InLocalHitPoint.Y > 0)//right top area of rect
	{
		auto Radius = TempCornerRadius.Y;
		auto CenterPos = FVector2D(Widget->GetLocalSpaceRight() - Radius, Widget->GetLocalSpaceTop() - Radius);
		if (InLocalHitPoint.X > CenterPos.X && InLocalHitPoint.Y > CenterPos.Y)
		{
			if (FVector2D::DistSquared(InLocalHitPoint, CenterPos) > Radius * Radius)
			{
				return false;
			}
		}
		return true;
	}
	else if (InLocalHitPoint.X < 0 && InLocalHitPoint.Y > 0)//left top area of rect
	{
		auto Radius = TempCornerRadius.Z;
		auto CenterPos = FVector2D(Widget->GetLocalSpaceLeft() + Radius, Widget->GetLocalSpaceTop() - Radius);
		if (InLocalHitPoint.X < CenterPos.X && InLocalHitPoint.Y > CenterPos.Y)
		{
			if (FVector2D::DistSquared(InLocalHitPoint, CenterPos) > Radius * Radius)
			{
				return false;
			}
		}
		return true;
	}
	else if (InLocalHitPoint.X < 0 && InLocalHitPoint.Y < 0)//left bottom area of rect
	{
		auto Radius = TempCornerRadius.W;
		auto CenterPos = FVector2D(Widget->GetLocalSpaceLeft() + Radius, Widget->GetLocalSpaceBottom() + Radius);
		if (InLocalHitPoint.X < CenterPos.X && InLocalHitPoint.Y < CenterPos.Y)
		{
			if (FVector2D::DistSquared(InLocalHitPoint, CenterPos) > Radius * Radius)
			{
				return false;
			}
		}
		return true;
	}
	return true;
}
bool ULexRectBlock::LineTraceUIRect(FLexUIHitResult& OutHit, const FVector& Start, const FVector& End)const
{
	auto Widget = GetWidget();
	auto InverseTf = Widget->GetWorldTransform().Inverse();
	auto LocalSpaceRayOrigin = InverseTf.TransformPosition(Start);
	auto LocalSpaceRayEnd = InverseTf.TransformPosition(End);

	//DrawDebugLine(this->GetWorld(), Start, End, FColor::Red, false);//just for test
	//start and end point must be different side of X plane
	if (FMath::Sign(LocalSpaceRayOrigin.X) != FMath::Sign(LocalSpaceRayEnd.X))
	{
		auto result = FMath::LinePlaneIntersection(LocalSpaceRayOrigin, LocalSpaceRayEnd, FVector::ZeroVector, FVector(1, 0, 0));
		//hit point inside rect area
		if (result.Y > Widget->GetLocalSpaceLeft() && result.Y < Widget->GetLocalSpaceRight() && result.Z > Widget->GetLocalSpaceBottom() && result.Z < Widget->GetLocalSpaceTop())
		{
			OutHit.TraceStart = Start;
			OutHit.TraceEnd = End;
			OutHit.Widget = Widget;
			OutHit.Location = Widget->GetWorldTransform().TransformPosition(result);
			OutHit.Normal = Widget->GetWorldTransform().TransformVector(FVector(1, 0, 0));
			OutHit.Normal.Normalize();
			OutHit.Distance = FVector::Distance(Start, OutHit.Location);
			OutHit.ImpactPoint = OutHit.Location;

			//check corner radius
			if (bRaycastSupportCornerRadius)
			{
				return LineTraceUI_CheckCornerRadius(FVector2D(result.Y, result.Z));
			}

			return true;
		}
	}
	return false;
}

void ULexRectBlock::OnDataTextureChanged(class UTexture* Texture)
{
	UIGeometry->Texture = GetTextureToCreateGeometry();
	MarkVerticesDirty(false, true, true, false);
}

void ULexRectBlock::OnUpdateGeometry(FLexUIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	auto Widget = GetWidget();
	static FLexUISpriteInfo SimpleRectSpriteData;
	FLexUIGeometry::UpdateRectBlockVertex(&InGeo
		, this->bEnableOuterShadow
		, this->GetOuterShadowOffset(Widget->GetWidth(), Widget->GetHeight())
		, this->GetValueWithUnitMode(OuterShadowSize, OuterShadowSizeUnitMode, Widget->GetWidth(), Widget->GetHeight(), 0.5f)
		, this->GetValueWithUnitMode(OuterShadowBlur, OuterShadowBlurUnitMode, Widget->GetWidth(), Widget->GetHeight(), 1)
		, this->bSoftEdge,
		Widget->GetWidth(), Widget->GetHeight(), FVector2f(Widget->GetPivot())
		, SimpleRectSpriteData, BodyTextureMode == ELexRectBlockTextureMode::Sprite ? (IsValid(BodySpriteTexture) ? BodySpriteTexture->GetSpriteInfo() : SimpleRectSpriteData) : SimpleRectSpriteData
		, Widget->GetRenderCanvas(), this, GetFinalColor(),
		InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
	);

	if (InTriangleChanged || InVertexPositionChanged || InVertexUVChanged || InVertexColorChanged)
	{
		auto& vertices = InGeo.Vertices;
		for (int i = 0; i < 4; i++)
		{
			vertices[i].TextureCoordinate[3].X = DataStartPosition;
		}
		bNeedUpdateBlockData = true;
	}

	if (bNeedUpdateBlockData)
	{
		bNeedUpdateBlockData = false;

		auto BlockSize = RectBlockData->GetBlockSizeInByte();
		TArray<uint8> BlockBuffer;
		BlockBuffer.SetNumUninitialized(BlockSize);
		FMemory::Memzero(BlockBuffer.GetData(), BlockSize);
		FillData(BlockBuffer.GetData(), Widget->GetWidth(), Widget->GetHeight());
		RectBlockData->UpdateBlock(DataStartPosition, MoveTemp(BlockBuffer));
	}
}

void ULexRectBlock::ApplyAtlasTextureChange_Implementation()
{
	if (BodyTextureMode != ELexRectBlockTextureMode::Sprite)return;
	check(BodySpriteTexture);
	UIGeometry->Texture = BodySpriteTexture->GetAtlasTexture();
	GetWidget()->MarkCanvasUpdate(true);
}

void ULexRectBlock::SetCornerRadius(const FVector4& value)
{
	this->CornerRadius = FVector4f(value);
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetEnableBody(bool value)
{
	this->bEnableBody = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}
void ULexRectBlock::SetBodyColor(const FColor& value)
{
	this->BodyColor = value;
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetBodyTexture(UTexture* value)
{
	this->BodyTexture = value;
	if (this->BodyTexture == nullptr)
	{
		this->BodyTexture = FLexUIUtils::GetDefaultWhiteTexture();
	}
	MarkTextureDirty();
}
void ULexRectBlock::SetBodySpriteTexture(ULexUISpriteData_BaseObject* value)
{
	if (this->BodySpriteTexture != value)
	{
		this->BodySpriteTexture = value;
		if ((!IsValid(BodySpriteTexture) || !IsValid(value))
			|| (BodySpriteTexture->GetAtlasTexture() != value->GetAtlasTexture()))
		{
			//remove from old
			if (IsValid(BodySpriteTexture))
			{
				BodySpriteTexture->RemoveUISprite(this);
				bHasAddToSprite = false;
			}
			//add to new
			if (IsValid(value))
			{
				value->AddUISprite(this);
				bHasAddToSprite = true;
			}
			MarkTextureDirty();
		}
		BodySpriteTexture = value;
		MarkVertexUVDirty();
	}
}
void ULexRectBlock::SetBodyTextureMode(ELexRectBlockTextureMode value)
{
	this->BodyTextureMode = value;
	MarkTextureDirty();
	MarkVertexUVDirty();
}
void ULexRectBlock::SetSizeFromBodyTexture()
{
	if (BodyTextureMode == ELexRectBlockTextureMode::Sprite)
	{
		if (IsValid(this->BodySpriteTexture))
		{
			GetWidget()->SetWidth(this->BodySpriteTexture->GetSpriteInfo().GetSourceWidth());
			GetWidget()->SetHeight(this->BodySpriteTexture->GetSpriteInfo().GetSourceHeight());
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d Sprite is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		}
	}
	else
	{
		if (IsValid(this->BodyTexture))
		{
			GetWidget()->SetWidth(this->BodyTexture->GetSurfaceWidth());
			GetWidget()->SetHeight(this->BodyTexture->GetSurfaceHeight());
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d Texture is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		}
	}
}
void ULexRectBlock::SetSoftEdge(bool value)
{
	this->bSoftEdge = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}
void ULexRectBlock::SetBodyTextureScaleMode(ELexRectBlockTextureScaleMode value)
{
	this->BodyTextureScaleMode = value;
	MarkNeedUpdateBlockData();
}

void ULexRectBlock::SetEnableBodyGradient(bool value)
{
	this->bEnableBodyGradient = value;
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetBodyGradientColor(const FColor& value)
{
	this->BodyGradientColor = value;
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetBodyGradientCenter(const FVector2D& value)
{
	this->BodyGradientCenter = FVector2f(value);
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetBodyGradientRadius(const FVector2D& value)
{
	this->BodyGradientRadius = FVector2f(value);
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetBodyGradientRotation(float value)
{
	this->BodyGradientRotation = value;
	MarkNeedUpdateBlockData();
}

void ULexRectBlock::SetEnableBorder(bool value)
{
	this->bEnableBorder = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}
void ULexRectBlock::SetBorderWidth(float value)
{
	this->BorderWidth = value;
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetBorderColor(const FColor& value)
{
	this->BorderColor = value;
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetEnableBorderGradient(bool value)
{
	this->bEnableBorderGradient = value;
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetBorderGradientColor(const FColor& value)
{
	this->BorderGradientColor = value;
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetBorderGradientCenter(const FVector2D& value)
{
	this->BorderGradientCenter = FVector2f(value);
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetBorderGradientRadius(const FVector2D& value)
{
	this->BorderGradientRadius = FVector2f(value);
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetBorderGradientRotation(float value)
{
	this->BorderGradientRotation = value;
	MarkNeedUpdateBlockData();
}

void ULexRectBlock::SetEnableInnerShadow(bool value)
{
	this->bEnableInnerShadow = value;
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetInnerShadowColor(const FColor& value)
{
	this->InnerShadowColor = value;
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetInnerShadowSize(float value)
{
	this->InnerShadowSize = value;
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetInnerShadowBlur(float value)
{
	this->InnerShadowBlur = value;
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetInnerShadowAngle(float value)
{
	this->InnerShadowAngle = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}
void ULexRectBlock::SetInnerShadowDistance(float value)
{
	this->InnerShadowDistance = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}

void ULexRectBlock::SetEnableRadialFill(bool value)
{
	this->bEnableRadialFill = value;
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetRadialFillCenter(const FVector2D& value)
{
	this->RadialFillCenter = FVector2f(value);
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetRadialFillRotation(float value)
{
	this->RadialFillRotation = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}
void ULexRectBlock::SetRadialFillAngle(float value)
{
	this->RadialFillAngle = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}

void ULexRectBlock::SetEnableOuterShadow(bool value)
{
	this->bEnableOuterShadow = value;
	MarkAllDirty();
}
void ULexRectBlock::SetOuterShadowColor(const FColor& value)
{
	this->OuterShadowColor = value;
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetOuterShadowSize(float value)
{
	this->OuterShadowSize = value;
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetOuterShadowBlur(float value)
{
	this->OuterShadowBlur = value;
	MarkNeedUpdateBlockData();
}
void ULexRectBlock::SetOuterShadowAngle(float value)
{
	this->OuterShadowAngle = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}
void ULexRectBlock::SetOuterShadowDistance(float value)
{
	this->OuterShadowDistance = value;
	bNeedUpdateBlockData = true;
	MarkVertexPositionDirty();
}

#define FunctionSetPropertyUnitMode(Property)\
void ULexRectBlock::Set##Property##UnitMode(ELexRectBlockUnitMode value)\
{\
	this->Property##UnitMode = value;\
	MarkNeedUpdateBlockData();\
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

void ULexRectBlock::SetRaycastSupportCornerRadius(bool value)
{
	bRaycastSupportCornerRadius = value;
}

#pragma region TweenAnimation
#include "LTweenManager.h"
ULTweener* ULexRectBlock::BodyColorTo(FColor endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this, FLTweenColorGetterFunction::CreateWeakLambda(this, [this] {
		return this->BodyColor;
		}), FLTweenColorSetterFunction::CreateWeakLambda(this, [this](FColor value) {
			this->SetBodyColor(value);
			}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetEase(ease)->SetDelay(delay);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(GetWidget(), Tweener);
	}
	return Tweener;
}
ULTweener* ULexRectBlock::BodyAlphaTo(float endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateWeakLambda(this, [this] {
		return FLexUIUtils::ByteToFloat01(this->BodyColor.A);
		}), FLTweenFloatSetterFunction::CreateWeakLambda(this, [this](float value) {
			auto PropertyValue = this->BodyColor;
			PropertyValue.A = static_cast<uint8>(value * 255.0f);
			this->SetBodyColor(PropertyValue);
			}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetEase(ease)->SetDelay(delay);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(GetWidget(), Tweener);
	}
	return Tweener;
}

#define FunctionPropertyAnimation(Property, EndValueType, GetterAndSetterType)\
ULTweener* ULexRectBlock::Property##To(EndValueType endValue, float duration, float delay, ELTweenEase ease)\
{\
	auto Tweener =  ULTweenManager::To(this, FLTween##GetterAndSetterType##GetterFunction::CreateWeakLambda(this, [this] {\
		return (EndValueType)this->Property;\
		}), FLTween##GetterAndSetterType##SetterFunction::CreateWeakLambda(this, [this](EndValueType value) {\
			this->Set##Property(value);\
			}), endValue, duration);\
	if (Tweener)\
	{\
		Tweener->SetEase(ease)->SetDelay(delay);\
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(GetWidget(), Tweener);\
	}\
	return Tweener;\
}

#define FunctionAlphaAnimation(Property, Function)\
ULTweener* ULexRectBlock::Function##AlphaTo(float endValue, float duration, float delay, ELTweenEase ease)\
{\
	auto Tweener =  ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateWeakLambda(this, [this] {\
		return FLexUIUtils::ByteToFloat01(this->BodyColor.A);\
		}), FLTweenFloatSetterFunction::CreateWeakLambda(this, [this](float value) {\
			auto PropertyValue = this->Property;\
			PropertyValue.A = (uint8)(value * 255.0f);\
			this->Set##Property(PropertyValue);\
			}), endValue, duration);\
	if (Tweener)\
	{\
		Tweener->SetEase(ease)->SetDelay(delay);\
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(GetWidget(), Tweener);\
	}\
	return Tweener;\
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


