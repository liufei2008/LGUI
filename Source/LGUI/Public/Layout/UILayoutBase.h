// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UIItem.h"
#include "CoreMinimal.h"
#include "Core/LGUILifeCycleUIBehaviour.h"
#include "Layout/ILGUILayoutInterface.h"
#include "Layout/ILGUILayoutElementInterface.h"
#include "UILayoutBase.generated.h"

UENUM(BlueprintType, Category = LGUI)
enum class ELGUILayoutAlignmentType :uint8
{
	UpperLeft,
	UpperCenter,
	UpperRight,
	MiddleLeft,
	MiddleCenter,
	MiddleRight,
	LowerLeft,
	LowerCenter,
	LowerRight,
};

class UUILayoutElement;

UCLASS(Abstract)
class LGUI_API UUILayoutBase :public ULGUILifeCycleUIBehaviour, public ILGUILayoutInterface
{
	GENERATED_BODY()
public:
	UUILayoutBase();
protected:
	virtual void Awake() override;
	virtual void OnEnable()override;
	virtual void OnDisable()override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo()override;
#endif
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
public:
	/** Mark layout children changed, so we need RebuildChildrenList when we need to. */
	void MarkNeedRebuildChildrenList();
	/**
	 * Rebuild layout immediately
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	virtual void OnRebuildLayout()PURE_VIRTUAL(UUILayoutBase::OnRebuildLayout, );
	// Begin LGUILayout interface
	virtual void OnUpdateLayout_Implementation()override;
	virtual void MarkRebuildLayout_Implementation()override { MarkNeedRebuildLayout(); MarkNeedRebuildChildrenList(); }
	// End LGUILayout interface

	/**
	 * Mark this layout need to be rebuild. Same as MarkRebuildLayout(Interface call).
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void MarkNeedRebuildLayout();

protected:

	virtual void OnUIDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)override;
	virtual void OnUIAttachmentChanged()override;
	virtual void OnUIActiveInHierachy(bool activeOrInactive)override;
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged);
	virtual void OnUIChildAcitveInHierarchy(UUIItem* InChild, bool InUIActive)override;
	virtual void OnUIChildAttachmentChanged(UUIItem* InChild, bool attachOrDetach)override;
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* InChild)override;

	struct FAvaliableChild
	{
		TWeakObjectPtr<UUIItem> uiItem;
		TWeakObjectPtr<UActorComponent> layoutElement;
		bool operator == (const FAvaliableChild& Other)const
		{
			return uiItem.Get() == Other.uiItem.Get();
		}
	};
	const TArray<FAvaliableChild>& GetLayoutUIItemChildren()const;
	void EnsureChildValid();
	void RebuildChildrenList()const;
	void SortChildrenList()const;
	virtual void GetLayoutElement(AActor* InActor, UActorComponent*& OutLayoutElement, bool& OutIgnoreLayout)const;

	uint8 bNeedRebuildLayout : 1;
	mutable uint8 bNeedRebuildChildrenList : 1;
	mutable uint8 bNeedSortChildrenList : 1;

	void ApplyUIItemWidth(UUIItem* InUIItem, const float& InWidth);
	void ApplyUIItemHeight(UUIItem* InUIItem, const float& InHeight);
	void ApplyUIItemAnchoredPosition(UUIItem* InUIItem, const FVector2D& InAnchoredPosition);
	void ApplyUIItemSizeDelta(UUIItem* InUIItem, const FVector2D& InSizedDelta);
private:
	mutable TArray<FAvaliableChild> LayoutUIItemChildrenArray;
};