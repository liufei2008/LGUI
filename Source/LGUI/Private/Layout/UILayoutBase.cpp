﻿// Copyright 2019 LexLiu. All Rights Reserved.

#include "Layout/UILayoutBase.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Layout/UILayoutElement.h"


UUILayoutBase::UUILayoutBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUILayoutBase::BeginPlay()
{
	Super::BeginPlay();

	//recreate list at start
	RebuildChildrenList();

	OnRebuildLayout();
}

void UUILayoutBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UUILayoutBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
void UUILayoutBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	OnRebuildLayout();
	if (CheckRootUIComponent())
	{
		RootUIComp->EditorForceUpdateImmediately();
	}
}
#endif

void UUILayoutBase::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
	{
		RebuildChildrenList();
	}
#endif
}
void UUILayoutBase::OnUnregister()
{
	Super::OnUnregister();
}

void UUILayoutBase::RebuildChildrenList()
{
	if (CheckRootUIComponent())
	{
		availableChildrenArray.Reset();
		const auto& children = RootUIComp->GetAttachChildren();
		for (auto item : children)
		{
			if (auto uiItem = Cast<UUIItem>(item))
			{
				if (uiItem->IsUIActiveInHierarchy())
				{
					UUILayoutElement* layoutElement = GetLayoutElement(uiItem->GetOwner());
					if (layoutElement)
					{
						if (layoutElement->GetIgnoreLayout())
						{
							continue;
						}
					}
					FAvaliableChild child;
					child.uiItem = uiItem;
					child.layoutElement = layoutElement;
					availableChildrenArray.Add(child);
					OnAttachValidChild(uiItem);
				}
			}
		}
		availableChildrenArray.Sort([](FAvaliableChild A, FAvaliableChild B)//sort children by HierarchyIndex
		{
			if (A.uiItem->GetHierarchyIndex() < B.uiItem->GetHierarchyIndex())
				return true;
			return false;
		});
	}
}


void UUILayoutBase::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
	if(sizeChanged)
		OnRebuildLayout();
}
void UUILayoutBase::OnUIAttachmentChanged()
{
	OnRebuildLayout();
}
void UUILayoutBase::OnUIActiveInHierachy(bool activeOrInactive)
{
	OnRebuildLayout();
}
void UUILayoutBase::OnUIChildAcitveInHierarchy(UUIItem* InChild, bool InUIActive)
{
	int32 index;
	FAvaliableChild childData;
	childData.uiItem = InChild;
	if (InUIActive)
	{
		if (!availableChildrenArray.Find(childData, index))
		{
			UUILayoutElement* layoutElement = GetLayoutElement(InChild->GetOwner());
			if (layoutElement)
			{
				if (layoutElement->GetIgnoreLayout())
				{
					return;
				}
			}
			childData.layoutElement = layoutElement;
			availableChildrenArray.Add(childData);

			availableChildrenArray.Sort([](FAvaliableChild A, FAvaliableChild B)//sort children by HierarchyIndex
			{
				if (A.uiItem->GetHierarchyIndex() < B.uiItem->GetHierarchyIndex())
					return true;
				return false;
			});
		}
	}
	else
	{
		if (availableChildrenArray.Find(childData, index))
		{
			availableChildrenArray.RemoveAt(index);
		}
	}

	OnRebuildLayout();
}
void UUILayoutBase::OnUIChildAttachmentChanged(UUIItem* InChild, bool attachOrDetach)
{
	int32 index;
	FAvaliableChild childData;
	childData.uiItem = InChild;
	if (attachOrDetach && InChild->IsUIActiveInHierarchy())
	{
		if (!availableChildrenArray.Find(childData, index))
		{
			UUILayoutElement* layoutElement = GetLayoutElement(InChild->GetOwner());
			if (layoutElement)
			{
				if (layoutElement->GetIgnoreLayout())
				{
					return;
				}
			}
			childData.layoutElement = layoutElement;
			availableChildrenArray.Add(childData);
			//sort by hierarchy index
			availableChildrenArray.Sort([](FAvaliableChild A, FAvaliableChild B)//sort children by HierarchyIndex
			{
				if (A.uiItem->GetHierarchyIndex() < B.uiItem->GetHierarchyIndex())
					return true;
				return false;
			});
			OnAttachValidChild(InChild);
		}
	}
	else
	{
		if (availableChildrenArray.Find(childData, index))
		{
			availableChildrenArray.RemoveAt(index);
			OnDetachValidChild(InChild);
		}
	}

	OnRebuildLayout();
}
void UUILayoutBase::OnUIChildHierarchyIndexChanged(UUIItem* InChild)
{
	int32 index;
	FAvaliableChild childData;
	childData.uiItem = InChild;
	if (availableChildrenArray.Find(childData, index))
	{
		availableChildrenArray.Sort([](FAvaliableChild A, FAvaliableChild B)//sort children by HierarchyIndex
		{
			if (A.uiItem->GetHierarchyIndex() < B.uiItem->GetHierarchyIndex())
				return true;
			return false;
		});
	}

	OnRebuildLayout();
}

FORCEINLINE UUILayoutElement* UUILayoutBase::GetLayoutElement(AActor* Target)
{
	return Target->FindComponentByClass<UUILayoutElement>();
}