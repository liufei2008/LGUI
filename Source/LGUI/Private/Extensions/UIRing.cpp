// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Extensions/UIRing.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"

UDEPRECATED_UIRing::UDEPRECATED_UIRing(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDEPRECATED_UIRing::OnCreateGeometry()
{
	//UIGeometry::FromUIRing(widget.width, widget.height, widget.pivot, startAngle, endAngle, segment, (uint8)uvType, GetFinalColor(), ringWidth, geometry, sprite->InitAndGetSpriteInfo(), RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent(), RenderCanvas->GetRequireUV1());
}
void UDEPRECATED_UIRing::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (InVertexUVChanged)
	{
		//UIGeometry::UpdateUIRingUV(geometry, (uint8)uvType, startAngle, endAngle, segment, ringWidth, widget.width, widget.height, sprite->InitAndGetSpriteInfo());
	}
	if (InVertexColorChanged)
	{
		//UIGeometry::UpdateUIColor(geometry, GetFinalColor());
	}
	if (InVertexPositionChanged)
	{
		//UIGeometry::UpdateUIRingVertex(geometry, widget.width, widget.height, widget.pivot, startAngle, endAngle, segment, ringWidth);
	}
}

void UDEPRECATED_UIRing::SetStartAngle(float newStartAngle) {
	startAngle = newStartAngle;
	MarkVertexPositionDirty();
	if (uvType == UIRingUVType::SpriteRect) MarkUVDirty();
}
void UDEPRECATED_UIRing::SetEndAngle(float newEndAngle) {
	endAngle = newEndAngle;
	MarkVertexPositionDirty();
	if (uvType == UIRingUVType::SpriteRect) MarkUVDirty();
}
void UDEPRECATED_UIRing::SetUVType(UIRingUVType newUVType) {
	uvType = newUVType;
	MarkUVDirty();
}
void UDEPRECATED_UIRing::SetSegment(uint8 newSegment) {
	segment = newSegment; 
	MarkTriangleDirty();
}
void UDEPRECATED_UIRing::SetRingWidth(float newRingWidth) {
	ringWidth = newRingWidth;
	MarkVertexPositionDirty();
	MarkUVDirty();
}


ADEPRECATED_UIRingActor::ADEPRECATED_UIRingActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIRing_DEPRECATED = CreateDefaultSubobject<UDEPRECATED_UIRing>(TEXT("UIRingComponent"));
	RootComponent = UIRing_DEPRECATED;
}
