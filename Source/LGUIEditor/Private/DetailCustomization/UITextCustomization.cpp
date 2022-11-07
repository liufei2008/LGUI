// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/UITextCustomization.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Core/ActorComponent/UIText.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "UITextCustomization"
FUITextCustomization::FUITextCustomization()
{
}

FUITextCustomization::~FUITextCustomization()
{
}

TSharedRef<IDetailCustomization> FUITextCustomization::MakeInstance()
{
	return MakeShareable(new FUITextCustomization);
}
void FUITextCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUIText>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[UITextCustomization]Get TargetScript is null"));
		return;
	}
	
	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIText, font));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIText, text));

	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIText, size));
	category.AddProperty(GET_MEMBER_NAME_CHECKED(UUIText, space));

	//text alignment
	{
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIText, hAlign));
		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UUIText, vAlign));
		const FMargin OuterPadding(2, 0);
		const FMargin ContentPadding(2);
		auto hAlignPropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIText, hAlign));
		auto vAlignPropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIText, vAlign));
		category.AddCustomRow(LOCTEXT("Alignment", "Alignment"))
		.CopyAction(FUIAction(
			FExecuteAction::CreateSP(this, &FUITextCustomization::OnCopyAlignment)
		))
		.PasteAction(FUIAction(
			FExecuteAction::CreateSP(this, &FUITextCustomization::OnPasteAlignment, hAlignPropertyHandle, vAlignPropertyHandle),
			FCanExecuteAction::CreateSP(this, &FUITextCustomization::OnCanPasteAlignment)
		))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Alignment", "Alignment"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(OuterPadding)
				[
					SNew( SCheckBox )
					.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
					.ToolTipText(LOCTEXT("AlignTextLeft", "Align Text Left"))
					.Padding(ContentPadding)
					.OnCheckStateChanged(this, &FUITextCustomization::HandleHorizontalAlignmentCheckStateChanged, hAlignPropertyHandle, UITextParagraphHorizontalAlign::Left)
					.IsChecked(this, &FUITextCustomization::GetHorizontalAlignmentCheckState, hAlignPropertyHandle, UITextParagraphHorizontalAlign::Left)
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("HorizontalAlignment_Left"))
					]
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(OuterPadding)
				[
					SNew(SCheckBox)
					.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
					.ToolTipText(LOCTEXT("AlignTextCenter", "Align Text Center"))
					.Padding(ContentPadding)
					.OnCheckStateChanged(this, &FUITextCustomization::HandleHorizontalAlignmentCheckStateChanged, hAlignPropertyHandle, UITextParagraphHorizontalAlign::Center)
					.IsChecked(this, &FUITextCustomization::GetHorizontalAlignmentCheckState, hAlignPropertyHandle, UITextParagraphHorizontalAlign::Center)
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("HorizontalAlignment_Center"))
					]
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(OuterPadding)
				[
					SNew(SCheckBox)
					.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
					.ToolTipText(LOCTEXT("AlignTextRight", "Align Text Right"))
					.Padding(ContentPadding)
					.OnCheckStateChanged(this, &FUITextCustomization::HandleHorizontalAlignmentCheckStateChanged, hAlignPropertyHandle, UITextParagraphHorizontalAlign::Right)
					.IsChecked(this, &FUITextCustomization::GetHorizontalAlignmentCheckState, hAlignPropertyHandle, UITextParagraphHorizontalAlign::Right)
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("HorizontalAlignment_Right"))
					]
				]
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(2, 0))
			[
				SNew(SBox)
				.WidthOverride(1)
				[
					SNew(SImage)
					.Image(FEditorStyle::GetBrush("PropertyEditor.VerticalDottedLine"))
					.ColorAndOpacity(FLinearColor(1, 1, 1, 0.2f))
				]
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(OuterPadding)
				[
					SNew( SCheckBox )
					.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
					.ToolTipText(LOCTEXT("VAlignTop", "Vertically Align Top"))
					.Padding(ContentPadding)
					.OnCheckStateChanged(this, &FUITextCustomization::HandleVerticalAlignmentCheckStateChanged, vAlignPropertyHandle, UITextParagraphVerticalAlign::Top)
					.IsChecked(this, &FUITextCustomization::GetVerticalAlignmentCheckState, vAlignPropertyHandle, UITextParagraphVerticalAlign::Top)
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("VerticalAlignment_Top"))
					]
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(OuterPadding)
				[
					SNew(SCheckBox)
					.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
					.ToolTipText(LOCTEXT("VAlignMiddle", "Vertically Align Middle"))
					.Padding(ContentPadding)
					.OnCheckStateChanged(this, &FUITextCustomization::HandleVerticalAlignmentCheckStateChanged, vAlignPropertyHandle, UITextParagraphVerticalAlign::Middle)
					.IsChecked(this, &FUITextCustomization::GetVerticalAlignmentCheckState, vAlignPropertyHandle, UITextParagraphVerticalAlign::Middle)
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("VerticalAlignment_Center"))
					]
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(OuterPadding)
				[
					SNew(SCheckBox)
					.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
					.ToolTipText(LOCTEXT("VAlignBottom", "Vertically Align Bottom"))
					.Padding(ContentPadding)
					.OnCheckStateChanged(this, &FUITextCustomization::HandleVerticalAlignmentCheckStateChanged, vAlignPropertyHandle, UITextParagraphVerticalAlign::Bottom)
					.IsChecked(this, &FUITextCustomization::GetVerticalAlignmentCheckState, vAlignPropertyHandle, UITextParagraphVerticalAlign::Bottom)
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("VerticalAlignment_Bottom"))
					]
				]
			]
		]
		;
	}

	auto OverflowTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIText, overflowType));
	OverflowTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUITextCustomization::ForceRefresh, &DetailBuilder));
	auto AdjustWidthHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIText, adjustWidth));
	AdjustWidthHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUITextCustomization::ForceRefresh, &DetailBuilder));
	auto AdjustHeightHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIText, adjustHeight));
	AdjustHeightHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FUITextCustomization::ForceRefresh, &DetailBuilder));

	uint8 OverflowType;
	OverflowTypeHandle->GetValue(OverflowType);
	TArray<FName> needToHidePropertyName;
	if (OverflowType == (uint8)(UITextOverflowType::HorizontalOverflow))
	{
		needToHidePropertyName.Add(GET_MEMBER_NAME_CHECKED(UUIText, adjustHeight));
	}
	else if (OverflowType == (uint8)(UITextOverflowType::VerticalOverflow))
	{
		needToHidePropertyName.Add(GET_MEMBER_NAME_CHECKED(UUIText, adjustWidth));
	}
	else
	{
		needToHidePropertyName.Add(GET_MEMBER_NAME_CHECKED(UUIText, adjustHeight));
		needToHidePropertyName.Add(GET_MEMBER_NAME_CHECKED(UUIText, adjustWidth));
	}

	for (auto item : needToHidePropertyName)
	{
		DetailBuilder.HideProperty(item);
	}

	auto fontHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIText, font));
	fontHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] {
		TargetScriptPtr->OnPostChangeFontProperty();
	}));
	fontHandle->SetOnPropertyValuePreChange(FSimpleDelegate::CreateLambda([=]{
		TargetScriptPtr->OnPreChangeFontProperty();
	}));
}
void FUITextCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (auto Script = TargetScriptPtr.Get())
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
void FUITextCustomization::HandleHorizontalAlignmentCheckStateChanged(ECheckBoxState InCheckboxState, TSharedRef<IPropertyHandle> PropertyHandle, UITextParagraphHorizontalAlign ToAlignment)
{
	PropertyHandle->SetValue((uint8)ToAlignment);
}
void FUITextCustomization::HandleVerticalAlignmentCheckStateChanged(ECheckBoxState InCheckboxState, TSharedRef<IPropertyHandle> PropertyHandle, UITextParagraphVerticalAlign ToAlignment)
{
	PropertyHandle->SetValue((uint8)ToAlignment);
}
ECheckBoxState FUITextCustomization::GetHorizontalAlignmentCheckState(TSharedRef<IPropertyHandle> PropertyHandle, UITextParagraphHorizontalAlign ForAlignment) const
{
	uint8 Value;
	if (PropertyHandle->GetValue(Value) == FPropertyAccess::Result::Success)
	{
		return Value == (uint8)ForAlignment ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	return ECheckBoxState::Unchecked;
}
ECheckBoxState FUITextCustomization::GetVerticalAlignmentCheckState(TSharedRef<IPropertyHandle> PropertyHandle, UITextParagraphVerticalAlign ForAlignment) const
{
	uint8 Value;
	if (PropertyHandle->GetValue(Value) == FPropertyAccess::Result::Success)
	{
		return Value == (uint8)ForAlignment ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	return ECheckBoxState::Unchecked;
}

#define BEGIN_ALIGNMENT_CLIPBOARD TEXT("Begin LGUI UIWidget")
void FUITextCustomization::OnCopyAlignment()
{
	if (TargetScriptPtr.IsValid())
	{
		FString CopiedText = FString::Printf(TEXT("%s, hAlign=%d, vAlign=%d"), BEGIN_ALIGNMENT_CLIPBOARD, (int)TargetScriptPtr->hAlign, (int)TargetScriptPtr->vAlign);
		FPlatformApplicationMisc::ClipboardCopy(*CopiedText);
	}
}
void FUITextCustomization::OnPasteAlignment(TSharedRef<IPropertyHandle> HAlignPropertyHandle, TSharedRef<IPropertyHandle> VAlignPropertyHandle)
{
	FString PastedText;
	FPlatformApplicationMisc::ClipboardPaste(PastedText);
	if (PastedText.StartsWith(BEGIN_ALIGNMENT_CLIPBOARD))
	{
		uint8 tempUInt8;
		FParse::Value(*PastedText, TEXT("hAlign="), tempUInt8);
		HAlignPropertyHandle->SetValue(tempUInt8);
		FParse::Value(*PastedText, TEXT("vAlign="), tempUInt8);
		VAlignPropertyHandle->SetValue(tempUInt8);
	}
}
bool FUITextCustomization::OnCanPasteAlignment()const
{
	FString PastedText;
	FPlatformApplicationMisc::ClipboardPaste(PastedText);
	return PastedText.StartsWith(BEGIN_ALIGNMENT_CLIPBOARD);
}
#undef LOCTEXT_NAMESPACE