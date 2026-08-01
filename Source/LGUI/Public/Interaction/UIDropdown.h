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
class ULexWidgetContainer;
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FText Text;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
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

	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		TWeakObjectPtr<ULexWidget> ListRoot;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		TWeakObjectPtr<ULexText> CaptionText;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		TWeakObjectPtr<ULexImage> CaptionImage;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		TWeakObjectPtr<UUIDropdownItemComponent> ItemTemplate;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		EUIDropdownVerticalPosition VerticalPosition = EUIDropdownVerticalPosition::Automatic;
	/** If list will overlap this button? Only valid if VerticalPosition NOT equal Middle, because Middle mode always overlay. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown", meta = (EditCondition = "VerticalPosition != EUIDropdownVerticalPosition::Middle"))
		bool VerticalOverlap = false;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		EUIDropdownHorizontalPosition HorizontalPosition = EUIDropdownHorizontalPosition::Center;
	
	/** Current selected option index. -1 means none selected */
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		int Value = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		TArray<FUIDropdownOptionData> Options;

	/** ListRoot's max height */
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown", AdvancedDisplay)
		float MaxHeight = 150;
	/** When show the list, create a overlay block to block input on other objects. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
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
	UPROPERTY(BlueprintAssignable, Category = "LGUI-Dropdown")
	FUIDropdownValueChangedEvent OnValueChanged;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown", DisplayName="OnValueChangedED")
	FLexUIEventDelegate OnValueChangedED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Int32);

	/** Bind this delegate and set custom data for option list item. */
	FUIDropdownComponentDelegate_SetItemCustomData OnSetItemCustomDataFunction;
	void SetValue(int InValue, bool FireEvent);
public:
	FLexUIMulticastDelegateInt32& GetOnValueChangedEvent(){return OnValueChangedCPP;}
	
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
		ULexWidget* GetListRoot()const { return ListRoot.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
		bool GetUseInteractionBlock()const { return bUseInteractionBlock; }

	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
	void SetValue(int InValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
	void SetValueWithoutNotify(int InValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
	void SetVerticalPosition(EUIDropdownVerticalPosition InValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
	void SetHorizontalPosition(EUIDropdownHorizontalPosition InValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
	void SetVerticalOverlap(bool InValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
	void SetOptions(const TArray<FUIDropdownOptionData>& InOptions);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
	void AddOptions(const TArray<FUIDropdownOptionData>& InOptions);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
	void SetMaxHeight(float InValue) { MaxHeight = InValue; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
	void SetUseInteractionBlock(bool InValue);

	//list items will be created at next time when show the list
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
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
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
	void SetItemCustomDataFunction(const FUIDropdownComponentDynamicDelegate_SetItemCustomData& InFunction);
	/**
	 * Clear the function set by "SetItemCustomDataFunction".
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
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
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		TWeakObjectPtr<ULexText> Text;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
		TWeakObjectPtr<ULexImage> Image;
	UPROPERTY(EditAnywhere, Category = "LGUI-Dropdown")
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
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Init"), Category = "LGUI-Dropdown")
	void ReceiveInit(int32 Index, const FUIDropdownOptionData& Data, const FUIDropdownItem_OnSelect& OnSelectCallback);
	/**
	 * Set this item's selection state.
	 * When select other item, then need to de-select this one.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "SetSelectionState"), Category = "LGUI-Dropdown")
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

	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
	ULexText* GetText()const { return Text.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
	ULexImage* GetImage()const { return Image.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Dropdown")
	UUIToggle* GetToggle()const;
};