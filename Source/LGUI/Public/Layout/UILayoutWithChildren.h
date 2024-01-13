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

	struct FAvaliableChild
	{
		TWeakObjectPtr<UUIItem> uiItem;
		TWeakObjectPtr<UActorComponent> layoutElement;
		bool operator == (const FAvaliableChild& Other)const
		{
			return uiItem.Get() == Other.uiItem.Get();
		}
	};
	mutable uint8 bNeedRebuildChildrenList : 1;
	mutable uint8 bNeedSortChildrenList : 1;
	mutable TArray<FAvaliableChild> LayoutUIItemChildrenArray;

	const TArray<FAvaliableChild>& GetLayoutUIItemChildren()const;
	void EnsureChildValid();
	void RebuildChildrenList()const;
	void SortChildrenList()const;
	virtual void GetLayoutElement(AActor* InActor, UActorComponent*& OutLayoutElement, bool& OutIgnoreLayout)const;

	virtual void OnUIChildAcitveInHierarchy(UUIItem* InChild, bool InUIActive)override;
	virtual void OnUIChildAttachmentChanged(UUIItem* InChild, bool attachOrDetach)override;
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* InChild)override;
public:
	/** Mark layout children changed, so we need RebuildChildrenList when we need to. */
	void MarkNeedRebuildChildrenList();
	virtual void MarkRebuildLayout_Implementation()override;
};
