// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexVisual.h"
#include "LGUI.h"
#include "Core/Components/LexCanvas.h"
#include "Utils/LexUIUtils.h"
#include "LGUI/Public/MeshModifier/LexMeshModifierBase.h"
#include "Core/Components/LexVisualBatchMesh.h"
#include "TextureResource.h"
#include "Core/LexUIClipData.h"
#include "Core/LexUIDataAsTexture.h"
#include "Core/Components/LexWidget.h"
#include "Engine/Texture2D.h"
#include "Event/LexPointerEventData.h"


bool ULexVisualCustomRaycast::Raycast(const ULexVisual* InVisual, const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, FVector& OutHitPoint, FVector& OutHitNormal)const
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		return ReceiveRaycast(InVisual, InLocalSpaceRayStart, InLocalSpaceRayEnd, OutHitPoint, OutHitNormal);
	}
	return false;
}

bool ULexVisualCustomRaycast::GetRaycastPixelFromUIBatchMeshVisual(const ULexVisualBatchMesh* InVisual, const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, FVector2D& OutUV, FColor& OutPixel, FVector& OutHitPoint, FVector& OutHitNormal)
{
	if (auto UIGeo = InVisual->GetGeometry())
	{
		//triangle hit test
		auto& originVertices = UIGeo->OriginVertices;
		auto& vertices = UIGeo->Vertices;
		auto& triangleIndices = UIGeo->Triangles;
		const int triangleCount = triangleIndices.Num() / 3;
		int index = 0;
		for (int i = 0; i < triangleCount; i++)
		{
			auto vertIndex0 = triangleIndices[index++];
			auto vertIndex1 = triangleIndices[index++];
			auto vertIndex2 = triangleIndices[index++];
			auto point0 = (FVector)(originVertices[vertIndex0].Position);
			auto point1 = (FVector)(originVertices[vertIndex1].Position);
			auto point2 = (FVector)(originVertices[vertIndex2].Position);
			if (FMath::SegmentTriangleIntersection(InLocalSpaceRayStart, InLocalSpaceRayEnd, point0, point1, point2, OutHitPoint, OutHitNormal))
			{
				auto baryCentric = FMath::ComputeBaryCentric2D(OutHitPoint, point0, point1, point2);
				auto& uv0 = vertices[vertIndex0].TextureCoordinate[0];
				auto& uv1 = vertices[vertIndex1].TextureCoordinate[0];
				auto& uv2 = vertices[vertIndex2].TextureCoordinate[0];
				OutUV = FVector2D(baryCentric.X * uv0 + baryCentric.Y * uv1 + baryCentric.Z * uv2);
				//get pixel
				if (auto Texture2D = Cast<UTexture2D>(UIGeo->Texture.Get()))
				{
					auto PlatformData = Texture2D->GetPlatformData();
					if (PlatformData && PlatformData->Mips.Num() > 0)
					{
						auto TexPosX = (int)(OutUV.X * PlatformData->SizeX);
						auto TexPosY = (int)(OutUV.Y * PlatformData->SizeY);
						auto TexPos = TexPosX + TexPosY * PlatformData->SizeX;

						if (auto Pixels = (FColor*)(PlatformData->Mips[0].BulkData.Lock(LOCK_READ_ONLY)))
						{
							OutPixel = Pixels[TexPos];
						}
						PlatformData->Mips[0].BulkData.Unlock();

						return true;
					}
				}
			}
		}
	}
	return false;
}


ULexVisual::ULexVisual(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	VisualType = ELexVisualType::None;

	bColorChanged = true;
	bTransformChanged = true;
	bClipDataPositionChanged = true;
	bWidgetPropertyDataStartPositionChanged = true;
	bWidgetPropertyDataFontMarkDirty = true;
}

void ULexVisual::OnRegister()
{
	Super::OnRegister();
	bColorChanged = true;
	bTransformChanged = true;
}

void ULexVisual::OnUnregister()
{
	Super::OnUnregister();
}

void ULexVisual::PostReinitProperties()
{
	Super::PostReinitProperties();
#if WITH_EDITOR
	if (!this->GetName().StartsWith("Default__"))
	{
		if (auto Widget = GetWidget())
		{
			if (auto World = Widget->GetWorld())
			{
				if (!World->IsGameWorld())
				{
					MarkAllDirty();
				}
			}
		}
	}
#endif
}

#if WITH_EDITOR
void ULexVisual::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (!this->GetName().StartsWith("Default__"))
	{
		MarkAllDirty();
	}
}

