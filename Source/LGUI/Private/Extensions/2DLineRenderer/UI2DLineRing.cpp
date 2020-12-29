// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Extensions/2DLineRenderer/UI2DLineRing.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"

UUI2DLineRing::UUI2DLineRing(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUI2DLineRing::BeginPlay()
{
	Super::BeginPlay();
}


AUI2DLineRingActor::AUI2DLineRingActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIElement = CreateDefaultSubobject<UUI2DLineRing>(TEXT("UIElement"));
	RootComponent = UIElement;
}


void UUI2DLineRing::CalculatePoints()
{
	if (Segment < 0)
	{
		return;
	}
	int pointCount = Segment + 2;
	CurrentPointArray.Reset(pointCount);

	float angle = FMath::DegreesToRadians(StartAngle);
	float angleInterval = FMath::DegreesToRadians((EndAngle - StartAngle) / (Segment + 1));
	float halfWidth = widget.width * 0.5f;
	float halfHeight = widget.height * 0.5f;
	//first point
	float x = halfWidth * FMath::Cos(angle);
	float y = halfHeight * FMath::Sin(angle);
	CurrentPointArray.Add(FVector2D(x, y));
	//rest point
	for (int i = 0, count = pointCount - 1; i < count; i++)
	{
		angle += angleInterval;
		x = halfWidth * FMath::Cos(angle);
		y = halfHeight * FMath::Sin(angle);
		CurrentPointArray.Add(FVector2D(x, y));
	}
}

FVector2D UUI2DLineRing::GetStartPointTangentDirection()
{
	float angle = FMath::DegreesToRadians(StartAngle);
	auto dir = FVector2D(FMath::Cos(angle), FMath::Sin(angle));
	auto tanDir = FVector2D(-widget.width * dir.Y, widget.height * dir.X);
	tanDir.Normalize();
	return tanDir;
}
FVector2D UUI2DLineRing::GetEndPointTangentDirection()
{
	float angle = FMath::DegreesToRadians(EndAngle);
	auto dir = FVector2D(FMath::Cos(angle), FMath::Sin(angle));
	auto tanDir = FVector2D(-widget.width * dir.Y, widget.height * dir.X);
	tanDir.Normalize();
	return tanDir;
}

void UUI2DLineRing::SetStartAngle(float newValue)
{
	if (StartAngle != newValue)
	{
		StartAngle = newValue;
		MarkVertexPositionDirty();
	}
}
void UUI2DLineRing::SetEndAngle(float newValue)
{
	if (EndAngle != newValue)
	{
		EndAngle = newValue;
		MarkVertexPositionDirty();
	}
}
void UUI2DLineRing::SetSegment(float newValue)
{
	if (Segment != newValue)
	{
		Segment = newValue;
		MarkTriangleDirty();
	}
}


ULTweener* UUI2DLineRing::StartAngleTo(float endValue, float duration, float delay, LTweenEase easeType)
{
	return ALTweenActor::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUI2DLineRing::GetStartAngle), FLTweenFloatSetterFunction::CreateUObject(this, &UUI2DLineRing::SetStartAngle), endValue, duration)
		->SetEase(easeType)->SetDelay(delay);
}
ULTweener* UUI2DLineRing::EndAngleTo(float endValue, float duration, float delay, LTweenEase easeType)
{
	return ALTweenActor::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUI2DLineRing::GetEndAngle), FLTweenFloatSetterFunction::CreateUObject(this, &UUI2DLineRing::SetEndAngle), endValue, duration)
		->SetEase(easeType)->SetDelay(delay);
}