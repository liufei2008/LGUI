// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIRenderableCustomRaycast.h"


void UUIRenderableCustomRaycast::Awake()
{
	Super::Awake();
	this->SetCanExecuteUpdate(false);
}

bool UUIRenderableCustomRaycast::OnRaycast(const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, const FVector2D& InHitPointOnPlane, FVector& OutHitPoint, FVector& OutHitNormal)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		return ReceiveOnRaycast(InLocalSpaceRayStart, InLocalSpaceRayEnd, InHitPointOnPlane, OutHitPoint, OutHitNormal);
	}
	return false;
}