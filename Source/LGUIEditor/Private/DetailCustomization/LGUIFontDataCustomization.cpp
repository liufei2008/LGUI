// Copyright 2019 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIFontDataCustomization.h"
#include "Misc/FileHelper.h"
#include "Core/LGUIFontData.h"
#include "Widget/LGUIFileBrowser.h"

#define LOCTEXT_NAMESPACE "LGUIFontDataCustomization"

TSharedRef<IDetailCustomization> FLGUIFontDataCustomization::MakeInstance()
{
	return MakeShareable(new FLGUIFontDataCustomization);
}

void FLGUIFontDataCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULGUIFontData>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
		return;
	}

	DetailBuilderPtr = &DetailBuilder;
	DetailBuilder.HideProperty("fontFilePath");
	IDetailCategoryBuilder& lguiCategory = DetailBuilder.EditCategory("LGUI");

	DetailBuilder.HideProperty("useRelativeFilePath");
	auto& fileManager = IFileManager::Get();
	lguiCategory.AddCustomRow(LOCTEXT("FontSourceFile","FontSourceFile"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FontSourceFile", "FontSourceFile"))
		]
		.ValueContent()
		.MinDesiredWidth(600)
		[	
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.MaxWidth(500)
			[
				SAssignNew(LGUIFontSourceFileBrowser, SLGUIFileBrowser)
				.FolderPath(TargetScriptPtr->fontFilePath.IsEmpty() ? fileManager.GetFilenameOnDisk(*FPaths::ProjectDir()) : TargetScriptPtr->fontFilePath)
				.DialogTitle(TEXT("Browse for a font data file"))
				.DefaultFileName("font.ttf")
				.Filter(TEXT("Font file(*.ttf,*.ttc,*.otf)|*.ttf;*.ttc;*.otf|Any font file|*.*"))
				.OnFilePathChanged(this, &FLGUIFontDataCustomization::OnPathTextChanged)
				.OnFilePathCommitted(this, &FLGUIFontDataCustomization::OnPathTextCommitted)
			]
			+SHorizontalBox::Slot()
			.MaxWidth(100)
			.Padding(FMargin(5, 0, 0, 0))
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([&]() {return TargetScriptPtr->useRelativeFilePath ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([&](ECheckBoxState State) {TargetScriptPtr->useRelativeFilePath = State == ECheckBoxState::Checked ? true : false; })
				[
					SNew(STextBlock)
					.Text(LOCTEXT("UseRelativePath","Relative To \"ProjectDir\""))
					.ToolTipText(LOCTEXT("Tooltip", "Font file use relative path(relative to ProjectDir) or absolute path. After build your game, remember to copy your font file to target path, unless \"UseExternalFileOrEmbedInToUAsset\" is false"))
				]
			]
		];
	TargetScriptPtr->InitFreeType();
	if (TargetScriptPtr->alreadyInitialized == false)
	{
		lguiCategory.AddCustomRow(LOCTEXT("ErrorTip", "ErrorTip"))
			.WholeRowContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Initialize font fail, check outputlog for detail")))
				.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
			];
	}
	lguiCategory.AddProperty("useExternalFileOrEmbedInToUAsset");

	lguiCategory.AddCustomRow(LOCTEXT("ReloadFont", "ReloadFont"))
		.WholeRowContent()
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.OnClicked(this, &FLGUIFontDataCustomization::OnReloadButtonClicked)
			.Text(LOCTEXT("ReloadFont", "ReloadFont"))
		];
}
void FLGUIFontDataCustomization::OnPathTextChanged(const FString& InString)
{
	TargetScriptPtr->fontFilePath = InString;
	TargetScriptPtr->MarkPackageDirty();
}
void FLGUIFontDataCustomization::OnPathTextCommitted(const FString& InString)
{
	FString pathString = InString;
	if (TargetScriptPtr->useRelativeFilePath)
	{
		if (pathString.StartsWith(FPaths::ProjectDir()))//is relative path
		{
			pathString.RemoveFromStart(FPaths::ProjectDir(), ESearchCase::CaseSensitive);
		}
	}
	TargetScriptPtr->fontFilePath = pathString;
	TargetScriptPtr->MarkPackageDirty();
	TargetScriptPtr->DeinitFreeType();
	ForceRefresh();
}
FReply FLGUIFontDataCustomization::OnReloadButtonClicked()
{
	TargetScriptPtr->ReloadFont();
	return FReply::Handled();
}
void FLGUIFontDataCustomization::ForceRefresh()
{
	if (DetailBuilderPtr)
	{
		DetailBuilderPtr->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE