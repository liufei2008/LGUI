// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UIItem.h"
#include "CoreMinimal.h"
#include "Core/UIComponentBase.h"
#include "UILayoutBase.generated.h"

UENUM(BlueprintType)
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

UCLASS(Abstract)
class LGUI_API UUILayoutBase :public UUIComponentBase
{
	GENERATED_BODY()

public:
	UUILayoutBase();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
public:
	void RebuildChildrenList();
	virtual void OnRebuildLayout() {};

	virtual bool CanControlChildAnchor() { return false; };
	virtual bool CanControlChildWidth() { return false; }
	virtual bool CanControlChildHeight() { return false; }
	virtual bool CanControlSelfHorizontalAnchor() { return false; }
	virtual bool CanControlSelfVerticalAnchor() { return false; }
	virtual bool CanControlSelfWidth() { return false; }
	virtual bool CanControlSelfHeight() { return false; }
protected:

	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)override;
	virtual void OnUIAttachmentChanged()override;
	virtual void OnUIActiveInHierachy(bool activeOrInactive)override;
	virtual void OnUIChildAcitveInHierarchy(UUIItem* InChild, bool InUIActive)override;
	virtual void OnUIChildAttachmentChanged(UUIItem* InChild, bool attachOrDetach)override;
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* InChild)override;

	//called when a child is attached, and is valid for layout
	virtual void OnAttachValidChild(UUIItem* InChild) {};
	//called when a valid child is detached
	virtual void OnDetachValidChild(UUIItem* InChild) {};

	class UUILayoutElement* GetLayoutElement(AActor* Target);
	struct FAvaliableChild
	{
		UUIItem* uiItem;
		class UUILayoutElement* layoutElement;
		bool operator == (const FAvaliableChild& Other)const
		{
			return uiItem == Other.uiItem;
		}
	};
	const TArray<FAvaliableChild>& GetAvailableChildren() { return availableChildrenArray; }
private:
	TArray<FAvaliableChild> availableChildrenArray;
};