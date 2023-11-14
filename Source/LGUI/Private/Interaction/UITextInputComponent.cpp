// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UITextInputComponent.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/Actor/UITextActor.h"
#include "Core/Actor/UISpriteActor.h"
#include "InputCoreTypes.h"
#include "Utils/LGUIUtils.h"
#include "Event/LGUIEventSystem.h"
#include "HAL/PlatformApplicationMisc.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Core/LGUIFontData.h"
#include "Core/LGUISpriteData.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

UUITextInputCustomValidation::UUITextInputCustomValidation()
{
	bCanExecuteBlueprintEvent = GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native);
}
bool UUITextInputCustomValidation::OnValidateInput(UUITextInputComponent* InTextInput, const FString& InString, int InIndexOfInsertedChar)
{
	if (bCanExecuteBlueprintEvent)
	{
		return ReceiveOnValidateInput(InTextInput, InString, InIndexOfInsertedChar);
	}
	return false;
}

void UUITextInputComponent::Awake()
{
	Super::Awake();
	
	if (!VirtualKeyboardEntry.IsValid())
	{
		VirtualKeyboardEntry = FVirtualKeyboardEntry::Create(this);
	}
	if (!TextInputMethodContext.IsValid())
	{
		TextInputMethodContext = FTextInputMethodContext::Create(this);
	}
	this->SetCanExecuteUpdate(true);
}
void UUITextInputComponent::Update(float DeltaTime)
{
	Super::Update(DeltaTime);
	if (bInputActive)
	{
		//blink caret
		if (CaretObject.IsValid())
		{
			ElapseTime += DeltaTime;
			if (NextCaretBlinkTime < ElapseTime)
			{
				CaretObject->SetAlpha(1.0f - CaretObject->GetAlpha());
				NextCaretBlinkTime = ElapseTime + CaretBlinkRate;
			}
		}
	}
}

void UUITextInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	DeactivateInput(EndPlayReason != EEndPlayReason::EndPlayInEditor);
	if (TextInputMethodContext.IsValid())
	{
		TextInputMethodContext->Dispose();
	}
}

#if WITH_EDITOR
bool UUITextInputComponent::CanEditChange(const FProperty* InProperty)const
{
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UUITextInputComponent, PasswordChar))
	{
		if (InputType != ELGUITextInputType::Password
			&& DisplayType != ELGUITextInputDisplayType::Password)
		{
			return false;
		}
	}
	return Super::CanEditChange(InProperty);
}
void UUITextInputComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto propertyName = Property->GetFName();
		if (propertyName == GET_MEMBER_NAME_CHECKED(UUITextInputComponent, PasswordChar))
		{
			if (PasswordChar.Len() > 1)
			{
				auto firstChar = PasswordChar[0];
				PasswordChar.Empty();
				PasswordChar.AppendChar(firstChar);
			}
			else if (PasswordChar.Len() == 0)
			{
				PasswordChar.AppendChar('*');
			}
		}
		else if (propertyName == GET_MEMBER_NAME_CHECKED(UUITextInputComponent, MultiLineSubmitFunctionKeys))
		{
			for (int i = 0; i < MultiLineSubmitFunctionKeys.Num(); i++)
			{
				if (MultiLineSubmitFunctionKeys[i] == EKeys::Enter)
				{
					MultiLineSubmitFunctionKeys[i] = EKeys::LeftControl;
				}
			}
			if (MultiLineSubmitFunctionKeys.Num() == 1
				&& MultiLineSubmitFunctionKeys[0] == FKey(NAME_None)
				)
			{
				MultiLineSubmitFunctionKeys[0] = EKeys::LeftControl;//first one set to LeftControl as default
			}
		}
	}
	if (TextActor != nullptr)
	{
		TextActor->GetUIText()->SetOverflowType(bAllowMultiLine ? EUITextOverflowType::VerticalOverflow : EUITextOverflowType::HorizontalOverflow);
		if (!TextActor->GetUIText()->GetText().IsCultureInvariant())
		{
			TextActor->GetUIText()->SetText(FText::AsCultureInvariant(Text));
			UE_LOG(LGUI, Error, TEXT("[UUITextInputComponent::PostEditChangeProperty]Input text should not change by culture, set it to not localizable."));
		}
	}
	UpdateUITextComponent();
	UpdatePlaceHolderComponent();
}
#endif
bool UUITextInputComponent::CheckPlayerController()
{
	if (PlayerController != nullptr)return true;
	PlayerController = this->GetWorld()->GetFirstPlayerController();
	if (PlayerController != nullptr)return true;
	return false;
}
void UUITextInputComponent::AnyKeyPressed(FKey Key)
{
	if (bInputActive == false)return;
	if (!CheckPlayerController())return;
	if (TextActor == nullptr)return;

	TCHAR inputChar = 127;
	bool ctrl = PlayerController->PlayerInput->IsCtrlPressed();
	bool shift = PlayerController->PlayerInput->IsShiftPressed();
	bool alt = PlayerController->PlayerInput->IsAltPressed();
	bool ctrlOnly = ctrl && !alt && !shift;
	bool shiftOnly = !ctrl && !alt && shift;

	//Function key
	if (Key == EKeys::BackSpace)
	{
		BackSpace();
		return;
	}
	else if (Key == EKeys::Delete)
	{
		ForwardSpace();
		return;
	}
	else if (Key == EKeys::Home)
	{
		MoveCaret(4, shiftOnly);
		return;
	}
	else if (Key == EKeys::End)
	{
		MoveCaret(5, shiftOnly);
		return;
	}
	//Select all
	else if (Key == EKeys::A)
	{
		if (ctrlOnly)
		{
			SelectAll();
			return;
		}
	}
	//Copy
	else if (Key == EKeys::C)
	{
		if (ctrlOnly)
		{
			Copy();
			return;
		}
	}
	//Paste
	else if (Key == EKeys::V)
	{
		if (ctrlOnly)
		{
			Paste();
			return;
		}
	}
	//Cut
	else if (Key == EKeys::X)
	{
		if (ctrlOnly)
		{
			if (SelectionPropertyArray.Num() != 0)
			{
				Cut();
			}
			return;
		}
	}
	//Arrows
	else if (Key == EKeys::Left)
	{
		MoveCaret(0, shiftOnly);
		return;
	}
	else if (Key == EKeys::Right)
	{
		MoveCaret(1, shiftOnly);
		return;
	}
	else if (Key == EKeys::Up)
	{
		MoveCaret(2, shiftOnly);
		return;
	}
	else if (Key == EKeys::Down)
	{
		MoveCaret(3, shiftOnly);
		return;
	}
	//Submit
	else if (Key == EKeys::Enter)
	{
		if (bAllowMultiLine)//multiline mode
		{
			if (MultiLineSubmitFunctionKeys.Num() > 0
				&& !MultiLineSubmitFunctionKeys.Contains(EKeys::Enter)//Enter is not allowed
				)
			{
					bool isSubmit = false;
					for (auto& SubmitFunctionKey : MultiLineSubmitFunctionKeys)
					{
						if (PlayerController->PlayerInput->IsPressed(SubmitFunctionKey))
						{
							isSubmit = true;
						}
					}
					if (isSubmit)//enter submit
					{
						OnSubmitCPP.Broadcast(Text);
						OnSubmit.FireEvent(Text);
						DeactivateInput();
						return;
					}
			}
			inputChar = '\n';//enter as new line
		}
		else//single line mode, enter means submit
		{
			OnSubmitCPP.Broadcast(Text);
			OnSubmit.FireEvent(Text);
			DeactivateInput();
			return;
		}
	}
	//Cancel
	else if (Key == EKeys::Escape)
	{
		return;
	}

	//space
	else if (Key == EKeys::SpaceBar)
	{
		inputChar = ' ';
	}

	//caps lock
	bool upperCase = false;
	if (FSlateApplication::Get().GetModifierKeys().AreCapsLocked())
	{
		if (!shift)
		{
			upperCase = true;
		}
	}
	else
	{
		if (shift)
		{
			upperCase = true;
		}
	}
	//input char
	if (Key == EKeys::A)
		inputChar = upperCase ? 'A' : 'a';
	else if (Key == EKeys::B)
		inputChar = upperCase ? 'B' : 'b';
	else if (Key == EKeys::C)
		inputChar = upperCase ? 'C' : 'c';
	else if (Key == EKeys::D)
		inputChar = upperCase ? 'D' : 'd';
	else if (Key == EKeys::E)
		inputChar = upperCase ? 'E' : 'e';
	else if (Key == EKeys::F)
		inputChar = upperCase ? 'F' : 'f';
	else if (Key == EKeys::G)
		inputChar = upperCase ? 'G' : 'g';
	else if (Key == EKeys::H)
		inputChar = upperCase ? 'H' : 'h';
	else if (Key == EKeys::I)
		inputChar = upperCase ? 'I' : 'i';
	else if (Key == EKeys::J)
		inputChar = upperCase ? 'J' : 'j';
	else if (Key == EKeys::K)
		inputChar = upperCase ? 'K' : 'k';
	else if (Key == EKeys::L)
		inputChar = upperCase ? 'L' : 'l';
	else if (Key == EKeys::M)
		inputChar = upperCase ? 'M' : 'm';
	else if (Key == EKeys::N)
		inputChar = upperCase ? 'N' : 'n';
	else if (Key == EKeys::O)
		inputChar = upperCase ? 'O' : 'o';
	else if (Key == EKeys::P)
		inputChar = upperCase ? 'P' : 'p';
	else if (Key == EKeys::Q)
		inputChar = upperCase ? 'Q' : 'q';
	else if (Key == EKeys::R)
		inputChar = upperCase ? 'R' : 'r';
	else if (Key == EKeys::S)
		inputChar = upperCase ? 'S' : 's';
	else if (Key == EKeys::T)
		inputChar = upperCase ? 'T' : 't';
	else if (Key == EKeys::U)
		inputChar = upperCase ? 'U' : 'u';
	else if (Key == EKeys::V)
		inputChar = upperCase ? 'V' : 'v';
	else if (Key == EKeys::W)
		inputChar = upperCase ? 'W' : 'w';
	else if (Key == EKeys::X)
		inputChar = upperCase ? 'X' : 'x';
	else if (Key == EKeys::Y)
		inputChar = upperCase ? 'Y' : 'y';
	else if (Key == EKeys::Z)
		inputChar = upperCase ? 'Z' : 'z';

	else if (Key == EKeys::Tilde)
	{
		if (shiftOnly)
			inputChar = '~';
		else
			inputChar = '`';
	}
	else if (Key == EKeys::One)
	{
		if (shiftOnly)
			inputChar = '!';
		else
			inputChar = '1';
	}
	else if (Key == EKeys::Two)
	{
		if (shiftOnly)
			inputChar = '@';
		else
			inputChar = '2';
	}
	else if (Key == EKeys::Three)
	{
		if (shiftOnly)
			inputChar = '#';
		else
			inputChar = '3';
	}
	else if (Key == EKeys::Four)
	{
		if (shiftOnly)
			inputChar = '$';
		else
			inputChar = '4';
	}
	else if (Key == EKeys::Five)
	{
		if (shiftOnly)
			inputChar = '%';
		else
			inputChar = '5';
	}
	else if (Key == EKeys::Six)
	{
		if (shiftOnly)
			inputChar = '^';
		else
			inputChar = '6';
	}
	else if (Key == EKeys::Seven)
	{
		if (shiftOnly)
			inputChar = '&';
		else
			inputChar = '7';
	}
	else if (Key == EKeys::Eight)
	{
		if (shiftOnly)
			inputChar = '*';
		else
			inputChar = '8';
	}
	else if (Key == EKeys::Nine)
	{
		if (shiftOnly)
			inputChar = '(';
		else
			inputChar = '9';
	}
	else if (Key == EKeys::Zero)
	{
		if (shiftOnly)
			inputChar = ')';
		else
			inputChar = '0';
	}
	else if (Key == EKeys::Hyphen)
	{
		if (shiftOnly)
			inputChar = '_';
		else
			inputChar = '-';
	}
	else if (Key == EKeys::Equals)
	{
		if (shiftOnly)
			inputChar = '+';
		else
			inputChar = '=';
	}

	else if (Key == EKeys::NumPadZero)
		inputChar = '0';
	else if (Key == EKeys::NumPadOne)
		inputChar = '1';
	else if (Key == EKeys::NumPadTwo)
		inputChar = '2';
	else if (Key == EKeys::NumPadThree)
		inputChar = '3';
	else if (Key == EKeys::NumPadFour)
		inputChar = '4';
	else if (Key == EKeys::NumPadFive)
		inputChar = '5';
	else if (Key == EKeys::NumPadSix)
		inputChar = '6';
	else if (Key == EKeys::NumPadSeven)
		inputChar = '7';
	else if (Key == EKeys::NumPadEight)
		inputChar = '8';
	else if (Key == EKeys::NumPadNine)
		inputChar = '9';

	else if (Key == EKeys::Multiply)
		inputChar = '*';
	else if (Key == EKeys::Add)
		inputChar = '+';
	else if (Key == EKeys::Subtract)
		inputChar = '-';
	else if (Key == EKeys::Decimal)
		inputChar = '.';
	else if (Key == EKeys::Divide)
		inputChar = '/';

	else if (Key == EKeys::LeftBracket)
	{
		if (shiftOnly)
			inputChar = '{';
		else
			inputChar = '[';
	}
	else if (Key == EKeys::RightBracket)
	{
		if (shiftOnly)
			inputChar = '}';
		else
			inputChar = ']';
	}
	else if (Key == EKeys::Backslash)
	{
		if (shiftOnly)
			inputChar = '|';
		else
			inputChar = '\\';
	}
	else if (Key == EKeys::Semicolon)
	{
		if (shiftOnly)
			inputChar = ':';
		else
			inputChar = ';';
	}
	else if (Key == EKeys::Apostrophe)
	{
		if (shiftOnly)
			inputChar = '\"';
		else
			inputChar = '\'';
	}
	else if (Key == EKeys::Comma)
	{
		if (shiftOnly)
			inputChar = '<';
		else
			inputChar = ',';
	}
	else if (Key == EKeys::Period)
	{
		if (shiftOnly)
			inputChar = '>';
		else
			inputChar = '.';
	}
	else if (Key == EKeys::Slash)
	{
		if (shiftOnly)
			inputChar = '?';
		else
			inputChar = '/';
	}


	if (IsValidChar(inputChar))
	{
		DeleteSelection(false);
		InsertCharAtCaretPosition(inputChar);
		UpdateAfterTextChange(true);
	}
}

