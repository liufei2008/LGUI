// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutWithAnimation.h"
#include "Core/LGUILifeCycleUIBehaviour.h"
#include "UISizeControlByOther.generated.h"

//this is only a helper component for UISizeControlByOther, no need to be serialized
UCLASS(Transient)
class LGUI_API UUISizeControlByOtherHelper :public ULGUILifeCycleUIBehaviour
{
	GENERATED_BODY()

private:
	virtual void Awake()override;
	virtual void OnUIDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)override;
	friend class UUISizeControlByOther;
	UPROPERTY(Transient)TWeakObjectPtr<class UUISizeControlByOther> TargetComp;
};

/**
 * CAUTION!!! This layout could result in loop size reference! Use it carefully!
 * Use other UI element to control the size of this one.
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUISizeControlByOther : public UUILayoutWithAnimation
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

	//Set TargetActor which will control size of this UI
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetTargetActor(class AUIBaseActor* value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetControlWidth(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAdditionalWidth(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetControlHeight(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAdditionalHeight(float value);

	virtual bool GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const override;
protected:
	friend class UUISizeControlByOtherHelper;
	virtual void OnRebuildLayout()override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
#endif

	//Target object's will control this UI's size
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

	virtual void OnUIDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)override;
	virtual void OnUIAttachmentChanged()override;

	//these will not affect this Layout
	virtual void OnUIChildAcitveInHierarchy(UUIItem* InChild, bool InUIActive)override {}
	virtual void OnUIChildAttachmentChanged(UUIItem* InChild, bool attachOrDetach)override {}
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* InChild)override {}
};