bool ULexVisual::CanEditChange(const FProperty* InProperty) const
{
	auto PropertyName = InProperty->GetFName();
	static auto RayCastType_Name = GET_MEMBER_NAME_CHECKED(ULexVisual, RaycastType);
	static auto CustomRaycastObject_Name = GET_MEMBER_NAME_CHECKED(ULexVisual, CustomRaycastObject);
	static auto VisiblePixelThreshold_Name = GET_MEMBER_NAME_CHECKED(ULexVisual, VisiblePixelThreshold);
	if (PropertyName == RayCastType_Name)
	{
		if (!this->GetName().StartsWith("Default__"))
		{
			if (!GetWidget()->GetRaycastableInHierarchy())
			{
				return false;
			}
		}
	}
	else if (PropertyName == CustomRaycastObject_Name)
	{
		if (!this->GetName().StartsWith("Default__"))
		{
			if (!GetWidget()->GetRaycastableInHierarchy() || RaycastType!=ELexVisualRaycastType::Custom)
			{
				return false;
			}
		}
	}
	else if (PropertyName == VisiblePixelThreshold_Name)
	{
		if (!this->GetName().StartsWith("Default__"))
		{
			if (!GetWidget()->GetRaycastableInHierarchy() || RaycastType!=ELexVisualRaycastType::VisiblePixel)
			{
				return false;
			}
		}
	}
	return UObject::CanEditChange(InProperty);
}
#endif

int ULexVisual::GetClipDataStartPosition() const
{
	if (auto ClipData = GetWidget()->GetClipData().Pin())
	{
		return ClipData->GetBufferStartPos();
	}
	return 0;
}

UTexture* ULexVisual::GetClipDataTexture() const
{
	if (auto RenderCanvas = GetWidget()->GetRenderCanvas())
	{
		return RenderCanvas->GetClipDataTexture();
	}
	return nullptr;
}

void ULexVisual::OnTransformChanged(bool InPositionChanged, bool InScaleChanged)
{
	bTransformChanged = true;
}

void ULexVisual::OnRenderCanvasChanged(ULexCanvas* InOldCanvas, ULexCanvas* InNewCanvas)
{
	bWidgetPropertyDataFontMarkDirty = true;
}

void ULexVisual::MarkColorDirty()
{
	bColorChanged = true;
	GetWidget()->MarkCanvasUpdate(false);
}

void ULexVisual::CheckClipDataStartPosition()
{
	auto NowClipDataStartPosition = GetClipDataStartPosition();
	if (ClipDataStartPosition != NowClipDataStartPosition)
	{
		ClipDataStartPosition = NowClipDataStartPosition;
		bClipDataPositionChanged = true;
	}
}

void ULexVisual::UpdateGeometryWidgetPropertyData(TArray<FLexUIMeshVertex>& InVertices, int InValidNumVertices, int InDataStartPosition)
{
	for (int i = 0; i < InValidNumVertices; i++)
	{
		InVertices[i].TextureCoordinate[1].X = InDataStartPosition;
	}
}

void ULexVisual::MarkAllDirty()
{
	bColorChanged = true;
	bTransformChanged = true;
	bClipDataPositionChanged = true;
	bWidgetPropertyDataStartPositionChanged = true;
	bWidgetPropertyDataFontMarkDirty = true;
	GetWidget()->MarkCanvasUpdate(true);
}

void ULexVisual::GetGeometryBoundsInLocalSpace(FVector2D& OutMinPoint, FVector2D& OutMaxPoint)const
{
	auto Widget = GetWidget();
	OutMinPoint = Widget->GetLocalSpaceLeftBottomPoint();
	OutMaxPoint = Widget->GetLocalSpaceRightTopPoint();
}

void ULexVisual::GetGeometryBounds3DInLocalSpace(FVector& OutMinPoint, FVector& OutMaxPoint)const
{
	auto Widget = GetWidget();
	const auto MinPoint2D = Widget->GetLocalSpaceLeftBottomPoint();
	const auto MaxPoint2D = Widget->GetLocalSpaceRightTopPoint();
	OutMinPoint = FVector(0.1f, MinPoint2D.X, MinPoint2D.Y);
	OutMaxPoint = FVector(0.1f, MaxPoint2D.X, MaxPoint2D.Y);
}

