﻿// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "Event/LGUIPointerDragInterface.h"
#include "UISelectableComponent.h"
#include "Event/LGUIDrawableEvent.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "Event/LGUIDelegateHandleWrapper.h"
#include "UISliderComponent.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUISliderDynamicDelegate, float, InFloat);

UENUM()
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
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	virtual void OnUIActiveInHierachy(bool ativeOrInactive)override;
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)override;

	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		float Value = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		float MinValue = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		float MaxValue = 1;
	//clamp to integer value
	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		bool WholeNumbers = false;
	//"Fill" can fill inside it's parent
	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		AUIBaseActor* FillActor;
	//Handle can move inside it's parent
	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		AUIBaseActor* HandleActor;
	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		UISliderDirectionType DirectionType;

	UPROPERTY(Transient)UUIItem* Fill;
	UPROPERTY(Transient)UUIItem* FillArea;
	UPROPERTY(Transient)UUIItem* Handle;
	UPROPERTY(Transient)UUIItem* HandleArea;

	FLGUIMulticastFloatDelegate OnValueChangeCPP;
	UPROPERTY(EditAnywhere, Category = "LGUI-Slider")
		FLGUIDrawableEvent OnValueChange = FLGUIDrawableEvent(LGUIDrawableEventParameterType::Float);
	
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")float GetValue() { return Value; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Slider")
		void SetValue(float InValue, bool FireEvent = true);

	void RegisterSlideEvent(const FLGUIFloatDelegate& InDelegate);
	void UnregisterSlideEvent(const FLGUIFloatDelegate& InDelegate);

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
private:
	bool CheckFill();
	bool CheckHandle();
	void CalculateInputValue(ULGUIPointerEventData* eventData);
	void ApplyValueToUI();
};