bool UUITextInputComponent::IsValidChar(TCHAR c)
{
	auto StringContainsChar = [](TCHAR testChar, const FString& string, int stringLength)
	{
		for (int i = 0; i < stringLength; i++)
		{
			if (string[i] == testChar)
			{
				return true;
			}
		}
		return false;
	};
	//delete key on mac
	if ((int)c == 127)
		return false;
	//input type
	switch (InputType)
	{
	case ELGUITextInputType::Standard:
		return true;
		break;
	case ELGUITextInputType::IntegerNumber:
	{
		if (c >= '0' && c <= '9')
		{
			if (CaretPositionIndex == 0)
			{
				if (StringContainsChar('-', Text, Text.Len()))
				{
					return false;
				}
			}
			return true;
		}
		if (c == '-')
		{
			if (CaretPositionIndex == 0 && !StringContainsChar('-', Text, Text.Len()))
			{
				return true;
			}
		}
		return false;
	}
		break;
	case ELGUITextInputType::DecimalNumber:
	{
		if (c >= '0' && c <= '9')
		{
			if (CaretPositionIndex == 0)
			{
				if (StringContainsChar('-', Text, Text.Len()))
				{
					return false;
				}
			}
			return true;
		}
		if (c == '.')
		{
			if (StringContainsChar('.', Text, Text.Len()))
			{
				return false;
			}
			else
			{
				if (CaretPositionIndex == 0)
				{
					if (!StringContainsChar('-', Text, Text.Len()))
					{
						return true;
					}
					else
					{
						return false;
					}
				}
				return true;
			}
		}
		if (c == '-')
		{
			if (CaretPositionIndex == 0 && !StringContainsChar('-', Text, Text.Len()))
			{
				return true;
			}
		}
		return false;
	}
		break;
	case ELGUITextInputType::Alphanumeric:
	{
		if (c >= 'A' && c <= 'Z') return true;
		if (c >= 'a' && c <= 'z') return true;
		if (c >= '0' && c <= '9') return true;
		return false;
	}
	break;
	case ELGUITextInputType::EmailAddress:
	{
		if (c >= 'A' && c <= 'Z') return true;
		if (c >= 'a' && c <= 'z') return true;
		if (c >= '0' && c <= '9') return true;
		if (c == '@')
		{
			return !StringContainsChar('@', Text, Text.Len());
		}
		static FString kEmailSpecialCharacters = "!#$%&'*+-/=?^_`{|}~";
		if (StringContainsChar(c, kEmailSpecialCharacters, kEmailSpecialCharacters.Len()))
		{
			return false;
		}
		if (c == '.')
		{
			auto LastChar = (Text.Len() > 0) ? Text[FMath::Clamp(CaretPositionIndex, 0, Text.Len() - 1)] : ' ';
			auto NextChar = (Text.Len() > 0) ? Text[FMath::Clamp(CaretPositionIndex + 1, 0, Text.Len() - 1)] : '\n';
			if (LastChar != '.' && NextChar != '.')
				return true;
			else
				return false;
		}
		return false;
	}
	break;
	case ELGUITextInputType::Password:
		//handled when UpdateUITextComponent
		return true;
		break;
	case ELGUITextInputType::CustomFunction:
	{
		if (CustomInputTypeFunction.IsBound())
		{
			auto TempText = Text;
			auto TempCaretPositionIndex = CaretPositionIndex;
			if (SelectionPropertyArray.Num() != 0)//delete selection frist
			{
				int32 startIndex = PressCaretPositionIndex > TempCaretPositionIndex ? TempCaretPositionIndex : PressCaretPositionIndex;
				TempText.RemoveAt(startIndex, FMath::Abs(TempCaretPositionIndex - PressCaretPositionIndex));
				TempCaretPositionIndex = PressCaretPositionIndex > TempCaretPositionIndex ? TempCaretPositionIndex : PressCaretPositionIndex;
			}
			TempText.InsertAt(TempCaretPositionIndex, c);

			return CustomInputTypeFunction.Execute(TempText, TempCaretPositionIndex);
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d InputType use CustomFunction but not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return true;
		}
	}
		break;
	case ELGUITextInputType::Custom:
	{
		if (IsValid(CustomValidation))
		{
			auto TempText = Text;
			auto TempCaretPositionIndex = CaretPositionIndex;
			if (SelectionPropertyArray.Num() != 0)//delete selection frist
			{
				int32 startIndex = PressCaretPositionIndex > TempCaretPositionIndex ? TempCaretPositionIndex : PressCaretPositionIndex;
				TempText.RemoveAt(startIndex, FMath::Abs(TempCaretPositionIndex - PressCaretPositionIndex));
				TempCaretPositionIndex = PressCaretPositionIndex > TempCaretPositionIndex ? TempCaretPositionIndex : PressCaretPositionIndex;
			}
			TempText.InsertAt(TempCaretPositionIndex, c);

			return CustomValidation->OnValidateInput(this, TempText, TempCaretPositionIndex);
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d InputType use CustomValidation, but the object is not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return true;
		}
	}
	break;
	}
	////new line and tab
	//if (c == '\n' || c == '\t')
	//	return true;
	return true;
}
bool UUITextInputComponent::DeleteSelection(bool InFireEvent)
{
	if (bReadOnly)return false;
	if (SelectionPropertyArray.Num() != 0)//delete selection frist
	{
		int32 startIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
		Text.RemoveAt(startIndex, FMath::Abs(CaretPositionIndex - PressCaretPositionIndex));
		CaretPositionIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
		UpdateAfterTextChange(InFireEvent);
		return true;
	}
	return false;
}
void UUITextInputComponent::InsertCharAtCaretPosition(TCHAR c)
{
	if (bReadOnly)return;
	TextActor->GetUIText()->SetText(FText::FromString(GetReplaceText()));
	auto CharIndex = TextActor->GetUIText()->GetCharIndexByCaretIndex(CaretPositionIndex);
	Text.InsertAt(CharIndex, c);
	TextActor->GetUIText()->SetText(FText::FromString(GetReplaceText()));
	CaretPositionIndex = TextActor->GetUIText()->GetCaretIndexByCharIndex(CharIndex) + 1;
	PressCaretPositionIndex = CaretPositionIndex;
}
void UUITextInputComponent::InsertStringAtCaretPosition(const FString& value)
{
	if (bReadOnly)return;
	TextActor->GetUIText()->SetText(FText::FromString(GetReplaceText()));
	auto CharIndex = TextActor->GetUIText()->GetCharIndexByCaretIndex(CaretPositionIndex);
	Text.InsertAt(CharIndex, value);
	CharIndex += value.Len();
	TextActor->GetUIText()->SetText(FText::FromString(GetReplaceText()));
	CaretPositionIndex = TextActor->GetUIText()->GetCaretIndexByCharIndex(CharIndex) + 1;
	PressCaretPositionIndex = CaretPositionIndex;
}

