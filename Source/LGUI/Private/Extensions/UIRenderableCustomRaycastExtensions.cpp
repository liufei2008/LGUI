// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Extensions/UIRenderableCustomRaycastExtensions.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "Utils/LGUIUtils.h"

#if 0
bool UUIRenderableCustomRaycast_Circle::Raycast(UUIBaseRenderable* InUIRenderable, const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, FVector& OutHitPoint, FVector& OutHitNormal)
{
	auto HitPointOnPlane = FMath::LinePlaneIntersection(InLocalSpaceRayStart, InLocalSpaceRayEnd, FVector::ZeroVector, FVector(1, 0, 0));
	auto CenterPoint = (InUIRenderable->GetLocalSpaceRightTopPoint() + InUIRenderable->GetLocalSpaceLeftBottomPoint()) * 0.5f;
	auto HitPointRelativeToCenter = FVector2D(HitPointOnPlane.Y - CenterPoint.X, HitPointOnPlane.Z - CenterPoint.Y);
	auto AngleInRadias = FMath::Atan2(HitPointRelativeToCenter.Y, HitPointRelativeToCenter.X);
	auto RadiusLerpValue = AngleInRadias * INV_PI * 2;
	auto Radius = 0.5f * FMath::Lerp(InUIRenderable->GetWidth(), InUIRenderable->GetHeight(), RadiusLerpValue);
	auto MaxPoint = FVector2D(Radius * FMath::Cos(AngleInRadias), Radius * FMath::Sin(AngleInRadias));
	{
		auto Point = InUIRenderable->GetComponentTransform().TransformPosition(HitPointOnPlane);
		DrawDebugPoint(InUIRenderable->GetWorld(), Point, 5, FColor::Green);
		auto LocalPoint = MaxPoint + CenterPoint;
		Point = InUIRenderable->GetComponentTransform().TransformPosition(FVector(0, LocalPoint.X, LocalPoint.Y));
		DrawDebugPoint(InUIRenderable->GetWorld(), Point, 5, FColor::Red);
	}
	UE_LOG(LGUI, Error, TEXT("HitPointRelativeToCenter:%s, Angle:%f, AngleLerpValue:%f, MaxHitPoint:%s"), *HitPointRelativeToCenter.ToString(), FMath::RadiansToDegrees(AngleInRadias), RadiusLerpValue, *(MaxPoint.ToString()));
	if (FMath::Abs(HitPointRelativeToCenter.X) < FMath::Abs(MaxPoint.X) && FMath::Abs(HitPointRelativeToCenter.Y) < FMath::Abs(MaxPoint.Y))
	{
		OutHitPoint = FVector(0, HitPointRelativeToCenter.X, HitPointRelativeToCenter.Y);
		OutHitNormal = FVector(1, 0, 0);
		return true;
	}
	return false;
}
#endif

bool UUIRenderableCustomRaycast_VisiblePixel::Raycast(UUIBaseRenderable* InUIRenderable, const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, FVector& OutHitPoint, FVector& OutHitNormal)
{
	if (auto BatchGeometry = Cast<UUIBatchGeometryRenderable>(InUIRenderable))
	{
		FVector2D HitUV; FColor HitPixel;
		if (UUIRenderableCustomRaycast::GetRaycastPixelFromUIBatchGeometryRenderable(BatchGeometry, InLocalSpaceRayStart, InLocalSpaceRayEnd, HitUV, HitPixel, OutHitPoint, OutHitNormal))
		{
			uint8 ChannelValue = 0;
			switch (PixelChannel)
			{
			default:
			case 0:ChannelValue = HitPixel.R; break;
			case 1:ChannelValue = HitPixel.G; break;
			case 2:ChannelValue = HitPixel.B; break;
			case 3:ChannelValue = HitPixel.A; break;
			}
			if (LGUIUtils::Color255To1_Table[ChannelValue] > VisibilityThreshold)
			{
				return true;
			}
		}
	}
	return false;
}

void UUIRenderableCustomRaycast_VisiblePixel::SetVisibilityThreshold(float value)
{
	VisibilityThreshold = value;
}
void UUIRenderableCustomRaycast_VisiblePixel::SetPixelChannel(uint8 value)
{
	PixelChannel = value;
}
