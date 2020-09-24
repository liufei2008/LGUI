// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Widgets/SCompoundWidget.h"
#include "DesktopPlatformModule.h"
#pragma once

#define LOCTEXT_NAMESPACE "LGUIEnumComboBox"

DECLARE_DELEGATE_OneParam(FOnEnumSelectionChanged, int32 /*Selection*/);
/**
 * 
 */
class SEnumCombobox : public SComboBox<TSharedPtr<int32>>
{
public:
	SLATE_BEGIN_ARGS(SEnumCombobox) {}

	SLATE_ATTRIBUTE(int32, CurrentValue)
	SLATE_EVENT(FOnEnumSelectionChanged, OnEnumSelectionChanged)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const UEnum* InEnum)
	{
		Enum = InEnum;
		CurrentValue = InArgs._CurrentValue;
		check(CurrentValue.IsBound());
		OnEnumSelectionChangedDelegate = InArgs._OnEnumSelectionChanged;

		bUpdatingSelectionInternally = false;

		for (int32 i = 0; i < Enum->NumEnums() - 1; i++)
		{
			if (Enum->HasMetaData(TEXT("Hidden"), i) == false)
			{
				VisibleEnumNameIndices.Add(MakeShareable(new int32(i)));
			}
		}

		SComboBox::Construct(SComboBox<TSharedPtr<int32>>::FArguments()
			.OptionsSource(&VisibleEnumNameIndices)
			.OnGenerateWidget_Lambda([this](TSharedPtr<int32> InItem)
		{
			return SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(Enum->GetDisplayNameTextByIndex(*InItem));
		})
			.OnSelectionChanged(this, &SEnumCombobox::OnComboSelectionChanged)
			.OnComboBoxOpening(this, &SEnumCombobox::OnComboMenuOpening)
			.ContentPadding(FMargin(2, 0))
			[
				SNew(STextBlock)
				.Font(FEditorStyle::GetFontStyle("Sequencer.AnimationOutliner.RegularFont"))
				.Text(this, &SEnumCombobox::GetCurrentValue)
			]);
	}

private:
	FText GetCurrentValue() const
	{
		int32 CurrentNameIndex = Enum->GetIndexByValue(CurrentValue.Get());
		return Enum->GetDisplayNameTextByIndex(CurrentNameIndex);
	}

	TSharedRef<SWidget> OnGenerateWidget(TSharedPtr<int32> InItem)
	{
		return SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(Enum->GetDisplayNameTextByIndex(*InItem));
	}

	void OnComboSelectionChanged(TSharedPtr<int32> InSelectedItem, ESelectInfo::Type SelectInfo)
	{
		if (bUpdatingSelectionInternally == false)
		{
			OnEnumSelectionChangedDelegate.ExecuteIfBound(*InSelectedItem);
		}
	}

	void OnComboMenuOpening()
	{
		int32 CurrentNameIndex = Enum->GetIndexByValue(CurrentValue.Get());
		TSharedPtr<int32> FoundNameIndexItem;
		for (int32 i = 0; i < VisibleEnumNameIndices.Num(); i++)
		{
			if (*VisibleEnumNameIndices[i] == CurrentNameIndex)
			{
				FoundNameIndexItem = VisibleEnumNameIndices[i];
				break;
			}
		}
		if (FoundNameIndexItem.IsValid())
		{
			bUpdatingSelectionInternally = true;
			SetSelectedItem(FoundNameIndexItem);
			bUpdatingSelectionInternally = false;
		}
	}

private:
	const UEnum* Enum;

	TAttribute<int32> CurrentValue;

	TArray<TSharedPtr<int32>> VisibleEnumNameIndices;

	bool bUpdatingSelectionInternally;

	FOnEnumSelectionChanged OnEnumSelectionChangedDelegate;
};
#undef LOCTEXT_NAMESPACE