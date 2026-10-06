// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/UISelectable.h"
#include "Event/Interface/LexPointerClickInterface.h"
#include "Event/LexUIEventDelegate.h"
#include "Event/LexDelegateDeclaration.h"
#include "UIDropdown.generated.h"

class UUIToggle;
class ULexImage;
class ULexWidget;
class ULexText;

/**
 * Dropdown option selection change.
 * @param InSelectIndex Selected item index
 * @param InSelectItem Selected item string
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FUIDropdownComponentDynamicDelegate, int32, InSelectIndex);
/**
 * Called when set data for every dropdown-option-list item.
 * @param InItemIndex Dropdown-option-list item index.
 * @param InItemScript The UIDropdownItemComponent script attached on dropdown-option-list item.
 * @param InItemWidget The dropdown-option-list item actor.
 */
DECLARE_DELEGATE_ThreeParams(FUIDropdownComponentDelegate_SetItemCustomData, int, class UUIDropdownItemComponent*, ULexWidget*);
/**
 * Called when set data for every dropdown-option-list item.
 * @param InItemIndex Dropdown-option-list item index.
 * @param InItemScript The UIDropdownItemComponent script attached on dropdown-option-list item.
 * @param InItemWidget The dropdown-option-list item actor.
 */
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FUIDropdownComponentDynamicDelegate_SetItemCustomData, int, InItemIndex, class UUIDropdownItemComponent*, InItemScript, ULexWidget*, InItemWidget);

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI")
		FText Text;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LexUI")
		FLexUIImageBrush ImageBrush;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUIDropdownValueChangedEvent, int32, Value);

