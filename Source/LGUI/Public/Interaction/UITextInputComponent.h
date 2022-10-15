// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "Interaction/UISelectableComponent.h"
#include "Components/InputComponent.h"
#include "Event/LGUIEventDelegate.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "Event/Interface/LGUIPointerClickInterface.h"
#include "Event/Interface/LGUIPointerDragInterface.h"
#include "Widgets/Input/IVirtualKeyboardEntry.h"
#include "GenericPlatform/ITextInputMethodSystem.h"
#include "Event/LGUIDelegateHandleWrapper.h"
#include "Core/ActorComponent/UIText.h"
#include "Widgets/Layout/SBox.h"
#include "UITextInputComponent.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUITextInputDynamicDelegate, FString, InString);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIInputActivateDynamicDelegate, bool, InActivate);

/**
 * Custom function to verify input string, return true if the input string is good to use, false otherwise.
 * @param	InString	The will display string value, for check if it is valid. If not, then display origin string value.
 * @param	InStartIndex	New insert char index in InString.
 * @return	Is "InString" valid?
 */
DECLARE_DELEGATE_RetVal_TwoParams(bool, FLGUITextInputCustomInputTypeDelegate, const FString&, int)
/**
 * Custom function to verify input string, return true if the input string is good to use, false otherwise.
 * @param	InString	The will display string value, for check if it is valid. If not, then display origin string value.
 * @param	InStartIndex	New insert char index in InString.
 * @return	Is "InString" valid?
 */
DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(bool, FLGUITextInputCustomInputTypeDynamicDelegate, const FString&, InString, int, InStartIndex);

UENUM(BlueprintType, Category = LGUI)
enum class ELGUITextInputType:uint8
{
	/** No validation. Any input is valid. */
	Standard = 0,
	/**
	 * Allow whole numbers (positive or negative).
	 * Characters 0-9 and - (dash / minus sign) are allowed. The dash is only allowed as the first character.
	 */
	IntegerNumber = 1,
	/**
	 * Allows decimal numbers (positive or negative).
	 * Characters 0-9, . (dot), and - (dash / minus sign) are allowed. The dash is only allowed as the first character. Only one dot in the string is allowed.
	 */
	DecimalNumber = 2,
	/**
	 * Allows letters A-Z, a-z and numbers 0-9.
	 */
	Alphanumeric = 5,
	/**
	 * Allows the characters that are allowed in an email address.
	 * Allows characters A-Z, a.z, 0-9, @, . (dot), !, #, $, %, &amp;, ', *, +, -, /, =, ?, ^, _, `, {, |, }, and ~.
	 * Only one @ is allowed in the string and more than one dot in a row are not allowed. Note that the character validation does not validate the entire string as being a valid email address since it only does validation on a per-character level, resulting in the typed character either being added to the string or not.
	 */
	EmailAddress = 6,
	/**
	 * Display as password, without any validation.
	 * NOTE!!! This type will be deprecate, use DisplayType.Password instead.
	 */
	Password = 3,
	/** Use *SetCustomInputTypeFunction* to check if input is valid. */
	CustomFunction = 4,
};
UENUM(BlueprintType, Category = LGUI)
enum class ELGUITextInputDisplayType :uint8
{
	Standard,
	/** Display as password. */
	Password,
};
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUITextInputComponent : public UUISelectableComponent, public ILGUIPointerClickInterface, public ILGUIPointerDragInterface
{
	GENERATED_BODY()
	
protected:	
	virtual void Awake() override;
	virtual void Update(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	friend class FUITextInputCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		TWeakObjectPtr<class AUITextActor> TextActor;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FString Text;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		ELGUITextInputType InputType;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		ELGUITextInputDisplayType DisplayType = ELGUITextInputDisplayType::Standard;
	//password display character
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FString PasswordChar = TEXT("*");
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		bool bAllowMultiLine = false;
	/**
	 * This will be used in multiline mode, when hit enter, if one of these keys is also pressing then the input will submit, otherwise a new line will be added.
	 * Commonly only use control/shift/alt key.
	 * Not allow "Enter" key.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI-Input", meta = (EditCondition="bAllowMultiLine"))
		TArray<FKey> MultiLineSubmitFunctionKeys;
	/** If PlaceHolderActor is a UITextActor, then mobile virtual keyboard's hint text will get from PlaceHolderActor. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		TWeakObjectPtr<class AUIBaseActor> PlaceHolderActor;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		float CaretBlinkRate = 0.5f;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		float CaretWidth = 2.0f;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FColor CaretColor = FColor(50, 50, 50, 255);
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FColor SelectionColor = FColor(168, 206, 255, 128);
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FVirtualKeyboardOptions VirtualKeyboradOptions;
	//Ignore these keys input. eg, if use tab and arrow keys for navigation then you should put tab and arrow keys in this array
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		TArray<FKey> IgnoreKeys;
	/** Automatic activate input when use navigation input and navigate in this. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		bool bAutoActivateInputWhenNavigateIn = false;

	FLGUIMulticastStringDelegate OnValueChangeCPP;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FLGUIEventDelegate OnValueChange = FLGUIEventDelegate(LGUIEventDelegateParameterType::String);
	FLGUIMulticastStringDelegate OnSubmitCPP;
	/** Input submit by "Enter" key. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FLGUIEventDelegate OnSubmit = FLGUIEventDelegate(LGUIEventDelegateParameterType::String);
	FLGUIMulticastBoolDelegate OnInputActivateCPP;
	/** Input activate or deactivate, means begin input or end input. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FLGUIEventDelegate OnInputActivate = FLGUIEventDelegate(LGUIEventDelegateParameterType::Bool);
	FLGUITextInputCustomInputTypeDelegate CustomInputTypeFunction;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		class UUIText* GetTextComponent()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		const FString& GetText()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		ELGUITextInputType GetInputType()const { return InputType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		ELGUITextInputDisplayType GetDisplayType()const { return DisplayType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		const FString& GetPasswordChar()const { return PasswordChar; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		bool GetAllowMultiLine()const { return bAllowMultiLine; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		const TArray<FKey>& GetMultiLineSubmitFunctionKeys()const { return MultiLineSubmitFunctionKeys; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		class AUIBaseActor* GetPlaceHolderActor()const { return PlaceHolderActor.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		float GetCaretBlinkRate()const { return CaretBlinkRate; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		float GetCaretWidth()const { return CaretWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		FColor GetCaretColor()const { return CaretColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		FColor GetSelectionColor()const { return SelectionColor; }
	UFUNCTION()
		FVirtualKeyboardOptions GetVirtualKeyboradOptions()const { return VirtualKeyboradOptions; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		const TArray<FKey>& GetIgnoreKeys()const { return IgnoreKeys; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		bool GetAutoActivateInputWhenNavigateIn()const { return bAutoActivateInputWhenNavigateIn; }

	/**
	 * Set text value.
	 * @return false if InText is wrong format.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		bool SetText(const FString& InText, bool InFireEvent = false);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetInputType(ELGUITextInputType newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetDisplayType(ELGUITextInputDisplayType newValue);
	/** Set password display char. Only allow one char in the value string */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetPasswordChar(const FString& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetAllowMultiLine(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetMultiLineSubmitFunctionKeys(const TArray<FKey>& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetPlaceHolderActor(class AUIBaseActor* value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetCaretBlinkRate(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetCaretWidth(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetCaretColor(FColor value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetSelectionColor(FColor value);
	UFUNCTION()
		void SetVirtualKeyboradOptions(FVirtualKeyboardOptions value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetIgnoreKeys(const TArray<FKey>& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetAutoActivateInputWhenNavigateIn(bool value);

	void ActivateInput(ULGUIPointerEventData* eventData = nullptr);
	void DeactivateInput(bool InFireEvent = true);

	FDelegateHandle RegisterValueChangeEvent(const FLGUIStringDelegate& InDelegate);
	FDelegateHandle RegisterValueChangeEvent(const TFunction<void(const FString&)>& InFunction);
	void UnregisterValueChangeEvent(const FDelegateHandle& InHandle);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		FLGUIDelegateHandleWrapper RegisterValueChangeEvent(const FLGUITextInputDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void UnregisterValueChangeEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);

	/** Input submit by "Enter" key. */
	FDelegateHandle RegisterSubmitEvent(const FLGUIStringDelegate& InDelegate);
	/** Input submit by "Enter" key. */
	FDelegateHandle RegisterSubmitEvent(const TFunction<void(const FString&)>& InFunction);
	void UnregisterSubmitEvent(const FDelegateHandle& InHandle);
	/** Input submit by "Enter" key. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		FLGUIDelegateHandleWrapper RegisterSubmitEvent(const FLGUITextInputDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void UnregisterSubmitEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);

	/** Input activate or deactivate, means begin input or end input. */
	FDelegateHandle RegisterInputActivateEvent(const FLGUIBoolDelegate& InDelegate);
	/** Input activate or deactivate, means begin input or end input. */
	FDelegateHandle RegisterInputActivateEvent(const TFunction<void(bool)>& InDelegate);
	void UnregisterInputActivateEvent(const FDelegateHandle& InHandle);
	/** Input activate or deactivate, means begin input or end input. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		FLGUIDelegateHandleWrapper RegisterInputActivateEvent(const FLGUIInputActivateDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void UnregisterInputActivateEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);

	/**
	 * Set custom function to verify input string, return true if the input string is good to use, false otherwise.
	 */
	void SetCustomInputTypeFunction(const FLGUITextInputCustomInputTypeDelegate& InFunction);
	/**
	 * Set custom function to verify input string, return true if the input string is good to use, false otherwise.
	 */
	void SetCustomInputTypeFunction(const TFunction<bool(const FString&, int)>& InFunction);
	/**
	 * Set custom function to verify input string, return true if the input string is good to use, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetCustomInputTypeFunction(const FLGUITextInputCustomInputTypeDynamicDelegate& InFunction);
	/** Remove the function set by "SetCustomInputTypeFunction" */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void ClearCustomInputTypeFunction();
private:
	void BindKeys();
	void UnbindKeys();
	void AnyKeyPressed();
	void AnyKeyReleased();
	bool IsValidChar(TCHAR c);
	/**
	 * delete selected chars if there is any.
	 * @return true if anything deleted.
	 */
	bool DeleteSelection(bool InFireEvent = true);
	void InsertCharAtCaretPosition(TCHAR c);
	FInputKeyBinding AnyKeyBinding;
	UPROPERTY(Transient) APlayerController* PlayerController = nullptr;
	bool CheckPlayerController();
	bool bInputActive = false;
	//Caret position of full text. caret is on left side of char
	int CaretPositionIndex = 0;
	//caret position line index of full text
	int CaretPositionLineIndex = 0;
	float NextCaretBlinkTime = 0;
	float ElapseTime = 0;
	void BackSpace();
	void ForwardSpace();
	void MoveToStart();
	void MoveToEnd();
	void MoveLeft(bool withSelection);
	void MoveRight(bool withSelection);
	void MoveUp(bool withSelection);
	void MoveDown(bool withSelection);
	void Copy();
	void Paste();
	void Cut();
	void SelectAll();

	void UpdateAfterTextChange(bool InFireEvent = true);

	void FireOnValueChangeEvent();
	void UpdateUITextComponent();
	void UpdatePlaceHolderComponent();
	void UpdateCaretPosition(bool InHideSelection = true);
	void UpdateCaretPosition(FVector2D InCaretPosition, bool InHideSelection = true);
	void UpdateSelection();
	void HideSelectionMask();
	//a sprite for caret, can blink, can represent current caret location
	UPROPERTY(Transient)TWeakObjectPtr<class UUISprite> CaretObject;
	//selection mask
	UPROPERTY(Transient)TArray<TWeakObjectPtr<class UUISprite>> SelectionMaskObjectArray;
	//range selection
	TArray<FUITextSelectionProperty> SelectionPropertyArray;
	//in single line mode, will clamp text if out of range. this is left start index of visible char
	//this property can only modify in UpdateUITextComponent function
	int VisibleCharStartIndex = 0;
	//in multi line mode, will clamp text line if out of range. this is top start line index of visible char
	//this property can only modify in UpdateUITextComponent function
	int VisibleCharStartLineIndex = 0;
	//max visible line count that can fit in rect
	int MaxVisibleLineCount = 0;

	//mouse position when press, world space
	FVector PressMousePosition = FVector(0, 0, 0);
	//caret position when press, UIText space
	FVector2D PressCaretPosition = FVector2D(0, 0);
	int PressCaretPositionIndex = 0, PressCaretPositionLineIndex = 0;
protected:
	virtual void OnUIActiveInHierachy(bool ativeOrInactive)override;
	virtual void OnUIInteractionStateChanged(bool interactableOrNot)override;

	virtual bool OnPointerEnter_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerExit_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerSelect_Implementation(ULGUIBaseEventData* eventData) override;
	virtual bool OnPointerDeselect_Implementation(ULGUIBaseEventData* eventData) override;
	virtual bool OnPointerClick_Implementation(ULGUIPointerEventData* eventData) override;
	virtual bool OnPointerBeginDrag_Implementation(ULGUIPointerEventData* eventData) override;
	virtual bool OnPointerDrag_Implementation(ULGUIPointerEventData* eventData) override;
	virtual bool OnPointerEndDrag_Implementation(ULGUIPointerEventData* eventData) override;
	virtual bool OnPointerDown_Implementation(ULGUIPointerEventData* eventData) override;
	virtual bool OnPointerUp_Implementation(ULGUIPointerEventData* eventData) override;

private:
	friend class FVirtualKeyboardEntry;
	class FVirtualKeyboardEntry :public IVirtualKeyboardEntry
	{
	public:
		static TSharedRef<FVirtualKeyboardEntry> Create(UUITextInputComponent* Input);

		virtual void SetTextFromVirtualKeyboard(const FText& InNewText, ETextEntryType TextEntryType) override;
		virtual void SetSelectionFromVirtualKeyboard(int InSelStart, int SelEnd)override;
		virtual bool GetSelection(int& OutSelStart, int& OutSelEnd) override;

		virtual FText GetText() const override;
		virtual FText GetHintText() const override;
		virtual EKeyboardType GetVirtualKeyboardType() const override;
		virtual FVirtualKeyboardOptions GetVirtualKeyboardOptions() const override;
		virtual bool IsMultilineEntry() const override;

	private:
		FVirtualKeyboardEntry(UUITextInputComponent* InInput);
		UUITextInputComponent* InputComp;
	};

private:
	friend class FTextInputMethodContext;
	class FTextInputMethodContext:public ITextInputMethodContext
	{
	public:
		static TSharedRef<FTextInputMethodContext> Create(UUITextInputComponent* Input);
		void Dispose();

		virtual bool IsComposing() override
		{
			return bIsComposing;
		}
	
		virtual bool IsReadOnly() override;
		virtual uint32 GetTextLength() override;
		virtual void GetSelectionRange(uint32& BeginIndex, uint32& Length, ECaretPosition& CaretPosition) override;
		virtual void SetSelectionRange(const uint32 BeginIndex, const uint32 Length, const ECaretPosition CaretPosition) override;
		virtual void GetTextInRange(const uint32 BeginIndex, const uint32 Length, FString& OutString) override;
		virtual void SetTextInRange(const uint32 BeginIndex, const uint32 Length, const FString& InString) override;
		virtual int32 GetCharacterIndexFromPoint(const FVector2D& Point) override;
		virtual bool GetTextBounds(const uint32 BeginIndex, const uint32 Length, FVector2D& Position, FVector2D& Size) override;
		virtual void GetScreenBounds(FVector2D& Position, FVector2D& Size) override;
		virtual TSharedPtr<FGenericWindow> GetWindow() override;
		virtual void BeginComposition() override;
		virtual void UpdateCompositionRange(const int32 InBeginIndex, const uint32 InLength) override;
		virtual void EndComposition() override;

	private:
		FTextInputMethodContext(UUITextInputComponent* InInput);
		UUITextInputComponent* InputComp;
		FString OriginString;
		bool bIsComposing = false;
		TSharedPtr<SBox> CachedWindow;
	};
private:
	TSharedPtr<FVirtualKeyboardEntry> VirtualKeyboardEntry;
	TSharedPtr<FTextInputMethodContext> TextInputMethodContext;
	TSharedPtr<ITextInputMethodChangeNotifier> TextInputMethodChangeNotifier;
	
};
