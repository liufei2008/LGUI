// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Extensions/UIPolygon.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/LGUISpriteData_BaseObject.h"

UUIPolygon::UUIPolygon(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIPolygon::OnCreateGeometry()
{
	Sides = FMath::Max(Sides, FullCycle ? 3 : 1);
	UIGeometry::FromUIPolygon(this->GetWidth(), this->GetHeight(), this->GetPivot()
		, StartAngle, EndAngle, Sides, UVType
		, VertexOffsetArray, FullCycle
		, GetFinalColor(), geometry, sprite->GetSpriteInfo()
		, RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent(), RenderCanvas->GetRequireUV1());
}
void UUIPolygon::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (InVertexUVChanged)
	{
		UIGeometry::UpdateUIPolygonUV(StartAngle, EndAngle, Sides, UVType
			, FullCycle
			, geometry, sprite->GetSpriteInfo());
	}
	if (InVertexColorChanged)
	{
		UIGeometry::UpdateUIColor(geometry, GetFinalColor());
	}
	if (InVertexPositionChanged)
	{
		UIGeometry::UpdateUIPolygonVertex(this->GetWidth(), this->GetHeight(), this->GetPivot()
			, StartAngle, EndAngle, Sides
			, VertexOffsetArray, FullCycle
			, geometry);
	}
}

void UUIPolygon::SetStartAngle(float value) {
	if (StartAngle != value)
	{
		StartAngle = value;
		MarkVertexPositionDirty();
		MarkUVDirty();
	}
}
void UUIPolygon::SetEndAngle(float value) {
	if (EndAngle != value)
	{
		EndAngle = value;
		MarkVertexPositionDirty();
		MarkUVDirty();
	}
}
void UUIPolygon::SetSides(int value) {
	if (Sides != value)
	{
		Sides = value;
		Sides = FMath::Max(Sides, FullCycle ? 3 : 1);
		MarkTriangleDirty();
	}
}
void UUIPolygon::SetUVType(UIPolygonUVType value)
{
	if (UVType != value)
	{
		UVType = value;
		MarkUVDirty();
	}
}
void UUIPolygon::SetVertexOffsetArray(const TArray<float>& value)
{
	if (VertexOffsetArray.Num() == value.Num())
	{
		VertexOffsetArray = value;
		MarkVertexPositionDirty();
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[UUIPolygon::SetVertexOffsetArray]Array count not equal! VertexOffsetArray:%d, value:%d"), VertexOffsetArray.Num(), value.Num());
	}
}
ULTweener* UUIPolygon::StartAngleTo(float endValue, float duration /* = 0.5f */, float delay /* = 0.0f */, LTweenEase easeType /* = LTweenEase::OutCubic */)
{
	return ALTweenActor::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIPolygon::GetStartAngle), FLTweenFloatSetterFunction::CreateUObject(this, &UUIPolygon::SetStartAngle), endValue, duration)
		->SetEase(easeType)->SetDelay(delay);
}
ULTweener* UUIPolygon::EndAngleTo(float endValue, float duration /* = 0.5f */, float delay /* = 0.0f */, LTweenEase easeType /* = LTweenEase::OutCubic */)
{
	return ALTweenActor::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIPolygon::GetEndAngle), FLTweenFloatSetterFunction::CreateUObject(this, &UUIPolygon::SetEndAngle), endValue, duration)
		->SetEase(easeType)->SetDelay(delay);
}


AUIPolygonActor::AUIPolygonActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIPolygon = CreateDefaultSubobject<UUIPolygon>(TEXT("UIPolygonComponent"));
	RootComponent = UIPolygon;
}
