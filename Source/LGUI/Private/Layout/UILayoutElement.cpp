// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Layout/UILayoutElement.h"
#include "LGUI.h"
#include "Layout/UILayoutBase.h"
#include "Core/ActorComponent/UIItem.h"

void UUILayoutElement::Awake()
{
	Super::Awake();
	if (CheckParentLayout())
	{
		ParentLayout->MarkNeedRebuildChildrenList();
		ParentLayout->MarkNeedRebuildLayout();
	}
	this->SetCanExecuteUpdate(false);
}

#if WITH_EDITOR
void UUILayoutElement::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	ParentLayout = nullptr;
	if (CheckParentLayout())
	{
		ParentLayout->MarkNeedRebuildChildrenList();
		ParentLayout->MarkNeedRebuildLayout();
	}
	if (auto owner = this->GetOwner())
	{
		if (auto uiItem = owner->FindComponentByClass<UUIItem>())
		{
			uiItem->EditorForceUpdate();
		}
	}
}
#endif

bool UUILayoutElement::CheckParentLayout()
{
	if (IsValid(ParentLayout))return true;
	if (auto owner = this->GetOwner())
	{
		if (auto parentActor = owner->GetAttachParentActor())
		{
			ParentLayout = parentActor->FindComponentByClass<UUILayoutBase>();
			if (IsValid(ParentLayout))
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
			ParentLayout->MarkNeedRebuildChildrenList();
			ParentLayout->MarkNeedRebuildLayout();
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
			ParentLayout->MarkNeedRebuildChildrenList();
			ParentLayout->MarkNeedRebuildLayout();
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
			ParentLayout->MarkNeedRebuildChildrenList();
			ParentLayout->MarkNeedRebuildLayout();
		}
	}
}