void UUITextInputComponent::BackSpace()
{
	if (bReadOnly)return;
	if (SelectionPropertyArray.Num() == 0)//no selection mask, use caret
	{
		if (CaretPositionIndex > 0)
		{
			CaretPositionIndex--;
			TextActor->GetUIText()->SetText(FText::FromString(GetReplaceText()));
			auto CharIndex = TextActor->GetUIText()->GetCharIndexByCaretIndex(CaretPositionIndex);
			int RemoveCount = 1;
			if (CharIndex + 1 < Text.Len())//not end char, could be rich text, so check delete count
			{
				auto NextCharIndex = TextActor->GetUIText()->GetCharIndexByCaretIndex(CaretPositionIndex + 1);
				RemoveCount = NextCharIndex - CharIndex;
			}
			Text.RemoveAt(CharIndex, RemoveCount);
			UpdateAfterTextChange(true);
			PressCaretPositionIndex = CaretPositionIndex;
		}
	}
	else//selection mask, delete 
	{
		TextActor->GetUIText()->SetText(FText::FromString(GetReplaceText()));
		auto CharIndexAtPressCaretPosition = TextActor->GetUIText()->GetCharIndexByCaretIndex(PressCaretPositionIndex);
		auto CharIndexAtCaretPosition = TextActor->GetUIText()->GetCharIndexByCaretIndex(CaretPositionIndex);
		int32 TempCharIndex = CharIndexAtPressCaretPosition > CharIndexAtCaretPosition ? CharIndexAtCaretPosition : CharIndexAtPressCaretPosition;
		Text.RemoveAt(TempCharIndex, FMath::Abs(CharIndexAtPressCaretPosition - CharIndexAtCaretPosition));
		CaretPositionIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
		UpdateAfterTextChange(true);
		PressCaretPositionIndex = CaretPositionIndex;
	}
}
void UUITextInputComponent::ForwardSpace()
{
	if (bReadOnly)return;
	if (SelectionPropertyArray.Num() == 0)//no selection mask, use caret
	{
		TextActor->GetUIText()->SetText(FText::FromString(GetReplaceText()));
		auto CharIndex = TextActor->GetUIText()->GetCharIndexByCaretIndex(CaretPositionIndex);
		int RemoveCount = 1;
		if (CharIndex + 1 < Text.Len())//not end char, could be rich text, so check delete count
		{
			auto NextCharIndex = TextActor->GetUIText()->GetCharIndexByCaretIndex(CaretPositionIndex + 1);
			RemoveCount = NextCharIndex - CharIndex;
		}
		if (CharIndex < Text.Len() && CharIndex + RemoveCount <= Text.Len())
		{
			Text.RemoveAt(CharIndex, RemoveCount);
			UpdateAfterTextChange(true);
			PressCaretPositionIndex = CaretPositionIndex;
		}
	}
	else//selection mask, delete 
	{
		TextActor->GetUIText()->SetText(FText::FromString(GetReplaceText()));
		auto CharIndexAtPressCaretPosition = TextActor->GetUIText()->GetCharIndexByCaretIndex(PressCaretPositionIndex);
		auto CharIndexAtCaretPosition = TextActor->GetUIText()->GetCharIndexByCaretIndex(CaretPositionIndex);
		int32 TempCharIndex = CharIndexAtPressCaretPosition > CharIndexAtCaretPosition ? CharIndexAtCaretPosition : CharIndexAtPressCaretPosition;
		Text.RemoveAt(TempCharIndex, FMath::Abs(CharIndexAtPressCaretPosition - CharIndexAtCaretPosition));
		CaretPositionIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
		UpdateAfterTextChange(true);
		PressCaretPositionIndex = CaretPositionIndex;
	}
}
void UUITextInputComponent::Copy()
{
	if (InputType == ELGUITextInputType::Password
		|| DisplayType == ELGUITextInputDisplayType::Password
		)return;//not allow copy password
	if (SelectionPropertyArray.Num() != 0)//have selection
	{
		TextActor->GetUIText()->SetText(FText::FromString(Text));
		auto CharIndexAtPressCaretPosition = TextActor->GetUIText()->GetCharIndexByCaretIndex(PressCaretPositionIndex);
		auto CharIndexAtCaretPosition = TextActor->GetUIText()->GetCharIndexByCaretIndex(CaretPositionIndex);
		int32 TempCharIndex = CharIndexAtPressCaretPosition > CharIndexAtCaretPosition ? CharIndexAtCaretPosition : CharIndexAtPressCaretPosition;
		auto CopyText = Text.Mid(TempCharIndex, FMath::Abs(CharIndexAtPressCaretPosition - CharIndexAtCaretPosition));
		FPlatformApplicationMisc::ClipboardCopy(*CopyText);
		
		UpdateUITextComponent();
	}
}
void UUITextInputComponent::Paste()
{
	if (bReadOnly)return;
	FString pasteString;
	FPlatformApplicationMisc::ClipboardPaste(pasteString);
	if (pasteString.Len() <= 0)return;
	pasteString.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
	if (!bAllowMultiLine)
	{
		for (int i = 0; i < pasteString.Len(); i++)
		{
			if (pasteString[i] == '\n' || pasteString[i] == '\r')
			{
				pasteString[i] = ' ';
			}
		}
	}

	bool bAnyDeleted = DeleteSelection(false);
	FString verifiedString;
	for (int i = 0; i < pasteString.Len(); i++)
	{
		TCHAR c = pasteString[i];
		if (IsValidChar(c))
		{
			verifiedString.AppendChar(c);
		}
	}
	if (verifiedString.Len() > 0)
	{
		InsertStringAtCaretPosition(verifiedString);
	}
	if (verifiedString.Len() > 0 || bAnyDeleted)
	{
		UpdateAfterTextChange(true);
	}
}
void UUITextInputComponent::Cut()
{
	if (bReadOnly)return;
	if (InputType == ELGUITextInputType::Password
		|| DisplayType == ELGUITextInputDisplayType::Password
		)return;//not allow copy password
	if (SelectionPropertyArray.Num() != 0)//have selection
	{
		Copy();
		DeleteSelection(true);
	}
}
void UUITextInputComponent::SelectAll()
{
	CaretPositionIndex = Text.Len() * 2;//just a large enough value to make sure it is the last caret
	PressCaretPositionIndex = 0;
	UpdateUITextComponent();
	TextActor->GetUIText()->GetSelectionProperty(PressCaretPositionIndex, CaretPositionIndex, SelectionPropertyArray);
	UpdateCaretPosition(false);
	UpdateSelection();
}

