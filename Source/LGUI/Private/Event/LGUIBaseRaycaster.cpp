// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Event/LGUIBaseRaycaster.h"
#include "Core/Actor/LGUIManager.h"
#include "Engine/SceneCapture2D.h"
#include "Core/ActorComponent/UIItem.h"

ULGUIBaseRaycaster::ULGUIBaseRaycaster()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = true;
	traceChannel = ETraceTypeQuery::TraceTypeQuery3;
}

void ULGUIBaseRaycaster::BeginPlay()
{
	Super::BeginPlay();
	clickThresholdSquare = clickThreshold * clickThreshold;
}

void ULGUIBaseRaycaster::Activate(bool bReset)
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
void ULGUIBaseRaycaster::Deactivate()
{
	Super::Deactivate();
	DeactivateRaycaster();
}
void ULGUIBaseRaycaster::ActivateRaycaster()
{
	ULGUIManagerWorldSubsystem::AddRaycaster(this);
}
void ULGUIBaseRaycaster::DeactivateRaycaster()
{
	ULGUIManagerWorldSubsystem::RemoveRaycaster(this);
}
void ULGUIBaseRaycaster::OnUnregister()
{
	Super::OnUnregister();
	DeactivateRaycaster();
}

bool ULGUIBaseRaycaster::ShouldStartDrag_HoldToDrag(ULGUIPointerEventData* InPointerEventData)
{
	if (holdToDrag)
	{
		return GetWorld()->TimeSeconds - InPointerEventData->pressTime > holdToDragTime;
	}
	return false;
}

bool ULGUIBaseRaycaster::RaycastUI(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)
{
	OutHoverArray.Reset();
	if (GenerateRay(InPointerEventData, OutRayOrigin, OutRayDirection))
	{
		CurrentRayOrigin = OutRayOrigin;
		CurrentRayDirection = OutRayDirection;
		//UI element need to check if hit visible
		multiHitResult.Reset();
		OutRayEnd = OutRayDirection * rayLength + OutRayOrigin;

		if (auto LGUIManager = ULGUIManagerWorldSubsystem::GetInstance(this->GetWorld()))
		{
			auto& AllCanvasArray = LGUIManager->GetCanvasArray();
			for (auto& CanvasItem : AllCanvasArray)
			{
				if (ShouldSkipCanvas(CanvasItem.Get()))continue;
				auto& AllUIItemArray = CanvasItem->GetUIItemArray();
				for (auto& uiItem : AllUIItemArray)
				{
					if (!IsValid(uiItem))continue;

					FHitResult thisHit;
					if (
						uiItem->IsRaycastTarget()
						&& uiItem->IsGroupAllowInteraction()
						&& uiItem->GetTraceChannel() == traceChannel
						&& uiItem->GetIsUIActiveInHierarchy()
						&& uiItem->GetRenderCanvas() != nullptr//must have valid canvas to render
						&& uiItem->LineTraceUI(thisHit, OutRayOrigin, OutRayEnd)
						)
					{
						if (uiItem->GetRenderCanvas()->CalculatePointVisibilityOnClip(thisHit.Location))
						{
							multiHitResult.Add(thisHit);
						}
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

bool ULGUIBaseRaycaster::RaycastWorld(bool InRequireFaceIndex, ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)
{
	OutHoverArray.Reset();
	if (GenerateRay(InPointerEventData, OutRayOrigin, OutRayDirection))
	{
		multiHitResult.Reset();
		OutRayEnd = OutRayDirection * rayLength + OutRayOrigin;

		FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
		queryParams.bReturnFaceIndex = InRequireFaceIndex;
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
			for (auto& item : multiHitResult)
			{
				OutHoverArray.Add(item.Component.Get());
			}
			return true;
		}
	}
	return false;
}


void ULGUIBaseRaycaster::SetClickThreshold(float value)
{
	clickThreshold = value;
	clickThresholdSquare = clickThreshold * clickThreshold;
}
void ULGUIBaseRaycaster::SetHoldToDrag(bool value)
{
	holdToDrag = value;
}
void ULGUIBaseRaycaster::SetHoldToDragTime(float value)
{
	holdToDragTime = value;
}
