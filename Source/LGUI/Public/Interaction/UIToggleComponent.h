// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Event/Interface/LGUIPointerClickInterface.h"
#include "UISelectableComponent.h"
#include "Event/LGUIEventDelegate.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "LGUIDelegateHandleWrapper.h"
#include "UIToggleComponent.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIToggleDynamicDelegate, bool, InBool);

UENUM(BlueprintType, Category = LGUI)
enum class UIToggleTransitionType :uint8
{
	None,
	Fade,
	ColorTint,
	/**
	 * You can implement a UISelectableTransitionComponent in c++ or blueprint to do the transition, and add this component to toggle actor.
	 * Use OnStartCustomTransition event in UISelectableTransitionComponent, and switch "On"/"Off" condition to do the transition.
	 */
	TransitionComponent,
};

UCLASS(ClassGroup = LGUI, Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIToggleComponent : public UUISelectableComponent, public ILGUIPointerClickInterface
{
	GENERATED_BODY()
	
protected:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void OnEnable()override;
	virtual void OnDisable()override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	friend class FUIToggleCustomization;
	/** If not assigned, use self. must have UIItem component */
	UPROPERTY(EditAnywhere, Category = "LGUI-Toggle")
		TWeakObjectPtr<class AUIBaseRenderableActor> ToggleActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Toggle")
		UIToggleTransitionType ToggleTransition = UIToggleTransitionType::Fade;
	UPROPERTY(Transient) TWeakObjectPtr<class UUISelectableTransitionComponent> ToggleTransitionComp = nullptr;
	bool CheckTarget();
#pragma region Transition
	UPROPERTY(Transient) TObjectPtr<class ULTweener> ToggleTransitionTweener = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Toggle")
		float OnAlpha = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Toggle")
		float OffAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Toggle")
		FColor OnColor = FColor(255, 255, 255, 255);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Toggle")
		FColor OffColor = FColor(128, 128, 128, 255);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Toggle")
		float ToggleDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Toggle")
		FName OnTransitionName = TEXT("On");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Toggle")
		FName OffTransitionName = TEXT("Off");
#pragma endregion
	UPROPERTY(EditAnywhere, Category = "LGUI-Toggle")
		bool IsOn = true;
	/** Must have UIToggleGroupComponent */
	UPROPERTY(EditAnywhere, Category = "LGUI-Toggle")
		TWeakObjectPtr<AActor> UIToggleGroupActor;
	UPROPERTY(Transient) TWeakObjectPtr<class UUIToggleGroupComponent> GroupComp = nullptr;

	FLGUIMulticastBoolDelegate OnToggleCPP;
	UPROPERTY(EditAnywhere, Category = "LGUI-Toggle")
		FLGUIEventDelegate OnToggle = FLGUIEventDelegate(ELGUIEventDelegateParameterType::Bool);

	void ApplyValueToUI(bool immediateSet);
	virtual bool OnPointerClick_Implementation(ULGUIPointerEventData* eventData)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-Toggle")
		AActor* GetToggleGroupActor()const { return UIToggleGroupActor.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Toggle")
		void SetToggleGroup(UUIToggleGroupComponent* InGroupComp);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Toggle")
		bool GetValue()const { return IsOn; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Toggle")
		virtual void SetValue(bool newValue, bool fireEvent = true);
	/**
	 * If this toggle added to a ToggleGroup, then return index in group. Return -1 if not add to ToggleGroup.
	 * Index is sorted by flatten-hierarchy-index, from RootComponent(UIItem).
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Toggle")
		virtual int32 GetIndexInGroup()const;

	FDelegateHandle RegisterToggleEvent(const FLGUIBoolDelegate& InDelegate);
	FDelegateHandle RegisterToggleEvent(const TFunction<void(bool)>& InFunction);
	void UnregisterToggleEvent(const FDelegateHandle& InHandle);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Toggle")
		FLGUIDelegateHandleWrapper RegisterToggleEvent(const FLGUIToggleDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Toggle")
		void UnregisterToggleEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);
};
