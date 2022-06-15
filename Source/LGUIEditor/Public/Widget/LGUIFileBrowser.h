// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Widgets/SCompoundWidget.h"
#include "DesktopPlatformModule.h"
#include "DetailLayoutBuilder.h"
#pragma once

#define LOCTEXT_NAMESPACE "LGUIFileBrowser"

DECLARE_DELEGATE_OneParam(FOnStringChanged, const FString&);
/**
 * 
 */
class SLGUIFileBrowser : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGUIFileBrowser)
		
		//: _LabelBackgroundColor(FLinearColor::Black)
		//, _LabelBackgroundBrush(FEditorStyle::GetBrush("WhiteBrush"))
	{}
	/** Attribute specifying the text to display in the folder input */
	SLATE_ATTRIBUTE(FText, FolderPath)
	SLATE_ATTRIBUTE(FString, DialogTitle)
	SLATE_ATTRIBUTE(FString, DefaultFileName)
	SLATE_ATTRIBUTE(FString, Filter)
	SLATE_EVENT(FOnStringChanged, OnFilePathChanged)
	SLATE_EVENT(FOnStringChanged, OnFilePathCommitted)
	//SLATE_EVENT(FOnTextCommitted, OnFilePathChanged)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		TSharedRef<SBox> MainContent =
			SNew(SBox)
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				[
					SAssignNew(FolderPathTextBox, SEditableTextBox)
					.Text(InArgs._FolderPath)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Padding(FMargin(5,3,25,3))
					.OnTextCommitted(this, &SLGUIFileBrowser::OnPathTextCommited)
					.OnTextChanged(this, &SLGUIFileBrowser::OnPathTextChanged)
				]
				+SOverlay::Slot()
				.HAlign(HAlign_Right)
				[
					SNew(SButton)
					.ContentPadding(FMargin(4.0f, 0.0f))
					.OnClicked(this, &SLGUIFileBrowser::OnBrowseButtonClicked, InArgs._DialogTitle, InArgs._FolderPath, InArgs._DefaultFileName, InArgs._Filter)
					.ToolTipText(LOCTEXT("BrowseForFile", "Browse for a file"))
					.Text(LOCTEXT("...", "..."))
				]
			]

		; ChildSlot
			[
				MainContent
			];

		OnFilePathChanged = InArgs._OnFilePathChanged;
		OnFilePathCommitted = InArgs._OnFilePathCommitted;
	}

private:
	FReply OnBrowseButtonClicked(TAttribute<FString> DialogTitle, TAttribute<FText> FilePath, TAttribute<FString> DefaultFileName, TAttribute<FString> Filter)
	{
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
		if (DesktopPlatform)
		{
			TArray<FString> OutFileNames;
			auto fileSelected = DesktopPlatform->OpenFileDialog(
				FSlateApplication::Get().FindBestParentWindowHandleForDialogs(AsShared()),
				DialogTitle.Get(),
				FilePath.Get().ToString(),
				DefaultFileName.Get(),
				Filter.Get(),
				EFileDialogFlags::None,
				OutFileNames
			);

			if(fileSelected)
			{
				auto& fileManager = IFileManager::Get();
				auto fileName = fileManager.GetFilenameOnDisk(*OutFileNames[0]);
				OnPathTextCommited(FText::FromString(fileName), ETextCommit::Default);
			}
		}

		return FReply::Handled();
	}
	void OnPathTextChanged(const FText& InText)
	{
		auto FilePathStr = InText.ToString();
		OnFilePathChanged.ExecuteIfBound(FilePathStr);
	}
	void OnPathTextCommited(const FText& NewText, ETextCommit::Type CommitInfo)
	{
		auto FilePathStr = NewText.ToString();
		OnFilePathCommitted.ExecuteIfBound(FilePathStr);
	}
private:
	TSharedPtr<SEditableTextBox> FolderPathTextBox;
	FOnStringChanged OnFilePathChanged;
	FOnStringChanged OnFilePathCommitted;
};

#undef LOCTEXT_NAMESPACE