void ULexVisual::SetWidgetPropertyDataStartPosition(int InPosition)
{
	if (WidgetPropertyDataStartPosition != InPosition)
	{
		WidgetPropertyDataStartPosition = InPosition;
		bWidgetPropertyDataStartPositionChanged = true;

		//these data store inside DataTexture and use WidgetPropertyDataStartPosition as coordinate, so mark these dirty to fill data
		bWidgetPropertyDataFontMarkDirty = true;
		bClipDataPositionChanged = true;
	}
}

bool ULexVisual::LineTraceUIRect(FLexUIHitResult& OutHit, const FVector& Start, const FVector& End)const
{
	auto Widget = GetWidget();
	auto InverseTf = Widget->GetWorldTransform().Inverse();
	auto LocalSpaceRayOrigin = InverseTf.TransformPosition(Start);
	auto LocalSpaceRayEnd = InverseTf.TransformPosition(End);

	//DrawDebugLine(this->GetWorld(), Start, End, FColor::Red, false);//just for test
	//start and end point must be different side of X plane
	if (FMath::Sign(LocalSpaceRayOrigin.X) != FMath::Sign(LocalSpaceRayEnd.X))
	{
		auto Result = FMath::LinePlaneIntersection(LocalSpaceRayOrigin, LocalSpaceRayEnd, FVector::ZeroVector, FVector(1, 0, 0));
		//hit point inside rect area
		if (Result.Y > Widget->GetLocalSpaceLeft() && Result.Y < Widget->GetLocalSpaceRight() && Result.Z > Widget->GetLocalSpaceBottom() && Result.Z < Widget->GetLocalSpaceTop())
		{
			OutHit.TraceStart = Start;
			OutHit.TraceEnd = End;
			OutHit.Widget = Widget;
			OutHit.Location = Widget->GetWorldTransform().TransformPosition(Result);
			OutHit.Normal = Widget->GetWorldTransform().TransformVector(FVector(1, 0, 0));
			OutHit.Normal.Normalize();
			OutHit.Distance = FVector::Distance(Start, OutHit.Location);
			OutHit.ImpactPoint = OutHit.Location;
			return true;
		}
	}
	return false;
}
bool ULexVisual::LineTraceUIGeometry(FLexUIGeometry* InGeo, FLexUIHitResult& OutHit, const FVector& Start, const FVector& End)const
{
	auto Widget = GetWidget();
	const auto InverseTf = Widget->GetWorldTransform().Inverse();
	const auto LocalSpaceRayOrigin = InverseTf.TransformPosition(Start);
	const auto LocalSpaceRayEnd = InverseTf.TransformPosition(End);

	//DrawDebugLine(this->GetWorld(), Start, End, FColor::Red, false, 5.0f);//just for test
	//check Line-Plane intersection first, then check Line-Triangle
	//start and end point must be different side of X plane
	if (FMath::Sign(LocalSpaceRayOrigin.X) != FMath::Sign(LocalSpaceRayEnd.X))
	{
		//triangle hit test
		auto& vertices = InGeo->OriginVertices;
		auto& triangleIndices = InGeo->Triangles;
		const int triangleCount = triangleIndices.Num() / 3;
		int index = 0;
		for (int i = 0; i < triangleCount; i++)
		{
			auto point0 = (FVector)(vertices[triangleIndices[index++]].Position);
			auto point1 = (FVector)(vertices[triangleIndices[index++]].Position);
			auto point2 = (FVector)(vertices[triangleIndices[index++]].Position);
			FVector HitPoint, HitNormal;
			if (FMath::SegmentTriangleIntersection(LocalSpaceRayOrigin, LocalSpaceRayEnd, point0, point1, point2, HitPoint, HitNormal))
			{
				OutHit.TraceStart = Start;
				OutHit.TraceEnd = End;
				OutHit.Widget = Widget;//actually this convert is incorrect, but I need this pointer
				OutHit.Location = Widget->GetWorldTransform().TransformPosition(HitPoint);
				OutHit.Normal = Widget->GetWorldTransform().TransformVector(HitNormal);
				OutHit.Normal.Normalize();
				OutHit.Distance = FVector::Distance(Start, OutHit.Location);
				OutHit.ImpactPoint = OutHit.Location;
				OutHit.FaceIndex = i;
				return true;
			}
		}
	}
	return false;
}

