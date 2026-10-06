// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LexUIFontData_FreeTypeRenderCustomization.h"
#include "Misc/FileHelper.h"
#include "Core/LexUIFontData_FreeTypeRender.h"
#include "Widget/LexUIFileBrowser.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "LGUIFreeTypeRenderFontDataCustomization"

TSharedRef<IDetailCustomization> FLexUIFontData_FreeTypeRenderCustomization::MakeInstance()
{
	return MakeShareable(new FLexUIFontData_FreeTypeRenderCustomization);
}

void FLexUIFontData_FreeTypeRenderCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULexUIFontData_FreeTypeRender>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[%s].%d Get TargetScript is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}

	auto fontTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, FontType));
	fontTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateRaw(this, &FLexUIFontData_FreeTypeRenderCustomization::ForceRefresh, &DetailBuilder));
	uint8 fontTypeUint8;
	fontTypeHandle->GetValue(fontTypeUint8);
	auto fontType = (ELexUIDynamicFontDataType)fontTypeUint8;

	IDetailCategoryBuilder& lguiCategory = DetailBuilder.EditCategory("LexUI");
	lguiCategory.AddCustomRow(LOCTEXT("ReloadFont", "ReloadFont"))
	.WholeRowContent()
	[
		SNew(SButton)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &FLexUIFontData_FreeTypeRenderCustomization::OnReloadButtonClicked, &DetailBuilder)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReloadFont", "ReloadFont"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	]
	;
	lguiCategory.AddProperty(fontTypeHandle);
	TArray<FName> propertiesNeedToHide;
	if (fontType == ELexUIDynamicFontDataType::EngineFont)
	{
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, FontFilePath));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, bUseRelativeFilePath));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, bUseExternalFileOrEmbedInToUAsset));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, FontFace));

		lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, EngineFont));
	}
	else
	{
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, FontFilePath));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, bUseRelativeFilePath));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, bUseExternalFileOrEmbedInToUAsset));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, EngineFont));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, FontFace));

		auto fontFilePathHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, FontFilePath));
		lguiCategory.AddCustomRow(LOCTEXT("FontSourceFileCategory","FontSourceFile"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FontSourceFile", "Font Source File"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		.MinDesiredWidth(600)
		[	
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.MaxWidth(500)
			[
				SNew(SLexUIFileBrowser)
				.FolderPath(this, &FLexUIFontData_FreeTypeRenderCustomization::OnGetFontFilePath)
				.DialogTitle(TEXT("Browse for a font data file"))
				.DefaultFileName("font.ttf")
				.Filter(TEXT("Font file(*.ttf,*.ttc,*.otf)|*.ttf;*.ttc;*.otf|Any font file|*.*"))
				.OnFilePathChanged(this, &FLexUIFontData_FreeTypeRenderCustomization::OnPathTextChanged, fontFilePathHandle)
				.OnFilePathCommitted(this, &FLexUIFontData_FreeTypeRenderCustomization::OnPathTextCommitted, fontFilePathHandle, &DetailBuilder)
			]
			+SHorizontalBox::Slot()
			.MaxWidth(100)
			.Padding(FMargin(5, 0, 0, 0))
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([&]() {return TargetScriptPtr->bUseRelativeFilePath ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([&](ECheckBoxState State) 
					{
						TargetScriptPtr->bUseRelativeFilePath = State == ECheckBoxState::Checked ? true : false; 
						TargetScriptPtr->ReloadFont();
						DetailBuilder.ForceRefreshDetails();
					})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("UseRelativePath","Relative To \"ProjectDir\""))
					.ToolTipText(LOCTEXT("Tooltip", "Font file use relative path(relative to ProjectDir) or absolute path. After build your game, remember to copy your font file to target path, unless \"UseExternalFileOrEmbedInToUAsset\" is false"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
		]
		;
		TargetScriptPtr->InitFreeType();
		if (TargetScriptPtr->bAlreadyInitialized == false)
		{
			lguiCategory.AddCustomRow(LOCTEXT("ErrorTip", "ErrorTip"))
			.WholeRowContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("InitializeFontFail", "Initialize font fail, check outputlog for detail"))
				.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			;
		}
		lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, bUseExternalFileOrEmbedInToUAsset));
	}

	//faces
	FontFaceOptions.Empty();
	for (auto Face : TargetScriptPtr->SubFaces)
	{
		FontFaceOptions.Add(MakeShareable(new FString(Face)));
	}
	auto fontFaceHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexUIFontData_FreeTypeRender, FontFace));
	lguiCategory.AddCustomRow(LOCTEXT("FontFace", "FontFace"))
		.NameContent()
		[
			fontFaceHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SComboButton)
			.HasDownArrow(true)
			.ButtonContent()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Font(DetailBuilder.GetDetailFont())
					.Text(this, &FLexUIFontData_FreeTypeRenderCustomization::FontFaceOptions_GetCurrentFace)
				]
			]
			.MenuContent()
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SListView<TSharedPtr<FString>>)
					.ListItemsSource(&FontFaceOptions)
					.OnGenerateRow(this, &FLexUIFontData_FreeTypeRenderCustomization::FontFaceOptions_GenerateComboItem, &DetailBuilder)
					.OnSelectionChanged(this, &FLexUIFontData_FreeTypeRenderCustomization::FontFaceOptions_OnComboChanged, fontFaceHandle, &DetailBuilder)
				]
			]
		]
		;

	for (auto& propertyName : propertiesNeedToHide)
	{
		DetailBuilder.HideProperty(propertyName);
	}
}

