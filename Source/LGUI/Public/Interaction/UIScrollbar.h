// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Event/Interface/LexPointerDragInterface.h"
#include "UISelectable.h"
#include "Event/LexUIEventDelegate.h"
#include "Event/LexDelegateDeclaration.h"
#include "UIScrollbar.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUIScrollbarValueChangedEvent, float, Value);

class ULexWidget;

UENUM(BlueprintType, Category = LGUI)
enum class EUIScrollbarDirectionType:uint8
{
	LeftToRight,
	RightToLeft,
	BottomToTop,
	TopToBottom,
};

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIScrollbar : public UUISelectable, public ILexPointerDragInterface
{
	GENERATED_BODY()
	
public:	
	UUIScrollbar();

	virtual void Awake() override;
	virtual void Start() override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	virtual void OnEnable()override;
	virtual void OnDimensionsChanged(bool PivotChanged, bool WidthChanged, bool HeightChanged)override;

	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float Value = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float Size = 0;
	/** Handle can move inside it's parent */
	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar")
		TWeakObjectPtr<ULexWidget> Handle;
	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar")
		EUIScrollbarDirectionType DirectionType;
	/** When use navigation input to change the scroll value, each press will change value as NavigationChangeInterval. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float NavigationChangeInterval = 0.1f;

	UPROPERTY(Transient)TWeakObjectPtr<ULexWidget> HandleArea;

	FLexUIMulticastDelegateFloat OnValueChangedCPP;
	UPROPERTY(BlueprintAssignable, Category = "LGUI-Scrollbar")
	FUIScrollbarValueChangedEvent OnValueChanged;
	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar", DisplayName="OnValueChangedED")
	FLexUIEventDelegate OnValueChangedED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Double);

	float PressValue = 0;
public:
	FLexUIMulticastDelegateFloat& GetOnValueChangedEvent(){return OnValueChangedCPP;}
	
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		float GetValue()const { return Value; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		float GetSize()const { return Size; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		float GetNavigationChangeInterval()const { return NavigationChangeInterval; }

	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
	void SetValue(float InValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
	void SetValueWithoutNotify(float InValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		void SetSize(float InSize);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		void SetValueAndSize(float InValue, float InSize, bool FireEvent = true);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")
		void SetNavigationChangeInterval(float InValue);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		ULexWidget* GetHandle()const { return Handle.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		EUIScrollbarDirectionType GetDirectionType()const { return DirectionType; }
	
	virtual bool OnPointerDown_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerUp_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerBeginDrag_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerDrag_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerEndDrag_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnNavigate_Implementation(ELexUINavigationDirection direction, TScriptInterface<ILexNavigationInterface>& result)override;
private:
	bool CheckHandle();
	void CalculateInputValue(ULexPointerEventData* EventData);
	void ApplyValueToVisual();
	void SetValue(float InValue, bool FireEvent);
};
