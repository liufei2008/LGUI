// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UISpriteBase_BP.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"

UUISpriteBase_BP::UUISpriteBase_BP(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUISpriteBase_BP::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(createGeometryHelper))
	{
		createGeometryHelper->ConditionalBeginDestroy();
	}
	if (IsValid(updateGeometryHelper))
	{
		updateGeometryHelper->ConditionalBeginDestroy();
	}

	Super::EndPlay(EndPlayReason);
}

void UUISpriteBase_BP::OnBeforeCreateOrUpdateGeometry()
{
	OnBeforeCreateOrUpdateGeometry_BP();
}
void UUISpriteBase_BP::OnCreateGeometry()
{
	if (!IsValid(createGeometryHelper))
	{
		createGeometryHelper = NewObject<ULGUICreateGeometryHelper>(this);
		createGeometryHelper->uiGeometry = geometry;
	}
	OnCreateGeometry_BP(createGeometryHelper, sprite);
}
void UUISpriteBase_BP::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (!IsValid(updateGeometryHelper))
	{
		updateGeometryHelper = NewObject<ULGUIUpdateGeometryHelper>(this);
		updateGeometryHelper->uiGeometry = geometry;
	}
	OnUpdateGeometry_BP(updateGeometryHelper, sprite, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
}
void UUISpriteBase_BP::MarkVertexChanged_BP()
{
	MarkVertexPositionDirty();
	MarkColorDirty();
	MarkUVDirty();
}
void UUISpriteBase_BP::MarkRebuildGeometry_BP()
{
	MarkTriangleDirty();
}