// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnBeforeCreateOrUpdateGeometry();
	}
}
void UUISpriteBase_BP::OnCreateGeometry()
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		if (!IsValid(createGeometryHelper))
		{
			createGeometryHelper = NewObject<ULGUICreateGeometryHelper>(this);
			createGeometryHelper->uiGeometry = geometry;
		}
		ReceiveOnCreateGeometry(createGeometryHelper, sprite);
	}
}
void UUISpriteBase_BP::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		if (!IsValid(updateGeometryHelper))
		{
			updateGeometryHelper = NewObject<ULGUIUpdateGeometryHelper>(this);
			updateGeometryHelper->uiGeometry = geometry;
		}
		updateGeometryHelper->BeginUpdateVertices();
		ReceiveOnUpdateGeometry(updateGeometryHelper, sprite, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
		updateGeometryHelper->EndUpdateVertices();
	}
}
void UUISpriteBase_BP::MarkVertexChanged()
{
	MarkVertexPositionDirty();
	MarkColorDirty();
	MarkUVDirty();
}
void UUISpriteBase_BP::MarkRebuildGeometry()
{
	MarkTriangleDirty();
}