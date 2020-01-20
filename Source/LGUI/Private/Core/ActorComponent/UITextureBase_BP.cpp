// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UITextureBase_BP.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"


UUITextureBase_BP::UUITextureBase_BP(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUITextureBase_BP::EndPlay(const EEndPlayReason::Type EndPlayReason)
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

void UUITextureBase_BP::OnBeforeCreateOrUpdateGeometry()
{
	OnBeforeCreateOrUpdateGeometry_BP();
}
void UUITextureBase_BP::OnCreateGeometry()
{
	if (!IsValid(createGeometryHelper))
	{
		createGeometryHelper = NewObject<ULGUICreateGeometryHelper>(this);
		createGeometryHelper->uiGeometry = geometry;
	}
	OnCreateGeometry_BP(createGeometryHelper);
}
void UUITextureBase_BP::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (!IsValid(updateGeometryHelper))
	{
		updateGeometryHelper = NewObject<ULGUIUpdateGeometryHelper>(this);
		updateGeometryHelper->uiGeometry = geometry;
	}
	OnUpdateGeometry_BP(updateGeometryHelper, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
}
void UUITextureBase_BP::MarkVertexChanged_BP()
{
	MarkVertexPositionDirty();
	MarkColorDirty();
	MarkUVDirty();
}
void UUITextureBase_BP::MarkRebuildGeometry_BP()
{
	MarkTriangleDirty();
}