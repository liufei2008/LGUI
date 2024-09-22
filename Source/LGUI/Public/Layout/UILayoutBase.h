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
	/**
	 * Rebuild layout immediately
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	virtual void OnRebuildLayout()PURE_VIRTUAL(UUILayoutBase::OnRebuildLayout, );
	// Begin LGUILayout interface
	virtual void OnUpdateLayout_Implementation()override;
	virtual void MarkRebuildLayout_Implementation()override { MarkNeedRebuildLayout(); }
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

	uint8 bNeedRebuildLayout : 1;

	void ApplyUIItemWidth(UUIItem* InUIItem, const float& InWidth);
	void ApplyUIItemHeight(UUIItem* InUIItem, const float& InHeight);
	void ApplyUIItemAnchoredPosition(UUIItem* InUIItem, const FVector2D& InAnchoredPosition);
	void ApplyUIItemSizeDelta(UUIItem* InUIItem, const FVector2D& InSizedDelta);
};