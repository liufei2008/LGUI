// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Event/LGUIWorldSpaceRaycaster.h"
#include "Event/RaycasterSource/LGUIWorldSpaceRaycasterSource_Mouse.h"
#include "Core/ActorComponent/UIItem.h"


ULGUIWorldSpaceRaycaster* ULGUIWorldSpaceRaycasterSource::GetInteractionObject()
{
	if (!InteractionObject.IsValid())
	{
		InteractionObject = Cast<ULGUIWorldSpaceRaycaster>(GetOuter());
	}
	return InteractionObject.Get();
}
bool ULGUIWorldSpaceRaycasterSource::EmitRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		return ReceiveEmitRay(InPointerEventData, OutRayOrigin, OutRayDirection);
	}
	return false;
}
bool ULGUIWorldSpaceRaycasterSource::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		return ReceiveShouldStartDrag(InPointerEventData);
	}
	return false;
}



ULGUIWorldSpaceRaycaster::ULGUIWorldSpaceRaycaster()
{
	
}

void ULGUIWorldSpaceRaycaster::BeginPlay()
{
	Super::BeginPlay();
}

void ULGUIWorldSpaceRaycaster::OnRegister()
{
	Super::OnRegister();
	if (InteractionSourceObject == nullptr)
	{
		InteractionSourceObject = NewObject<ULGUIWorldSpaceRaycasterSource_Mouse>(this);
	}
	InteractionSourceObject->SetInteractionObject(this);
}

bool ULGUIWorldSpaceRaycaster::ShouldSkipUIItem(class UUIItem* UIItem)
{
	if (UIItem != nullptr)
	{
		return !UIItem->IsWorldSpaceUI();
	}
	return true;
}

bool ULGUIWorldSpaceRaycaster::Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)
{
	switch (interactionTarget)
	{
	case ELGUIInteractionTarget::UI:
		return Super::RaycastUI(InPointerEventData, OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult, OutHoverArray);
	case ELGUIInteractionTarget::World:
		return Super::RaycastWorld(InPointerEventData, OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult, OutHoverArray);
	case ELGUIInteractionTarget::UIAndWorld:
	{
		FVector UIRayOrigin, UIRayDirection, UIRayEnd;
		FVector WorldRayOrigin, WorldRayDirection, WorldRayEnd;
		FHitResult UIHitResult, WorldHitResult;
		TArray<USceneComponent*> UIHoverArray, WorldHoverArray;
		auto HitUI = Super::RaycastUI(InPointerEventData, UIRayOrigin, UIRayDirection, UIRayEnd, UIHitResult, UIHoverArray);
		auto HitWorld = Super::RaycastWorld(InPointerEventData, WorldRayOrigin, WorldRayDirection, WorldRayEnd, WorldHitResult, WorldHoverArray);
		if (HitUI && HitWorld)
		{
			if (UIHitResult.Distance >= WorldHitResult.Distance)
			{
				goto HIT_UI;
			}
			else
			{
				goto HIT_WORLD;
			}
		}
		else
		{
			if (HitUI)
			{
				goto HIT_UI;
			}
			if (HitWorld)
			{
				goto HIT_WORLD;
			}
			return false;
		}
		HIT_UI:
		{
			OutRayOrigin = UIRayOrigin;
			OutRayDirection = UIRayDirection;
			OutRayEnd = UIRayEnd;
			OutHitResult = UIHitResult;
			OutHoverArray = UIHoverArray;
			return true;
		}
		HIT_WORLD:
		{
			OutRayOrigin = WorldRayOrigin;
			OutRayDirection = WorldRayDirection;
			OutRayEnd = WorldRayEnd;
			OutHitResult = WorldHitResult;
			OutHoverArray = WorldHoverArray;
			return true;
		}
	}
	}
	return false;
}

bool ULGUIWorldSpaceRaycaster::GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection)
{
	if (!IsValid(InteractionSourceObject))return false;
	return InteractionSourceObject->EmitRay(InPointerEventData, OutRayOrigin, OutRayDirection);
}

bool ULGUIWorldSpaceRaycaster::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData) 
{
	if (!IsValid(InteractionSourceObject))return false;
	if (ShouldStartDrag_HoldToDrag(InPointerEventData))return true;
	return InteractionSourceObject->ShouldStartDrag(InPointerEventData);
}

