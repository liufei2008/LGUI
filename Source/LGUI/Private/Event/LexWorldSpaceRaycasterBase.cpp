// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Event/LexWorldSpaceRaycasterBase.h"
#include "Core/LexUISettings.h"


ULexWorldSpaceRaycasterSource::ULexWorldSpaceRaycasterSource()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void ULexWorldSpaceRaycasterSource::BeginPlay()
{
	Super::BeginPlay();
	DragThresholdSquare = DragThreshold * DragThreshold;
}

void ULexWorldSpaceRaycasterSource::SetRayLength(float Value)
{
	RayLength = Value;
}
void ULexWorldSpaceRaycasterSource::SetDragThreshold(float Value)
{
	DragThreshold = Value;
	DragThresholdSquare = DragThreshold * DragThreshold;
}
void ULexWorldSpaceRaycasterSource::SetHoldToDrag(bool Value)
{
	bHoldToDrag = Value;
}
void ULexWorldSpaceRaycasterSource::SetHoldToDragTime(float Value)
{
	HoldToDragTime = Value;
}

bool ULexWorldSpaceRaycasterSource::GenerateRay(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		return ReceiveGenerateRay(InPointerEventData, OutRayOrigin, OutRayDirection, OutRayEnd);
	}
	return false;
}
bool ULexWorldSpaceRaycasterSource::ShouldStartDrag(ULexPointerEventData* InPointerEventData)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		return ReceiveShouldStartDrag(InPointerEventData);
	}
	return false;
}

ALexWorldSpaceRaycasterSourceActor::ALexWorldSpaceRaycasterSourceActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

ULexWorldSpaceRaycasterBase::ULexWorldSpaceRaycasterBase()
{
	TraceChannel = TraceTypeQuery1;
}

void ULexWorldSpaceRaycasterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ULexWorldSpaceRaycasterBase::OnRegister()
{
	Super::OnRegister();
}

bool ULexWorldSpaceRaycasterBase::GetAffectByGamePause()const
{
	return GetDefault<ULexUISettings>()->bWorldSpaceUIAffectByGamePause;
}

bool ULexWorldSpaceRaycasterBase::GenerateRay(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, float& OutRayLength)
{
	auto RaycasterSrc = GetRaycasterSourceObject();
	if (!RaycasterSrc)
	{
		auto DebugMsg = FString::Printf(TEXT("[%s].%d There is no RayCasterSourceObject! WorldSpaceRaycaster '%s' will not work!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__
#if WITH_EDITOR
			, *(this->GetOwner()->GetActorLabel())
#else
			, *this->GetPathName()
#endif
			);
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, DebugMsg);
		return false;
	}
	OutRayLength = RaycasterSrc->GetRayLength();
	return RaycasterSrc->GenerateRay(InPointerEventData, OutRayOrigin, OutRayDirection, OutRayEnd);
}

bool ULexWorldSpaceRaycasterBase::ShouldStartDrag(ULexPointerEventData* InPointerEventData) 
{
	auto RaycasterSrc = GetRaycasterSourceObject();
	if (!RaycasterSrc)return true;
	return RaycasterSrc->ShouldStartDrag(InPointerEventData);
}

ULexWorldSpaceRaycasterSource* ULexWorldSpaceRaycasterBase::GetRaycasterSourceObject() const
{
	if (!RaycasterSourceObject.IsValid())
	{
		if (RaycasterSourceActor.IsValid())
		{
			RaycasterSourceObject = RaycasterSourceActor->GetRaycasterSource();
		}
	}
	return RaycasterSourceObject.Get();
}

void ULexWorldSpaceRaycasterBase::SetTraceChannel(TEnumAsByte<ETraceTypeQuery> Value)
{
	TraceChannel = Value;
}
void ULexWorldSpaceRaycasterBase::SetRaycasterSourceObject(ULexWorldSpaceRaycasterSource* Value)
{
	RaycasterSourceObject = Value;
}

void ULexWorldSpaceRaycasterBase::SetRaycasterSourceActor(ALexWorldSpaceRaycasterSourceActor* Value)
{
	RaycasterSourceActor = Value;
}