bool ULexVisual::LineTraceUICustom(FLexUIHitResult& OutHit, const FVector& Start, const FVector& End)const
{
	if (!IsValid(CustomRaycastObject))
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d EUIRenderableRaycastType::Custom need a UUIRenderableCustomRaycast component on this actor!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return false;
	}
	auto Widget = GetWidget();
	const auto InverseTf = Widget->GetWorldTransform().Inverse();
	const auto LocalSpaceRayOrigin = InverseTf.TransformPosition(Start);
	const auto LocalSpaceRayEnd = InverseTf.TransformPosition(End);

	if (FMath::Sign(LocalSpaceRayOrigin.X) != FMath::Sign(LocalSpaceRayEnd.X))
	{
		FVector HitPoint, HitNormal;
		if (CustomRaycastObject->Raycast(this, LocalSpaceRayOrigin, LocalSpaceRayEnd, HitPoint, HitNormal))
		{
			OutHit.TraceStart = Start;
			OutHit.TraceEnd = End;
			OutHit.Widget = Widget;//actually this convert is incorrect, but I need this pointer
			OutHit.Location = Widget->GetWorldTransform().TransformPosition(HitPoint);
			OutHit.Normal = Widget->GetWorldTransform().TransformVector(HitNormal);
			OutHit.Normal.Normalize();
			OutHit.Distance = FVector::Distance(Start, OutHit.Location);
			OutHit.ImpactPoint = OutHit.Location;
			return true;
		}
	}
	return false;
}

void ULexVisual::SetColor(FColor Value)
{
	if (Color != Value)
	{
		Color = Value;
		MarkColorDirty();
	}
}
void ULexVisual::SetAlpha(float Value)
{
	Value = FMath::Clamp(Value, 0.0f, 1.0f);
	auto uintAlpha = (uint8)(Value * 255);
	if (Color.A != uintAlpha)
	{
		MarkColorDirty();
		Color.A = uintAlpha;
	}
}

void ULexVisual::SetRaycastTarget(bool Value)
{
	bRaycastTarget = Value;
}

void ULexVisual::SetCustomRaycastObject(ULexVisualCustomRaycast* Value)
{
	CustomRaycastObject = Value;
}

FColor ULexVisual::GetFinalColor()const
{
	FColor Result = this->Color;
	Result.A = Result.A * GetWidget()->GetFinalRenderOpacity();
	return Result;
}

uint8 ULexVisual::GetFinalAlpha()const
{
	return Color.A * GetWidget()->GetFinalRenderOpacity();
}

float ULexVisual::GetFinalAlpha01()const
{
	return FLexUIUtils::ByteToFloat01(GetFinalAlpha());
}

bool ULexVisual::LineTraceUI(FLexUIHitResult& OutHit, const FVector& Start, const FVector& End)const
{
	return LineTraceUIRect(OutHit, Start, End);
}

DECLARE_CYCLE_STAT(TEXT("LexVisual FillWidgetPropertyDataForMaterial"), STAT_FillWidgetPropertyData, STATGROUP_LGUI);
int ULexVisual::WidgetPropertyDataLength =
	sizeof(float)//1st pixel, byte1- font mark, byte2- extra marks
	+ sizeof(float)//2nd pixel, clip data coordinate
	+ sizeof(float)//3rd pixel, widget width & height, half precision float
	+ sizeof(float)//4th pixel, widget rect center position in canvas space, half precision float
;
void ULexVisual::FillWidgetPropertyDataForMaterial(bool bNeedSize, bool bNeedCenterPosition)const
{
	SCOPE_CYCLE_COUNTER(STAT_FillWidgetPropertyData);
	auto StartPosition = this->WidgetPropertyDataStartPosition;
	if (StartPosition == INDEX_NONE)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d WidgetPropertyDataStartPosition is invalid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		check(0);
		return;
	}
	auto Widget = this->GetWidget();
	if (!Widget)return;
	auto Canvas = Widget->GetRenderCanvas();
	if (!Canvas)return;
	auto Data = Canvas->GetWidgetPropertyDataAsTexture();
	if (!Data)return;
	TArray<uint8> BlockBuffer;
	BlockBuffer.SetNumUninitialized(8);
	FMemory::Memzero(BlockBuffer.GetData(), 8);
	int BlockBufferOffset = 0;
	
	//width & height
	if (bNeedSize)
	{
		auto Size = FVector2DHalf(Widget->GetWidth(), Widget->GetHeight());
		FMemory::Memcpy(BlockBuffer.GetData() + BlockBufferOffset, &Size, sizeof(FVector2DHalf));
	}
	BlockBufferOffset += sizeof(FVector2DHalf);
	
	//widget rect center position in canvas space
	if (bNeedCenterPosition)
	{
		auto WidgetToWorldMatrix = Widget->GetWorldTransform().ToMatrixWithScale();
		auto WidgetLocalSpaceCenter = Widget->GetLocalSpaceCenter();
		auto CenterPositionInWorldSpace = WidgetToWorldMatrix.TransformPosition(FVector(0, WidgetLocalSpaceCenter.X, WidgetLocalSpaceCenter.Y));
		auto CenterPositionInCanvasSpace = Canvas->GetWidget()->GetWorldTransform().InverseTransformPosition(CenterPositionInWorldSpace);
		auto CenterPosition2D = FVector2DHalf(CenterPositionInCanvasSpace.Y, CenterPositionInCanvasSpace.Z);
		FMemory::Memcpy(BlockBuffer.GetData() + BlockBufferOffset, &CenterPosition2D, sizeof(FVector2DHalf));
	}
	BlockBufferOffset += sizeof(FVector2DHalf);
	
	Data->UpdateBlock(2, StartPosition, MoveTemp(BlockBuffer), 2);
}

