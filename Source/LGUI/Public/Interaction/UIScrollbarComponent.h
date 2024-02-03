// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Event/Interface/LGUIPointerDragInterface.h"
#include "UISelectableComponent.h"
#include "Event/LGUIEventDelegate.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "LGUIDelegateHandleWrapper.h"
#include "UIScrollbarComponent.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIScrollbarDynamicDelegate, float, InFloat);

class AUIBaseActor;
class UUIItem;

UENUM(BlueprintType, Category = LGUI)
enum class UIScrollbarDirectionType:uint8
{
	LeftToRight,
	RightToLeft,
	BottomToTop,
	TopToBottom,
};

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIScrollbarComponent : public UUISelectableComponent, public ILGUIPointerDragInterface
{
	GENERATED_BODY()
	
public:	
	UUIScrollbarComponent();

	virtual void Awake() override;
	virtual void Start() override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	virtual void OnUIActiveInHierachy(bool ativeOrInactive)override;
	virtual void OnUIDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)override;

	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float Value = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float Size = 0;
	/** Handle can move inside it's parent */
	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar")
		TWeakObjectPtr<AUIBaseActor> HandleActor;
	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar")
		UIScrollbarDirectionType DirectionType;
	/** When use navigation input to change the scroll value, each press will change value as NavigationChangeInterval. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float NavigationChangeInterval = 0.1f;

	UPROPERTY(Transient)TWeakObjectPtr<UUIItem> Handle;
	UPROPERTY(Transient)TWeakObjectPtr<UUIItem> HandleArea;

	FLGUIMulticastFloatDelegate OnValueChangeCPP;
	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar")
		FLGUIEventDelegate OnValueChange = FLGUIEventDelegate(ELGUIEventDelegateParameterType::Double);

	float PressValue = 0;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		float GetValue()const { return Value; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		float GetSize()const { return Size; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		float GetNavigationChangeInterval()const { return NavigationChangeInterval; }

	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		void SetValue(float InValue, bool FireEvent = true);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		void SetSize(float InSize);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		void SetValueAndSize(float InValue, float InSize, bool FireEvent = true);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")
		void SetNavigationChangeInterval(float InValue);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		AUIBaseActor* GetHandleActor()const { return HandleActor.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		UIScrollbarDirectionType GetDirectionType()const { return DirectionType; }

	FDelegateHandle RegisterSlideEvent(const FLGUIFloatDelegate& InDelegate);
	FDelegateHandle RegisterSlideEvent(const TFunction<void(float)>& InFunction);
	void UnregisterSlideEvent(const FDelegateHandle& InHandle);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		FLGUIDelegateHandleWrapper RegisterSlideEvent(const FLGUIScrollbarDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		void UnregisterSlideEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);
public:
	virtual bool OnPointerDown_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerUp_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerBeginDrag_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerDrag_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerEndDrag_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnNavigate_Implementation(ELGUINavigationDirection direction, TScriptInterface<ILGUINavigationInterface>& result)override;
private:
	bool CheckHandle();
	void CalculateInputValue(ULGUIPointerEventData* eventData);
	void ApplyValueToUI();

#if WITH_EDITOR
public:
	/** This function is only for update from LGUI2 to LGUI3 */
	void ForUpgrade2to3_ApplyValueToUI() { ApplyValueToUI(); }
#endif
};
