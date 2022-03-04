// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
void UUITextInputComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto propertyName = Property->GetFName();
		if (propertyName == TEXT("PasswordChar"))
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
	}
	if (TextActor != nullptr)
	{
		TextActor->GetUIText()->SetOverflowType(bAllowMultiLine ? UITextOverflowType::VerticalOverflow : UITextOverflowType::HorizontalOverflow);
		if (TextActor->GetUIText()->GetRichText() == true)
		{
			TextActor->GetUIText()->SetRichText(false);//rich text not support input
			UE_LOG(LGUI, Error, TEXT("[UUITextInputComponent::PostEditChangeProperty]RichText not support with TextInput! Set it to false."));
		}
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
void UUITextInputComponent::AnyKeyPressed()
{
	if (bInputActive == false)return;
	if (!CheckPlayerController())return;
	if (TextActor == nullptr)return;

	char inputChar = 127;
	bool ctrl = PlayerController->PlayerInput->IsCtrlPressed();
	bool shift = PlayerController->PlayerInput->IsShiftPressed();
	bool alt = PlayerController->PlayerInput->IsAltPressed();
	bool ctrlOnly = ctrl && !alt && !shift;
	bool shiftOnly = !ctrl && !alt && shift;

	//Function key
	if (PlayerController->IsInputKeyDown(EKeys::BackSpace))
	{
		if (IsValidString(BackSpaceResultString()))
		{
			BackSpace();
		}
		return;
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Delete))
	{
		if (IsValidString(ForwardSpaceResultString()))
		{
			ForwardSpace();
		}
		return;
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Home))
	{
		MoveToStart();
		return;
	}
	else if (PlayerController->IsInputKeyDown(EKeys::End))
	{
		MoveToEnd();
		return;
	}
	//Select all
	else if (PlayerController->IsInputKeyDown(EKeys::A))
	{
		if (ctrlOnly)
		{
			SelectAll();
			return;
		}
	}
	//Copy
	else if (PlayerController->IsInputKeyDown(EKeys::C))
	{
		if (ctrlOnly)
		{
			Copy();
			return;
		}
	}
	//Paste
	else if (PlayerController->IsInputKeyDown(EKeys::V))
	{
		if (ctrlOnly)
		{
			if (IsValidString(PasteResultString()))
			{
				Paste();
			}
			return;
		}
	}
	//Cut
	else if (PlayerController->IsInputKeyDown(EKeys::X))
	{
		if (ctrlOnly)
		{
			if (SelectionPropertyArray.Num() != 0)
			{
				if (IsValidString(BackSpaceResultString()))
				{
					Cut();
				}
			}
			return;
		}
	}
	//Arrows
	else if (PlayerController->IsInputKeyDown(EKeys::Left))
	{
		if (shiftOnly)
			MoveLeft(true);
		else
			MoveLeft(false);
		return;
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Right))
	{
		if (shiftOnly)
			MoveRight(true);
		else
			MoveRight(false);
		return;
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Up))
	{
		if (shiftOnly)
			MoveUp(true);
		else
			MoveUp(false);
		return;
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Down))
	{
		if (shiftOnly)
			MoveDown(true);
		else
			MoveDown(false);
		return;
	}
	//Submit
	else if (PlayerController->IsInputKeyDown(EKeys::Enter))
	{
		if (bAllowMultiLine)//if multiline mode, enter means new line
		{
			inputChar = '\n';
		}
		else//single line mode, enter means submit
		{
			if (OnSubmitCPP.IsBound())OnSubmitCPP.Broadcast(Text);
			OnSubmit.FireEvent(Text);
			DeactivateInput();
			return;
		}
	}
	//Cancel
	else if (PlayerController->IsInputKeyDown(EKeys::Escape))
	{
		return;
	}

	//space
	else if (PlayerController->IsInputKeyDown(EKeys::SpaceBar))
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
	if (PlayerController->IsInputKeyDown(EKeys::A))
		inputChar = upperCase ? 'A' : 'a';
	else if (PlayerController->IsInputKeyDown(EKeys::B))
		inputChar = upperCase ? 'B' : 'b';
	else if (PlayerController->IsInputKeyDown(EKeys::C))
		inputChar = upperCase ? 'C' : 'c';
	else if (PlayerController->IsInputKeyDown(EKeys::D))
		inputChar = upperCase ? 'D' : 'd';
	else if (PlayerController->IsInputKeyDown(EKeys::E))
		inputChar = upperCase ? 'E' : 'e';
	else if (PlayerController->IsInputKeyDown(EKeys::F))
		inputChar = upperCase ? 'F' : 'f';
	else if (PlayerController->IsInputKeyDown(EKeys::G))
		inputChar = upperCase ? 'G' : 'g';
	else if (PlayerController->IsInputKeyDown(EKeys::H))
		inputChar = upperCase ? 'H' : 'h';
	else if (PlayerController->IsInputKeyDown(EKeys::I))
		inputChar = upperCase ? 'I' : 'i';
	else if (PlayerController->IsInputKeyDown(EKeys::J))
		inputChar = upperCase ? 'J' : 'j';
	else if (PlayerController->IsInputKeyDown(EKeys::K))
		inputChar = upperCase ? 'K' : 'k';
	else if (PlayerController->IsInputKeyDown(EKeys::L))
		inputChar = upperCase ? 'L' : 'l';
	else if (PlayerController->IsInputKeyDown(EKeys::M))
		inputChar = upperCase ? 'M' : 'm';
	else if (PlayerController->IsInputKeyDown(EKeys::N))
		inputChar = upperCase ? 'N' : 'n';
	else if (PlayerController->IsInputKeyDown(EKeys::O))
		inputChar = upperCase ? 'O' : 'o';
	else if (PlayerController->IsInputKeyDown(EKeys::P))
		inputChar = upperCase ? 'P' : 'p';
	else if (PlayerController->IsInputKeyDown(EKeys::Q))
		inputChar = upperCase ? 'Q' : 'q';
	else if (PlayerController->IsInputKeyDown(EKeys::R))
		inputChar = upperCase ? 'R' : 'r';
	else if (PlayerController->IsInputKeyDown(EKeys::S))
		inputChar = upperCase ? 'S' : 's';
	else if (PlayerController->IsInputKeyDown(EKeys::T))
		inputChar = upperCase ? 'T' : 't';
	else if (PlayerController->IsInputKeyDown(EKeys::U))
		inputChar = upperCase ? 'U' : 'u';
	else if (PlayerController->IsInputKeyDown(EKeys::V))
		inputChar = upperCase ? 'V' : 'v';
	else if (PlayerController->IsInputKeyDown(EKeys::W))
		inputChar = upperCase ? 'W' : 'w';
	else if (PlayerController->IsInputKeyDown(EKeys::X))
		inputChar = upperCase ? 'X' : 'x';
	else if (PlayerController->IsInputKeyDown(EKeys::Y))
		inputChar = upperCase ? 'Y' : 'y';
	else if (PlayerController->IsInputKeyDown(EKeys::Z))
		inputChar = upperCase ? 'Z' : 'z';

	else if (PlayerController->IsInputKeyDown(EKeys::Tilde))
	{
		if (shiftOnly)
			inputChar = '~';
		else
			inputChar = '`';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::One))
	{
		if (shiftOnly)
			inputChar = '!';
		else
			inputChar = '1';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Two))
	{
		if (shiftOnly)
			inputChar = '@';
		else
			inputChar = '2';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Three))
	{
		if (shiftOnly)
			inputChar = '#';
		else
			inputChar = '3';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Four))
	{
		if (shiftOnly)
			inputChar = '$';
		else
			inputChar = '4';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Five))
	{
		if (shiftOnly)
			inputChar = '%';
		else
			inputChar = '5';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Six))
	{
		if (shiftOnly)
			inputChar = '^';
		else
			inputChar = '6';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Seven))
	{
		if (shiftOnly)
			inputChar = '&';
		else
			inputChar = '7';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Eight))
	{
		if (shiftOnly)
			inputChar = '*';
		else
			inputChar = '8';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Nine))
	{
		if (shiftOnly)
			inputChar = '(';
		else
			inputChar = '9';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Zero))
	{
		if (shiftOnly)
			inputChar = ')';
		else
			inputChar = '0';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Hyphen))
	{
		if (shiftOnly)
			inputChar = '_';
		else
			inputChar = '-';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Equals))
	{
		if (shiftOnly)
			inputChar = '+';
		else
			inputChar = '=';
	}

	else if (PlayerController->IsInputKeyDown(EKeys::NumPadZero))
		inputChar = '0';
	else if (PlayerController->IsInputKeyDown(EKeys::NumPadOne))
		inputChar = '1';
	else if (PlayerController->IsInputKeyDown(EKeys::NumPadTwo))
		inputChar = '2';
	else if (PlayerController->IsInputKeyDown(EKeys::NumPadThree))
		inputChar = '3';
	else if (PlayerController->IsInputKeyDown(EKeys::NumPadFour))
		inputChar = '4';
	else if (PlayerController->IsInputKeyDown(EKeys::NumPadFive))
		inputChar = '5';
	else if (PlayerController->IsInputKeyDown(EKeys::NumPadSix))
		inputChar = '6';
	else if (PlayerController->IsInputKeyDown(EKeys::NumPadSeven))
		inputChar = '7';
	else if (PlayerController->IsInputKeyDown(EKeys::NumPadEight))
		inputChar = '8';
	else if (PlayerController->IsInputKeyDown(EKeys::NumPadNine))
		inputChar = '9';

	else if (PlayerController->IsInputKeyDown(EKeys::Multiply))
		inputChar = '*';
	else if (PlayerController->IsInputKeyDown(EKeys::Add))
		inputChar = '+';
	else if (PlayerController->IsInputKeyDown(EKeys::Subtract))
		inputChar = '-';
	else if (PlayerController->IsInputKeyDown(EKeys::Decimal))
		inputChar = '.';
	else if (PlayerController->IsInputKeyDown(EKeys::Divide))
		inputChar = '/';

	else if (PlayerController->IsInputKeyDown(EKeys::LeftBracket))
	{
		if (shiftOnly)
			inputChar = '{';
		else
			inputChar = '[';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::RightBracket))
	{
		if (shiftOnly)
			inputChar = '}';
		else
			inputChar = ']';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Backslash))
	{
		if (shiftOnly)
			inputChar = '|';
		else
			inputChar = '\\';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Semicolon))
	{
		if (shiftOnly)
			inputChar = ':';
		else
			inputChar = ';';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Apostrophe))
	{
		if (shiftOnly)
			inputChar = '\"';
		else
			inputChar = '\'';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Comma))
	{
		if (shiftOnly)
			inputChar = '<';
		else
			inputChar = ',';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Period))
	{
		if (shiftOnly)
			inputChar = '>';
		else
			inputChar = '.';
	}
	else if (PlayerController->IsInputKeyDown(EKeys::Slash))
	{
		if (shiftOnly)
			inputChar = '?';
		else
			inputChar = '/';
	}


	if (IsValidChar(inputChar))
	{
		AppendChar(inputChar);
	}
}
void UUITextInputComponent::AnyKeyReleased()
{
	//UE_LOG(LGUI, Log, TEXT("AnyKeyReleased"));
}
FString UUITextInputComponent::PasteResultString()
{
	auto TempText = Text;
	auto TempCaretPositionIndex = CaretPositionIndex;
	FString pasteString;
	FPlatformApplicationMisc::ClipboardPaste(pasteString);
	if (pasteString.Len() <= 0)return TempText;
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
	if (SelectionPropertyArray.Num() != 0)//delete selection frist
	{
		int32 startIndex = PressCaretPositionIndex > TempCaretPositionIndex ? TempCaretPositionIndex : PressCaretPositionIndex;
		TempText.RemoveAt(startIndex, FMath::Abs(TempCaretPositionIndex - PressCaretPositionIndex));
		TempCaretPositionIndex = PressCaretPositionIndex > TempCaretPositionIndex ? TempCaretPositionIndex : PressCaretPositionIndex;
	}
	TempText.InsertAt(TempCaretPositionIndex, pasteString);
	return TempText;
}
bool UUITextInputComponent::IsValidChar(char c)
{
	auto OriginHaveChar = [](char testChar, const FString& string, int stringLength)
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
				if (OriginHaveChar('-', Text, Text.Len()))
				{
					return false;
				}
			}
			return true;
		}
		if (c == '-')
		{
			if (CaretPositionIndex == 0 && !OriginHaveChar('-', Text, Text.Len()))
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
				if (OriginHaveChar('-', Text, Text.Len()))
				{
					return false;
				}
			}
			return true;
		}
		if (c == '.')
		{
			if (OriginHaveChar('.', Text, Text.Len()))
			{
				return false;
			}
			else
			{
				if (CaretPositionIndex == 0)
				{
					if (!OriginHaveChar('-', Text, Text.Len()))
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
			if (CaretPositionIndex == 0 && !OriginHaveChar('-', Text, Text.Len()))
			{
				return true;
			}
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

			return CustomInputTypeFunction.Execute(TempText);
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[UUITextInputComponent::IsValidChar]InputType use CustomFunction but not valid!"));
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
bool UUITextInputComponent::IsValidString(const FString& InString)
{
	//input type
	switch (InputType)
	{
	case ELGUITextInputType::Standard:
		return true;
		break;
	case ELGUITextInputType::IntegerNumber:
	{
		int32 textLength = InString.Len();
		for (int i = 0; i < textLength; i++)
		{
			TCHAR c = InString[i];
			if (c == '-')
			{
				if (i != 0)
				{
					return false;
				}
			}
			else
			{
				if (c < '0' || c > '9')
				{
					return false;
				}
			}
		}
		return true;
	}
	break;
	case ELGUITextInputType::DecimalNumber:
	{
		//check number and dot
		int dotCount = 0;
		int32 textLength = InString.Len();
		for (int i = 0; i < textLength; i++)
		{
			TCHAR c = InString[i];
			if (c == '-')
			{
				if (i != 0)
				{
					return false;
				}
			}
			else
			{
				if (c == '.')
				{
					dotCount++;
					if (dotCount > 1)
					{
						return false;
					}
				}
				else if (c < '0' || c > '9')
				{
					return false;
				}
			}
		}
		return true;
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
			return CustomInputTypeFunction.Execute(InString);
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[UUITextInputComponent::IsValidString]InputType use CustomFunction but not valid!"));
			return true;
		}
	}
	break;
	}
	return true;
}
void UUITextInputComponent::AppendChar(char c)
{
	if (TextActor == nullptr)return;

	if (SelectionPropertyArray.Num() != 0)//delete selection frist
	{
		int32 startIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
		Text.RemoveAt(startIndex, FMath::Abs(CaretPositionIndex - PressCaretPositionIndex));
		CaretPositionIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
	}
	Text.InsertAt(CaretPositionIndex, c);
	CaretPositionIndex++;
	UpdateAfterTextChange();
	PressCaretPositionIndex = CaretPositionIndex;
}
void UUITextInputComponent::BackSpace()
{
	if (SelectionPropertyArray.Num() == 0)//no selection mask, use caret
	{
		if (CaretPositionIndex > 0)
		{
			CaretPositionIndex--;
			Text.RemoveAt(CaretPositionIndex);
			UpdateAfterTextChange();
			PressCaretPositionIndex = CaretPositionIndex;
		}
	}
	else//selection mask, delete 
	{
		int32 startIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
		Text.RemoveAt(startIndex, FMath::Abs(CaretPositionIndex - PressCaretPositionIndex));
		CaretPositionIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
		UpdateAfterTextChange();
		PressCaretPositionIndex = CaretPositionIndex;
	}
}
FString UUITextInputComponent::BackSpaceResultString()
{
	auto TempText = Text;

	if (SelectionPropertyArray.Num() == 0)//no selection mask, use caret
	{
		if (CaretPositionIndex > 0)
		{
			TempText.RemoveAt(CaretPositionIndex - 1);
		}
	}
	else//selection mask, delete 
	{
		int32 startIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
		TempText.RemoveAt(startIndex, FMath::Abs(CaretPositionIndex - PressCaretPositionIndex));
	}
	return TempText;
}
void UUITextInputComponent::ForwardSpace()
{
	if (SelectionPropertyArray.Num() == 0)//no selection mask, use caret
	{
		if (CaretPositionIndex < Text.Len())
		{
			Text.RemoveAt(CaretPositionIndex);
			UpdateAfterTextChange();
			PressCaretPositionIndex = CaretPositionIndex;
		}
	}
	else//selection mask, delete 
	{
		int32 startIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
		Text.RemoveAt(startIndex, FMath::Abs(CaretPositionIndex - PressCaretPositionIndex));
		CaretPositionIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
		UpdateAfterTextChange();
		PressCaretPositionIndex = CaretPositionIndex;
	}
}
FString UUITextInputComponent::ForwardSpaceResultString()
{
	auto TempText = Text;

	if (SelectionPropertyArray.Num() == 0)//no selection mask, use caret
	{
		if (CaretPositionIndex < TempText.Len())
		{
			TempText.RemoveAt(CaretPositionIndex);
		}
	}
	else//selection mask, delete 
	{
		int32 startIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
		TempText.RemoveAt(startIndex, FMath::Abs(CaretPositionIndex - PressCaretPositionIndex));
	}
	return TempText;
}
void UUITextInputComponent::Copy()
{
	if (SelectionPropertyArray.Num() != 0)//have selection
	{
		int32 startIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
		auto copyText = Text.Mid(startIndex, FMath::Abs(CaretPositionIndex - PressCaretPositionIndex));
		FPlatformApplicationMisc::ClipboardCopy(*copyText);
	}
}
void UUITextInputComponent::Paste()
{
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
	if (SelectionPropertyArray.Num() != 0)//have selection
	{
		int32 startIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
		Text.RemoveAt(startIndex, FMath::Abs(CaretPositionIndex - PressCaretPositionIndex));
		CaretPositionIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
	}

	Text.InsertAt(CaretPositionIndex, pasteString);
	CaretPositionIndex = PressCaretPositionIndex = Text.Len();
	UpdateAfterTextChange();
	PressCaretPositionLineIndex = CaretPositionLineIndex;
}
void UUITextInputComponent::Cut()
{
	if (SelectionPropertyArray.Num() != 0)//have selection
	{
		Copy();
		BackSpace();
	}
}
void UUITextInputComponent::SelectAll()
{
	//update selection
	TextActor->GetUIText()->GetSelectionProperty(0, TextActor->GetUIText()->GetText().ToString().Len(), SelectionPropertyArray);
	CaretPositionIndex = Text.Len();
	PressCaretPositionIndex = 0;
	UpdateUITextComponent();
	UpdateCaretPosition(false);
	UpdateSelection();
}
void UUITextInputComponent::UpdateAfterTextChange(bool InFireEvent)
{
	if (bAllowMultiLine)
	{
		if (TextActor != nullptr)
		{
			//set to full text, and find CaretPositionLineIndex
			TextActor->GetUIText()->SetText(FText::FromString(Text));
			FVector2f caretPosition;
			TextActor->GetUIText()->FindCaretByIndex(CaretPositionIndex, caretPosition, CaretPositionLineIndex);
			//calculate MaxVisibleLineCount
			MaxVisibleLineCount = (int)((TextActor->GetUIText()->GetHeight() + TextActor->GetUIText()->GetFontSpace().Y) / (TextActor->GetUIText()->GetFontSize() + TextActor->GetUIText()->GetFontSpace().Y));
		}
	}
	UpdateUITextComponent();
	UpdateCaretPosition();
	UpdateInputComposition();
	UpdatePlaceHolderComponent();
	if (InFireEvent)
	{
		FireOnValueChangeEvent();
	}
}
void UUITextInputComponent::MoveToStart()
{
	CaretPositionIndex = 0;
	UpdateUITextComponent();
	UpdateCaretPosition();
	UpdateInputComposition();
	PressCaretPositionIndex = CaretPositionIndex;
}
void UUITextInputComponent::MoveToEnd()
{
	CaretPositionIndex = Text.Len();
	UpdateUITextComponent();
	UpdateCaretPosition();
	UpdateInputComposition();
	PressCaretPositionIndex = CaretPositionIndex;
}
void UUITextInputComponent::MoveLeft(bool withSelection)
{
	if (CaretPositionIndex > 0)
	{
		CaretPositionIndex--;
		UpdateUITextComponent();
		UpdateCaretPosition(!withSelection);
		UpdateInputComposition();

		if (withSelection)
		{
			int tempStartCaretPositionIndex = PressCaretPositionIndex - VisibleCharStartIndex;
			tempStartCaretPositionIndex = FMath::Clamp(tempStartCaretPositionIndex, 0, TextActor->GetUIText()->GetText().ToString().Len());
			TextActor->GetUIText()->GetSelectionProperty(tempStartCaretPositionIndex, CaretPositionIndex - VisibleCharStartIndex, SelectionPropertyArray);
			UpdateSelection();
		}
		else
		{
			PressCaretPositionIndex = CaretPositionIndex;
		}
	}
}
void UUITextInputComponent::MoveRight(bool withSelection)
{
	if (CaretPositionIndex < Text.Len())
	{
		CaretPositionIndex++;
		if (bAllowMultiLine)//multiline should check vertical range
		{
			if (CaretPositionIndex - VisibleCharStartIndex > TextActor->GetUIText()->GetText().ToString().Len())//if caret is more than visible text
			{
				if (VisibleCharStartIndex + TextActor->GetUIText()->GetText().ToString().Len() < Text.Len())//if have more lines
				{
					CaretPositionLineIndex++;
				}
			}
		}
		UpdateUITextComponent();
		UpdateCaretPosition(!withSelection);
		UpdateInputComposition();

		if (withSelection)
		{
			int tempStartCaretPositionIndex = PressCaretPositionIndex - VisibleCharStartIndex;
			tempStartCaretPositionIndex = FMath::Clamp(tempStartCaretPositionIndex, 0, TextActor->GetUIText()->GetText().ToString().Len());
			TextActor->GetUIText()->GetSelectionProperty(tempStartCaretPositionIndex, CaretPositionIndex - VisibleCharStartIndex, SelectionPropertyArray);
			UpdateSelection();
		}
		else
		{
			PressCaretPositionIndex = CaretPositionIndex;
		}
	}
}
void UUITextInputComponent::MoveUp(bool withSelection)
{
	if (TextActor != nullptr)
	{
		if (!bAllowMultiLine)return;
		if (CaretPositionLineIndex <= 0)return;//already at first line
		CaretPositionLineIndex--;

		UpdateUITextComponent();
		auto CaretRelativeLocation = CaretObject->GetRelativeLocation();
		FVector2f caretPosition(CaretRelativeLocation.Y, CaretRelativeLocation.Z);
		TextActor->GetUIText()->FindCaretUp(caretPosition, CaretPositionLineIndex - VisibleCharStartLineIndex, CaretPositionIndex);
		CaretPositionIndex += VisibleCharStartIndex;
		UpdateCaretPosition(caretPosition, !withSelection);
		UpdateInputComposition();

		if (withSelection)
		{
			int tempStartCaretPositionIndex = PressCaretPositionIndex - VisibleCharStartIndex;
			tempStartCaretPositionIndex = FMath::Clamp(tempStartCaretPositionIndex, 0, TextActor->GetUIText()->GetText().ToString().Len());
			TextActor->GetUIText()->GetSelectionProperty(tempStartCaretPositionIndex, CaretPositionIndex - VisibleCharStartIndex, SelectionPropertyArray);
			UpdateSelection();
		}
		else
		{
			PressCaretPositionIndex = CaretPositionIndex;
		}
	}
}
void UUITextInputComponent::MoveDown(bool withSelection)
{
	if (TextActor != nullptr)
	{
		if (!bAllowMultiLine)return;
		if (VisibleCharStartIndex + TextActor->GetUIText()->GetText().ToString().Len() >= Text.Len()//no more invisible line
			&& CaretPositionLineIndex + 1 == MaxVisibleLineCount + VisibleCharStartLineIndex//and caret is at last line
			)//move caret to last
		{
			CaretPositionIndex = Text.Len();

			UpdateUITextComponent();
			UpdateCaretPosition(!withSelection);
		}
		else//have invisible line, move caret down
		{
			CaretPositionLineIndex++;

			UpdateUITextComponent();
			auto CaretRelativeLocation = CaretObject->GetRelativeLocation();
			FVector2f caretPosition(CaretRelativeLocation.Y, CaretRelativeLocation.Z);
			TextActor->GetUIText()->FindCaretDown(caretPosition, CaretPositionLineIndex - VisibleCharStartLineIndex, CaretPositionIndex);
			CaretPositionIndex += VisibleCharStartIndex;
			UpdateCaretPosition(caretPosition, !withSelection);
		}

		UpdateInputComposition();

		if (withSelection)
		{
			int tempStartCaretPositionIndex = PressCaretPositionIndex - VisibleCharStartIndex;
			tempStartCaretPositionIndex = FMath::Clamp(tempStartCaretPositionIndex, 0, TextActor->GetUIText()->GetText().ToString().Len());
			TextActor->GetUIText()->GetSelectionProperty(tempStartCaretPositionIndex, CaretPositionIndex - VisibleCharStartIndex, SelectionPropertyArray);
			UpdateSelection();
		}
		else
		{
			PressCaretPositionIndex = CaretPositionIndex;
		}
	}
}

void UUITextInputComponent::FireOnValueChangeEvent()
{
	if (OnValueChangeCPP.IsBound())OnValueChangeCPP.Broadcast(Text);
	OnValueChange.FireEvent(Text);
}
void UUITextInputComponent::UpdateUITextComponent()
{
	if (TextActor != nullptr)
	{
		auto uiText = TextActor->GetUIText();
		FString replaceText;
		switch (InputType)
		{
		case ELGUITextInputType::Password:
		{
			int len = Text.Len();
			replaceText.Reset(len);
			auto psChar = PasswordChar[0];
			for (int i = 0; i < len; i++)
			{
				replaceText.AppendChar(psChar);
			}
		}
		break;
		default:
			replaceText = Text;
			break;
		}
		
		if (bAllowMultiLine)//multi line, handle out of range chars
		{
			if (CaretPositionIndex - VisibleCharStartIndex < 0)//caret position move left and out of current range, then do move up
			{
				VisibleCharStartLineIndex--;
				CaretPositionLineIndex--;
			}
			if (CaretPositionIndex - VisibleCharStartIndex > uiText->GetText().ToString().Len())//caret position move right and out of current range, then do move down
			{
				VisibleCharStartLineIndex++;
				CaretPositionLineIndex++;
			}

			if (VisibleCharStartLineIndex > CaretPositionLineIndex)
			{
				VisibleCharStartLineIndex = CaretPositionLineIndex;
			}
			if (VisibleCharStartLineIndex + (MaxVisibleLineCount - 1) < CaretPositionLineIndex)
			{
				VisibleCharStartLineIndex = CaretPositionLineIndex - (MaxVisibleLineCount - 1);
			}
			int VisibleCharEndIndex = 0;
			int VisibleCharEndLineIndex = VisibleCharStartLineIndex + (MaxVisibleLineCount - 1);
			replaceText = uiText->GetSubStringByLine(replaceText, VisibleCharStartLineIndex, VisibleCharEndLineIndex, VisibleCharStartIndex, VisibleCharEndIndex);
		}
		else//single line, handle out of range chars
		{
			float maxWidth = uiText->GetWidth();
			float width = 0;
			bool outOfRange = false;
			int VisibleCharEndIndex = CaretPositionIndex;
			if (VisibleCharStartIndex > CaretPositionIndex)
			{
				VisibleCharStartIndex = CaretPositionIndex;
			}

			//search from caret to left, until reach VisibleCharStartIndex,
			//then search form caret to right,
			//then search from VisibleCharStartIndex to left,
			//all above step will break if out-of-range

			//same as UIGeometry's function "UpdateUIText"
			auto GetCharGeoXAdv = [](TCHAR charCode, ULGUIFontData_BaseObject* font, int overrideFontSize)
			{
				if (charCode == ' ')
				{
					return overrideFontSize * 0.5f;
				}
				else if (charCode == '\t')
				{
					return (float)(overrideFontSize + overrideFontSize);
				}
				else
				{
					auto charData = font->GetCharData(charCode, (uint16)overrideFontSize);
					return charData.xadvance;
				}
			};

			auto fontSize = uiText->GetFontSize();
			auto fontSpace = uiText->GetFontSpace();
			auto font = uiText->GetFont();
			auto fontStyle = uiText->GetFontStyle();

			//check from caret to left
			for (int i = CaretPositionIndex - 1; i >= 0 && i >= VisibleCharStartIndex; i--)
			{
				auto expendSpace = GetCharGeoXAdv(replaceText[i], font, fontSize) + fontSpace.X;
				width += expendSpace;
				if (width >= maxWidth)
				{
					width -= expendSpace;
					outOfRange = true;
					VisibleCharStartIndex = i + 1;
					break;
				}
			}
			if (!outOfRange)
			{
				//check from caret to right
				for (int i = CaretPositionIndex, charCount = replaceText.Len(); i < charCount; i++)
				{
					auto expendSpace = GetCharGeoXAdv(replaceText[i], font, fontSize) + fontSpace.X;
					width += expendSpace;
					if (width > maxWidth)
					{
						width -= expendSpace;
						outOfRange = true;
						break;
					}
					VisibleCharEndIndex++;
				}
				if (!outOfRange)
				{
					//check from VisibleCharStartIndex to left
					for (int i = VisibleCharStartIndex - 1; i >= 0; i--)
					{
						auto expendSpace = GetCharGeoXAdv(replaceText[i], font, fontSize) + fontSpace.X;
						width += expendSpace;
						if (width >= maxWidth)
						{
							width -= expendSpace;
							outOfRange = true;
							break;
						}
						VisibleCharStartIndex--;
					}
				}
			}

			if (outOfRange)
			{
				replaceText = replaceText.Mid(VisibleCharStartIndex, VisibleCharEndIndex - VisibleCharStartIndex);
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
		TextActor->GetUIText()->FindCaretByIndex(CaretPositionIndex - VisibleCharStartIndex, caretPos, tempCaretPositionLineIndex);
		CaretPositionLineIndex = tempCaretPositionLineIndex + VisibleCharStartLineIndex;

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
	SelectionPropertyArray.Empty();//clear selection mask
	//TextInputMethodContext->SetSelectionRange(0, 0, ITextInputMethodContext::ECaretPosition::Beginning);
}
bool UUITextInputComponent::DeleteIfSelection(int& OutCaretOffset)
{
	if (SelectionPropertyArray.Num() != 0)
	{
		int32 startIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
		Text.RemoveAt(startIndex, FMath::Abs(CaretPositionIndex - PressCaretPositionIndex));
		OutCaretOffset = PressCaretPositionIndex > CaretPositionIndex ? 0 : CaretPositionIndex - PressCaretPositionIndex;
		CaretPositionIndex = PressCaretPositionIndex > CaretPositionIndex ? CaretPositionIndex : PressCaretPositionIndex;
		UpdateInputComposition();
		return true;
	}
	return false;
}

void UUITextInputComponent::UpdateInputComposition()
{
	if (TextInputMethodContext.IsValid())
	{
		TextInputMethodContext->UpdateInputComposition();
	}
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

bool UUITextInputComponent::OnPointerEnter_Implementation(ULGUIPointerEventData* eventData)
{
	Super::OnPointerEnter_Implementation(eventData);
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
			TextActor->GetUIText()->FindCaretByPosition(eventData->GetWorldPointInPlane(), caretPosition, tempCaretPositionLineIndex, CaretPositionIndex);
			int displayTextLength = TextActor->GetUIText()->GetText().ToString().Len();//visible text length

			//@todo:caret move speed depend on drag distance
			if (CaretPositionIndex == 0)//caret position at left most
			{
				CaretPositionIndex = VisibleCharStartIndex - 1;//move caret to left
				CaretPositionIndex = FMath::Max(CaretPositionIndex, 0);
			}
			else if (CaretPositionIndex >= displayTextLength)//caret position at right most
			{
				CaretPositionIndex = CaretPositionIndex + VisibleCharStartIndex + 1;//move caret to right
				CaretPositionIndex = FMath::Min(CaretPositionIndex, Text.Len());
				if (bAllowMultiLine)//multiline should check vertical range
				{
					if (CaretPositionIndex - VisibleCharStartIndex > displayTextLength)//if caret is more than visible text
					{
						if (VisibleCharStartIndex + displayTextLength < Text.Len())//if have more lines
						{
							CaretPositionLineIndex++;
						}
					}
				}
			}
			else//not drag out-of-range
			{
				CaretPositionIndex += VisibleCharStartIndex;
			}

			UpdateUITextComponent();
			//selectionStartCaretIndex may out of range, need clamp.
			int selectionStartCaretIndex = PressCaretPositionIndex - VisibleCharStartIndex;
			selectionStartCaretIndex = FMath::Clamp(selectionStartCaretIndex, 0, TextActor->GetUIText()->GetText().ToString().Len());
			TextActor->GetUIText()->GetSelectionProperty(selectionStartCaretIndex, CaretPositionIndex - VisibleCharStartIndex, SelectionPropertyArray);
			UpdateSelection();
			UpdateCaretPosition(false);
			UpdateInputComposition();
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
			UpdateUITextComponent();
			TextActor->GetUIText()->FindCaretByPosition(eventData->GetWorldPointInPlane(), PressCaretPosition, PressCaretPositionLineIndex, PressCaretPositionIndex);
			PressCaretPositionIndex = PressCaretPositionIndex + VisibleCharStartIndex;
			CaretPositionIndex = PressCaretPositionIndex;
			PressCaretPositionLineIndex = PressCaretPositionLineIndex + VisibleCharStartLineIndex;
			CaretPositionLineIndex = PressCaretPositionLineIndex;
			UpdateCaretPosition(PressCaretPosition);
			UpdateInputComposition();
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
		CaretPositionIndex = Text.Len();
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
	else//default select all
	{
		SelectAll();
	}
	//end update selection
	UpdateInputComposition();
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
	if (OnInputActivateCPP.IsBound())OnInputActivateCPP.Broadcast(bInputActive);
	OnInputActivate.FireEvent(bInputActive);
}

#define BINDKEY_IF_NOT_IGNORE(key)\
if(!IgnoreKeys.Contains(key))\
{\
	inputComp->BindKey(key, EInputEvent::IE_Repeat, this, &UUITextInputComponent::AnyKeyPressed); \
	inputComp->BindKey(key, EInputEvent::IE_Pressed, this, &UUITextInputComponent::AnyKeyPressed);\
}//register pressed and repeat

void UUITextInputComponent::BindKeys()
{
	if (GetOwner()->InputComponent == nullptr)
	{
		GetOwner()->AutoReceiveInput = EAutoReceiveInput::Player0;
		GetOwner()->PreInitializeComponents();
	}
	auto inputComp = GetOwner()->InputComponent;

	BINDKEY_IF_NOT_IGNORE(EKeys::BackSpace);
	BINDKEY_IF_NOT_IGNORE(EKeys::Tab);
	BINDKEY_IF_NOT_IGNORE(EKeys::Enter);
	BINDKEY_IF_NOT_IGNORE(EKeys::Pause);

	BINDKEY_IF_NOT_IGNORE(EKeys::CapsLock);
	BINDKEY_IF_NOT_IGNORE(EKeys::Escape);
	BINDKEY_IF_NOT_IGNORE(EKeys::SpaceBar);
	BINDKEY_IF_NOT_IGNORE(EKeys::PageUp);
	BINDKEY_IF_NOT_IGNORE(EKeys::PageDown);
	BINDKEY_IF_NOT_IGNORE(EKeys::End);
	BINDKEY_IF_NOT_IGNORE(EKeys::Home);

	BINDKEY_IF_NOT_IGNORE(EKeys::Left);
	BINDKEY_IF_NOT_IGNORE(EKeys::Up);
	BINDKEY_IF_NOT_IGNORE(EKeys::Right);
	BINDKEY_IF_NOT_IGNORE(EKeys::Down);

	BINDKEY_IF_NOT_IGNORE(EKeys::Insert);
	BINDKEY_IF_NOT_IGNORE(EKeys::Delete);

	BINDKEY_IF_NOT_IGNORE(EKeys::Zero);
	BINDKEY_IF_NOT_IGNORE(EKeys::One);
	BINDKEY_IF_NOT_IGNORE(EKeys::Two);
	BINDKEY_IF_NOT_IGNORE(EKeys::Three);
	BINDKEY_IF_NOT_IGNORE(EKeys::Four);
	BINDKEY_IF_NOT_IGNORE(EKeys::Five);
	BINDKEY_IF_NOT_IGNORE(EKeys::Six);
	BINDKEY_IF_NOT_IGNORE(EKeys::Seven);
	BINDKEY_IF_NOT_IGNORE(EKeys::Eight);
	BINDKEY_IF_NOT_IGNORE(EKeys::Nine);

	BINDKEY_IF_NOT_IGNORE(EKeys::A);
	BINDKEY_IF_NOT_IGNORE(EKeys::B);
	BINDKEY_IF_NOT_IGNORE(EKeys::C);
	BINDKEY_IF_NOT_IGNORE(EKeys::D);
	BINDKEY_IF_NOT_IGNORE(EKeys::E);
	BINDKEY_IF_NOT_IGNORE(EKeys::F);
	BINDKEY_IF_NOT_IGNORE(EKeys::G);
	BINDKEY_IF_NOT_IGNORE(EKeys::H);
	BINDKEY_IF_NOT_IGNORE(EKeys::I);
	BINDKEY_IF_NOT_IGNORE(EKeys::J);
	BINDKEY_IF_NOT_IGNORE(EKeys::K);
	BINDKEY_IF_NOT_IGNORE(EKeys::L);
	BINDKEY_IF_NOT_IGNORE(EKeys::M);
	BINDKEY_IF_NOT_IGNORE(EKeys::N);
	BINDKEY_IF_NOT_IGNORE(EKeys::O);
	BINDKEY_IF_NOT_IGNORE(EKeys::P);
	BINDKEY_IF_NOT_IGNORE(EKeys::Q);
	BINDKEY_IF_NOT_IGNORE(EKeys::R);
	BINDKEY_IF_NOT_IGNORE(EKeys::S);
	BINDKEY_IF_NOT_IGNORE(EKeys::T);
	BINDKEY_IF_NOT_IGNORE(EKeys::U);
	BINDKEY_IF_NOT_IGNORE(EKeys::V);
	BINDKEY_IF_NOT_IGNORE(EKeys::W);
	BINDKEY_IF_NOT_IGNORE(EKeys::X);
	BINDKEY_IF_NOT_IGNORE(EKeys::Y);
	BINDKEY_IF_NOT_IGNORE(EKeys::Z);

	BINDKEY_IF_NOT_IGNORE(EKeys::NumPadZero);
	BINDKEY_IF_NOT_IGNORE(EKeys::NumPadOne);
	BINDKEY_IF_NOT_IGNORE(EKeys::NumPadTwo);
	BINDKEY_IF_NOT_IGNORE(EKeys::NumPadThree);
	BINDKEY_IF_NOT_IGNORE(EKeys::NumPadFour);
	BINDKEY_IF_NOT_IGNORE(EKeys::NumPadFive);
	BINDKEY_IF_NOT_IGNORE(EKeys::NumPadSix);
	BINDKEY_IF_NOT_IGNORE(EKeys::NumPadSeven);
	BINDKEY_IF_NOT_IGNORE(EKeys::NumPadEight);
	BINDKEY_IF_NOT_IGNORE(EKeys::NumPadNine);

	BINDKEY_IF_NOT_IGNORE(EKeys::Multiply);
	BINDKEY_IF_NOT_IGNORE(EKeys::Add);
	BINDKEY_IF_NOT_IGNORE(EKeys::Subtract);
	BINDKEY_IF_NOT_IGNORE(EKeys::Decimal);
	BINDKEY_IF_NOT_IGNORE(EKeys::Divide);

	BINDKEY_IF_NOT_IGNORE(EKeys::LeftShift);
	BINDKEY_IF_NOT_IGNORE(EKeys::RightShift);
	BINDKEY_IF_NOT_IGNORE(EKeys::LeftControl);
	BINDKEY_IF_NOT_IGNORE(EKeys::RightControl);
	BINDKEY_IF_NOT_IGNORE(EKeys::LeftAlt);
	BINDKEY_IF_NOT_IGNORE(EKeys::RightAlt);
	BINDKEY_IF_NOT_IGNORE(EKeys::LeftCommand);
	BINDKEY_IF_NOT_IGNORE(EKeys::RightCommand);

	BINDKEY_IF_NOT_IGNORE(EKeys::Semicolon);
	BINDKEY_IF_NOT_IGNORE(EKeys::Equals);
	BINDKEY_IF_NOT_IGNORE(EKeys::Comma);
	BINDKEY_IF_NOT_IGNORE(EKeys::Underscore);
	BINDKEY_IF_NOT_IGNORE(EKeys::Hyphen);
	BINDKEY_IF_NOT_IGNORE(EKeys::Period);
	BINDKEY_IF_NOT_IGNORE(EKeys::Slash);
	BINDKEY_IF_NOT_IGNORE(EKeys::Tilde);
	BINDKEY_IF_NOT_IGNORE(EKeys::LeftBracket);
	BINDKEY_IF_NOT_IGNORE(EKeys::Backslash);
	BINDKEY_IF_NOT_IGNORE(EKeys::RightBracket);
	BINDKEY_IF_NOT_IGNORE(EKeys::Apostrophe);

	BINDKEY_IF_NOT_IGNORE(EKeys::Ampersand);
	BINDKEY_IF_NOT_IGNORE(EKeys::Asterix);
	BINDKEY_IF_NOT_IGNORE(EKeys::Caret);
	BINDKEY_IF_NOT_IGNORE(EKeys::Colon);
	BINDKEY_IF_NOT_IGNORE(EKeys::Dollar);
	BINDKEY_IF_NOT_IGNORE(EKeys::Exclamation);
	BINDKEY_IF_NOT_IGNORE(EKeys::LeftParantheses);
	BINDKEY_IF_NOT_IGNORE(EKeys::RightParantheses);
	BINDKEY_IF_NOT_IGNORE(EKeys::Quote);
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

			TextInputMethodContextRef->AbortComposition();

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
		if (OnInputActivateCPP.IsBound())OnInputActivateCPP.Broadcast(bInputActive);
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
FString UUITextInputComponent::GetText()const
{
	return Text;
}
void UUITextInputComponent::SetText(FString InText, bool InFireEvent)
{
	if (Text != InText)
	{
		if (!bAllowMultiLine)
		{
			InText = InText.Replace(TEXT("\n"), TEXT("")).Replace(TEXT("\t"), TEXT(""));
		}
		if (IsValidString(InText))
		{
			Text = InText;
			CaretPositionIndex = 0;
			UpdateAfterTextChange(InFireEvent);
		}
	}
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
		if (InDelegate.IsBound())InDelegate.Execute(InText);
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
		if (InDelegate.IsBound())InDelegate.Execute(InText);
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
		if (InDelegate.IsBound())InDelegate.Execute(activate);
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
void UUITextInputComponent::SetCustomInputTypeFunction(const TFunction<bool(const FString&)>& InFunction)
{
	CustomInputTypeFunction = FLGUITextInputCustomInputTypeDelegate::CreateLambda(InFunction);
}
void UUITextInputComponent::SetCustomInputTypeFunction(const FLGUITextInputCustomInputTypeDynamicDelegate& InFunction)
{
	CustomInputTypeFunction = FLGUITextInputCustomInputTypeDelegate::CreateLambda([InFunction](const FString& InString) {
		if (InFunction.IsBound())
		{
			return InFunction.Execute(InString);
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[UUITextInputComponent::SetCustomInputTypeFunction]CustomInputType function not valid!"));
			return false;
		}
	});
}
void UUITextInputComponent::ClearCustomInputTypeEvent()
{
	CustomInputTypeFunction.Unbind();
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
void UUITextInputComponent::FTextInputMethodContext::UpdateInputComposition()
{
	CompositionBeginIndex = InputComp->CaretPositionIndex;
	CaretPosition = ITextInputMethodContext::ECaretPosition::Ending;
	CompositionLength = 0;
	//UE_LOG(LGUI, Log, TEXT("UpdateInputComposition, BeginIndex:%d"), CompositionBeginIndex);
}
UUITextInputComponent::FTextInputMethodContext::FTextInputMethodContext(UUITextInputComponent* InInput)
{
	InputComp = InInput;
}
bool UUITextInputComponent::FTextInputMethodContext::IsReadOnly()
{
	return false;
}
uint32 UUITextInputComponent::FTextInputMethodContext::GetTextLength()
{
	return InputComp->Text.Len();
}
void UUITextInputComponent::FTextInputMethodContext::GetSelectionRange(uint32& BeginIndex, uint32& Length, ECaretPosition& OutCaretPosition)
{
	BeginIndex = CompositionBeginIndex;
	Length = CompositionLength;
	OutCaretPosition = CaretPosition;
	//UE_LOG(LogTemp, Log, TEXT("GetSelectionRange, BeginIndex:%d, Length:%d, InCaretPosition:%d"), BeginIndex, Length, (int32)OutCaretPosition);
}
void UUITextInputComponent::FTextInputMethodContext::SetSelectionRange(const uint32 BeginIndex, const uint32 Length, const ECaretPosition InCaretPosition)
{
	CompositionBeginIndex = BeginIndex;
	CompositionLength = Length;
	CaretPosition = InCaretPosition;
	//UE_LOG(LGUI, Log, TEXT("SetSelectionRange, BeginIndex:%d, Length:%d, InCaretPosition:%d"), BeginIndex, Length, (int32)InCaretPosition)
}
void UUITextInputComponent::FTextInputMethodContext::GetTextInRange(const uint32 BeginIndex, const uint32 Length, FString& OutString)
{
	OutString = InputComp->Text.Mid(BeginIndex, Length);
	//UE_LOG(LogTemp, Log, TEXT("GetTextInRange, BeginIndex:%d, Length:%d, OutString:%s"), BeginIndex, Length, *(OutString));
}
void UUITextInputComponent::FTextInputMethodContext::SetTextInRange(const uint32 BeginIndex, const uint32 Length, const FString& InString)
{
	auto beginIndex = BeginIndex - CompositionCaretOffset;
	InputComp->Text.RemoveAt(beginIndex, Length);
	InputComp->Text.InsertAt(beginIndex, InString);
	InputComp->CaretPositionIndex = beginIndex + InString.Len();
	InputComp->UpdateAfterTextChange();
	//UE_LOG(LGUI, Log, TEXT("SetTextInRange, BeginIndex:%d, Length:%d, InString:%s"), BeginIndex, Length, *(InString));
}
int32 UUITextInputComponent::FTextInputMethodContext::GetCharacterIndexFromPoint(const FVector2D& Point)
{
	//UE_LOG(LogTemp, Log, TEXT("GetCharacterIndexFromPoint:%s"), *(Point.ToString()));
	return 0;
}
bool UUITextInputComponent::FTextInputMethodContext::GetTextBounds(const uint32 BeginIndex, const uint32 Length, FVector2D& Position, FVector2D& Size)
{
	Position = FVector2D(0, 0);
	Size = FVector2D(1000, 1000);
	//UE_LOG(LogTemp, Log, TEXT("GetTextBounds:%s"), *(Position.ToString()));
	return false;
}
void UUITextInputComponent::FTextInputMethodContext::GetScreenBounds(FVector2D& Position, FVector2D& Size)
{
	Position = FVector2D(0, 0);
	Size = FVector2D(1000, 1000);
	//UE_LOG(LogTemp, Log, TEXT("GetScreenBounds, Position:%s, Size:%s"), *(Position.ToString()), *(Size.ToString()));
}
TSharedPtr<FGenericWindow> UUITextInputComponent::FTextInputMethodContext::GetWindow()
{
	//UE_LOG(LogTemp, Log, TEXT("GetWindow"));
	if (!CachedWindow.IsValid())
	{
		CachedWindow = SNew(SBox);
		GEngine->GameViewport->AddViewportWidgetContent(CachedWindow.ToSharedRef());
	}
	const TSharedPtr<SWindow> SlateWindow = FSlateApplication::Get().FindWidgetWindow(CachedWindow.ToSharedRef());
	return SlateWindow->GetNativeWindow();
}
void UUITextInputComponent::FTextInputMethodContext::BeginComposition()
{
	CompositionCaretOffset = 0;
	InputComp->DeleteIfSelection(CompositionCaretOffset);
	//UE_LOG(LGUI, Log, TEXT("BeginComposition, CompositionCaretOffset:%d"), CompositionCaretOffset);
	if (!bIsComposing)
	{
		bIsComposing = true;
	}
}
void UUITextInputComponent::FTextInputMethodContext::UpdateCompositionRange(const int32 InBeginIndex, const uint32 InLength)
{
	//UE_LOG(LogTemp, Log, TEXT("UpdateCompositionRange"));
	if (bIsComposing)
	{
		CompositionBeginIndex = InBeginIndex;
		CompositionLength = InLength;
	}
}
void UUITextInputComponent::FTextInputMethodContext::EndComposition()
{
	//UE_LOG(LGUI, Log, TEXT("EndComposition"));
	if (bIsComposing)
	{
		bIsComposing = false;
	}
}