void ULexVisual::FillWidgetPropertyDataForMaterial_ClipDataCoordinate(ULexUIDataAsTexture* DataAsTexture)const
{
	auto StartPosition = this->WidgetPropertyDataStartPosition;
	if (StartPosition == INDEX_NONE)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d WidgetPropertyDataStartPosition is invalid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}
	TArray<uint8> BlockBuffer;
	BlockBuffer.SetNumUninitialized(4);

	FMemory::Memcpy(BlockBuffer.GetData(), &this->ClipDataStartPosition, 4);
	
	DataAsTexture->UpdateBlock(1, StartPosition, MoveTemp(BlockBuffer), 1);
}

void ULexVisual::FillWidgetPropertyDataForMaterial_InitialMark(ULexUIDataAsTexture* DataAsTexture, uint8 FontMark) const
{
	auto StartPosition = this->WidgetPropertyDataStartPosition;
	if (StartPosition == INDEX_NONE)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d WidgetPropertyDataStartPosition is invalid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}
	
	TArray<uint8> BlockBuffer;
	BlockBuffer.SetNumUninitialized(4);

	uint8 ExtraMark = 0;
	uint32 Marks =
		FontMark << 24
		| ExtraMark << 16
	;
	FMemory::Memcpy(BlockBuffer.GetData(), &Marks, 4);
	DataAsTexture->UpdateBlock(0, StartPosition, MoveTemp(BlockBuffer), 1);
}

#pragma region TweenAnimation
#include "LTweenManager.h"
ULTweener* ULexVisual::ColorTo(FColor endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this, FLTweenColorGetterFunction::CreateUObject(this, &ULexVisual::GetColor), FLTweenColorSetterFunction::CreateUObject(this, &ULexVisual::SetColor), endValue, duration);
	if (Tweener)
	{
		Tweener->SetEase(ease)->SetDelay(delay);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(GetWidget(), Tweener);
	}
	return Tweener;
}
ULTweener* ULexVisual::ColorFrom(FColor startValue, float duration, float delay, ELTweenEase ease)
{
	auto endValue = this->GetColor();
	this->SetColor(startValue);
	auto Tweener = ULTweenManager::To(this, FLTweenColorGetterFunction::CreateUObject(this, &ULexVisual::GetColor), FLTweenColorSetterFunction::CreateUObject(this, &ULexVisual::SetColor), endValue, duration);
	if (Tweener)
	{
		Tweener->SetEase(ease)->SetDelay(delay);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(GetWidget(), Tweener);
	}
	return Tweener;
}

ULTweener* ULexVisual::AlphaTo(float endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &ULexVisual::GetAlpha), FLTweenFloatSetterFunction::CreateUObject(this, &ULexVisual::SetAlpha), endValue, duration);
	if (Tweener)
	{
		Tweener->SetEase(ease)->SetDelay(delay);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(GetWidget(), Tweener);
	}
	return Tweener;
}
ULTweener* ULexVisual::AlphaFrom(float startValue, float duration, float delay, ELTweenEase ease)
{
	auto endValue = this->GetAlpha();
	this->SetAlpha(startValue);
	auto Tweener = ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &ULexVisual::GetAlpha), FLTweenFloatSetterFunction::CreateUObject(this, &ULexVisual::SetAlpha), endValue, duration);
	if (Tweener)
	{
		Tweener->SetEase(ease)->SetDelay(delay);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(GetWidget(), Tweener);
	}
	return Tweener;
}
#pragma endregion


