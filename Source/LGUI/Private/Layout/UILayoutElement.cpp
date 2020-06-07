// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Layout/UILayoutElement.h"
#include "LGUI.h"
#include "Layout/UILayoutBase.h"
#include "Core/ActorComponent/UIItem.h"

UUILayoutElement::UUILayoutElement()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUILayoutElement::BeginPlay()
{
	Super::BeginPlay();
	if (CheckParentLayout())
	{
		ParentLayout->RebuildChildrenList();
		ParentLayout->OnRebuildLayout();
	}
}

void UUILayoutElement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

#if WITH_EDITOR
void UUILayoutElement::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	ParentLayout = nullptr;
	if (CheckParentLayout())
	{
		ParentLayout->RebuildChildrenList();
		ParentLayout->OnRebuildLayout();
	}
	if (auto owner = this->GetOwner())
	{
		if (auto uiItem = owner->FindComponentByClass<UUIItem>())
		{
			uiItem->EditorForceUpdateImmediately();
		}
	}
}
#endif

bool UUILayoutElement::CheckParentLayout()
{
	if (ParentLayout != nullptr)return true;
	if (auto owner = this->GetOwner())
	{
		if (auto parentActor = owner->GetAttachParentActor())
		{
			ParentLayout = parentActor->FindComponentByClass<UUILayoutBase>();
			if (ParentLayout)
				return true;
		}
	}
	return false;
}

void UUILayoutElement::SetLayoutType(ELayoutElementType InType)
{
	if (InType != LayoutElementType)
	{
		LayoutElementType = InType;
		if (CheckParentLayout())
		{
			ParentLayout->RebuildChildrenList();
			ParentLayout->OnRebuildLayout();
		}
	}
}

void UUILayoutElement::SetConstantSize(float value)
{
	if (ConstantSize != value)
	{
		ConstantSize = value;
		if (CheckParentLayout())
		{
			ParentLayout->RebuildChildrenList();
			ParentLayout->OnRebuildLayout();
		}
	}
}

void UUILayoutElement::SetRatioSize(float value)
{
	if (RatioSize != value)
	{
		RatioSize = value;
		if (CheckParentLayout())
		{
			ParentLayout->RebuildChildrenList();
			ParentLayout->OnRebuildLayout();
		}
	}
}