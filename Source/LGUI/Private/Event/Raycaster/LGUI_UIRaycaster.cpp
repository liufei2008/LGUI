// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/Raycaster/LGUI_UIRaycaster.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/Actor/LGUIManagerActor.h"

ULGUI_UIRaycaster::ULGUI_UIRaycaster()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	traceChannel = ETraceTypeQuery::TraceTypeQuery3;
}

bool ULGUI_UIRaycaster::IsHitVisibleUI(UUIItem* HitUI, const FVector& HitPoint)
{
	if (HitUI->IsUIActiveInHierarchy())
	{
		auto renderCanvas = HitUI->GetRenderCanvas();
		if (renderCanvas)
		{
			if (renderCanvas->IsPointVisible(HitPoint))//visible point
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else//no RenderCanvas means not render yet
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool ULGUI_UIRaycaster::IsUIInteractionGroupAllowHit(UUIItem* HitUI)
{
	return HitUI->IsGroupAllowInteraction();
}

bool ULGUI_UIRaycaster::Raycast(FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult)
{
	traceIgnoreActorArray.Reset();
	traceOnlyActorArray.Reset();
	if (GenerateRay(OutRayOrigin, OutRayDirection, traceOnlyActorArray, traceIgnoreActorArray))
	{
		//UI element need to check if hit visible
		multiUIHitResult.Reset();
		OutRayEnd = OutRayDirection * rayLength + OutRayOrigin;
		if (traceOnlyActorArray.Num() == 0)//trace all
		{
			if (ALGUIManagerActor::Instance != nullptr)
			{
				auto allUIItemArray = ALGUIManagerActor::Instance->GetAllUIItem();
				for (auto uiItem : allUIItemArray)
				{
					if (ShouldSkipUIItem(uiItem))continue;
					if (traceIgnoreActorArray.Num() > 0)
					{
						int index;
						if (traceIgnoreActorArray.Find(uiItem->GetOwner(), index))
						{
							traceIgnoreActorArray.RemoveAt(index);
							continue;
						}
					}
					FHitResult thisHit;
					if (uiItem->IsRegistered()
						&& uiItem->GetTraceChannel() == traceChannel
						&& uiItem->LineTraceUI(thisHit, OutRayOrigin, OutRayEnd)
						)
					{
						multiUIHitResult.Add(thisHit);
					}
				}
			}
			else
			{
				return false;
			}
		}
		else//trace only specific actors
		{
			//todo:need to get root object, because this function will go through hierarchy to check all children. find root object can avoid repetition
			LineTraceUIHierarchy(multiUIHitResult, false, traceOnlyActorArray, OutRayOrigin, OutRayEnd, traceChannel);
		}
		int hitCount = multiUIHitResult.Num();
		if (hitCount > 0)
		{
			EUIRaycastSortType sortType = uiSortType;
			float threshold = uiSortDependOnDistanceThreshold;
			multiUIHitResult.Sort([sortType, threshold](const FHitResult& A, const FHitResult& B)//sort on depth
			{
				if (sortType == EUIRaycastSortType::DependOnUIDepth//sort on depth
					|| FMath::Abs(A.Distance - B.Distance) < threshold//if distance less than threshold, then sort on depth too
					)
				{
					auto AUIItem = (UUIItem*)(A.Component.Get());
					auto BUIItem = (UUIItem*)(B.Component.Get());
					if (AUIItem != nullptr && BUIItem != nullptr)
					{
						if (AUIItem->GetRenderCanvas() == nullptr && BUIItem->GetRenderCanvas() != nullptr) return false;//if A not render yet
						if (AUIItem->GetRenderCanvas() != nullptr && BUIItem->GetRenderCanvas() == nullptr) return true;//if B not render yet
						if (AUIItem->GetRenderCanvas() == nullptr && BUIItem->GetRenderCanvas() == nullptr) return true;//if A and B not renderred, doesnt matter which one

						if (AUIItem->GetRenderCanvas() == BUIItem->GetRenderCanvas())//if Canvas's depth is equal then sort on item's depth
						{
							if (AUIItem->GetDepth() == BUIItem->GetDepth())//if item's depth is equal then sort on distance
							{
								return A.Distance < B.Distance;
							}
							else
								return AUIItem->GetDepth() > BUIItem->GetDepth();
						}
						else//if Canvas's depth not equal then sort on Canvas's SortOrder
						{
							return AUIItem->GetRenderCanvas()->GetSortOrder() > BUIItem->GetRenderCanvas()->GetSortOrder();
						}
					}
					else//if not a ui element, sort on depth
					{
						return A.Distance < B.Distance;
					}
				}
				else//sort on distance
				{
					return A.Distance < B.Distance;
				}
				return true;
			});

			//consider UI may not visible or InteractionGroup not allow interaction, so we cannot take first one as result, we need to check from start
			for (int i = 0; i < hitCount; i++)
			{
				auto hit = multiUIHitResult[i];
				if (auto hitUIItem = (UUIItem*)(hit.Component.Get()))
				{
					if (IsHitVisibleUI(hitUIItem, hit.Location) && IsUIInteractionGroupAllowHit(hitUIItem))
					{
						OutHitResult = hit;
						return true;
					}
				}
				else
				{
					OutHitResult = hit;
					return true;
				}
			}
		}
	}
	return false;
}

void ULGUI_UIRaycaster::LineTraceUIHierarchy(TArray<FHitResult>& OutHitArray, bool InSortResult, const TArray<AActor*>& InActorArray, const FVector& InRayOrign, const FVector& InRayEnd, ETraceTypeQuery InTraceChannel)
{
	for (int actorIndex = 0, actorCount = InActorArray.Num(); actorIndex < actorCount; actorIndex++)
	{
		auto actor = InActorArray[actorIndex];
		if (IsValid(actor))
		{
			LineTraceUIHierarchyRecursive(OutHitArray, actor->GetRootComponent(), InRayOrign, InRayEnd, InTraceChannel);
		}
	}
	if (InSortResult)
	{
		OutHitArray.Sort([](const FHitResult& A, const FHitResult& B)//sort on depth
		{
			return A.Distance < B.Distance;
		});
	}
}
void ULGUI_UIRaycaster::LineTraceUIHierarchyRecursive(TArray<FHitResult>& OutHitArray, USceneComponent* InSceneComp, const FVector& InRayOrign, const FVector& InRayEnd, ETraceTypeQuery InTraceChannel)
{
	const auto& childrenComp = InSceneComp->GetAttachChildren();
	for (int compIndex = 0, compCount = childrenComp.Num(); compIndex < compCount; compIndex++)
	{
		auto childSceneComp = childrenComp[compIndex];
		if (IsValid(childSceneComp))
		{
			if (auto uiItem = Cast<UUIItem>(childSceneComp))
			{
				FHitResult thisHit;
				if (uiItem->IsRegistered()
					&& uiItem->GetTraceChannel() == InTraceChannel
					&& uiItem->LineTraceUI(thisHit, InRayOrign, InRayEnd)
					)
				{
					OutHitArray.Add(thisHit);
				}
			}
			LineTraceUIHierarchyRecursive(OutHitArray, childSceneComp, InRayOrign, InRayEnd, InTraceChannel);
		}
	}
}