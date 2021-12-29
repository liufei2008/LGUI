// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutBase.h"
#include "Core/LGUILifeCycleUIBehaviour.h"
#include "UISizeControlByOther.generated.h"

//this is only a helper component for UISizeControlByOther, no need to be serialized
UCLASS(Transient)
class LGUI_API UUISizeControlByOtherHelper :public ULGUILifeCycleUIBehaviour
{
	GENERATED_BODY()

private:
	virtual void Awake()override;
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)override;
	friend class UUISizeControlByOther;
	UPROPERTY(Transient)TWeakObjectPtr<class UUISizeControlByOther> TargetComp;
};

/**
 * Use other UI element to control the size of this one.
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUISizeControlByOther : public UUILayoutBase
{
	GENERATED_BODY()

public:	
	virtual void Awake()override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		class AUIBaseActor* GetTargetActor()const { return TargetActor.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetControlWidth()const { return ControlWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAdditionalWidth()const { return AdditionalWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetControlHeight()const { return ControlHeight; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAdditionalHeight()const { return AdditionalHeight; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetControlWidth(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAdditionalWidth(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetControlHeight(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAdditionalHeight(float value);

	virtual bool CanControlChildAnchor_Implementation()const override;
	virtual bool CanControlChildHorizontalAnchoredPosition_Implementation()const override;
	virtual bool CanControlChildVerticalAnchoredPosition_Implementation()const override;
	virtual bool CanControlChildWidth_Implementation()const override;
	virtual bool CanControlChildHeight_Implementation()const override;
	virtual bool CanControlChildAnchorLeft_Implementation()const override;
	virtual bool CanControlChildAnchorRight_Implementation()const override;
	virtual bool CanControlChildAnchorBottom_Implementation()const override;
	virtual bool CanControlChildAnchorTop_Implementation()const override;

	virtual bool CanControlSelfAnchor_Implementation()const override;
	virtual bool CanControlSelfHorizontalAnchoredPosition_Implementation()const override;
	virtual bool CanControlSelfVerticalAnchoredPosition_Implementation()const override;
	virtual bool CanControlSelfWidth_Implementation()const override;
	virtual bool CanControlSelfHeight_Implementation()const override;
	virtual bool CanControlSelfAnchorLeft_Implementation()const override;
	virtual bool CanControlSelfAnchorRight_Implementation()const override;
	virtual bool CanControlSelfAnchorBottom_Implementation()const override;
	virtual bool CanControlSelfAnchorTop_Implementation()const override;
protected:
	friend class UUISizeControlByOtherHelper;
	virtual void OnRebuildLayout()override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
#endif

	//Target object's size control this object's size
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TWeakObjectPtr<class AUIBaseActor> TargetActor;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ControlWidth = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float AdditionalWidth = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ControlHeight = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float AdditionalHeight = 0;

	UPROPERTY(Transient) TWeakObjectPtr<class UUIItem> TargetUIItem = nullptr;
	UPROPERTY(Transient) TWeakObjectPtr<class UUISizeControlByOtherHelper> HelperComp = nullptr;
	bool CheckTargetUIItem();

	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)override;
	virtual void OnUIAttachmentChanged()override;

	//these will not affect this Layout
	virtual void OnUIChildAcitveInHierarchy(UUIItem* InChild, bool InUIActive)override {}
	virtual void OnUIChildAttachmentChanged(UUIItem* InChild, bool attachOrDetach)override {}
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* InChild)override {}
};
