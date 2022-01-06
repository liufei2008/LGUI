// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/UISelectableComponent.h"
#include "Event/Interface/LGUIPointerSelectDeselectInterface.h"
#include "Event/Interface/LGUIPointerClickInterface.h"
#include "LGUIComponentReference.h"
#include "Event/LGUIEventDelegate.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "Core/ActorComponent/UICanvasGroup.h"
#include "UIDropdownComponent.generated.h"

class ULGUISpriteData_BaseObject;

/**
 * @param InSelectIndex Selected item index
 * @param InSelectItem Selected item string
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FUIDropdownComponentDynamicDelegate, int32, InSelectIndex);

UENUM(BlueprintType, Category = LGUI)
enum class EUIDropdownVerticalPosition : uint8
{
	Bottom,
	Middle,
	Top,
	//automatically choose bottom or top
	Automatic,
};
UENUM(BlueprintType, Category = LGUI)
enum class EUIDropdownHorizontalPosition : uint8
{
	Left,
	Center,
	Right,
	//automatically choose left or right
	Automatic,
};

USTRUCT(BlueprintType)
struct FUIDropdownOptionData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FText Text;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", meta = (DisplayThumbnail = false))
		ULGUISpriteData_BaseObject* Sprite;
};

UCLASS( ClassGroup=(LGUI), Blueprintable, meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIDropdownComponent : public UUISelectableComponent, public ILGUIPointerClickInterface
{
	GENERATED_BODY()

public:	
	UUIDropdownComponent();

protected:
	virtual void Awake()override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
#endif

	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		TWeakObjectPtr<class AUIBaseActor> ListRoot;
	TWeakObjectPtr<class UUICanvasGroup> CanvasGroupOnListRoot;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		TWeakObjectPtr<class AUITextActor> CaptionText;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		TWeakObjectPtr<class AUISpriteActor> CaptionSprite;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		FLGUIComponentReference ItemTemplate;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		EUIDropdownVerticalPosition VerticalPosition = EUIDropdownVerticalPosition::Automatic;
	/** If list will overlap this button? Only valid if VerticalPosition NOT equal Middle, because Middle mode always overlay. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown", meta = (EditCondition = "VerticalPosition != EUIDropdownVerticalPosition::Middle"))
		bool VerticalOverlap = false;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		EUIDropdownHorizontalPosition HorizontalPosition = EUIDropdownHorizontalPosition::Center;
	
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		int Value;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		TArray<FUIDropdownOptionData> Options;

	bool IsShow = false;
	bool NeedRecreate = true;
	float MaxHeight = 150;
	TWeakObjectPtr<ULTweener> ShowOrHideTweener;
	TWeakObjectPtr<class AUIContainerActor> BlockerActor;
	UPROPERTY(Transient) TArray<TWeakObjectPtr<class UUIDropdownItemComponent>> CreatedItemArray;
	virtual bool OnPointerClick_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerDeselect_Implementation(ULGUIBaseEventData* eventData)override;
	void OnSelectItem(int index);
	void ApplyValueToUI();
	virtual void CreateBlocker();
	virtual void CreateListItems();

	FLGUIMulticastInt32Delegate OnSelectionChangeCPP;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		FLGUIEventDelegate OnSelectionChange = FLGUIEventDelegate(LGUIEventDelegateParameterType::Int32);
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		void Show();
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		void Hide();

	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		int GetValue()const { return Value; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		EUIDropdownVerticalPosition GetVerticalPosition()const { return VerticalPosition; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		EUIDropdownHorizontalPosition GetHorizontalPosition()const { return HorizontalPosition; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		bool GetVerticalOverlap()const { return VerticalOverlap; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		const TArray<FUIDropdownOptionData>& GetOptions()const { return Options; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		FUIDropdownOptionData GetOption(int index)const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		FUIDropdownOptionData GetCurrentOption()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		float GetMaxHeight()const { return MaxHeight; }

	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		void SetValue(int newValue, bool fireEvent = true);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		void SetVerticalPosition(EUIDropdownVerticalPosition InValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		void SetHorizontalPosition(EUIDropdownHorizontalPosition InValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		void SetVerticalOverlap(bool newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		void SetOptions(const TArray<FUIDropdownOptionData>& InOptions);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		void AddOptions(const TArray<FUIDropdownOptionData>& InOptions);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		void SetMaxHeight(float newValue) { MaxHeight = newValue; }

	//list items will be created at next time when show the list
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		void MarkRecreateList() { NeedRecreate = true; }

	FDelegateHandle RegisterSelectionChangeEvent(const FLGUIInt32Delegate& InDelegate);
	FDelegateHandle RegisterSelectionChangeEvent(const TFunction<void(int)>& InFunction);
	void UnregisterSelectionChangeEvent(const FDelegateHandle& InHandle);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		FLGUIDelegateHandleWrapper RegisterSelectionChangeEvent(const FUIDropdownComponentDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		void UnregisterSelectionChangeEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);
};



UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIDropdownItemComponent : public UActorComponent, public ILGUIPointerClickInterface
{
	GENERATED_BODY()

public:
	UUIDropdownItemComponent();

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		TWeakObjectPtr<class AUITextActor> TextActor;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		TWeakObjectPtr<class AUISpriteActor> SpriteActor;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		FLGUIComponentReference Toggle;
	int32 Index;
public:
	void Init(const FUIDropdownOptionData& Data, const TFunction<void()>& OnSelect);
	void SetSelectionState(const bool& InSelect);
	virtual bool OnPointerClick_Implementation(ULGUIPointerEventData* eventData)override;
};