bool UUITextInputComponent::VerifyAndInsertStringAtCaretPosition(const FString& value)
{
	if (bReadOnly)return false;
	FString verifiedString;
	for (int i = 0; i < value.Len(); i++)
	{
		TCHAR c = value[i];
		if (IsValidChar(c))
		{
			verifiedString.AppendChar(c);
		}
	}
	if (verifiedString.Len() > 0)
	{
		InsertStringAtCaretPosition(verifiedString);
		UpdateAfterTextChange(true);
		return true;
	}
	return false;
}
bool UUITextInputComponent::VerifyAndInsertCharAtCaretPosition(TCHAR value)
{
	if (bReadOnly)return false;
	if (IsValidChar(value))
	{
		InsertCharAtCaretPosition(value);
		UpdateAfterTextChange(true);
		return true;
	}
	return false;
}

void UUITextInputComponent::UpdateAfterTextChange(bool InFireEvent)
{
	UpdateUITextComponent();
	UpdateCaretPosition();
	UpdatePlaceHolderComponent();
	if (InFireEvent)
	{
		FireOnValueChangeEvent();
	}
}

FString UUITextInputComponent::GetReplaceText()const
{
	FString replaceText;
	if (InputType == ELGUITextInputType::Password
		|| DisplayType == ELGUITextInputDisplayType::Password)
	{
		int len = Text.Len();
		replaceText.Reset(len);
		auto psChar = PasswordChar[0];
		for (int i = 0; i < len; i++)
		{
			replaceText.AppendChar(psChar);
		}
	}
	else
	{
		replaceText = Text;
	}
	return replaceText;
}

void UUITextInputComponent::MoveCaret(int32 moveType, bool withSelection)
{
	auto uiText = TextActor->GetUIText();
	auto originText = uiText->GetText();
	auto replaceText = GetReplaceText();
	uiText->SetText(FText::FromString(replaceText));

	auto CaretPosition3D = CaretObject->GetRelativeLocation();
	auto CaretPosition = FVector2f(CaretPosition3D.Y, CaretPosition3D.Z);
	if (uiText->MoveCaret(moveType, CaretPositionIndex, CaretPositionLineIndex, CaretPosition))
	{
		UpdateUITextComponent();

		UpdateCaretPosition(!withSelection);

		if (withSelection)
		{
			uiText->GetSelectionProperty(PressCaretPositionIndex - VisibleCaretStartIndex, CaretPositionIndex - VisibleCaretStartIndex, SelectionPropertyArray);
			UpdateSelection();
		}
		else
		{
			PressCaretPositionIndex = CaretPositionIndex;
		}
	}
	else
	{
		uiText->SetText(originText);
	}
}

void UUITextInputComponent::FireOnValueChangeEvent()
{
	OnValueChangeCPP.Broadcast(Text);
	OnValueChange.FireEvent(Text);
}
void UUITextInputComponent::UpdateUITextComponent()
{
	if (TextActor != nullptr)
	{
		auto uiText = TextActor->GetUIText();
		if (!uiText->GetRenderCanvas())return;//need render canvas to calculate geometry
		auto replaceText = GetReplaceText();
		//set to full text
		uiText->SetText(FText::FromString(replaceText));
		
		if (bAllowMultiLine)//multi line, handle out of range chars
		{
			//recalculate MaxVisibleLineCount
			if (OverflowType == ELGUITextInputOverflowType::ClampContent)
			{
				auto SingleLineHeight = uiText->GetFont()->GetLineHeight(uiText->GetFontSize());
				MaxLineCount = (int)(uiText->GetHeight()
					/ (SingleLineHeight + uiText->GetFontSpace().Y));
			}

			int VisibleCharStartIndex = 0;
			int VisibleCharCount = 0;
			if (uiText->GetVisibleCharRangeForMultiLine(CaretPositionIndex, CaretPositionLineIndex, VisibleCaretStartLineIndex, VisibleCaretStartIndex, MaxLineCount, VisibleCharStartIndex, VisibleCharCount))
			{
				replaceText = replaceText.Mid(VisibleCharStartIndex, VisibleCharCount);
			}
		}
		else//single line, handle out of range chars
		{
			float maxWidth = OverflowType == ELGUITextInputOverflowType::ClampContent
				? uiText->GetWidth()
				: MaxLineWidth
				;

			int VisibleCharStartIndex = 0;
			int VisibleCharCount = 0;
			if (uiText->GetVisibleCharRangeForSingleLine(CaretPositionIndex, VisibleCaretStartIndex, maxWidth, VisibleCharStartIndex, VisibleCharCount))
			{
				replaceText = replaceText.Mid(VisibleCharStartIndex, VisibleCharCount);
			}
		}
		uiText->SetText(FText::FromString(replaceText));
		
#if WITH_EDITOR
		if (auto world = this->GetWorld())
		{
			if (world->WorldType == EWorldType::Editor || world->WorldType == EWorldType::EditorPreview)
			{
				uiText->EditorForceUpdate();
			}
		}
#endif
	}
}
void UUITextInputComponent::UpdatePlaceHolderComponent()
{
	if (bInputActive || !Text.IsEmpty())
	{
		if (PlaceHolderActor.IsValid())
		{
			PlaceHolderActor->GetUIItem()->SetIsUIActive(false);
		}
	}
	else
	{
		if (PlaceHolderActor.IsValid())
		{
			PlaceHolderActor->GetUIItem()->SetIsUIActive(true);
		}
	}
}

