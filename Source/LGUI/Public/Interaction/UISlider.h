// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Event/Interface/LexPointerDragInterface.h"
#include "UISelectable.h"
#include "Event/LexUIEventDelegate.h"
#include "Event/LexDelegateDeclaration.h"
#include "UISlider.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUISliderValueChangedEvent, float, Value);

class ULexWidget;

UENUM(BlueprintType, Category = LGUI)
enum class EUISliderDirectionType:uint8
{
	LeftToRight,
	RightToLeft,
	BottomToTop,
	TopToBottom,
};

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUISlider : public UUISelectable, public ILexPointerDragInterface
{
	GENERATED_BODY()
	
protected:	
	virtual void Awake() override;
	virtual void Start() override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	virtual void OnDimensionsChanged(bool PivotChanged, bool WidthChanged, bool HeightChanged)override;

	UPROPERTY(EditAnywhere, Category = "LexUI-Slider")
		float Value = 0;
	UPROPERTY(EditAnywhere, Category = "LexUI-Slider")
		float MinValue = 0;
	UPROPERTY(EditAnywhere, Category = "LexUI-Slider")
		float MaxValue = 1;
	/** clamp to integer value */
	UPROPERTY(EditAnywhere, Category = "LexUI-Slider")
		bool WholeNumbers = false;
	/** "Fill" can fill inside it's parent */
	UPROPERTY(EditAnywhere, Category = "LexUI-Slider")
		TWeakObjectPtr<ULexWidget> Fill;
	/** Handle can move inside it's parent */
	UPROPERTY(EditAnywhere, Category = "LexUI-Slider")
		TWeakObjectPtr<ULexWidget> Handle;
	UPROPERTY(EditAnywhere, Category = "LexUI-Slider")
		EUISliderDirectionType DirectionType;
	/** When use navigation input to change the slider value, each press will change value as (MaxValue - MinValue) * NavigationChangeInterval. */
	UPROPERTY(EditAnywhere, Category = "LexUI-Slider", meta=(ClampMin = "0.0", ClampMax = "1.0"))
		float NavigationChangeInterval = 0.1f;

	UPROPERTY(Transient)TWeakObjectPtr<ULexWidget> FillArea;
	UPROPERTY(Transient)TWeakObjectPtr<ULexWidget> HandleArea;

	FLexUIMulticastDelegateFloat OnValueChangedCPP;
	UPROPERTY(BlueprintAssignable, Category = "LexUI-Slider")
	FUISliderValueChangedEvent OnValueChanged;
	UPROPERTY(EditAnywhere, Category = "LexUI-Slider", DisplayName="OnValueChanged")
	FLexUIEventDelegate OnValueChangedED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Double);
	
public:
	FLexUIMulticastDelegateFloat& GetOnValueChangedEvent(){return OnValueChangedCPP;}
	
	UFUNCTION(BlueprintCallable, Category = "LexUI-Slider")
		float GetValue()const { return Value; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Slider")
		float GetMinValue()const { return MinValue; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Slider")
		float GetMaxValue()const { return MaxValue; }

	UFUNCTION(BlueprintCallable, Category = "LexUI-Slider")
		bool GetWholeNumber()const { return WholeNumbers; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Slider")
		ULexWidget* GetFill()const { return Fill.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Slider")
		ULexWidget* GetHandle()const { return Handle.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Slider")
		EUISliderDirectionType GetDirectionType()const { return DirectionType; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Slider")
		float GetNavigationChangeInterval()const { return NavigationChangeInterval; }

	/**
	 * Set Value and send callback event.
	 * NOTE!!! This will send callback event, if you don't want to send callback event, use SetValueWithoutNotify instead.
	 * @param	InValue				New value set for Value
	 */
	UFUNCTION(BlueprintCallable, Category = "LexUI-Slider")
	void SetValue(float InValue);
	/** Set Value and NOT send callback event */
	UFUNCTION(BlueprintCallable, Category = "LexUI-Slider")
	void SetValueWithoutNotify(float InValue);
	/** 
	 * @param	InMinValue			New value set for MinValue
	 * @param	KeepRelativeValue	Keep percentage value, eg: if origin value is 0.25 from 0.0 to 1.0, then it will be 25.0 from 0.0 to 100.0, or be -7.5 from -10.0 to 0.0
	 * @param	FireEvent			Should execute callback event?
	 */
	UFUNCTION(BlueprintCallable, Category = "LexUI-Slider")
	void SetMinValue(float InMinValue, bool KeepRelativeValue, bool FireEvent = true);
	/**
	 * @param	InMaxValue			New value set for MaxValue
	 * @param	KeepRelativeValue	Keep percentage value, eg: if origin value is 0.25 from 0.0 to 1.0, then it will be 25.0 from 0.0 to 100.0, or be -7.5 from -10.0 to 0.0
	 * @param	FireEvent			Should execute callback event?
	 */
	UFUNCTION(BlueprintCallable, Category = "LexUI-Slider")
	void SetMaxValue(float InMaxValue, bool KeepRelativeValue, bool FireEvent = true);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Slider")
	void SetNavigationChangeInterval(float InValue);
	
	virtual bool OnPointerDown_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerUp_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerBeginDrag_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerDrag_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerEndDrag_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnNavigate_Implementation(ELexUINavigationDirection direction, TScriptInterface<ILexNavigationInterface>& result)override;
private:
	bool CheckFill();
	bool CheckHandle();
	void CalculateInputValue(ULexPointerEventData* EventData);
	void SetValue(float InValue, bool FireEvent);
	void ApplyValueToVisual();

};
