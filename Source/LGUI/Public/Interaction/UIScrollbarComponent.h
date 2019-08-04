﻿// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Event/LGUIPointerDragInterface.h"
#include "UISelectableComponent.h"
#include "Event/LGUIDrawableEvent.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "Event/LGUIDelegateHandleWrapper.h"
#include "UIScrollbarComponent.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIScrollbarDynamicDelegate, float, InFloat);

UENUM(BlueprintType)
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

	virtual void BeginPlay() override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	virtual void OnUIActiveInHierachy(bool ativeOrInactive)override;
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)override;

	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float Value = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float Size = 0;
	//Handle can move inside it's parent
	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar")
		AUIBaseActor* HandleActor;
	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar")
		UIScrollbarDirectionType DirectionType;

	UPROPERTY(BlueprintReadWrite, Category = "LGUI-Scrollbar")
		UUIItem* Handle;
	UPROPERTY(BlueprintReadWrite, Category = "LGUI-Scrollbar")
		UUIItem* HandleArea;

	FLGUIMulticastFloatDelegate OnValueChangeCPP;
	UPROPERTY(EditAnywhere, Category = "LGUI-Scrollbar")
		FLGUIDrawableEvent OnValueChange = FLGUIDrawableEvent(LGUIDrawableEventParameterType::Float);

	float PressValue;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		float GetValue()const { return Value; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		float GetSize()const { return Size; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		void SetValue(float InValue, bool FireEvent = true);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		void SetSize(float InSize);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		void SetValueAndSize(float InValue, float InSize, bool FireEvent = true);

	void RegisterSlideEvent(const FLGUIFloatDelegate& InDelegate);
	void UnregisterSlideEvent(const FLGUIFloatDelegate& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		FLGUIDelegateHandleWrapper RegisterSlideEvent(const FLGUIScrollbarDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Scrollbar")
		void UnregisterSlideEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);
public:
	virtual bool OnPointerDown_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerUp_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerBeginDrag_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerDrag_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerEndDrag_Implementation(const FLGUIPointerEventData& eventData)override;
private:
	bool CheckHandle();
	void CalculateInputValue(const FLGUIPointerEventData& eventData);
	void ApplyValueToUI();
};