void UUITextInputComponent::UpdateCaretPosition(bool InHideSelection)
{
	if (!bInputActive)
	{
		if (CaretObject.IsValid())
		{
			CaretObject->SetIsUIActive(false);
		}
	}
	else
	{
		FVector2f caretPos;
		int tempCaretPositionLineIndex = 0;
		int tempVisibleCaretStartIndex = 0;
		int tempCaretPositionIndex = CaretPositionIndex - VisibleCaretStartIndex;
		TextActor->GetUIText()->FindCaretByIndex(tempCaretPositionIndex, caretPos, tempCaretPositionLineIndex, tempVisibleCaretStartIndex);
		CaretPositionLineIndex = tempCaretPositionLineIndex + VisibleCaretStartLineIndex;

		UpdateCaretPosition(caretPos, InHideSelection);
	}
}
void UUITextInputComponent::UpdateCaretPosition(FVector2f InCaretPosition, bool InHideSelection)
{
	if (!TextActor.IsValid())return;
	if (!CaretObject.IsValid())
	{
		auto caretActor = this->GetWorld()->SpawnActor<AUISpriteActor>();
		caretActor->AttachToActor(TextActor.Get(), FAttachmentTransformRules::KeepRelativeTransform);
#if WITH_EDITOR
		caretActor->SetActorLabel(TEXT("Caret"));
#endif
		CaretObject = caretActor->GetUISprite();
		auto uiText = TextActor->GetUIText();
		CaretObject->SetWidth(CaretWidth);
		CaretObject->SetHeight(uiText->GetFontSize());
		CaretObject->SetColor(CaretColor);
		CaretObject->SetSprite(ULGUISpriteData::GetDefaultWhiteSolid(), false);
	}
	CaretObject->SetRelativeLocation(FVector(0, InCaretPosition.X, InCaretPosition.Y));
	CaretObject->SetIsUIActive(true);
	
	//force display caret
	NextCaretBlinkTime = 0.8f;
	ElapseTime = 0.0f;
	CaretObject->SetAlpha(1.0f);

	if (InHideSelection) HideSelectionMask();//if use caret, then hide selection mask
}
void UUITextInputComponent::UpdateSelection()
{
	if (!TextActor.IsValid())return;
	int32 createdSelectionMaskCount = SelectionMaskObjectArray.Num();
	if (SelectionPropertyArray.Num() > createdSelectionMaskCount)//need more selection mask object
	{
		int32 needToCreateSelectionMaskCount = SelectionPropertyArray.Num() - createdSelectionMaskCount;
		for (int32 i = 0; i < needToCreateSelectionMaskCount; i++)
		{
			auto spriteActor = this->GetWorld()->SpawnActor<AUISpriteActor>();
			spriteActor->AttachToActor(TextActor.Get(), FAttachmentTransformRules::KeepRelativeTransform);
#if WITH_EDITOR
			spriteActor->SetActorLabel(FString::Printf(TEXT("Selection%d"), i + createdSelectionMaskCount));
#endif
			auto uiSprite = spriteActor->GetUISprite();
			auto uiText = TextActor->GetUIText();
			uiSprite->SetHeight(uiText->GetFontSize());
			uiSprite->SetColor(SelectionColor);
			uiSprite->SetPivot(FVector2D(0, 0.5f));
			uiSprite->SetSprite(ULGUISpriteData::GetDefaultWhiteSolid(), false);
			SelectionMaskObjectArray.Add(uiSprite);
		}
	}
	else if (SelectionPropertyArray.Num() < createdSelectionMaskCount)//hide extra selection mask object
	{
		int32 needToHideTextureCount = createdSelectionMaskCount - SelectionPropertyArray.Num();
		for (int32 i = 0; i < needToHideTextureCount; i++)
		{
			auto uiSprite = SelectionMaskObjectArray[i + SelectionPropertyArray.Num()];
			if (uiSprite.IsValid())
			{
				uiSprite->SetIsUIActive(false);
			}
		}
	}

	for (int32 i = 0; i < SelectionPropertyArray.Num(); i++)
	{
		auto uiSprite = SelectionMaskObjectArray[i];
		if (uiSprite.IsValid())
		{
			uiSprite->SetIsUIActive(true);
			auto& selectionProperty = SelectionPropertyArray[i];
			uiSprite->SetRelativeLocation(FVector(0, selectionProperty.Pos.X, selectionProperty.Pos.Y));
			uiSprite->SetWidth(selectionProperty.Size);
		}
	}
}
void UUITextInputComponent::HideSelectionMask()
{
	for (int i = 0; i < SelectionMaskObjectArray.Num(); i++)
	{
		auto uiSprite = SelectionMaskObjectArray[i];
		if (uiSprite.IsValid())
		{
			SelectionMaskObjectArray[i]->SetIsUIActive(false);
		}
	}
	SelectionPropertyArray.Reset();//clear selection mask
	//TextInputMethodContext->SetSelectionRange(0, 0, ITextInputMethodContext::ECaretPosition::Beginning);
}

void UUITextInputComponent::OnUIActiveInHierachy(bool ativeOrInactive)
{
	Super::OnUIActiveInHierachy(ativeOrInactive);
	DeactivateInput();
}
void UUITextInputComponent::OnUIInteractionStateChanged(bool interactableOrNot)
{
	Super::OnUIInteractionStateChanged(interactableOrNot);
	DeactivateInput();
}
void UUITextInputComponent::OnUIDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)
{
	Super::OnUIDimensionsChanged(horizontalPositionChanged, verticalPositionChanged, widthChanged, heightChanged);
	if (widthChanged || heightChanged)
	{
		this->UpdateAfterTextChange(false);//if size change, need to recalculate text input area
	}
}

bool UUITextInputComponent::OnPointerEnter_Implementation(ULGUIPointerEventData* eventData)
{
	Super::OnPointerEnter_Implementation(eventData);
	if (bAutoActivateInputWhenNavigateIn)
	{
		if (eventData->inputType == ELGUIPointerInputType::Navigation)
		{
			ActivateInput(eventData);
		}
	}
	if (APlayerController* pc = this->GetWorld()->GetFirstPlayerController())
	{
		pc->CurrentMouseCursor = EMouseCursor::TextEditBeam;
	}
	return AllowEventBubbleUp;
}
bool UUITextInputComponent::OnPointerExit_Implementation(ULGUIPointerEventData* eventData)
{
	Super::OnPointerExit_Implementation(eventData);
	if (APlayerController* pc = this->GetWorld()->GetFirstPlayerController())
	{
		pc->CurrentMouseCursor = EMouseCursor::Default;
	}
	return AllowEventBubbleUp;
}
bool UUITextInputComponent::OnPointerSelect_Implementation(ULGUIBaseEventData* eventData)
{
	Super::OnPointerSelect_Implementation(eventData);
	//ActivateInput();//handled at PointerClick
	return AllowEventBubbleUp;
}
bool UUITextInputComponent::OnPointerDeselect_Implementation(ULGUIBaseEventData* eventData)
{
	Super::OnPointerDeselect_Implementation(eventData);
	DeactivateInput();
	return AllowEventBubbleUp;
}
bool UUITextInputComponent::OnPointerClick_Implementation(ULGUIPointerEventData* eventData)
{
	if (!bInputActive)//need active input
	{
		ActivateInput();
	}
	return AllowEventBubbleUp;
}
bool UUITextInputComponent::OnPointerBeginDrag_Implementation(ULGUIPointerEventData* eventData)
{
	if (bInputActive)
	{
		return AllowEventBubbleUp;
	}
	else
	{
		return true;
	}
}
bool UUITextInputComponent::OnPointerDrag_Implementation(ULGUIPointerEventData* eventData)
{
	if (bInputActive)
	{
		if (TextActor != nullptr)
		{
			FVector2f caretPosition;
			int tempCaretPositionLineIndex;
			TextActor->GetUIText()->FindCaretByWorldPosition(eventData->GetWorldPointInPlane(), caretPosition, tempCaretPositionLineIndex, CaretPositionIndex);
			auto displayCaretCount = TextActor->GetUIText()->GetLastCaret() + 1;

			//@todo:caret move speed depend on drag distance
			if (CaretPositionIndex == 0)//caret position at left most
			{
				CaretPositionIndex = VisibleCaretStartIndex - 1;//move caret to left
				CaretPositionIndex = FMath::Max(CaretPositionIndex, 0);
				if (CaretPositionIndex < VisibleCaretStartIndex)
				{
					if (CaretPositionLineIndex > 0)
					{
						CaretPositionLineIndex--;
					}
				}
			}
			else if (CaretPositionIndex + 1 >= displayCaretCount)//caret position at right most
			{
				CaretPositionIndex = CaretPositionIndex + VisibleCaretStartIndex + 1;//move caret to right
				CaretPositionIndex = FMath::Min(CaretPositionIndex, VisibleCaretStartIndex + displayCaretCount);
				if (CaretPositionIndex - VisibleCaretStartIndex > displayCaretCount)//if caret is more than visible text
				{
					CaretPositionLineIndex++;
				}
			}
			else//not drag out-of-range
			{
				CaretPositionIndex += VisibleCaretStartIndex;
			}

			UpdateUITextComponent();
			//selectionStartCaretIndex may out of range, need clamp.
			TextActor->GetUIText()->GetSelectionProperty(PressCaretPositionIndex - VisibleCaretStartIndex, CaretPositionIndex - VisibleCaretStartIndex, SelectionPropertyArray);
			UpdateSelection();
			UpdateCaretPosition(false);
		}
		return AllowEventBubbleUp;
	}
	else
	{
		return true;
	}
}
bool UUITextInputComponent::OnPointerEndDrag_Implementation(ULGUIPointerEventData* eventData)
{
	if (bInputActive)
	{
		return AllowEventBubbleUp;
	}
	else
	{
		return true;
	}
}
bool UUITextInputComponent::OnPointerDown_Implementation(ULGUIPointerEventData* eventData)
{
	Super::OnPointerDown_Implementation(eventData);
	if (bInputActive)//if already active, then put caret position at mouse position
	{
		if (TextActor != nullptr)
		{
			//caret position when press, UIText space
			auto PressCaretPosition = FVector2f(0, 0);
			TextActor->GetUIText()->FindCaretByWorldPosition(eventData->GetWorldPointInPlane(), PressCaretPosition, PressCaretPositionLineIndex, PressCaretPositionIndex);
			PressCaretPositionIndex = PressCaretPositionIndex + VisibleCaretStartIndex;
			CaretPositionIndex = PressCaretPositionIndex;
			PressCaretPositionLineIndex = PressCaretPositionLineIndex + VisibleCaretStartLineIndex;
			CaretPositionLineIndex = PressCaretPositionLineIndex;
			UpdateUITextComponent();
			UpdateCaretPosition(PressCaretPosition);
		}
	}

	return AllowEventBubbleUp;
}
bool UUITextInputComponent::OnPointerUp_Implementation(ULGUIPointerEventData* eventData)
{
	Super::OnPointerUp_Implementation(eventData);
	return AllowEventBubbleUp;
}
void UUITextInputComponent::ActivateInput(ULGUIPointerEventData* eventData)
{
	if (TextActor == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[UUITextInputComponent::ActivateInput]TextActor is null!"));
		return;
	}
	else
	{
		UpdateUITextComponent();
	}
	if (bInputActive)
	{
		//if already active, then update caret position
		TextActor->GetUIText()->SetText(FText::FromString(GetReplaceText()));
		CaretPositionIndex = TextActor->GetUIText()->GetLastCaret();
		UpdateUITextComponent();
		UpdateCaretPosition();
		return;
	}
	if (FPlatformApplicationMisc::RequiresVirtualKeyboard())
	{
		if (!VirtualKeyboardEntry.IsValid())
		{
			VirtualKeyboardEntry = FVirtualKeyboardEntry::Create(this);
		}
		FSlateApplication::Get().ShowVirtualKeyboard(true, 0, VirtualKeyboardEntry);
	}
	else
	{
		ITextInputMethodSystem* const TextInputMethodSystem = FSlateApplication::Get().GetTextInputMethodSystem();
		if (TextInputMethodSystem)
		{
			if (!TextInputMethodContext.IsValid())
			{
				TextInputMethodContext = FTextInputMethodContext::Create(this);
			}
			TextInputMethodChangeNotifier = TextInputMethodSystem->RegisterContext(TextInputMethodContext.ToSharedRef());
			TextInputMethodSystem->ActivateContext(TextInputMethodContext.ToSharedRef());
		}
		if (TextInputMethodChangeNotifier.IsValid())
		{
			TextInputMethodChangeNotifier->NotifyLayoutChanged(ITextInputMethodChangeNotifier::ELayoutChangeType::Changed);
		}
	}
	bInputActive = true;
	//caret and selection
	if (Text.Len() == 0)//if no text, use caret
	{
		CaretPositionIndex = 0;
		PressCaretPositionIndex = 0;
		UpdateUITextComponent();
		UpdateCaretPosition();
	}
	else if (bSelectAllWhenActivateInput)//select all
	{
		SelectAll();
	}

	BindKeys();
	UpdatePlaceHolderComponent();
	//set is selected
	if (auto eventSystem = ULGUIEventSystem::GetLGUIEventSystemInstance(this))
	{
		if (CheckRootUIComponent())
		{
			if (IsValid(eventData))
			{
				eventSystem->SetSelectComponent(RootUIComp.Get(), eventData, eventData->pressComponentEventFireType);
			}
			else
			{
				eventSystem->SetSelectComponentWithDefault(RootUIComp.Get());
			}
		}
	}
	//fire event
	OnInputActivateCPP.Broadcast(bInputActive);
	OnInputActivate.FireEvent(bInputActive);
}

