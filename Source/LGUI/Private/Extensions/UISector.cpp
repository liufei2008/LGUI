// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Extensions/UISector.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"


UDEPRECATED_UISector::UDEPRECATED_UISector(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDEPRECATED_UISector::OnCreateGeometry()
{
	//UIGeometry::FromUISector(widget.width, widget.height, widget.pivot, startAngle, endAngle, segment, (uint8)uvType, GetFinalColor(), geometry, sprite->InitAndGetSpriteInfo(), RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent(), RenderCanvas->GetRequireUV1());
}
void UDEPRECATED_UISector::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (InVertexUVChanged)
	{
		//UIGeometry::UpdateUISectorUV(geometry, (uint8)uvType, startAngle, endAngle, segment, sprite->InitAndGetSpriteInfo());
	}
	if (InVertexColorChanged)
	{
		//UIGeometry::UpdateUIColor(geometry, GetFinalColor());
	}
	if (InVertexPositionChanged)
	{
		//UIGeometry::UpdateUISectorVertex(geometry, widget.width, widget.height, widget.pivot, startAngle, endAngle, segment);
	}
}

void UDEPRECATED_UISector::SetStartAngle(float newStartAngle) {
	startAngle = newStartAngle;
	MarkVertexPositionDirty();
	if (uvType == UISectorUVType::SpriteRect) MarkUVDirty();
}
void UDEPRECATED_UISector::SetEndAngle(float newEndAngle) {
	endAngle = newEndAngle;
	MarkVertexPositionDirty();
	if (uvType == UISectorUVType::SpriteRect) MarkUVDirty();
}
void UDEPRECATED_UISector::SetUVType(UISectorUVType newUVType) {
	uvType = newUVType;
	MarkUVDirty();
}
void UDEPRECATED_UISector::SetSegment(uint8 newSegment) {
	segment = newSegment;
	MarkTriangleDirty();
}


ADEPRECATED_UISectorActor::ADEPRECATED_UISectorActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UISector_DEPRECATED = CreateDefaultSubobject<UDEPRECATED_UISector>(TEXT("UISectorComponent"));
	RootComponent = UISector_DEPRECATED;
}
