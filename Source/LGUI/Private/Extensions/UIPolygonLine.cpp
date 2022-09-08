// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Extensions/UIPolygonLine.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LTweenManager.h"

UUIPolygonLine::UUIPolygonLine(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	EndType = EUI2DLineRenderer_EndType::ConnectStartAndEnd;
}

void UUIPolygonLine::CalculatePoints()
{
	Sides = FMath::Max(Sides, FullCycle ? 3 : 1);
	int pointCount = FullCycle ? Sides : (Sides + 1);//ring's point count, not include center point
	CurrentPointArray.Reset(pointCount);
	//vert offset
	int vertexOffsetCount = FullCycle ? Sides : (Sides + 1);
	if (VertexOffsetArray.Num() != vertexOffsetCount)
	{
		if (VertexOffsetArray.Num() > vertexOffsetCount)
		{
			VertexOffsetArray.SetNumUninitialized(vertexOffsetCount);
		}
		else
		{
			for (int i = VertexOffsetArray.Num(); i < vertexOffsetCount; i++)
			{
				VertexOffsetArray.Add(1.0f);
			}
		}
	}

	float calcEndAngle = EndAngle;
	if (FullCycle)calcEndAngle = StartAngle + 360.0f;
	float angle = FMath::DegreesToRadians(StartAngle);
	float angleInterval = FMath::DegreesToRadians((calcEndAngle - StartAngle) / Sides);
	float halfWidth = this->GetWidth() * 0.5f;
	float halfHeight = this->GetHeight() * 0.5f;
	if (!FullCycle)
	{
		CurrentPointArray.Add(FVector2D(0, 0));
	}
	//full cycle points
	for (int i = 0; i < pointCount; i++)
	{
		float x = halfWidth * VertexOffsetArray[i] * FMath::Cos(angle);
		float y = halfHeight * VertexOffsetArray[i] * FMath::Sin(angle);
		CurrentPointArray.Add(FVector2D(x, y));
		angle += angleInterval;
	}
}

FVector2D UUIPolygonLine::GetStartPointTangentDirection()
{
	float angle = FMath::DegreesToRadians(StartAngle);
	auto dir = FVector2D(FMath::Cos(angle), FMath::Sin(angle));
	auto tanDir = FVector2D(-this->GetWidth() * dir.Y, this->GetHeight() * dir.X);
	tanDir.Normalize();
	return tanDir;
}
FVector2D UUIPolygonLine::GetEndPointTangentDirection()
{
	if (FullCycle)
	{
		return GetStartPointTangentDirection();
	}
	else
	{
		float angle = FMath::DegreesToRadians(EndAngle);
		auto dir = FVector2D(FMath::Cos(angle), FMath::Sin(angle));
		auto tanDir = FVector2D(-this->GetWidth() * dir.Y, this->GetHeight() * dir.X);
		tanDir.Normalize();
		return tanDir;
	}
}

void UUIPolygonLine::SetStartAngle(float value) {
	if (StartAngle != value)
	{
		StartAngle = value;
		MarkVerticesDirty(false, true, true, false);
	}
}
void UUIPolygonLine::SetEndAngle(float value) {
	if (EndAngle != value)
	{
		EndAngle = value;
		MarkVerticesDirty(false, true, true, false);
	}
}
void UUIPolygonLine::SetSides(int value) {
	if (Sides != value)
	{
		Sides = value;
		Sides = FMath::Max(Sides, FullCycle ? 3 : 1);
		MarkVerticesDirty(true, true, true, true);
	}
}
void UUIPolygonLine::SetVertexOffsetArray(const TArray<float>& value)
{
	if (VertexOffsetArray.Num() == value.Num())
	{
		VertexOffsetArray = value;
		MarkVertexPositionDirty();
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[UUIPolygonLine::SetVertexOffsetArray]Array count not equal! VertexOffsetArray:%d, value:%d"), VertexOffsetArray.Num(), value.Num());
	}
}
ULTweener* UUIPolygonLine::StartAngleTo(float endValue, float duration /* = 0.5f */, float delay /* = 0.0f */, LTweenEase easeType /* = LTweenEase::OutCubic */)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIPolygonLine::GetStartAngle), FLTweenFloatSetterFunction::CreateUObject(this, &UUIPolygonLine::SetStartAngle), endValue, duration)
		->SetEase(easeType)->SetDelay(delay);
}
ULTweener* UUIPolygonLine::EndAngleTo(float endValue, float duration /* = 0.5f */, float delay /* = 0.0f */, LTweenEase easeType /* = LTweenEase::OutCubic */)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIPolygonLine::GetEndAngle), FLTweenFloatSetterFunction::CreateUObject(this, &UUIPolygonLine::SetEndAngle), endValue, duration)
		->SetEase(easeType)->SetDelay(delay);
}


AUIPolygonLineActor::AUIPolygonLineActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIPolygonLine = CreateDefaultSubobject<UUIPolygonLine>(TEXT("UIPolygonLineComponent"));
	RootComponent = UIPolygonLine;
}