void UUITextInputComponent::BindKeys()
{
	if (GetOwner()->InputComponent == nullptr)
	{
		GetOwner()->AutoReceiveInput = EAutoReceiveInput::Player0;
		GetOwner()->PreInitializeComponents();
	}

	static TArray<FKey> AllKeys = {
	EKeys::BackSpace,
	EKeys::Tab,
	EKeys::Enter,
	EKeys::Pause,

	EKeys::CapsLock,
	EKeys::Escape,
	EKeys::SpaceBar,
	EKeys::PageUp,
	EKeys::PageDown,
	EKeys::End,
	EKeys::Home,

	EKeys::Left,
	EKeys::Up,
	EKeys::Right,
	EKeys::Down,

	EKeys::Insert,
	EKeys::Delete,

	EKeys::Zero,
	EKeys::One,
	EKeys::Two,
	EKeys::Three,
	EKeys::Four,
	EKeys::Five,
	EKeys::Six,
	EKeys::Seven,
	EKeys::Eight,
	EKeys::Nine,

	EKeys::A,
	EKeys::B,
	EKeys::C,
	EKeys::D,
	EKeys::E,
	EKeys::F,
	EKeys::G,
	EKeys::H,
	EKeys::I,
	EKeys::J,
	EKeys::K,
	EKeys::L,
	EKeys::M,
	EKeys::N,
	EKeys::O,
	EKeys::P,
	EKeys::Q,
	EKeys::R,
	EKeys::S,
	EKeys::T,
	EKeys::U,
	EKeys::V,
	EKeys::W,
	EKeys::X,
	EKeys::Y,
	EKeys::Z,

	EKeys::NumPadZero,
	EKeys::NumPadOne,
	EKeys::NumPadTwo,
	EKeys::NumPadThree,
	EKeys::NumPadFour,
	EKeys::NumPadFive,
	EKeys::NumPadSix,
	EKeys::NumPadSeven,
	EKeys::NumPadEight,
	EKeys::NumPadNine,

	EKeys::Multiply,
	EKeys::Add,
	EKeys::Subtract,
	EKeys::Decimal,
	EKeys::Divide,

	EKeys::LeftShift,
	EKeys::RightShift,
	EKeys::LeftControl,
	EKeys::RightControl,
	EKeys::LeftAlt,
	EKeys::RightAlt,
	EKeys::LeftCommand,
	EKeys::RightCommand,

	EKeys::Semicolon,
	EKeys::Equals,
	EKeys::Comma,
	EKeys::Underscore,
	EKeys::Hyphen,
	EKeys::Period,
	EKeys::Slash,
	EKeys::Tilde,
	EKeys::LeftBracket,
	EKeys::Backslash,
	EKeys::RightBracket,
	EKeys::Apostrophe,

	EKeys::Ampersand,
	EKeys::Asterix,
	EKeys::Caret,
	EKeys::Colon,
	EKeys::Dollar,
	EKeys::Exclamation,
	EKeys::LeftParantheses,
	EKeys::RightParantheses,
	EKeys::Quote,
	};

	auto inputComp = GetOwner()->InputComponent;
	for (auto& Key : AllKeys)
	{
		if (!IgnoreKeys.Contains(Key))
		{
			inputComp->BindKey(Key, EInputEvent::IE_Pressed, this, &UUITextInputComponent::AnyKeyPressed);
			inputComp->BindKey(Key, EInputEvent::IE_Repeat, this, &UUITextInputComponent::AnyKeyPressed);
		}
	}
}
void UUITextInputComponent::UnbindKeys()
{
	if (GetOwner()->InputComponent == nullptr)
		return;
	GetOwner()->InputComponent->DestroyComponent();
	GetOwner()->InputComponent = nullptr;
}
void UUITextInputComponent::DeactivateInput(bool InFireEvent)
{
	if (!bInputActive)return;
	ITextInputMethodSystem* const TextInputMethodSystem = FSlateApplication::IsInitialized() ? FSlateApplication::Get().GetTextInputMethodSystem() : nullptr;
	if (TextInputMethodSystem)
	{
		if (TextInputMethodContext.IsValid())
		{
			TSharedRef<FTextInputMethodContext> TextInputMethodContextRef = TextInputMethodContext.ToSharedRef();

			if (TextInputMethodSystem->IsActiveContext(TextInputMethodContextRef))
			{
				TextInputMethodSystem->DeactivateContext(TextInputMethodContextRef);
			}

			TextInputMethodSystem->UnregisterContext(TextInputMethodContextRef);
		}
	}
	if (FSlateApplication::IsInitialized() && FPlatformApplicationMisc::RequiresVirtualKeyboard())
	{
		FSlateApplication::Get().ShowVirtualKeyboard(false, 0);
	}
	bInputActive = false;
	//hide caret
	if (CaretObject.IsValid())
	{
		CaretObject->SetIsUIActive(false);
	}
	//hide selection
	HideSelectionMask();

	UnbindKeys();
	UpdatePlaceHolderComponent();
	//fire event
	if (InFireEvent)
	{
		OnInputActivateCPP.Broadcast(bInputActive);
		OnInputActivate.FireEvent(bInputActive);
	}
}
UUIText* UUITextInputComponent::GetTextComponent()const
{
	if (TextActor != nullptr)
	{
		return TextActor->GetUIText();
	}
	return nullptr;
}
const FString& UUITextInputComponent::GetText()const
{
	return Text;
}
bool UUITextInputComponent::SetText(const FString& InText, bool InFireEvent)
{
	if (Text != InText)
	{
		FString TempText;
		for (int i = 0; i < InText.Len(); i++)
		{
			TCHAR c = InText[i];
			if (IsValidChar(c))
			{
				TempText.AppendChar(c);
			}
		}
		if (bAllowMultiLine)
		{
			Text = TempText;
		}
		else
		{
			Text = TempText.Replace(TEXT("\n"), TEXT("")).Replace(TEXT("\t"), TEXT(""));
		}
		
		CaretPositionIndex = 0;
		UpdateAfterTextChange(InFireEvent);
	}
	return true;
}
void UUITextInputComponent::SetInputType(ELGUITextInputType newValue)
{
	if (InputType != newValue)
	{
		InputType = newValue;
		CaretPositionIndex = 0;
		UpdateUITextComponent();
	}
}
void UUITextInputComponent::SetCustomValidation(UUITextInputCustomValidation* value)
{
	if (CustomValidation != value)
	{
		CustomValidation = value;
		if (InputType == ELGUITextInputType::Custom)
		{
			CaretPositionIndex = 0;
			UpdateUITextComponent();
		}
	}
}
void UUITextInputComponent::SetDisplayType(ELGUITextInputDisplayType newValue)
{
	if (DisplayType != newValue)
	{
		DisplayType = newValue;
		CaretPositionIndex = 0;
		UpdateUITextComponent();
	}
}
void UUITextInputComponent::SetPasswordChar(const FString& value)
{
	if (PasswordChar != value)
	{
		if (value.Len() != 1)
		{
			return;
		}
		PasswordChar = value;
		if (InputType == ELGUITextInputType::Password || DisplayType == ELGUITextInputDisplayType::Password)
		{
			UpdateUITextComponent();
		}
	}
}
void UUITextInputComponent::SetAllowMultiLine(bool value)
{
	if (bAllowMultiLine != value)
	{
		bAllowMultiLine = value;
	}
}
void UUITextInputComponent::SetMultiLineSubmitFunctionKeys(const TArray<FKey>& value)
{
	MultiLineSubmitFunctionKeys = value;
}
void UUITextInputComponent::SetPlaceHolderActor(class AUIBaseActor* value)
{
	PlaceHolderActor = value;
}
void UUITextInputComponent::SetCaretBlinkRate(float value)
{
	CaretBlinkRate = value;
}
void UUITextInputComponent::SetCaretWidth(float value)
{
	if (CaretWidth != value)
	{
		CaretWidth = value;
		if (CaretObject.IsValid())
		{
			CaretObject->SetWidth(CaretWidth);
		}
	}
}
void UUITextInputComponent::SetCaretColor(FColor value)
{
	if (CaretColor != value)
	{
		CaretColor = value;
		if (CaretObject.IsValid())
		{
			CaretObject->SetColor(CaretColor);
		}
	}
}
void UUITextInputComponent::SetSelectionColor(FColor value)
{
	if (SelectionColor != value)
	{
		SelectionColor = value;
		if (SelectionMaskObjectArray.Num() > 0)
		{
			for (auto& Item : SelectionMaskObjectArray)
			{
				if (Item.IsValid())
				{
					Item->SetColor(SelectionColor);
				}
			}
		}
	}
}
void UUITextInputComponent::SetVirtualKeyboradOptions(FVirtualKeyboardOptions value)
{
	VirtualKeyboradOptions = value;
}
void UUITextInputComponent::SetIgnoreKeys(const TArray<FKey>& value)
{
	IgnoreKeys = value;
}
void UUITextInputComponent::SetAutoActivateInputWhenNavigateIn(bool value)
{
	bAutoActivateInputWhenNavigateIn = value;
}
void UUITextInputComponent::SetReadOnly(bool value)
{
	bReadOnly = value;
}

