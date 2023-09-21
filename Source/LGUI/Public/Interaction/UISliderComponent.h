// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Event/Interface/LGUIPointerDragInterface.h"
#include "UISelectableComponent.h"
#include "Event/LGUIEventDelegate.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "Event/LGUIDelegateHandleWrapper.h"
#include "UISliderComponent.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUISliderDynamicDelegate, float, InFloat);

class AUIBaseActor;
class UUIItem;

UENUM(BlueprintType, Category = LGUI)
enum class UISliderDirectionType:uint8
{
	LeftToRight,
	RightToLeft,
	BottomToTop,
	TopToBottom,
};

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUISliderComponent : public UUISelectableComponent, public ILGUIPointerDragInterface
{
	GENERATED_BODY()
	
protected:	
	virtual void Awake() override;
	virtual void Start() override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	virtual void OnUIActiveInHierachy(bool ativeOrInactive)override;
	virtual void OnUIDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)override;

	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		float Value = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		float MinValue = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		float MaxValue = 1;
	/** clamp to integer value */
	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		bool WholeNumbers = false;
	/** "Fill" can fill inside it's parent */
	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		TWeakObjectPtr<AUIBaseActor> FillActor;
	/** Handle can move inside it's parent */
	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		TWeakObjectPtr<AUIBaseActor> HandleActor;
	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		UISliderDirectionType DirectionType;

	UPROPERTY(Transient)TWeakObjectPtr<UUIItem> Fill;
	UPROPERTY(Transient)TWeakObjectPtr<UUIItem> FillArea;
	UPROPERTY(Transient)TWeakObjectPtr<UUIItem> Handle;
	UPROPERTY(Transient)TWeakObjectPtr<UUIItem> HandleArea;

	FLGUIMulticastFloatDelegate OnValueChangeCPP;
	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		FLGUIEventDelegate OnValueChange = FLGUIEventDelegate(ELGUIEventDelegateParameterType::Double);
	
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")
		float GetValue()const { return Value; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")
		float GetMinValue()const { return MinValue; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")
		float GetMaxValue()const { return MaxValue; }

	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")
		bool GetWholeNumber()const { return WholeNumbers; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")
		AUIBaseActor* GetFillActor()const { return FillActor.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")
		AUIBaseActor* GetHandleActor()const { return HandleActor.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")
		UISliderDirectionType GetDirectionType()const { return DirectionType; }

	/**
	 * @param	InValue				New value set for Value
	 * @param	FireEvent			Should execute callback event?
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")
		void SetValue(float InValue, bool FireEvent = true);
	/** 
	 * @param	InMinValue			New value set for MinValue
	 * @param	KeepRelativeValue	Keep percentage value, eg: if origin value is 0.25 from 0.0 to 1.0, then it will be 25.0 from 0.0 to 100.0, or be -7.5 from -10.0 to 0.0
	 * @param	FireEvent			Should execute callback event?
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")
		void SetMinValue(float InMinValue, bool KeepRelativeValue, bool FireEvent = true);
	/**
	 * @param	InMaxValue			New value set for MaxValue
	 * @param	KeepRelativeValue	Keep percentage value, eg: if origin value is 0.25 from 0.0 to 1.0, then it will be 25.0 from 0.0 to 100.0, or be -7.5 from -10.0 to 0.0
	 * @param	FireEvent			Should execute callback event?
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")
		void SetMaxValue(float InMaxValue, bool KeepRelativeValue, bool FireEvent = true);

	FDelegateHandle RegisterSlideEvent(const FLGUIFloatDelegate& InDelegate);
	FDelegateHandle RegisterSlideEvent(const TFunction<void(float)>& InFunction);
	void UnregisterSlideEvent(const FDelegateHandle& InHandle);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")
		FLGUIDelegateHandleWrapper RegisterSlideEvent(const FLGUISliderDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")
		void UnregisterSlideEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);
public:
	virtual bool OnPointerDown_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerUp_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerBeginDrag_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerDrag_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerEndDrag_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnNavigate_Implementation(ELGUINavigationDirection direction, TScriptInterface<ILGUINavigationInterface>& result)override;
private:
	bool CheckFill();
	bool CheckHandle();
	void CalculateInputValue(ULGUIPointerEventData* eventData);
	void ApplyValueToUI();
#if WITH_EDITOR
public:
	/** This function is only for update from LGUI2 to LGUI3 */
	void ForUpgrade2to3_ApplyValueToUI() { ApplyValueToUI(); }
#endif
};
