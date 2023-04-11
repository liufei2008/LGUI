// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UIFlexibleGridLayoutElement.h"
#include "Layout/UIFlexibleGridLayout.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"

void UUIFlexibleGridLayoutElement::Awake()
{
	Super::Awake();
	this->SetCanExecuteUpdate(false);
}

void UUIFlexibleGridLayoutElement::OnEnable()
{
	Super::OnEnable();
	if (CheckParentLayout())
	{
		ParentLayout->MarkNeedRebuildChildrenList();
		ParentLayout->MarkNeedRebuildLayout();
	}
}
void UUIFlexibleGridLayoutElement::OnDisable()
{
	Super::OnDisable();
	if (IsValid(ParentLayout))
	{
		ParentLayout->MarkNeedRebuildChildrenList();
		ParentLayout->MarkNeedRebuildLayout();
	}
}

#if WITH_EDITOR
void UUIFlexibleGridLayoutElement::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (CheckParentLayout())
	{
		RowIndex = FMath::Clamp(RowIndex, 0, ParentLayout->GetRowCount() - 1);
		RowCount = FMath::Clamp(RowCount, 1, ParentLayout->GetRowCount() - RowIndex);
		ColumnIndex = FMath::Clamp(ColumnIndex, 0, ParentLayout->GetColumnCount() - 1);
		ColumnCount = FMath::Clamp(ColumnCount, 1, ParentLayout->GetColumnCount() - ColumnIndex);

		ParentLayout->MarkNeedRebuildChildrenList();
		ParentLayout->MarkNeedRebuildLayout();
	}
}
#endif

void UUIFlexibleGridLayoutElement::OnRegister()
{
	Super::OnRegister();
	if (CheckParentLayout())
	{
		ParentLayout->MarkNeedRebuildChildrenList();
		ParentLayout->MarkNeedRebuildLayout();
	}
}
void UUIFlexibleGridLayoutElement::OnUnregister()
{
	Super::OnUnregister();
	if (IsValid(ParentLayout))
	{
		ParentLayout->MarkNeedRebuildChildrenList();
		ParentLayout->MarkNeedRebuildLayout();
	}
}

void UUIFlexibleGridLayoutElement::SetRowIndex(int value)
{
	if (RowIndex != value)
	{
		RowIndex = value;
		if (CheckParentLayout())
		{
			ParentLayout->MarkNeedRebuildLayout();
		}
	}
}
void UUIFlexibleGridLayoutElement::SetRowCount(int value)
{
	if (RowCount != value)
	{
		RowCount = value;
		if (CheckParentLayout())
		{
			ParentLayout->MarkNeedRebuildLayout();
		}
	}
}
void UUIFlexibleGridLayoutElement::SetColumnIndex(int value)
{
	if (ColumnIndex != value)
	{
		ColumnIndex = value;
		if (CheckParentLayout())
		{
			ParentLayout->MarkNeedRebuildLayout();
		}
	}
}
void UUIFlexibleGridLayoutElement::SetColumnCount(int value)
{
	if (ColumnCount != value)
	{
		ColumnCount = value;
		if (CheckParentLayout())
		{
			ParentLayout->MarkNeedRebuildLayout();
		}
	}
}
void UUIFlexibleGridLayoutElement::SetIgnoreLayout(bool value)
{
	if (bIgnoreLayout != value)
	{
		bIgnoreLayout = value;
		if (CheckParentLayout())
		{
			ParentLayout->MarkNeedRebuildChildrenList();
			ParentLayout->MarkNeedRebuildLayout();
		}
	}
}

bool UUIFlexibleGridLayoutElement::CheckParentLayout()
{
	if (IsValid(ParentLayout))return true;
	if (auto owner = this->GetOwner())
	{
		if (auto parentActor = owner->GetAttachParentActor())
		{
			ParentLayout = parentActor->FindComponentByClass<UUIFlexibleGridLayout>();
			if (IsValid(ParentLayout))
				return true;
		}
	}
	return false;
}