FDelegateHandle UUITextInputComponent::RegisterValueChangeEvent(const FLGUIStringDelegate& InDelegate)
{
	return OnValueChangeCPP.Add(InDelegate);
}
FDelegateHandle UUITextInputComponent::RegisterValueChangeEvent(const TFunction<void(const FString&)>& InFunction)
{
	return OnValueChangeCPP.AddLambda(InFunction);
}
void UUITextInputComponent::UnregisterValueChangeEvent(const FDelegateHandle& InHandle)
{
	OnValueChangeCPP.Remove(InHandle);
}
FLGUIDelegateHandleWrapper UUITextInputComponent::RegisterValueChangeEvent(const FLGUITextInputDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnValueChangeCPP.AddLambda([InDelegate](FString InText) {
		InDelegate.ExecuteIfBound(InText);
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUITextInputComponent::UnregisterValueChangeEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnValueChangeCPP.Remove(InDelegateHandle.DelegateHandle);
}

FDelegateHandle UUITextInputComponent::RegisterSubmitEvent(const FLGUIStringDelegate& InDelegate)
{
	return OnSubmitCPP.Add(InDelegate);
}
FDelegateHandle UUITextInputComponent::RegisterSubmitEvent(const TFunction<void(const FString&)>& InFunction)
{
	return OnSubmitCPP.AddLambda(InFunction);
}
void UUITextInputComponent::UnregisterSubmitEvent(const FDelegateHandle& InHandle)
{
	OnSubmitCPP.Remove(InHandle);
}
FLGUIDelegateHandleWrapper UUITextInputComponent::RegisterSubmitEvent(const FLGUITextInputDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnSubmitCPP.AddLambda([InDelegate](FString InText) {
		InDelegate.ExecuteIfBound(InText);
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUITextInputComponent::UnregisterSubmitEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnSubmitCPP.Remove(InDelegateHandle.DelegateHandle);
}

FDelegateHandle UUITextInputComponent::RegisterInputActivateEvent(const FLGUIBoolDelegate& InDelegate)
{
	return OnInputActivateCPP.Add(InDelegate);
}
FDelegateHandle UUITextInputComponent::RegisterInputActivateEvent(const TFunction<void(bool)>& InDelegate)
{
	return OnInputActivateCPP.AddLambda(InDelegate);
}
void UUITextInputComponent::UnregisterInputActivateEvent(const FDelegateHandle& InHandle)
{
	OnInputActivateCPP.Remove(InHandle);
}
FLGUIDelegateHandleWrapper UUITextInputComponent::RegisterInputActivateEvent(const FLGUIInputActivateDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnInputActivateCPP.AddLambda([InDelegate](bool activate) {
		InDelegate.ExecuteIfBound(activate);
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUITextInputComponent::UnregisterInputActivateEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnInputActivateCPP.Remove(InDelegateHandle.DelegateHandle);
}

void UUITextInputComponent::SetCustomInputTypeFunction(const FLGUITextInputCustomInputTypeDelegate& InFunction)
{
	CustomInputTypeFunction = InFunction;
}
void UUITextInputComponent::SetCustomInputTypeFunction(const TFunction<bool(const FString&, int)>& InFunction)
{
	CustomInputTypeFunction.BindLambda(InFunction);
}
void UUITextInputComponent::SetCustomInputTypeFunction(const FLGUITextInputCustomInputTypeDynamicDelegate& InFunction)
{
	CustomInputTypeFunction.BindLambda([InFunction](const FString& InString, int InStartIndex) {
		if (InFunction.IsBound())
		{
			return InFunction.Execute(InString, InStartIndex);
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[%s].d CustomInputType function not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return false;
		}
	});
}
void UUITextInputComponent::ClearCustomInputTypeFunction()
{
	CustomInputTypeFunction = FLGUITextInputCustomInputTypeDelegate();
}



TSharedRef<UUITextInputComponent::FVirtualKeyboardEntry> UUITextInputComponent::FVirtualKeyboardEntry::Create(UUITextInputComponent* Input)
{
	return MakeShareable(new FVirtualKeyboardEntry(Input));
}
UUITextInputComponent::FVirtualKeyboardEntry::FVirtualKeyboardEntry(UUITextInputComponent* InInput)
{
	InputComp = InInput;
}
void UUITextInputComponent::FVirtualKeyboardEntry::SetTextFromVirtualKeyboard(const FText& InNewText, ETextEntryType TextEntryType)
{
	InputComp->SetText(InNewText.ToString());
}
void UUITextInputComponent::FVirtualKeyboardEntry::SetSelectionFromVirtualKeyboard(int InSelStart, int SelEnd)
{
	//@todo
}
bool UUITextInputComponent::FVirtualKeyboardEntry::GetSelection(int& OutSelStart, int& OutSelEnd)
{
	//check(IsInGameThread());

	//const FTextLocation CursorInteractionPosition = OwnerLayout->CursorInfo.GetCursorInteractionLocation();
	//FTextLocation SelectionLocation = OwnerLayout->SelectionStart.Get(CursorInteractionPosition);
	//FTextSelection Selection(SelectionLocation, CursorInteractionPosition);

	//OutSelStart = Selection.GetBeginning().GetOffset();
	//OutSelEnd = Selection.GetEnd().GetOffset();
	return true;
}
FText UUITextInputComponent::FVirtualKeyboardEntry::GetText() const
{
	return FText::FromString(InputComp->GetText());
}
FText UUITextInputComponent::FVirtualKeyboardEntry::GetHintText() const
{
	if (InputComp->PlaceHolderActor.IsValid())
	{
		if (auto uiText = Cast<UUIText>(InputComp->PlaceHolderActor->GetUIItem()))
		{
			return uiText->GetText();
		}
	}
	return FText::FromString(TEXT(""));
}
EKeyboardType UUITextInputComponent::FVirtualKeyboardEntry::GetVirtualKeyboardType() const
{
	if (InputComp->DisplayType == ELGUITextInputDisplayType::Password)
	{
		return EKeyboardType::Keyboard_Password;
	}
	switch (InputComp->InputType)
	{
	default:
	case ELGUITextInputType::Standard:
		return EKeyboardType::Keyboard_Default;
		break;
	case ELGUITextInputType::Password:
		return EKeyboardType::Keyboard_Password;
		break;
	case ELGUITextInputType::DecimalNumber:
		return EKeyboardType::Keyboard_Number;
		break;
	}
}
FVirtualKeyboardOptions UUITextInputComponent::FVirtualKeyboardEntry::GetVirtualKeyboardOptions() const
{
	return InputComp->VirtualKeyboradOptions;
}
bool UUITextInputComponent::FVirtualKeyboardEntry::IsMultilineEntry() const
{
	return InputComp->bAllowMultiLine;
}


#define LGUI_LOG_TextInputMethodContext 0
TSharedRef<UUITextInputComponent::FTextInputMethodContext> UUITextInputComponent::FTextInputMethodContext::Create(UUITextInputComponent* Input)
{
	return MakeShareable(new FTextInputMethodContext(Input));
}
void UUITextInputComponent::FTextInputMethodContext::Dispose()
{
	if (CachedWindow.IsValid())
	{
		if (IsValid(GEngine))
		{
			if (IsValid(GEngine->GameViewport))
			{
				GEngine->GameViewport->RemoveViewportWidgetContent(CachedWindow.ToSharedRef());
			}
		}
	}
}
UUITextInputComponent::FTextInputMethodContext::FTextInputMethodContext(UUITextInputComponent* InInput)
{
	InputComp = InInput;
}
bool UUITextInputComponent::FTextInputMethodContext::IsReadOnly()
{
#if LGUI_LOG_TextInputMethodContext
	//UE_LOG(LGUI, Log, TEXT("IsReadOnly"));
#endif
	return InputComp->GetReadOnly();
}
uint32 UUITextInputComponent::FTextInputMethodContext::GetTextLength()
{
#if LGUI_LOG_TextInputMethodContext
	UE_LOG(LGUI, Log, TEXT("GetTextLength, Text:%s, Length:%d"), *InputComp->Text, InputComp->Text.Len());
#endif
	return InputComp->Text.Len();
}
void UUITextInputComponent::FTextInputMethodContext::GetSelectionRange(uint32& BeginIndex, uint32& Length, ECaretPosition& OutCaretPosition)
{
	Length = FMath::Abs(InputComp->CaretPositionIndex - InputComp->PressCaretPositionIndex);
	OutCaretPosition = InputComp->PressCaretPositionIndex <= InputComp->CaretPositionIndex ? ECaretPosition::Ending : ECaretPosition::Beginning;
	if (OutCaretPosition == ECaretPosition::Beginning)
	{
		BeginIndex = InputComp->CaretPositionIndex;
	}
	else
	{
		BeginIndex = InputComp->PressCaretPositionIndex;
	}

#if LGUI_LOG_TextInputMethodContext
	UE_LOG(LogTemp, Log, TEXT("GetSelectionRange, BeginIndex:%d, Length:%d, InCaretPosition:%d, Text:%s"), BeginIndex, Length, (int32)OutCaretPosition, *InputComp->Text);
#endif
}
void UUITextInputComponent::FTextInputMethodContext::SetSelectionRange(const uint32 BeginIndex, const uint32 Length, const ECaretPosition InCaretPosition)
{
	InputComp->PressCaretPositionIndex = BeginIndex;
	if (Length > 0)
	{
		if (InCaretPosition == ECaretPosition::Beginning)
		{
			InputComp->CaretPositionIndex = BeginIndex - Length;
		}
		else
		{
			InputComp->CaretPositionIndex = BeginIndex + Length;
		}
	}
	else
	{
		InputComp->CaretPositionIndex = BeginIndex;
	}
	if (InputComp->Text.Len() == 0)
	{
		InputComp->CaretPositionIndex = 0;
	}
	else
	{
		InputComp->CaretPositionIndex = FMath::Clamp(InputComp->CaretPositionIndex, 0, InputComp->Text.Len());
	}
#if LGUI_LOG_TextInputMethodContext
	UE_LOG(LGUI, Warning, TEXT("SetSelectionRange, BeginIndex:%d, Length:%d, InCaretPosition:%d, CaretPositionIndex:%d, PressCaretPositionIndex:%d"), BeginIndex, Length, (int32)InCaretPosition, InputComp->CaretPositionIndex, InputComp->PressCaretPositionIndex)
#endif
}
void UUITextInputComponent::FTextInputMethodContext::GetTextInRange(const uint32 BeginIndex, const uint32 Length, FString& OutString)
{
	OutString = InputComp->Text.Mid(BeginIndex, Length);
#if LGUI_LOG_TextInputMethodContext
	UE_LOG(LogTemp, Log, TEXT("GetTextInRange, BeginIndex:%d, Length:%d, OutString:%s"), BeginIndex, Length, *(OutString));
#endif
}
void UUITextInputComponent::FTextInputMethodContext::SetTextInRange(const uint32 BeginIndex, const uint32 Length, const FString& InString)
{
	InputComp->Text.RemoveAt(BeginIndex, Length);
	int InsertCharCount = 0;
	for (int i = 0; i < InString.Len(); i++)
	{
		TCHAR c = InString[i];
		if (InputComp->IsValidChar(c))
		{
			InputComp->Text.InsertAt(BeginIndex + InsertCharCount, c);
			InsertCharCount++;
		}
	}
	InputComp->CaretPositionIndex = BeginIndex + InsertCharCount;
	InputComp->UpdateAfterTextChange(false);
#if LGUI_LOG_TextInputMethodContext
	UE_LOG(LGUI, Log, TEXT("SetTextInRange, BeginIndex:%d, Length:%d, InString:%s"), BeginIndex, Length, *(InString));
#endif
}
int32 UUITextInputComponent::FTextInputMethodContext::GetCharacterIndexFromPoint(const FVector2D& Point)
{
#if LGUI_LOG_TextInputMethodContext
	UE_LOG(LogTemp, Log, TEXT("GetCharacterIndexFromPoint:%s"), *(Point.ToString()));
#endif
	return 0;
}
bool UUITextInputComponent::FTextInputMethodContext::GetTextBounds(const uint32 BeginIndex, const uint32 Length, FVector2D& Position, FVector2D& Size)
{
	Position = FVector2D(0, 0);
	Size = FVector2D(1000, 1000);
#if LGUI_LOG_TextInputMethodContext
	UE_LOG(LogTemp, Log, TEXT("GetTextBounds:%s"), *(Position.ToString()));
#endif
	return false;
}
void UUITextInputComponent::FTextInputMethodContext::GetScreenBounds(FVector2D& Position, FVector2D& Size)
{
	Position = FVector2D(0, 0);
	Size = FVector2D(1000, 1000);
#if LGUI_LOG_TextInputMethodContext
	UE_LOG(LogTemp, Log, TEXT("GetScreenBounds, Position:%s, Size:%s"), *(Position.ToString()), *(Size.ToString()));
#endif
}
TSharedPtr<FGenericWindow> UUITextInputComponent::FTextInputMethodContext::GetWindow()
{
	if (!CachedWindow.IsValid())
	{
		CachedWindow = SNew(SBox);
		GEngine->GameViewport->AddViewportWidgetContent(CachedWindow.ToSharedRef());
	}
	const TSharedPtr<SWindow> SlateWindow = FSlateApplication::Get().FindWidgetWindow(CachedWindow.ToSharedRef());
#if LGUI_LOG_TextInputMethodContext
	UE_LOG(LogTemp, Log, TEXT("GetWindow, Text:%s"), *InputComp->Text);
#endif
	return SlateWindow->GetNativeWindow();
}
void UUITextInputComponent::FTextInputMethodContext::BeginComposition()
{
	bIsComposing = true;
	OriginString = InputComp->Text;
#if LGUI_LOG_TextInputMethodContext
	UE_LOG(LGUI, Log, TEXT("BeginComposition"));
#endif
}
void UUITextInputComponent::FTextInputMethodContext::UpdateCompositionRange(const int32 InBeginIndex, const uint32 InLength)
{
#if LGUI_LOG_TextInputMethodContext
	UE_LOG(LogTemp, Log, TEXT("UpdateCompositionRange"));
#endif
}
void UUITextInputComponent::FTextInputMethodContext::EndComposition()
{
	bIsComposing = false;
	if (OriginString != InputComp->Text)
	{
		InputComp->UpdateAfterTextChange(true);
	}
#if LGUI_LOG_TextInputMethodContext
	UE_LOG(LGUI, Log, TEXT("EndComposition, ResultString:%s"), *(InputComp->Text));
#endif
}

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif

