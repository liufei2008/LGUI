// Copyright 2019 LexLiu. All Rights Reserved.

#include "Extensions/UIRing.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"

UUIRing::UUIRing(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIRing::OnCreateGeometry()
{
	UIGeometry::FromUIRing(widget.width, widget.height, widget.pivot, startAngle, endAngle, segment, (uint8)uvType, GetFinalColor(), ringWidth, geometry, sprite->GetSpriteInfo(), RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent(), RenderCanvas->GetRequireUV1());
}
void UUIRing::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (InVertexUVChanged)
	{
		UIGeometry::UpdateUIRingUV(geometry, (uint8)uvType, startAngle, endAngle, segment, ringWidth, widget.width, widget.height, sprite->GetSpriteInfo());
	}
	if (InVertexColorChanged)
	{
		UIGeometry::UpdateUIColor(geometry, GetFinalColor());
	}
	if (InVertexPositionChanged)
	{
		UIGeometry::UpdateUIRingVertex(geometry, widget.width, widget.height, widget.pivot, startAngle, endAngle, segment, ringWidth);
	}
}

void UUIRing::SetStartAngle(float newStartAngle) {
	startAngle = newStartAngle;
	MarkVertexPositionDirty();
	if (uvType == UIRingUVType::SpriteRect) MarkUVDirty();
}
void UUIRing::SetEndAngle(float newEndAngle) {
	endAngle = newEndAngle;
	MarkVertexPositionDirty();
	if (uvType == UIRingUVType::SpriteRect) MarkUVDirty();
}
void UUIRing::SetUVType(UIRingUVType newUVType) {
	uvType = newUVType;
	MarkUVDirty();
}
void UUIRing::SetSegment(uint8 newSegment) {
	segment = newSegment; 
	MarkTriangleDirty();
}
void UUIRing::SetRingWidth(float newRingWidth) {
	ringWidth = newRingWidth;
	MarkVertexPositionDirty();
	MarkUVDirty();
}


AUIRingActor::AUIRingActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIRing = CreateDefaultSubobject<UUIRing>(TEXT("UIRingComponent"));
	RootComponent = UIRing;
}
