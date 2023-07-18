// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Extensions/UIRing.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LTweenManager.h"

UUIRing::UUIRing(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIRing::BeginPlay()
{
	Super::BeginPlay();
}


AUIRingActor::AUIRingActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIElement = CreateDefaultSubobject<UUIRing>(TEXT("UIElement"));
	RootComponent = UIElement;
}


void UUIRing::CalculatePoints()
{
	Segment = FMath::Max(0, Segment);
	int pointCount = Segment + 2;
	CurrentPointArray.Reset(pointCount);

	float angle = FMath::DegreesToRadians(StartAngle);
	float angleInterval = FMath::DegreesToRadians((EndAngle - StartAngle) / (Segment + 1));
	float halfWidth = this->GetWidth() * 0.5f;
	float halfHeight = this->GetHeight() * 0.5f;
	//points
	for (int i = 0; i < pointCount; i++)
	{
		float x = halfWidth * FMath::Cos(angle);
		float y = halfHeight * FMath::Sin(angle);
		CurrentPointArray.Add(FVector2D(x, y));
		angle += angleInterval;
	}
}

FVector2D UUIRing::GetStartPointTangentDirection()
{
	float angle = FMath::DegreesToRadians(StartAngle);
	auto dir = FVector2D(FMath::Cos(angle), FMath::Sin(angle));
	auto tanDir = FVector2D(-this->GetWidth() * dir.Y, this->GetHeight() * dir.X);
	tanDir.Normalize();
	return tanDir;
}
FVector2D UUIRing::GetEndPointTangentDirection()
{
	float angle = FMath::DegreesToRadians(EndAngle);
	auto dir = FVector2D(FMath::Cos(angle), FMath::Sin(angle));
	auto tanDir = FVector2D(-this->GetWidth() * dir.Y, this->GetHeight() * dir.X);
	tanDir.Normalize();
	return tanDir;
}

void UUIRing::SetStartAngle(float newValue)
{
	if (StartAngle != newValue)
	{
		StartAngle = newValue;
		MarkVertexPositionDirty();
	}
}
void UUIRing::SetEndAngle(float newValue)
{
	if (EndAngle != newValue)
	{
		EndAngle = newValue;
		MarkVertexPositionDirty();
	}
}
void UUIRing::SetSegment(int newValue)
{
	newValue = FMath::Max(0, newValue);
	if (Segment != newValue)
	{
		Segment = newValue;
		MarkVerticesDirty(true, true, true, true);
	}
}


ULTweener* UUIRing::StartAngleTo(float endValue, float duration, float delay, ELTweenEase easeType)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIRing::GetStartAngle), FLTweenFloatSetterFunction::CreateUObject(this, &UUIRing::SetStartAngle), endValue, duration)
		->SetEase(easeType)->SetDelay(delay);
}
ULTweener* UUIRing::EndAngleTo(float endValue, float duration, float delay, ELTweenEase easeType)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIRing::GetEndAngle), FLTweenFloatSetterFunction::CreateUObject(this, &UUIRing::SetEndAngle), endValue, duration)
		->SetEase(easeType)->SetDelay(delay);
}