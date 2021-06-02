// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UIItem.h"
#include "CoreMinimal.h"
#include "Core/LGUIBehaviour.h"
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

UENUM(BlueprintType, Category = LGUI)
enum class EUILayoutChangePositionAnimationType :uint8
{
	/** Immediately change position */
	Immediately,
	/** Change position with ease animation */
	EaseAnimation,
	/** Register custom event and do the transition */
	//Custom,
};

class UUILayoutElement;

UCLASS(Abstract)
class LGUI_API UUILayoutBase :public ULGUIBehaviour
{
	GENERATED_BODY()

protected:
	virtual void Awake() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
public:
	void RebuildChildrenList();
	/**
	 * Rebuild layout immediately
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	virtual void OnRebuildLayout()PURE_VIRTUAL(UUILayoutBase::OnRebuildLayout, );
	/**
	 * Called by LGUIManager. Will check "bNeedRebuildLayout" then decide if we rebuild
	 */
	void ConditionalRebuildLayout();
	/**
	 * Mark this layout need to be rebuild, will do rebuild after all LGUIBehaviour's Update function.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void MarkNeedRebuildLayout() { bNeedRebuildLayout = true; }
#if WITH_EDITOR
	virtual bool CanControlChildAnchor() { return false; };
	virtual bool CanControlChildAnchorOffsetX() { return false; }
	virtual bool CanControlChildAnchorOffsetY() { return false; }
	virtual bool CanControlChildWidth() { return false; }
	virtual bool CanControlChildHeight() { return false; }
	virtual bool CanControlSelfHorizontalAnchor() { return false; }
	virtual bool CanControlSelfVerticalAnchor() { return false; }
	virtual bool CanControlSelfAnchorOffsetX() { return false; }
	virtual bool CanControlSelfAnchorOffsetY() { return false; }
	virtual bool CanControlSelfWidth() { return false; }
	virtual bool CanControlSelfHeight() { return false; }
	virtual bool CanControlSelfStrengthLeft() { return false; }
	virtual bool CanControlSelfStrengthRight() { return false; }
	virtual bool CanControlSelfStrengthTop() { return false; }
	virtual bool CanControlSelfStrengthBottom() { return false; }
#endif
protected:

	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)override;
	virtual void OnUIAttachmentChanged()override;
	virtual void OnUIActiveInHierachy(bool activeOrInactive)override;
	virtual void OnUIChildAcitveInHierarchy(UUIItem* InChild, bool InUIActive)override;
	virtual void OnUIChildAttachmentChanged(UUIItem* InChild, bool attachOrDetach)override;
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* InChild)override;

	/** called when a child is attached, and is valid for layout */
	virtual void OnAttachValidChild(UUIItem* InChild) {};
	/** called when a valid child is detached */
	virtual void OnDetachValidChild(UUIItem* InChild) {};

	UUILayoutElement* GetLayoutElement(AActor* Target);
	struct FAvaliableChild
	{
		TWeakObjectPtr<UUIItem> uiItem;
		TWeakObjectPtr<UUILayoutElement> layoutElement;
		bool operator == (const FAvaliableChild& Other)const
		{
			return uiItem.Get() == Other.uiItem.Get();
		}
	};
	const TArray<FAvaliableChild>& GetAvailableChildren() { return availableChildrenArray; }

	bool bNeedRebuildLayout = false;
private:
	TArray<FAvaliableChild> availableChildrenArray;
};