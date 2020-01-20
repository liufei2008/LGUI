// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "Event/LGUIPointerClickInterface.h"
#include "UISelectableComponent.h"
#include "Event/LGUIDrawableEvent.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "Event/LGUIDelegateHandleWrapper.h"
#include "UIToggleComponent.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIToggleDynamicDelegate, bool, InBool);

UENUM(BlueprintType)
enum class UIToggleTransitionType :uint8
{
	None				UMETA(DisplayName = "None"),
	Fade				UMETA(DisplayName = "Fade"),
	ColorTint			UMETA(DisplayName = "ColorTint"),
	//You can implement a UISelectableTransitionComponent in c++ or blueprint to do the transition, and add this component to toggle actor
	TransitionComponent			UMETA(DisplayName = "TransitionComponent"),
};

UCLASS(ClassGroup = LGUI, Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIToggleComponent : public UUISelectableComponent, public ILGUIPointerClickInterface
{
	GENERATED_BODY()
	
public:	
	UUIToggleComponent();

	virtual void BeginPlay() override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	friend class FUIToggleCustomization;
	//If not assigned, use self. must have UIItem component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Toggle")
		class AUIBaseActor* ToggleActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Toggle")
		UIToggleTransitionType ToggleTransition = UIToggleTransitionType::Fade;

	UPROPERTY(Transient) USceneComponent* TargetComp = nullptr;
	bool CheckTarget();
#pragma region Transition
	UPROPERTY(Transient) class ULTweener* ToggleTransitionTweener = nullptr;
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
	//Must have UIToggleGroupComponent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Toggle")
		AActor* UIToggleGroupActor;
	UPROPERTY(Transient) class UUIToggleGroupComponent* GroupComp = nullptr;

	FLGUIMulticastBoolDelegate OnToggleCPP;
	UPROPERTY(EditAnywhere, Category = "LGUI-Toggle")
		FLGUIDrawableEvent OnToggle = FLGUIDrawableEvent(LGUIDrawableEventParameterType::Bool);
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-Toggle")bool GetState() { return IsOn; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Toggle")
		virtual void SetState(bool newState, bool fireEvent = true);
	virtual bool OnPointerClick_Implementation(const FLGUIPointerEventData& eventData)override;

	void RegisterToggleEvent(const FLGUIBoolDelegate& InDelegate);
	void UnregisterToggleEvent(const FLGUIBoolDelegate& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Toggle")
		FLGUIDelegateHandleWrapper RegisterToggleEvent(const FLGUIToggleDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Toggle")
		void UnregisterToggleEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);
};
