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
	Super::EndPlay(EndPlayReason);
}

void UUITextureBase_BP::OnBeforeCreateOrUpdateGeometry()
{
	if (!IsValid(createGeometryHelper))
	{
		createGeometryHelper = NewObject<ULGUICreateGeometryHelper>(this);
	}
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnBeforeCreateOrUpdateGeometry();
	}
}

DECLARE_CYCLE_STAT(TEXT("UUITextureBase_BP.OnUpdateGeometry"), STAT_UITexture_OnUpdateGeometry, STATGROUP_LGUI);
void UUITextureBase_BP::OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		createGeometryHelper->UIGeo = &InGeo;
		SCOPE_CYCLE_COUNTER(STAT_UITexture_OnUpdateGeometry);
		ReceiveOnUpdateGeometry(createGeometryHelper, InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
	}
}