UCLASS( ClassGroup=(LGUI), Blueprintable, meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIDropdown : public UUISelectable, public ILexPointerClickInterface
{
	GENERATED_BODY()

public:	
	UUIDropdown();

protected:
	virtual void Awake()override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
#endif

	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown")
		TWeakObjectPtr<ULexWidget> ListRoot;
	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown")
		TWeakObjectPtr<ULexWidget> Placeholder;
	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown")
		TWeakObjectPtr<ULexText> CaptionText;
	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown")
		TWeakObjectPtr<ULexImage> CaptionImage;
	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown")
		TWeakObjectPtr<UUIDropdownItemComponent> ItemTemplate;
	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown")
		EUIDropdownVerticalPosition VerticalPosition = EUIDropdownVerticalPosition::Automatic;
	/** If list will overlap this button? Only valid if VerticalPosition NOT equal Middle, because Middle mode always overlay. */
	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown", meta = (EditCondition = "VerticalPosition != EUIDropdownVerticalPosition::Middle"))
		bool VerticalOverlap = false;
	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown")
		EUIDropdownHorizontalPosition HorizontalPosition = EUIDropdownHorizontalPosition::Center;
	
	/** Current selected option index. -1 means none selected */
	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown")
		int Value = -1;
	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown")
		TArray<FUIDropdownOptionData> Options;

	/** ListRoot's max height */
	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown", AdvancedDisplay)
		float MaxHeight = 150;
	/** When show the list, create a overlay block to block input on other objects. */
	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown")
		bool bUseInteractionBlock = true;

	bool bIsShow = false;
	bool bNeedRecreate = true;
	TWeakObjectPtr<ULTweener> ShowOrHideTweener;
	TWeakObjectPtr<ULexWidget> BlockerWidget;
	UPROPERTY(Transient) TArray<TWeakObjectPtr<class UUIDropdownItemComponent>> CreatedItemArray;
	virtual bool OnPointerClick_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnDeselect_Implementation(ULexBaseEventData* EventData)override;
	void OnSelectItem(int Index);
	void ApplyValueToVisual();
	virtual void CreateBlocker();
	virtual void CreateListItems();

	FLexUIMulticastDelegateInt32 OnValueChangedCPP;
	UPROPERTY(BlueprintAssignable, Category = "LexUI-Dropdown")
	FUIDropdownValueChangedEvent OnValueChanged;
	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown", DisplayName="OnValueChanged")
	FLexUIEventDelegate OnValueChangedED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Int32);

	/** Bind this delegate and set custom data for option list item. */
	FUIDropdownComponentDelegate_SetItemCustomData OnSetItemCustomDataFunction;
	void SetValue(int InValue, bool FireEvent);
public:
	FLexUIMulticastDelegateInt32& GetOnValueChangedEvent(){return OnValueChangedCPP;}
	
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
		void Show();
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
		void Hide();

	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
		int GetValue()const { return Value; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
		EUIDropdownVerticalPosition GetVerticalPosition()const { return VerticalPosition; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
		EUIDropdownHorizontalPosition GetHorizontalPosition()const { return HorizontalPosition; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
		bool GetVerticalOverlap()const { return VerticalOverlap; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
		const TArray<FUIDropdownOptionData>& GetOptions()const { return Options; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
		FUIDropdownOptionData GetOption(int index)const;
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
		FUIDropdownOptionData GetCurrentOption()const;
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
		float GetMaxHeight()const { return MaxHeight; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
		ULexWidget* GetListRoot()const { return ListRoot.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
		bool GetUseInteractionBlock()const { return bUseInteractionBlock; }

	/**
	 * Set current selected option index (Value) and send callback event.
	 * NOTE!!! This will send callback event, if you don't want to send callback event, use SetValueWithoutNotify instead.
	 */
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
	void SetValue(int InValue);
	/** Set current selected option index (Value) and NOT send callback event */
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
	void SetValueWithoutNotify(int InValue);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
	void SetVerticalPosition(EUIDropdownVerticalPosition InValue);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
	void SetHorizontalPosition(EUIDropdownHorizontalPosition InValue);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
	void SetVerticalOverlap(bool InValue);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
	void SetOptions(const TArray<FUIDropdownOptionData>& InOptions);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
	void AddOptions(const TArray<FUIDropdownOptionData>& InOptions);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
	void SetMaxHeight(float InValue) { MaxHeight = InValue; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
	void SetUseInteractionBlock(bool InValue);

	//list items will be created at next time when show the list
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
	void MarkRecreateList() { bNeedRecreate = true; }
	
	/**
	 * Set custom function to customize option-list item, called when set data for every dropdown-option-list item.
	 */
	void SetItemCustomDataFunction(const FUIDropdownComponentDelegate_SetItemCustomData& InFunction);
	/**
	 * Set custom function to customize option-list item, called when set data for every dropdown-option-list item.
	 */
	void SetItemCustomDataFunction(const TFunction<void(int, class UUIDropdownItemComponent*, ULexWidget*)>& InFunction);
	/**
	 * Set custom function to customize option-list item, called when set data for every dropdown-option-list item.
	 */
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
	void SetItemCustomDataFunction(const FUIDropdownComponentDynamicDelegate_SetItemCustomData& InFunction);
	/**
	 * Clear the function set by "SetItemCustomDataFunction".
	 */
	UFUNCTION(BlueprintCallable, Category = "LexUI-Input")
	void ClearItemCustomDataFunction();
};


DECLARE_DYNAMIC_DELEGATE(FUIDropdownItem_OnSelect);

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIDropdownItemComponent : public ULexUIBehaviour, public ILexPointerClickInterface
{
	GENERATED_BODY()

public:
	UUIDropdownItemComponent();
	virtual void Awake()override;
protected:
	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown")
		TWeakObjectPtr<ULexText> Text;
	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown")
		TWeakObjectPtr<ULexImage> Image;
	UPROPERTY(EditAnywhere, Category = "LexUI-Dropdown")
		TWeakObjectPtr<UUIToggle> Toggle;

private:
	FSimpleDelegate OnSelectCPP;
	UPROPERTY()FUIDropdownItem_OnSelect OnSelectDynamic;
	UFUNCTION()void DynamicDelegate_OnSelect() { OnSelectCPP.ExecuteIfBound(); }
protected:
	/**
	 * Called by UIDropdownComponent when create a item. Use this to initialize.
	 * @param Index Item's index.
	 * @param Data Item's data.
	 * @param OnSelectCallback Callback function that need to be executed by user, when select this item.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Init"), Category = "LexUI-Dropdown")
	void ReceiveInit(int32 Index, const FUIDropdownOptionData& Data, const FUIDropdownItem_OnSelect& OnSelectCallback);
	/**
	 * Set this item's selection state.
	 * When select other item, then need to de-select this one.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "SetSelectionState"), Category = "LexUI-Dropdown")
	void ReceiveSetSelectionState(bool InSelect);
public:
	/**
	 * Called by UIDropdownComponent when create a item. Use this to initialize.
	 * @param Index Item's index.
	 * @param Data Item's data.
	 * @param OnSelectCallback Callback function that need to be executed by user, when select this item.
	 */
	virtual void Init(int32 Index, const FUIDropdownOptionData& Data, const TFunction<void()>& OnSelectCallback);
	/**
	 * Set this item's selection state.
	 * When select other item, then need to de-select this one.
	 */
	virtual void SetSelectionState(const bool& InSelect);
	virtual bool OnPointerClick_Implementation(ULexPointerEventData* EventData)override;

	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
	ULexText* GetText()const { return Text.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
	ULexImage* GetImage()const { return Image.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Dropdown")
	UUIToggle* GetToggle()const;
};