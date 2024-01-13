// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UILayoutElement.h"
#include "LGUI.h"
#include "Layout/UILayoutWithChildren.h"
#include "Core/ActorComponent/UIItem.h"

void UUILayoutElement::Awake()
{
	Super::Awake();
	this->SetCanExecuteUpdate(false);
}

void UUILayoutElement::OnEnable()
{
	Super::OnEnable();
	if (CheckParentLayout())
	{
		ParentLayout->MarkNeedRebuildChildrenList();
		ParentLayout->MarkNeedRebuildLayout();
	}
}
void UUILayoutElement::OnDisable()
{
	Super::OnDisable();
	if (IsValid(ParentLayout))
	{
		ParentLayout->MarkNeedRebuildChildrenList();
		ParentLayout->MarkNeedRebuildLayout();
	}
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
void UUILayoutElement::OnRegister()
{
	Super::OnRegister();
	if (CheckParentLayout())
	{
		ParentLayout->MarkNeedRebuildChildrenList();
		ParentLayout->MarkNeedRebuildLayout();
	}
}
void UUILayoutElement::OnUnregister()
{
	Super::OnUnregister();
	if (IsValid(ParentLayout))
	{
		ParentLayout->MarkNeedRebuildChildrenList();
		ParentLayout->MarkNeedRebuildLayout();
	}
}

bool UUILayoutElement::CheckParentLayout()
{
	if (IsValid(ParentLayout))return true;
	if (auto owner = this->GetOwner())
	{
		if (auto parentActor = owner->GetAttachParentActor())
		{
			ParentLayout = parentActor->FindComponentByClass<UUILayoutWithChildren>();
			if (IsValid(ParentLayout))
				return true;
		}
	}
	return false;
}

void UUILayoutElement::OnUIAttachmentChanged()
{
	ParentLayout = nullptr;
}

ELayoutElementType UUILayoutElement::GetLayoutType_Implementation()const { return LayoutElementType; }
float UUILayoutElement::GetConstantSize_Implementation(ELayoutElementSizeType type)const
{
	switch (ConstantSizeType)
	{
	case EUILayoutElement_ConstantSizeType::UseUIItemSize:
	{
		if (auto UIItem = GetRootUIComponent())
		{
			return type == ELayoutElementSizeType::Horizontal
				? UIItem->GetWidth()
				: UIItem->GetHeight();
				;
		}
		return ConstantSize;
	}
		break;
	default:
	case EUILayoutElement_ConstantSizeType::UseCustomSize:
		return ConstantSize;
		break;
	}
}
float UUILayoutElement::GetRatioSize_Implementation(ELayoutElementSizeType type)const
{
	return RatioSize; 
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
			ParentLayout->MarkNeedRebuildLayout();
		}
	}
}

void UUILayoutElement::SetConstantSizeType(EUILayoutElement_ConstantSizeType value)
{
	if (ConstantSizeType != value)
	{
		ConstantSizeType = value;
		if (CheckParentLayout())
		{
			ParentLayout->MarkNeedRebuildLayout();
		}
	}
}
