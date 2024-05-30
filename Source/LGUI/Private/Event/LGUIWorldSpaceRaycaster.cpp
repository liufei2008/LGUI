// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Event/LGUIWorldSpaceRaycaster.h"
#include "Event/RaycasterSource/LGUIWorldSpaceRaycasterSource_Mouse.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/LGUISettings.h"


ULGUIBaseRaycaster* ULGUIWorldSpaceRaycasterSource::GetRaycasterObject()const
{
	return RaycasterObject.Get();
}
void ULGUIWorldSpaceRaycasterSource::Init(ULGUIBaseRaycaster* InRaycaster)
{
	RaycasterObject = InRaycaster;
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveInit(InRaycaster);
	}
}
bool ULGUIWorldSpaceRaycasterSource::GenerateRay(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		return ReceiveGenerateRay(InPointerEventData, OutRayOrigin, OutRayDirection);
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
	if (RaycasterSourceObject == nullptr)
	{
		RaycasterSourceObject = NewObject<ULGUIWorldSpaceRaycasterSource_Mouse>(this);
	}
	RaycasterSourceObject->Init(this);
	RenderModeArray = { ELGUIRenderMode::WorldSpace, ELGUIRenderMode::WorldSpace_LGUI };
}

bool ULGUIWorldSpaceRaycaster::ShouldSkipCanvas(class ULGUICanvas* UICanvas)
{
	return false;
}
bool ULGUIWorldSpaceRaycaster::GetAffectByGamePause()const
{
	return GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByGamePause;
}
bool ULGUIWorldSpaceRaycaster::Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)
{
	switch (interactionTarget)
	{
	case ELGUIInteractionTarget::UI:
		return Super::RaycastUI(InPointerEventData, RenderModeArray, OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult, OutHoverArray);
	case ELGUIInteractionTarget::World:
		return Super::RaycastWorld(bRequireFaceIndex, InPointerEventData, OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult, OutHoverArray);
	case ELGUIInteractionTarget::UIAndWorld:
	{
		FVector UIRayOrigin, UIRayDirection, UIRayEnd;
		FVector WorldRayOrigin, WorldRayDirection, WorldRayEnd;
		FHitResult UIHitResult, WorldHitResult;
		TArray<USceneComponent*> UIHoverArray, WorldHoverArray;
		auto HitUI = Super::RaycastUI(InPointerEventData, RenderModeArray, UIRayOrigin, UIRayDirection, UIRayEnd, UIHitResult, UIHoverArray);
		auto HitWorld = Super::RaycastWorld(bRequireFaceIndex, InPointerEventData, WorldRayOrigin, WorldRayDirection, WorldRayEnd, WorldHitResult, WorldHoverArray);
		if (HitUI && HitWorld)
		{
			if (UIHitResult.Distance <= WorldHitResult.Distance)
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
	if (!IsValid(RaycasterSourceObject))return false;
	return RaycasterSourceObject->GenerateRay(InPointerEventData, OutRayOrigin, OutRayDirection);
}

bool ULGUIWorldSpaceRaycaster::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData) 
{
	if (!IsValid(RaycasterSourceObject))return false;
	if (ShouldStartDrag_HoldToDrag(InPointerEventData))return true;
	return RaycasterSourceObject->ShouldStartDrag(InPointerEventData);
}

void ULGUIWorldSpaceRaycaster::SetRaycasterSourceObject(ULGUIWorldSpaceRaycasterSource* NewSource)
{
	RaycasterSourceObject = NewSource;
}
