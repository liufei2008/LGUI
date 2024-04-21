// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutWithAnimation.h"
#include "UILayoutWithChildren.generated.h"

/**
 * Provide some functions to get layout children (ILayoutElement).
 */
UCLASS(Abstract)
class LGUI_API UUILayoutWithChildren : public UUILayoutWithAnimation
{
	GENERATED_BODY()
public:
	UUILayoutWithChildren();
protected:
	virtual void Awake() override;

	struct FLayoutChild
	{
		TWeakObjectPtr<UUIItem> ChildUIItem;
		TWeakObjectPtr<UObject> LayoutInterface;
		bool operator == (const FLayoutChild& Other)const
		{
			return ChildUIItem.Get() == Other.ChildUIItem.Get();
		}
	};
	mutable uint8 bNeedRebuildChildrenList : 1;
	mutable uint8 bNeedSortChildrenList : 1;
	mutable TArray<FLayoutChild> LayoutUIItemChildrenArray;

	const TArray<FLayoutChild>& GetLayoutUIItemChildren()const;
	void EnsureChildValid();
	void RebuildChildrenList()const;
	void SortChildrenList()const;
	virtual void GetLayoutElement(UUIItem* InChild, UObject*& OutLayoutElement, bool& OutIgnoreLayout)const;

	virtual void OnUIChildAcitveInHierarchy(UUIItem* InChild, bool InUIActive)override;
	virtual void OnUIChildAttachmentChanged(UUIItem* InChild, bool attachOrDetach)override;
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* InChild)override;
public:
	/** Mark layout children changed, so we need RebuildChildrenList when we need to. */
	void MarkNeedRebuildChildrenList();
	virtual void MarkRebuildLayout_Implementation()override;
};
