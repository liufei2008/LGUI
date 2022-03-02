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
	Super::EndPlay(EndPlayReason);
}

void UUISpriteBase_BP::OnBeforeCreateOrUpdateGeometry()
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		if (!IsValid(createGeometryHelper))
		{
			createGeometryHelper = NewObject<ULGUICreateGeometryHelper>(this);
		}
		ReceiveOnBeforeCreateOrUpdateGeometry();
	}
}

DECLARE_CYCLE_STAT(TEXT("UUISpriteBase_BP.OnUpdateGeometry"), STAT_UISprite_OnUpdateGeometry, STATGROUP_LGUI);
void UUISpriteBase_BP::OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		createGeometryHelper->UIGeo = &InGeo;
		SCOPE_CYCLE_COUNTER(STAT_UISprite_OnUpdateGeometry);
		ReceiveOnUpdateGeometry(createGeometryHelper, sprite, InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
	}
}
