// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIBaseRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Utils/LGUIUtils.h"
#include "GeometryModifier/UIGeometryModifierBase.h"
#include "Core/ActorComponent/UICanvasGroup.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

bool UUIRenderableCustomRaycast::Raycast(UUIBaseRenderable* InUIRenderable, const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, FVector& OutHitPoint, FVector& OutHitNormal)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		return ReceiveRaycast(InUIRenderable, InLocalSpaceRayStart, InLocalSpaceRayEnd, OutHitPoint, OutHitNormal);
	}
	return false;
}

bool UUIRenderableCustomRaycast::GetRaycastPixelFromUIBatchGeometryRenderable(UUIBatchGeometryRenderable* InUIRenderable, const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, FVector2D& OutUV, FColor& OutPixel, FVector& OutHitPoint, FVector& OutHitNormal)
{
	if (auto UIGeo = InUIRenderable->GetGeometry())
	{
		//triangle hit test
		auto& originVertices = UIGeo->originVertices;
		auto& vertices = UIGeo->vertices;
		auto& triangleIndices = UIGeo->triangles;
		const int triangleCount = triangleIndices.Num() / 3;
		int index = 0;
		for (int i = 0; i < triangleCount; i++)
		{
			auto vertIndex0 = triangleIndices[index++];
			auto vertIndex1 = triangleIndices[index++];
			auto vertIndex2 = triangleIndices[index++];
			auto point0 = originVertices[vertIndex0].Position;
			auto point1 = originVertices[vertIndex1].Position;
			auto point2 = originVertices[vertIndex2].Position;
			if (FMath::SegmentTriangleIntersection(InLocalSpaceRayStart, InLocalSpaceRayEnd, point0, point1, point2, OutHitPoint, OutHitNormal))
			{
				auto baryCentric = FMath::ComputeBaryCentric2D(OutHitPoint, point0, point1, point2);
				auto& uv0 = vertices[vertIndex0].TextureCoordinate[0];
				auto& uv1 = vertices[vertIndex1].TextureCoordinate[0];
				auto& uv2 = vertices[vertIndex2].TextureCoordinate[0];
				OutUV = FVector2D(baryCentric.X * uv0 + baryCentric.Y * uv1 + baryCentric.Z * uv2);
				//get pixel
				if (auto Texture2D = Cast<UTexture2D>(UIGeo->texture.Get()))
				{
					auto PlatformData = Texture2D->PlatformData;
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


UUIBaseRenderable::UUIBaseRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	UIRenderableType = EUIRenderableType::None;

	bColorChanged = true;
	bTransformChanged = true;
}

void UUIBaseRenderable::BeginPlay()
{
	Super::BeginPlay();
	bColorChanged = true;
	bTransformChanged = true;
}

void UUIBaseRenderable::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

#if WITH_EDITOR
void UUIBaseRenderable::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UUIBaseRenderable::OnRegister()
{
	Super::OnRegister();
}
void UUIBaseRenderable::OnUnregister()
{
	Super::OnUnregister();
}

void UUIBaseRenderable::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
	bTransformChanged = true;
	MarkCanvasUpdate(false, true, false);
}

void UUIBaseRenderable::ApplyUIActiveState(bool InStateChange)
{
	Super::ApplyUIActiveState(InStateChange);//this line must line before AddUIRenderable/RemoveUIRenderable, because UIActiveStateChangedDelegate need to call first. (UIActiveStateChangedDelegate lead to canvas: ParentCanvas->UIRenderableList.Add/Remove)

	if (GetIsUIActiveInHierarchy())
	{
		if (RenderCanvas.IsValid())
		{
			RenderCanvas->AddUIRenderable(this);
		}
	}
	else
	{
		if (RenderCanvas.IsValid())
		{
			RenderCanvas->RemoveUIRenderable(this);
		}
	}
}
void UUIBaseRenderable::OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)
{
	if (IsValid(OldCanvas))
	{
		OldCanvas->RemoveUIItem(this);
		OldCanvas->RemoveUIRenderable(this);
	}
	if (IsValid(NewCanvas))
	{
		NewCanvas->AddUIItem(this);
		if (GetIsUIActiveInHierarchy())
		{
			NewCanvas->AddUIRenderable(this);
		}
	}
}

void UUIBaseRenderable::OnCanvasGroupAlphaChange()
{
	MarkColorDirty();
}

void UUIBaseRenderable::MarkColorDirty()
{
	bColorChanged = true;
	MarkCanvasUpdate(false, false, false);
}

void UUIBaseRenderable::MarkAllDirty()
{
	bColorChanged = true;
	Super::MarkAllDirty();
}

void UUIBaseRenderable::MarkCanvasUpdate(bool bMaterialOrTextureChanged, bool bTransformOrVertexPositionChanged, bool bHierarchyOrderChanged, bool bForceRebuildDrawcall)
{
	if (RenderCanvas.IsValid())
	{
		RenderCanvas->MarkCanvasUpdate(bMaterialOrTextureChanged, bTransformOrVertexPositionChanged, bHierarchyOrderChanged, bForceRebuildDrawcall);
		if(bTransformOrVertexPositionChanged)
		{
			RenderCanvas->MarkItemTransformOrVertexPositionChanged(this);
		}
	}
}

void UUIBaseRenderable::GetGeometryBoundsInLocalSpace(FVector2D& OutMinPoint, FVector2D& OutMaxPoint)const
{
	OutMinPoint = GetLocalSpaceLeftBottomPoint();
	OutMaxPoint = GetLocalSpaceRightTopPoint();
}

#if WITH_EDITOR
void UUIBaseRenderable::GetGeometryBounds3DInLocalSpace(FVector& OutMinPoint, FVector& OutMaxPoint)const
{
	const auto MinPoint2D = GetLocalSpaceLeftBottomPoint();
	const auto MaxPoint2D = GetLocalSpaceRightTopPoint();
	OutMinPoint = FVector(0.1f, MinPoint2D.X, MinPoint2D.Y);
	OutMaxPoint = FVector(0.1f, MaxPoint2D.X, MaxPoint2D.Y);
}
#endif

bool UUIBaseRenderable::LineTraceUIGeometry(UIGeometry* InGeo, FHitResult& OutHit, const FVector& Start, const FVector& End)
{
	const auto InverseTf = GetComponentTransform().Inverse();
	const auto LocalSpaceRayOrigin = InverseTf.TransformPosition(Start);
	const auto LocalSpaceRayEnd = InverseTf.TransformPosition(End);

	//DrawDebugLine(this->GetWorld(), Start, End, FColor::Red, false, 5.0f);//just for test
	//check Line-Plane intersection first, then check Line-Triangle
	//start and end point must be different side of X plane
	if (FMath::Sign(LocalSpaceRayOrigin.X) != FMath::Sign(LocalSpaceRayEnd.X))
	{
		//triangle hit test
		auto& vertices = InGeo->originVertices;
		auto& triangleIndices = InGeo->triangles;
		const int triangleCount = triangleIndices.Num() / 3;
		int index = 0;
		for (int i = 0; i < triangleCount; i++)
		{
			auto point0 = (vertices[triangleIndices[index++]].Position);
			auto point1 = (vertices[triangleIndices[index++]].Position);
			auto point2 = (vertices[triangleIndices[index++]].Position);
			FVector HitPoint, HitNormal;
			if (FMath::SegmentTriangleIntersection(LocalSpaceRayOrigin, LocalSpaceRayEnd, point0, point1, point2, HitPoint, HitNormal))
			{
				OutHit.TraceStart = Start;
				OutHit.TraceEnd = End;
				OutHit.Actor = GetOwner();
				OutHit.Component = (UPrimitiveComponent*)this;//acturally this convert is incorrect, but I need this pointer
				OutHit.Location = GetComponentTransform().TransformPosition(HitPoint);
				OutHit.Normal = GetComponentTransform().TransformVector(HitNormal);
				OutHit.Normal.Normalize();
				OutHit.Distance = FVector::Distance(Start, OutHit.Location);
				OutHit.ImpactPoint = OutHit.Location;
				OutHit.ImpactNormal = OutHit.Normal;
				return true;
			}
		}
	}
	return false;
}

bool UUIBaseRenderable::LineTraceUICustom(FHitResult& OutHit, const FVector& Start, const FVector& End)
{
	if (!IsValid(CustomRaycastObject))
	{
		UE_LOG(LGUI, Error, TEXT("[UUIBaseRenderable::LineTraceUIGeometry]EUIRenderableRaycastType::Custom need a UUIRenderableCustomRaycast component on this actor!"));
		return false;
	}
	const auto InverseTf = GetComponentTransform().Inverse();
	const auto LocalSpaceRayOrigin = InverseTf.TransformPosition(Start);
	const auto LocalSpaceRayEnd = InverseTf.TransformPosition(End);

	if (FMath::Sign(LocalSpaceRayOrigin.X) != FMath::Sign(LocalSpaceRayEnd.X))
	{
		FVector HitPoint, HitNormal;
		if (CustomRaycastObject->Raycast(this, LocalSpaceRayOrigin, LocalSpaceRayEnd, HitPoint, HitNormal))
		{
			OutHit.TraceStart = Start;
			OutHit.TraceEnd = End;
			OutHit.Actor = GetOwner();
			OutHit.Component = (UPrimitiveComponent*)this;//acturally this convert is incorrect, but I need this pointer
			OutHit.Location = GetComponentTransform().TransformPosition(HitPoint);
			OutHit.Normal = GetComponentTransform().TransformVector(HitNormal);
			OutHit.Normal.Normalize();
			OutHit.Distance = FVector::Distance(Start, OutHit.Location);
			OutHit.ImpactPoint = OutHit.Location;
			OutHit.ImpactNormal = OutHit.Normal;
			return true;
		}
	}
	return false;
}

void UUIBaseRenderable::SetColor(FColor value)
{
	if (Color != value)
	{
		Color = value;
		MarkColorDirty();
	}
}
void UUIBaseRenderable::SetAlpha(float value)
{
	value = FMath::Clamp(value, 0.0f, 1.0f);
	auto uintAlpha = (uint8)(value * 255);
	if (Color.A != uintAlpha)
	{
		MarkColorDirty();
		Color.A = uintAlpha;
	}
}

void UUIBaseRenderable::SetCustomRaycastObject(UUIRenderableCustomRaycast* Value)
{
	CustomRaycastObject = Value;
}

FColor UUIBaseRenderable::GetFinalColor()const
{
	FColor ResultColor = Color;
	if (CanvasGroup.IsValid())
	{
		ResultColor.A = Color.A * CanvasGroup->GetFinalAlpha();
	}
	return ResultColor;
}

uint8 UUIBaseRenderable::GetFinalAlpha()const
{
	if (CanvasGroup.IsValid())
	{
		return (uint8)(Color.A * CanvasGroup->GetFinalAlpha());
	}
	return Color.A;
}

float UUIBaseRenderable::GetFinalAlpha01()const
{
	return LGUIUtils::Color255To1_Table[GetFinalAlpha()];
}

#pragma region TweenAnimation
#include "LTweenManager.h"
ULTweener* UUIBaseRenderable::ColorTo(FColor endValue, float duration, float delay, LTweenEase ease)
{
	return ULTweenManager::To(this, FLTweenColorGetterFunction::CreateUObject(this, &UUIBaseRenderable::GetColor), FLTweenColorSetterFunction::CreateUObject(this, &UUIBaseRenderable::SetColor), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* UUIBaseRenderable::ColorFrom(FColor startValue, float duration, float delay, LTweenEase ease)
{
	auto endValue = this->GetColor();
	this->SetColor(startValue);
	return ULTweenManager::To(this, FLTweenColorGetterFunction::CreateUObject(this, &UUIBaseRenderable::GetColor), FLTweenColorSetterFunction::CreateUObject(this, &UUIBaseRenderable::SetColor), endValue, duration)->SetEase(ease)->SetDelay(delay);
}

ULTweener* UUIBaseRenderable::AlphaTo(float endValue, float duration, float delay, LTweenEase ease)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIBaseRenderable::GetAlpha), FLTweenFloatSetterFunction::CreateUObject(this, &UUIBaseRenderable::SetAlpha), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* UUIBaseRenderable::AlphaFrom(float startValue, float duration, float delay, LTweenEase ease)
{
	auto endValue = this->GetAlpha();
	this->SetAlpha(startValue);
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIBaseRenderable::GetAlpha), FLTweenFloatSetterFunction::CreateUObject(this, &UUIBaseRenderable::SetAlpha), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
#pragma endregion

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
