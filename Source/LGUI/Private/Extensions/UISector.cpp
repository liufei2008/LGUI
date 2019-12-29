// Copyright 2019 LexLiu. All Rights Reserved.

#include "Extensions/UISector.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"


UUISector::UUISector(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUISector::OnCreateGeometry()
{
	UIGeometry::FromUISector(widget.width, widget.height, widget.pivot, startAngle, endAngle, segment, (uint8)uvType, GetFinalColor(), geometry, sprite->InitAndGetSpriteInfo(), RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent(), RenderCanvas->GetRequireUV1());
}
void UUISector::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (InVertexUVChanged)
	{
		UIGeometry::UpdateUISectorUV(geometry, (uint8)uvType, startAngle, endAngle, segment, sprite->InitAndGetSpriteInfo());
	}
	if (InVertexColorChanged)
	{
		UIGeometry::UpdateUIColor(geometry, GetFinalColor());
	}
	if (InVertexPositionChanged)
	{
		UIGeometry::UpdateUISectorVertex(geometry, widget.width, widget.height, widget.pivot, startAngle, endAngle, segment);
	}
}

void UUISector::SetStartAngle(float newStartAngle) {
	startAngle = newStartAngle;
	MarkVertexPositionDirty();
	if (uvType == UISectorUVType::SpriteRect) MarkUVDirty();
}
void UUISector::SetEndAngle(float newEndAngle) {
	endAngle = newEndAngle;
	MarkVertexPositionDirty();
	if (uvType == UISectorUVType::SpriteRect) MarkUVDirty();
}
void UUISector::SetUVType(UISectorUVType newUVType) {
	uvType = newUVType;
	MarkUVDirty();
}
void UUISector::SetSegment(uint8 newSegment) {
	segment = newSegment;
	MarkTriangleDirty();
}


AUISectorActor::AUISectorActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UISector = CreateDefaultSubobject<UUISector>(TEXT("UISectorComponent"));
	RootComponent = UISector;
}
