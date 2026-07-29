// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Extensions/LexUIRenderTargetInteraction.h"
#include "Core/Components/LexWidget.h"
#include "Core/Components/LexCanvas.h"
#include "LGUI.h"
#include "Event/LexWorldSpaceRaycasterBase.h"
#include "Extensions/LexUIRenderTargetGeometrySource.h"
#include "Event/LexScreenSpaceRaycaster.h"
#include "Engine/World.h"
#include "Event/LexEventSystem.h"
#include "Event/InputModule/LexPointerInputModule.h"

#define LOCTEXT_NAMESPACE "LGUIRenderTargetInteraction"

ULexUIRenderTargetInteraction::ULexUIRenderTargetInteraction()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void ULexUIRenderTargetInteraction::BeginPlay()
{
	Super::BeginPlay();
	PointerEventData = NewObject<ULexPointerEventData>(this);
	PointerEventData->PointerID = -1;//make it -1, different from LGUIEventSystem created
}

void ULexUIRenderTargetInteraction::OnRegister()
{
	Super::OnRegister();
}

void ULexUIRenderTargetInteraction::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(LineTraceSource))
	{
		LineTraceSource = GetOwner()->FindComponentByInterface(ULexUIRenderTargetInteractionSourceInterface::StaticClass());
		if (!IsValid(LineTraceSource))
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d InteractionSource is not valid! LGUIRenderTargetInteraction need a valid component which inherit %s on the same actor!")
				, ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(ULexUIRenderTargetInteractionSourceInterface::StaticClass()->GetName()));
			return;
		}
	}
	if (!TargetCanvas.IsValid())
	{
		TargetCanvas = ILexUIRenderTargetInteractionSourceInterface::Execute_GetTargetCanvas(LineTraceSource);
		if (!TargetCanvas.IsValid())
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d TargetCanvas is not valid! LGUIRenderTargetInteraction need to get a vaild LGUICanvas from InteractionSource!")
				, ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return;
		}
	}
	if (!InputPointerEventData.IsValid())
		return;

	FLexUIHitResultContainer hitResultContainer;
	bool lineTraceHitSomething = LineTrace(hitResultContainer);
	bool resultHitSomething = false;
	FLexUIHitResult hitResult;
	ULexPointerInputModule::ProcessPointerEvent(nullptr, PointerEventData, lineTraceHitSomething, hitResultContainer, resultHitSomething, hitResult);
}

void ULexUIRenderTargetInteraction::ActivateRaycaster()
{
	//skip Activate && Deactivate, because ULGUIRenderTargetInteraction will process input and interaction by itself
}
void ULexUIRenderTargetInteraction::DeactivateRaycaster()
{
	
}

bool ULexUIRenderTargetInteraction::LineTrace(FLexUIHitResultContainer& OutHitResult)
{
	if (InputPointerEventData->Raycaster == nullptr)return false;
	auto RayOrigin = InputPointerEventData->Raycaster->GetRayOrigin();
	auto RayDirection = InputPointerEventData->Raycaster->GetRayDirection();

	auto RayEnd = RayOrigin + RayDirection * RayLength;

	FVector2D HitUV;
	if (ILexUIRenderTargetInteractionSourceInterface::Execute_PerformLineTrace(LineTraceSource, InputPointerEventData->FaceIndex, InputPointerEventData->WorldPoint, RayOrigin, RayEnd, HitUV))
	{
		auto ViewProjectionMatrix = TargetCanvas->GetViewProjectionMatrix();
		FVector2D mousePos01 = HitUV;
		PointerEventData->PointerPosition = FVector(mousePos01 * TargetCanvas->GetViewportSize(), 0);

		FVector OutRayOrigin, OutRayDirection;
		ULexScreenSpaceRaycaster::DeprojectViewPointToWorld(ViewProjectionMatrix, mousePos01, OutRayOrigin, OutRayDirection);

		TArray<FLexUIHitResult> HitResultArray;
		this->Raycast(PointerEventData, OutRayOrigin, OutRayDirection, RayEnd, HitResultArray);
		if (HitResultArray.Num() > 0)
		{
			FLexUIHitResultContainer LexHitResult;
			LexHitResult.HitResult = HitResultArray[0];
			LexHitResult.Raycaster = this;
			LexHitResult.RayOrigin = OutRayOrigin;
			LexHitResult.RayDirection = OutRayDirection;
			LexHitResult.RayEnd = RayEnd;
			for (auto& HitItem : HitResultArray)
			{
				LexHitResult.HoverArray.Add(HitItem.Widget.Get());
			}
			OutHitResult = LexHitResult;
		}

		return true;
	}
	return false;
}

bool ULexUIRenderTargetInteraction::ShouldStartDrag(ULexPointerEventData* InPointerEventData)
{
	if (bHoldToDrag)
	{
		if (GetWorld()->TimeSeconds - InPointerEventData->PressTime > HoldToDragTime)
		{
			return true;
		}
	}
	FVector2D mousePos = FVector2D(InPointerEventData->PointerPosition);
	FVector2D pressMousePos = FVector2D(InPointerEventData->PressPointerPosition);
	return FVector2D::DistSquared(pressMousePos, mousePos) > DragThresholdSquare;
}
void ULexUIRenderTargetInteraction::Raycast(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, TArray<FLexUIHitResult>& OutHitResultArray)
{
	return Super::RaycastUI(InPointerEventData, TargetCanvas.Get(), OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResultArray);
}


void ULexUIRenderTargetInteraction::OnPointerEnter_Implementation(ULexPointerEventData* EventData)
{
	InputPointerEventData = EventData;
}
void ULexUIRenderTargetInteraction::OnPointerExit_Implementation(ULexPointerEventData* EventData)
{
}
bool ULexUIRenderTargetInteraction::OnPointerDown_Implementation(ULexPointerEventData* EventData)
{
	PointerEventData->PressPointerPosition = PointerEventData->PointerPosition;
	PointerEventData->PressTime = GetWorld()->TimeSeconds;
	PointerEventData->bNowIsTriggerPressed = true;
	PointerEventData->MouseButtonType = EventData->MouseButtonType;
	return bAllowEventBubbleUp;
}
bool ULexUIRenderTargetInteraction::OnPointerUp_Implementation(ULexPointerEventData* EventData)
{
	PointerEventData->ReleaseTime = GetWorld()->TimeSeconds;
	PointerEventData->bNowIsTriggerPressed = false;
	return bAllowEventBubbleUp;
}
bool ULexUIRenderTargetInteraction::OnPointerScroll_Implementation(ULexPointerEventData* EventData)
{
	auto inAxisValue = EventData->ScrollAxisValue;
	if (IsValid(PointerEventData->EnterWidget))
	{
		if (inAxisValue != FVector2D::ZeroVector || PointerEventData->ScrollAxisValue != inAxisValue)
		{
			PointerEventData->ScrollAxisValue = inAxisValue;
			ULexEventSystem::ExecuteEvent_OnPointerScroll(PointerEventData->EnterWidget, PointerEventData, true);
		}
	}
	return bAllowEventBubbleUp;
}

#undef LOCTEXT_NAMESPACE