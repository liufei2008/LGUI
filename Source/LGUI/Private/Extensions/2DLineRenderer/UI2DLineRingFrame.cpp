// Copyright 2019 LexLiu. All Rights Reserved.

#include "Extensions/2DLineRenderer/UI2DLineRingFrame.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"

UUI2DLineRingFrame::UUI2DLineRingFrame(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	ConnectStartEndPoint = true;
}

void UUI2DLineRingFrame::BeginPlay()
{
	Super::BeginPlay();
}


AUI2DLineRingFrameActor::AUI2DLineRingFrameActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIElement = CreateDefaultSubobject<UUI2DLineRingFrame>(TEXT("UIElement"));
	RootComponent = UIElement;
}


void UUI2DLineRingFrame::OnCreateGeometry()
{
	CalculatePoints();
	Super::OnCreateGeometry();
}
void UUI2DLineRingFrame::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	CalculatePoints();
	Super::OnUpdateGeometry(InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
}

void UUI2DLineRingFrame::CalculatePoints()
{
	if (Segment < 0)
	{
		return;
	}
	int pointCount = Segment + 2;
	CurrentPointArray.Reset(pointCount * 2);

	float angle = FMath::DegreesToRadians(AngleBegin);
	float angleInterval = FMath::DegreesToRadians((AngleEnd - AngleBegin) / (Segment + 1));
	float radius = (UseWidthOrHeightAsRadius ? widget.width : widget.height) * 0.5f;
	//first ring, first point
	float x = radius * FMath::Cos(angle);
	float y = radius * FMath::Sin(angle);
	CurrentPointArray.Add(FVector2D(x, y));
	//first ring, rest point
	for (int i = 0, count = pointCount - 1; i < count; i++)
	{
		angle += angleInterval;
		x = radius * FMath::Cos(angle);
		y = radius * FMath::Sin(angle);
		CurrentPointArray.Add(FVector2D(x, y));
	}
	//second ring, first point(connect line)
	radius = radius - FrameWidth;
	x = radius * FMath::Cos(angle);
	y = radius * FMath::Sin(angle);
	CurrentPointArray.Add(FVector2D(x, y));
	//reset point
	for (int i = 0, count = pointCount - 1; i < count; i++)
	{
		angle -= angleInterval;
		x = radius * FMath::Cos(angle);
		y = radius * FMath::Sin(angle);
		CurrentPointArray.Add(FVector2D(x, y));
	}
}

void UUI2DLineRingFrame::SetAngleBegin(float newValue)
{
	if (AngleBegin != newValue)
	{
		AngleBegin = newValue;
		MarkVertexPositionDirty();
	}
}
void UUI2DLineRingFrame::SetAngleEnd(float newValue)
{
	if (AngleEnd != newValue)
	{
		AngleEnd = newValue;
		MarkVertexPositionDirty();
	}
}
void UUI2DLineRingFrame::SetFrameWidth(float newValue)
{
	if (FrameWidth != newValue)
	{
		FrameWidth = newValue;
		MarkVertexPositionDirty();
	}
}
void UUI2DLineRingFrame::SetSegment(float newValue)
{
	if (Segment != newValue)
	{
		Segment = newValue;
		MarkTriangleDirty();
	}
}



ULTweener* UUI2DLineRingFrame::AngleBeginTo(float endValue, float duration, float delay, LTweenEase easeType)
{
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateUObject(this, &UUI2DLineRingFrame::GetAngleBegin), FLTweenFloatSetterFunction::CreateUObject(this, &UUI2DLineRingFrame::SetAngleBegin), endValue, duration)
		->SetEase(easeType)->SetDelay(delay);
}
ULTweener* UUI2DLineRingFrame::AngleEndTo(float endValue, float duration, float delay, LTweenEase easeType)
{
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateUObject(this, &UUI2DLineRingFrame::GetAngleEnd), FLTweenFloatSetterFunction::CreateUObject(this, &UUI2DLineRingFrame::SetAngleEnd), endValue, duration)
		->SetEase(easeType)->SetDelay(delay);
}
ULTweener* UUI2DLineRingFrame::FrameWidthTo(float endValue, float duration, float delay, LTweenEase easeType)
{
	return ALTweenActor::To(FLTweenFloatGetterFunction::CreateUObject(this, &UUI2DLineRingFrame::GetFrameWidth), FLTweenFloatSetterFunction::CreateUObject(this, &UUI2DLineRingFrame::SetFrameWidth), endValue, duration)
		->SetEase(easeType)->SetDelay(delay);
}