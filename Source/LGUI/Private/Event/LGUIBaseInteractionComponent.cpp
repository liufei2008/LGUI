﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Event/LGUIBaseInteractionComponent.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Engine/SceneCapture2D.h"
#include "Core/ActorComponent/UIItem.h"

ULGUIBaseInteractionComponent::ULGUIBaseInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = true;
	traceChannel = ETraceTypeQuery::TraceTypeQuery3;
}

void ULGUIBaseInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	clickThresholdSquare = clickThreshold * clickThreshold;
}

void ULGUIBaseInteractionComponent::Activate(bool bReset)
{
	Super::Activate(bReset);
	if (this->GetWorld() == nullptr)return;
#if WITH_EDITOR
	if (this->GetWorld()->IsGameWorld())
#endif
	{
		ActivateRaycaster();
	}
}
void ULGUIBaseInteractionComponent::Deactivate()
{
	Super::Deactivate();
	DeactivateRaycaster();
}
void ULGUIBaseInteractionComponent::ActivateRaycaster()
{
	ALGUIManagerActor::AddRaycaster(this);
}
void ULGUIBaseInteractionComponent::DeactivateRaycaster()
{
	ALGUIManagerActor::RemoveRaycaster(this);
}
void ULGUIBaseInteractionComponent::OnUnregister()
{
	Super::OnUnregister();
	DeactivateRaycaster();
}

bool ULGUIBaseInteractionComponent::ShouldStartDrag_HoldToDrag(ULGUIPointerEventData* InPointerEventData)
{
	if (holdToDrag)
	{
		return GetWorld()->TimeSeconds - InPointerEventData->pressTime > holdToDragTime;
	}
	return false;
}

bool ULGUIBaseInteractionComponent::RaycastUI(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)
{
	OutHoverArray.Reset();
	if (GenerateRay(InPointerEventData, OutRayOrigin, OutRayDirection))
	{
		CurrentRayOrigin = OutRayOrigin;
		CurrentRayDirection = OutRayDirection;
		//UI element need to check if hit visible
		multiHitResult.Reset();
		OutRayEnd = OutRayDirection * rayLength + OutRayOrigin;

		if (auto LGUIManagerActor = ALGUIManagerActor::GetLGUIManagerActorInstance(this->GetWorld()))
		{
			const auto& AllUIItemArray = LGUIManagerActor->GetAllUIItemArray();
			for (auto& uiItem : AllUIItemArray)
			{
				if (ShouldSkipUIItem(uiItem.Get()))continue;

				FHitResult thisHit;
				if (uiItem->IsRegistered()
					&& uiItem->GetTraceChannel() == traceChannel
					&& uiItem->IsRaycastTarget()
					&& uiItem->GetIsUIActiveInHierarchy()
					&& uiItem->GetRenderCanvas() != nullptr
					&& uiItem->IsGroupAllowInteraction()
					&& uiItem->LineTraceUI(thisHit, OutRayOrigin, OutRayEnd)
					)
				{
					if (uiItem->GetRenderCanvas()->IsPointVisible(thisHit.Location))
					{
						multiHitResult.Add(thisHit);
					}
				}
			}
		}
		else
		{
			return false;
		}
		
		int hitCount = multiHitResult.Num();
		if (hitCount > 0)
		{
			multiHitResult.Sort([](const FHitResult& A, const FHitResult& B)
				{
					auto AUIItem = (UUIItem*)(A.Component.Get());
					auto BUIItem = (UUIItem*)(B.Component.Get());
					if (AUIItem != nullptr && BUIItem != nullptr)
					{
						if (AUIItem->GetRenderCanvas() == nullptr && BUIItem->GetRenderCanvas() != nullptr) return false;//if A not render yet
						if (AUIItem->GetRenderCanvas() != nullptr && BUIItem->GetRenderCanvas() == nullptr) return true;//if B not render yet
						if (AUIItem->GetRenderCanvas() == nullptr && BUIItem->GetRenderCanvas() == nullptr) return true;//if A and B not renderred, doesnt matter which one

						auto ACanvasSortOrder = AUIItem->GetRenderCanvas()->GetActualSortOrder();
						auto BCanvasSortOrder = BUIItem->GetRenderCanvas()->GetActualSortOrder();
						if (AUIItem->GetRenderCanvas() != BUIItem->GetRenderCanvas() && ACanvasSortOrder != BCanvasSortOrder)//not in same sort order
						{
							return ACanvasSortOrder > BCanvasSortOrder;
						}
						else//same Canvas, sort on item's hierarchy order
						{
							return AUIItem->GetFlattenHierarchyIndex() > BUIItem->GetFlattenHierarchyIndex();
						}
					}
					return true;
				});

			//consider UI may not visible or CanvasGroup not allow interaction, so we cannot take first one as result, we need to check from start
			bool haveValidHitResult = false;
			for (int i = 0; i < hitCount; i++)
			{
				auto hit = multiHitResult[i];
				auto hitUIItem = (UUIItem*)(hit.Component.Get());
				if (!haveValidHitResult)
				{
					OutHitResult = hit;
					haveValidHitResult = true;
				}
				OutHoverArray.Add(hit.Component.Get());
			}
			if (haveValidHitResult)
				return true;
		}
	}
	return false;
}

bool ULGUIBaseInteractionComponent::RaycastWorld(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)
{
	OutHoverArray.Reset();
	if (GenerateRay(InPointerEventData, OutRayOrigin, OutRayDirection))
	{
		multiHitResult.Reset();
		OutRayEnd = OutRayDirection * rayLength + OutRayOrigin;

		FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
		this->GetWorld()->LineTraceMultiByChannel(multiHitResult, OutRayOrigin, OutRayEnd, UEngineTypes::ConvertToCollisionChannel(traceChannel), queryParams);

		int hitCount = multiHitResult.Num();
		if (hitCount > 0)
		{
			//this->GetWorld()->LineTraceMultiByChannel() result is sorted
			//multiWorldHitResult.Sort([](const FHitResult& A, const FHitResult& B)//sort on depth
			//{
			//	return A.Distance < B.Distance;
			//});
			OutHitResult = multiHitResult[0];
			for (auto item : multiHitResult)
			{
				OutHoverArray.Add(item.Component.Get());
			}
			return true;
		}
	}
	return false;
}


void ULGUIBaseInteractionComponent::SetClickThreshold(float value)
{
	clickThreshold = value;
	clickThresholdSquare = clickThreshold * clickThreshold;
}
void ULGUIBaseInteractionComponent::SetHoldToDrag(bool value)
{
	holdToDrag = value;
}
void ULGUIBaseInteractionComponent::SetHoldToDragTime(float value)
{
	holdToDragTime = value;
}
