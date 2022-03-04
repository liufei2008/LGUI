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

DECLARE_DELEGATE_RetVal_OneParam(bool, FLGUITextInputCustomInputTypeDelegate, const FString&)
/**
 * @param	InString	The will display string value, for check if it is valid. If not, then display origin string value.
 * @return	Is "InString" valid?
 */
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FLGUITextInputCustomInputTypeDynamicDelegate, const FString&, InString);

UENUM(BlueprintType, Category = LGUI)
enum class ELGUITextInputType:uint8
{
	Standard,
	IntegerNumber,
	DecimalNumber,
	Password,
	/** Use *SetCustomInputTypeFunction* to check if input is valid. */
	CustomFunction,
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
	//password display character
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FString PasswordChar = TEXT("*");
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		bool bAllowMultiLine = false;
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

	FLGUIMulticastStringDelegate OnValueChangeCPP;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FLGUIEventDelegate OnValueChange = FLGUIEventDelegate(LGUIEventDelegateParameterType::String);
	FLGUIMulticastStringDelegate OnSubmitCPP;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FLGUIEventDelegate OnSubmit = FLGUIEventDelegate(LGUIEventDelegateParameterType::String);
	FLGUIMulticastBoolDelegate OnInputActivateCPP;
	UPROPERTY(EditAnywhere, Category = "LGUI-Input")
		FLGUIEventDelegate OnInputActivate = FLGUIEventDelegate(LGUIEventDelegateParameterType::Bool);
	FLGUITextInputCustomInputTypeDelegate CustomInputTypeFunction;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		class UUIText* GetTextComponent()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		FString GetText()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetText(FString InText, bool InFireEvent = false);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		ELGUITextInputType GetInputType()const { return InputType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetInputType(ELGUITextInputType newValue);
	void ActivateInput(ULGUIPointerEventData* eventData = nullptr);
	void DeactivateInput(bool InFireEvent = true);

	FDelegateHandle RegisterValueChangeEvent(const FLGUIStringDelegate& InDelegate);
	FDelegateHandle RegisterValueChangeEvent(const TFunction<void(const FString&)>& InFunction);
	void UnregisterValueChangeEvent(const FDelegateHandle& InHandle);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		FLGUIDelegateHandleWrapper RegisterValueChangeEvent(const FLGUITextInputDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void UnregisterValueChangeEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);

	FDelegateHandle RegisterSubmitEvent(const FLGUIStringDelegate& InDelegate);
	FDelegateHandle RegisterSubmitEvent(const TFunction<void(const FString&)>& InFunction);
	void UnregisterSubmitEvent(const FDelegateHandle& InHandle);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		FLGUIDelegateHandleWrapper RegisterSubmitEvent(const FLGUITextInputDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void UnregisterSubmitEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);

	FDelegateHandle RegisterInputActivateEvent(const FLGUIBoolDelegate& InDelegate);
	FDelegateHandle RegisterInputActivateEvent(const TFunction<void(bool)>& InDelegate);
	void UnregisterInputActivateEvent(const FDelegateHandle& InHandle);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		FLGUIDelegateHandleWrapper RegisterInputActivateEvent(const FLGUIInputActivateDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void UnregisterInputActivateEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);

	void SetCustomInputTypeFunction(const FLGUITextInputCustomInputTypeDelegate& InFunction);
	void SetCustomInputTypeFunction(const TFunction<bool(const FString&)>& InFunction);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void SetCustomInputTypeFunction(const FLGUITextInputCustomInputTypeDynamicDelegate& InFunction);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Input")
		void ClearCustomInputTypeEvent();
private:
	void BindKeys();
	void UnbindKeys();
	void AnyKeyPressed();
	void AnyKeyReleased();
	bool IsValidChar(char c);
	bool IsValidString(const FString& InString);
	FString PasteResultString();
	FString ForwardSpaceResultString();
	FString BackSpaceResultString();
	void AppendChar(char c);
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
	void UpdateCaretPosition(FVector2f InCaretPosition, bool InHideSelection = true);
	void UpdateSelection();
	void UpdateInputComposition();
	void HideSelectionMask();
	//return true if have selection
	bool DeleteIfSelection(int& OutCaretOffset);
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
	FVector3f PressMousePosition = FVector3f(0, 0, 0);
	//caret position when press, UIText space
	FVector2f PressCaretPosition = FVector2f(0, 0);
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

		bool IsComposing() 
		{
			return bIsComposing;
		}
		void AbortComposition()
		{
			bIsComposing = false;
		}
		void UpdateInputComposition();
	
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

		bool bIsComposing = false;
		int32 CompositionBeginIndex = 0;
		uint32 CompositionLength = 0;
		TSharedPtr<SBox> CachedWindow;
		ECaretPosition CaretPosition = ITextInputMethodContext::ECaretPosition::Ending;
		int32 CompositionCaretOffset = 0;//if some chars is selected when input, we need to delete selected chars, and after that caret position will change. this is caret offset value
	};
private:
	TSharedPtr<FVirtualKeyboardEntry> VirtualKeyboardEntry;
	TSharedPtr<FTextInputMethodContext> TextInputMethodContext;
	TSharedPtr<ITextInputMethodChangeNotifier> TextInputMethodChangeNotifier;
	
};