FText FLexUIFontData_FreeTypeRenderCustomization::FontFaceOptions_GetCurrentFace()const
{
	if (TargetScriptPtr->SubFaces.Num() == 0 || TargetScriptPtr->FontFace >= TargetScriptPtr->SubFaces.Num())
	{
		return LOCTEXT("NoFontFace", "(No Valid Face)");
	}
	return FText::FromString(TargetScriptPtr->SubFaces[TargetScriptPtr->FontFace]);
}

TSharedRef<ITableRow> FLexUIFontData_FreeTypeRenderCustomization::FontFaceOptions_GenerateComboItem(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable, IDetailLayoutBuilder* DetailBuilder)
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(SBox)
			.Padding(FMargin(8, 2))
			[
				SNew(STextBlock)
				.Font(DetailBuilder->GetDetailFont())
				.Text(FText::FromString(*InItem))
			]
		];
}

void FLexUIFontData_FreeTypeRenderCustomization::FontFaceOptions_OnComboChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> InProperty, IDetailLayoutBuilder* DetailBuilder)
{
	int FoundIndex = FontFaceOptions.IndexOfByKey(Item);
	if (FoundIndex != INDEX_NONE)
	{
		InProperty->SetValue(FoundIndex);
		DetailBuilder->ForceRefreshDetails();
	}
}

FText FLexUIFontData_FreeTypeRenderCustomization::OnGetFontFilePath()const
{
	auto& fileManager = IFileManager::Get();
	return FText::FromString(TargetScriptPtr->FontFilePath.IsEmpty() ? fileManager.GetFilenameOnDisk(*FPaths::ProjectDir()) : TargetScriptPtr->FontFilePath);
}

FText FLexUIFontData_FreeTypeRenderCustomization::GetCurrentValue() const
{
	auto faceName = TargetScriptPtr->SubFaces[TargetScriptPtr->FontFace];
	return FText::FromString(faceName);
}
void FLexUIFontData_FreeTypeRenderCustomization::OnFontFaceComboSelectionChanged(TSharedPtr<FString> InSelectedItem, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> fontFaceHandle)
{
	int selectedIndex = 0;
	for (int i = 0; i < TargetScriptPtr->SubFaces.Num(); i++)
	{
		if (TargetScriptPtr->SubFaces[i] == *InSelectedItem)
		{
			selectedIndex = i;
		}
	}
	fontFaceHandle->SetValue(selectedIndex);
}

void FLexUIFontData_FreeTypeRenderCustomization::OnPathTextChanged(const FString& InString, TSharedRef<IPropertyHandle> InPathProperty)
{
	InPathProperty->SetValue(InString);
}
void FLexUIFontData_FreeTypeRenderCustomization::OnPathTextCommitted(const FString& InString, TSharedRef<IPropertyHandle> InPathProperty, IDetailLayoutBuilder* DetailBuilderPtr)
{
	FString pathString = InString;
	if (TargetScriptPtr->bUseRelativeFilePath)
	{
		if (pathString.StartsWith(FPaths::ProjectDir()))//is relative path
		{
			pathString.RemoveFromStart(FPaths::ProjectDir(), ESearchCase::CaseSensitive);
		}
	}
	InPathProperty->SetValue(pathString);
	TargetScriptPtr->ReloadFont();
	DetailBuilderPtr->ForceRefreshDetails();
}
FReply FLexUIFontData_FreeTypeRenderCustomization::OnReloadButtonClicked(IDetailLayoutBuilder* DetailBuilderPtr)
{
	TargetScriptPtr->ReloadFont();
	DetailBuilderPtr->ForceRefreshDetails();
	return FReply::Handled();
}
void FLexUIFontData_FreeTypeRenderCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilderPtr)
{
	if (DetailBuilderPtr)
	{
		DetailBuilderPtr->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE