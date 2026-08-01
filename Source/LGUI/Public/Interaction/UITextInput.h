// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Interaction/UISelectable.h"
#include "Components/InputComponent.h"
#include "Event/LexUIEventDelegate.h"
#include "Event/LexDelegateDeclaration.h"
#include "Event/Interface/LexPointerClickInterface.h"
#include "Event/Interface/LexPointerDragInterface.h"
#include "Widgets/Input/IVirtualKeyboardEntry.h"
#include "GenericPlatform/ITextInputMethodSystem.h"
#include "Core/Components/LexText.h"
#include "Widgets/Layout/SBox.h"
#include "UITextInput.generated.h"


class ULexSprite;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUITextInputValueChangedEvent, FString, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUITextInputActivateEvent, bool, Value);

UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class ULexTextInputCustomValidation : public UObject
{
	GENERATED_BODY()
public:
	ULexTextInputCustomValidation();
	/**
	 * Verify input string, return true if the input string is good to use, false otherwise.
	 * @param InTextInput	The UITextInputComponent object reference which call this function.
	 * @param InString	The will display string value, for check if it is valid. If not, then display origin string value.
	 * @param InIndexOfInsertedChar	New inserted char index in InString.
	 * @return true if the input string is good to use, false otherwise.
	 */
	virtual bool OnValidateInput(UUITextInput* InTextInput, const FString& InString, int InIndexOfInsertedChar);
protected:
	/** use this to tell if the class is compiled from blueprint, only blueprint can execute ReceiveXXX. */
	bool bCanExecuteBlueprintEvent = false;
	/**
	 * Verify input string, return true if the input string is good to use, false otherwise.
	 * @param InTextInput	The UITextInputComponent object reference which call this function.
	 * @param InString	The will display string value, for check if it is valid. If not, then display origin string value.
	 * @param InIndexOfInsertedChar	New inserted char index in InString.
	 * @return true if the input string is good to use, false otherwise.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnValidateInput"), Category = "LGUI")
		bool ReceiveOnValidateInput(UUITextInput* InTextInput, const FString& InString, int InIndexOfInsertedChar);
};

UENUM(BlueprintType, Category = LGUI)
enum class EUITextInputType:uint8
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
	/** Use a user implemented *CustomValidation* to do custom input check. */
	Custom = 7,
};
UENUM(BlueprintType, Category = LGUI)
enum class EUITextInputDisplayType :uint8
{
	Standard,
	/** Display as password. */
	Password,
};

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUITextInput : public UUISelectable, public ILexPointerClickInterface, public ILexPointerDragInterface
{
	GENERATED_BODY()
	
protected:	
	virtual void Awake() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnDestroy() override;
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	friend class FUITextInputCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		TWeakObjectPtr<ULexText> TextVisual;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FString Text;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		EUITextInputType InputType;
	/** Use this to do custom validation. Only valid when InputType = Custom */
	UPROPERTY(EditAnywhere, Instanced, Category = "LGUI-Input", meta = (EditCondition = "InputType==EUITextInputType::Custom"))
		TObjectPtr<ULexTextInputCustomValidation> CustomValidation = nullptr;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		EUITextInputDisplayType DisplayType = EUITextInputDisplayType::Standard;
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
		TWeakObjectPtr<ULexWidget> PlaceHolder;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		float CaretBlinkRate = 0.5f;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		float CaretWidth = 2.0f;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FColor CaretColor = FColor(50, 50, 50, 255);
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FColor SelectionColor = FColor(168, 206, 255, 128);
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FVirtualKeyboardOptions VirtualKeyboardOptions;
	//Ignore these keys input. eg, if use tab and arrow keys for navigation then you should put tab and arrow keys in this array
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		TArray<FKey> IgnoreKeys;
	/** Automatic activate input when use navigation input and navigate in this. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		bool bAutoActivateInputWhenNavigateIn = false;
	/** Select all text value when activate input. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		bool bSelectAllWhenActivateInput = true;
	/** Read only text block, can copy text content, but not editable. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		bool bReadOnly = false;

	FLexUIMulticastDelegateString OnValueChangedCPP;
	UPROPERTY(BlueprintAssignable, Category = "LGUI-Input")
	FUITextInputValueChangedEvent OnValueChanged;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input", DisplayName="OnValueChangedED")
	FLexUIEventDelegate OnValueChangedED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::String);
	
	FLexUIMulticastDelegateString OnSubmitCPP;
	UPROPERTY(BlueprintAssignable, Category = "LGUI-Input")
	FUITextInputValueChangedEvent OnSubmit;
	/** Input submit by "Enter" key. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Input", DisplayName="OnSubmit")
	FLexUIEventDelegate OnSubmitED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::String);
	
	FLexUIMulticastDelegateBool OnInputActivateCPP;
	UPROPERTY(BlueprintAssignable, Category = "LGUI-Input")
	FUITextInputActivateEvent OnInputActivate;
	/** Input activate or deactivate, means begin input or end input. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Input", DisplayName="OnInputActivate")
	FLexUIEventDelegate OnInputActivateED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Bool);

	void SetText(const FString& InText, bool InFireEvent);
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		class ULexText* GetTextComponent()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		const FString& GetText()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		EUITextInputType GetInputType()const { return InputType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		ULexTextInputCustomValidation* GetCustomValidation()const { return CustomValidation; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		EUITextInputDisplayType GetDisplayType()const { return DisplayType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		const FString& GetPasswordChar()const { return PasswordChar; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		bool GetAllowMultiLine()const { return bAllowMultiLine; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		const TArray<FKey>& GetMultiLineSubmitFunctionKeys()const { return MultiLineSubmitFunctionKeys; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		ULexWidget* GetPlaceHolderActor()const { return PlaceHolder.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		float GetCaretBlinkRate()const { return CaretBlinkRate; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		float GetCaretWidth()const { return CaretWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		FColor GetCaretColor()const { return CaretColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		FColor GetSelectionColor()const { return SelectionColor; }
	UFUNCTION()
		FVirtualKeyboardOptions GetVirtualKeyboardOptions()const { return VirtualKeyboardOptions; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		const TArray<FKey>& GetIgnoreKeys()const { return IgnoreKeys; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		bool GetAutoActivateInputWhenNavigateIn()const { return bAutoActivateInputWhenNavigateIn; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		bool GetReadOnly()const { return bReadOnly; }

	/** Set text value and send callback event */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
	void SetText(const FString& InText);
	/** Set text value and NOT send callback event */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
	void SetTextWithoutNotify(const FString& InText);
	
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetInputType(EUITextInputType Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetCustomValidation(ULexTextInputCustomValidation* Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetDisplayType(EUITextInputDisplayType Value);
	/** Set password display char. Only allow one char in the value string */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetPasswordChar(const FString& Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetAllowMultiLine(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetMultiLineSubmitFunctionKeys(const TArray<FKey>& Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetPlaceHolder(ULexWidget* Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetCaretBlinkRate(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetCaretWidth(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetCaretColor(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetSelectionColor(FColor Value);
	UFUNCTION()
		void SetVirtualKeyboradOptions(FVirtualKeyboardOptions Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetIgnoreKeys(const TArray<FKey>& Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetAutoActivateInputWhenNavigateIn(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetReadOnly(bool Value);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
	void ActivateInput(ULexPointerEventData* EventData = nullptr);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
	void DeactivateInput(bool InFireEvent = true);
	
	/**
	 * Verify input string value and insert the string value to text value at current caret position.
	 * @param Value string value to check and insert.
	 * @return true- if any char added.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		bool VerifyAndInsertStringAtCaretPosition(const FString& Value);
	/**
	 * Verify input char value and insert the char value to text value at current caret position.
	 * @param Value char value to check and insert.
	 * @return true- if verify success and added.
	 */
	bool VerifyAndInsertCharAtCaretPosition(TCHAR Value);
private:
	TWeakObjectPtr<AActor> InputComponentAgent;
	void BindKeys();
	void UnbindKeys();
	void AnyKeyPressed(FKey key);
	bool IsValidChar(TCHAR c);
	/**
	 * delete selected chars if there is any.
	 * @return true if anything deleted.
	 */
	bool DeleteSelection(bool InFireEvent = true);
	void InsertCharAtCaretPosition(TCHAR c);
	void InsertStringAtCaretPosition(const FString& value);
	FInputKeyBinding AnyKeyBinding;
	UPROPERTY(Transient) TObjectPtr<APlayerController> PlayerController = nullptr;
	bool CheckPlayerController();
	bool bInputActive = false;
	float NextCaretBlinkTime = 0;
	float ElapseTime = 0;
	void BackSpace();
	void ForwardSpace();
	/**
	 * .
	 * @param moveType 0-left, 1-right, 2-up, 3-down, 4-start, 5-end
	 */
	void MoveCaret(int32 moveType, bool withSelection);
	void Copy();
	void Paste();
	void Cut();
	void SelectAll();

	FString GetReplaceText()const;

	void UpdateAfterTextChange(bool InFireEvent = true);

	void FireOnValueChangedEvent();
	void UpdateUITextComponent();
	void UpdatePlaceHolderComponent();
	void UpdateCaretPosition(bool InHideSelection = true);
	void UpdateCaretPosition(FVector2f InCaretPosition, bool InHideSelection = true);
	void UpdateSelection();
	void HideSelectionMask();
	//a Sprite for caret, can blink, can represent current caret location
	UPROPERTY(Transient)TWeakObjectPtr<ULexWidget> CaretWidget;
	//selection mask
	UPROPERTY(Transient)TArray<TWeakObjectPtr<ULexVisual>> SelectionMaskObjectArray;
	//range selection
	TArray<FLexUITextSelectionProperty> SelectionPropertyArray;
	//Caret position of full text. caret is on left side of char
	int CaretPositionIndex = 0;
	//caret position line index of full text
	int CaretPositionLineIndex = 0;
	//in single line mode, will clamp text if out of range. this is left start index of visible char
	//this property can only modify in UpdateUITextComponent function
	int VisibleCaretStartIndex = 0;
	//in multi line mode, will clamp text line if out of range. this is top start line index of visible char
	//this property can only modify in UpdateUITextComponent function
	int VisibleCaretStartLineIndex = 0;

	int PressCaretPositionIndex = 0, PressCaretPositionLineIndex = 0;
protected:
	virtual void OnEnable() override;
	virtual void OnInteractableChanged(bool Interactable) override;
	virtual void OnDimensionsChanged(bool PivotChanged, bool WidthChanged, bool HeightChanged)override;

	virtual void OnPointerEnter_Implementation(ULexPointerEventData* EventData)override;
	virtual void OnPointerExit_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnSelect_Implementation(ULexBaseEventData* EventData) override;
	virtual bool OnDeselect_Implementation(ULexBaseEventData* EventData) override;
	virtual bool OnPointerClick_Implementation(ULexPointerEventData* EventData) override;
	virtual bool OnPointerBeginDrag_Implementation(ULexPointerEventData* EventData) override;
	virtual bool OnPointerDrag_Implementation(ULexPointerEventData* EventData) override;
	virtual bool OnPointerEndDrag_Implementation(ULexPointerEventData* EventData) override;
	virtual bool OnPointerDown_Implementation(ULexPointerEventData* EventData) override;
	virtual bool OnPointerUp_Implementation(ULexPointerEventData* EventData) override;

private:
	friend class FVirtualKeyboardEntry;
	class FVirtualKeyboardEntry :public IVirtualKeyboardEntry
	{
	public:
		static TSharedRef<FVirtualKeyboardEntry> Create(UUITextInput* Input);

		virtual void SetTextFromVirtualKeyboard(const FText& InNewText, ETextEntryType TextEntryType) override;
		virtual void SetSelectionFromVirtualKeyboard(int InSelStart, int SelEnd)override;
		virtual bool GetSelection(int& OutSelStart, int& OutSelEnd) override;

		virtual FText GetText() const override;
		virtual FText GetHintText() const override;
		virtual EKeyboardType GetVirtualKeyboardType() const override;
		virtual FVirtualKeyboardOptions GetVirtualKeyboardOptions() const override;
		virtual bool IsMultilineEntry() const override;

	private:
		FVirtualKeyboardEntry(UUITextInput* InInput);
		UUITextInput* InputComp;
	};

private:
	friend class FTextInputMethodContext;
	class FTextInputMethodContext:public ITextInputMethodContext
	{
	public:
		static TSharedRef<FTextInputMethodContext> Create(UUITextInput* Input);
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
		FTextInputMethodContext(UUITextInput* InInput);
		UUITextInput* InputComp;
		FString OriginString;
		bool bIsComposing = false;
		TSharedPtr<SBox> CachedWindow;
	};
private:
	TSharedPtr<FVirtualKeyboardEntry> VirtualKeyboardEntry;
	TSharedPtr<FTextInputMethodContext> TextInputMethodContext;
	TSharedPtr<ITextInputMethodChangeNotifier> TextInputMethodChangeNotifier;
	
};
