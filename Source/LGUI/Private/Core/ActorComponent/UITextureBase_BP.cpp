// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
	if (!IsValid(createGeometryHelper))
	{
		createGeometryHelper = NewObject<ULGUICreateGeometryHelper>(this);
		createGeometryHelper->UIBatchGeometryRenderable = this;
	}
	if (!IsValid(updateGeometryHelper))
	{
		updateGeometryHelper = NewObject<ULGUIUpdateGeometryHelper>(this);
		createGeometryHelper->UIBatchGeometryRenderable = this;
	}
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnBeforeCreateOrUpdateGeometry();
	}
}
void UUITextureBase_BP::OnCreateGeometry()
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnCreateGeometry(createGeometryHelper);
	}
}
DECLARE_CYCLE_STAT(TEXT("UUITextureBase_BP.OnUpdateGeometry"), STAT_UITexture_OnUpdateGeometry, STATGROUP_LGUI);
void UUITextureBase_BP::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		updateGeometryHelper->BeginUpdateVertices();
		{
			SCOPE_CYCLE_COUNTER(STAT_UITexture_OnUpdateGeometry);
			ReceiveOnUpdateGeometry(updateGeometryHelper, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
		}
		updateGeometryHelper->EndUpdateVertices();
	}
}
void UUITextureBase_BP::MarkVertexChanged()
{
	MarkVertexPositionDirty();
	MarkColorDirty();
	MarkUVDirty();
}
void UUITextureBase_BP::MarkRebuildGeometry()
{
	MarkTriangleDirty();
}