﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIFontDataCustomization.h"
#include "Misc/FileHelper.h"
#include "Core/LGUIFontData.h"
#include "Widget/LGUIFileBrowser.h"
#include "Widgets/Input/STextComboBox.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "LGUIEditorTools.h"
#include "LGUIHeaders.h"

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

	auto fontTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIFontData, fontType));
	fontTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateRaw(this, &FLGUIFontDataCustomization::ForceRefresh, &DetailBuilder));
	uint8 fontTypeUint8;
	fontTypeHandle->GetValue(fontTypeUint8);
	auto fontType = (ELGUIDynamicFontDataType)fontTypeUint8;

	IDetailCategoryBuilder& lguiCategory = DetailBuilder.EditCategory("LGUI");
	lguiCategory.AddProperty(fontTypeHandle);
	TArray<FName> propertiesNeedToHide;
	if (fontType == ELGUIDynamicFontDataType::UnrealFont)
	{
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFontData, fontFilePath));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFontData, useRelativeFilePath));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFontData, useExternalFileOrEmbedInToUAsset));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFontData, fontFace));

		lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUIFontData, unrealFont));
	}
	else
	{
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFontData, fontFilePath));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFontData, useRelativeFilePath));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFontData, useExternalFileOrEmbedInToUAsset));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFontData, unrealFont));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFontData, fontFace));

		auto fontFilePathHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIFontData, fontFilePath));
		lguiCategory.AddCustomRow(LOCTEXT("FontSourceFile","FontSourceFile"))
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
				SNew(SLGUIFileBrowser)
				.FolderPath(this, &FLGUIFontDataCustomization::OnGetFontFilePath)
				.DialogTitle(TEXT("Browse for a font data file"))
				.DefaultFileName("font.ttf")
				.Filter(TEXT("Font file(*.ttf,*.ttc,*.otf)|*.ttf;*.ttc;*.otf|Any font file|*.*"))
				.OnFilePathChanged(this, &FLGUIFontDataCustomization::OnPathTextChanged, fontFilePathHandle)
				.OnFilePathCommitted(this, &FLGUIFontDataCustomization::OnPathTextCommitted, fontFilePathHandle, &DetailBuilder)
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
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
		]
		;
		TargetScriptPtr->InitFreeType();
		if (TargetScriptPtr->alreadyInitialized == false)
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
		lguiCategory.AddProperty("useExternalFileOrEmbedInToUAsset");
	}

	lguiCategory.AddCustomRow(LOCTEXT("ReloadFont", "ReloadFont"))
	.WholeRowContent()
	[
		SNew(SButton)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &FLGUIFontDataCustomization::OnReloadButtonClicked, &DetailBuilder)
		.Text(LOCTEXT("ReloadFont", "ReloadFont"))
	]
	;
	//faces
	faceSelections.Reset();
	auto count = TargetScriptPtr->subFaces.Num();
	TSharedPtr<FString> currentSelected;
	for (int i = 0; i < count; i++)
	{
		auto item = MakeShareable(new FString(TargetScriptPtr->subFaces[i]));
		faceSelections.Add(item);
		if (TargetScriptPtr->fontFace == i)
		{
			currentSelected = item;
		}
	}
	auto fontFaceHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIFontData, fontFace));
	lguiCategory.AddCustomRow(LOCTEXT("FontFace", "FontFace"))
		.NameContent()
		[
			fontFaceHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(STextComboBox)
			.OptionsSource(&faceSelections)
			.InitiallySelectedItem(currentSelected)
			.OnSelectionChanged(this, &FLGUIFontDataCustomization::OnFontFaceComboSelectionChanged, fontFaceHandle)
			.OnComboBoxOpening(this, &FLGUIFontDataCustomization::OnFontFaceComboMenuOpening)
		]
		;
	//packing tag
	auto PackingTagProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIFontData, packingTag));
	DetailBuilder.HideProperty(PackingTagProperty);
	RefreshNameList(nullptr);
	if (ULGUIAtlasManager::Instance != nullptr)
	{
		ULGUIAtlasManager::Instance->OnAtlasMapChanged.AddSP(this, &FLGUIFontDataCustomization::RefreshNameList, &DetailBuilder);
	}
	lguiCategory.AddCustomRow(LOCTEXT("PackingTag", "Packing Tag"))
	.NameContent()
	[
		PackingTagProperty->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.MinDesiredWidth(120)
			.Padding(FMargin(0, 2, 0, 2))
			[
				//PackingTagProperty->CreatePropertyValueWidget()
				SNew(SComboButton)
				.HasDownArrow(true)
				.ButtonContent()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					[
						SNew(SEditableText)
						.OnTextCommitted(this, &FLGUIFontDataCustomization::OnPackingTagTextCommited, PackingTagProperty, &DetailBuilder)
						.Text(this, &FLGUIFontDataCustomization::GetPackingTagText, PackingTagProperty)
					]
				]
				.MenuContent()
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SListView<TSharedPtr<FName>>)
						.ListItemsSource(&NameList)
						.OnGenerateRow(this, &FLGUIFontDataCustomization::GenerateComboItem)
						.OnSelectionChanged(this, &FLGUIFontDataCustomization::HandleRequiredParamComboChanged, PackingTagProperty, &DetailBuilder)
					]
				]
			]
		]
		+SHorizontalBox::Slot()
		.Padding(FMargin(2))
		.AutoWidth()
		[
			TargetScriptPtr->packingTag.IsNone() 
			?
			SNew(SBox)
			:
			SNew(SBox)
			[
			SNew(SButton)
			.VAlign(EVerticalAlignment::VAlign_Center)
			.Text(LOCTEXT("OpenAtals", "Open Atals Viewer"))
			.OnClicked_Lambda([]() {
			LGUIEditorTools::OpenAtlasViewer_Impl();
			return FReply::Handled();
				})
			]
		]
	]
	;

	for (auto& propertyName : propertiesNeedToHide)
	{
		DetailBuilder.HideProperty(propertyName);
	}
}

void FLGUIFontDataCustomization::RefreshNameList(IDetailLayoutBuilder* DetailBuilder)
{
	NameList.Reset();
	NameList.Add(MakeShareable(new FName(NAME_None)));
	if (ULGUIAtlasManager::Instance != nullptr)
	{
		auto& AtlasMap = ULGUIAtlasManager::Instance->GetAtlasMap();
		for (auto KeyValue : AtlasMap)
		{
			NameList.Add(TSharedPtr<FName>(new FName(KeyValue.Key)));
		}
	}
	if (DetailBuilder != nullptr)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}

void FLGUIFontDataCustomization::OnPackingTagTextCommited(const FText& InText, ETextCommit::Type CommitType, TSharedRef<IPropertyHandle> InProperty, IDetailLayoutBuilder* DetailBuilder)
{
	FName packingTag = FName(InText.ToString());
	InProperty->SetValue(packingTag);
	TargetScriptPtr->ReloadFont();
	DetailBuilder->ForceRefreshDetails();
}

TSharedRef<ITableRow> FLGUIFontDataCustomization::GenerateComboItem(TSharedPtr<FName> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FName>>, OwnerTable)
		[
			SNew(STextBlock).Text(FText::FromName(*InItem))
		];
}

void FLGUIFontDataCustomization::HandleRequiredParamComboChanged(TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> InProperty, IDetailLayoutBuilder* DetailBuilder)
{
	InProperty->SetValue(*Item.Get());
	TargetScriptPtr->ReloadFont();
	DetailBuilder->ForceRefreshDetails();
}

FText FLGUIFontDataCustomization::GetPackingTagText(TSharedRef<IPropertyHandle> InProperty)const
{
	FName packingTag;
	InProperty->GetValue(packingTag);
	return FText::FromName(packingTag);
}

FText FLGUIFontDataCustomization::OnGetFontFilePath()const
{
	auto& fileManager = IFileManager::Get();
	return FText::FromString(TargetScriptPtr->fontFilePath.IsEmpty() ? fileManager.GetFilenameOnDisk(*FPaths::ProjectDir()) : TargetScriptPtr->fontFilePath);
}

FText FLGUIFontDataCustomization::GetCurrentValue() const
{
	auto faceName = TargetScriptPtr->subFaces[TargetScriptPtr->fontFace];
	return FText::FromString(faceName);
}
void FLGUIFontDataCustomization::OnFontFaceComboSelectionChanged(TSharedPtr<FString> InSelectedItem, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> fontFaceHandle)
{
	int selectedIndex = 0;
	for (int i = 0; i < TargetScriptPtr->subFaces.Num(); i++)
	{
		if (TargetScriptPtr->subFaces[i] == *InSelectedItem)
		{
			selectedIndex = i;
		}
	}
	fontFaceHandle->SetValue(selectedIndex);
}
void FLGUIFontDataCustomization::OnFontFaceComboMenuOpening()
{
	//int32 CurrentNameIndex = TargetScriptPtr->fontFace;
	//TSharedPtr<int32> FoundNameIndexItem;
	//for (int32 i = 0; i < VisibleEnumNameIndices.Num(); i++)
	//{
	//	if (*VisibleEnumNameIndices[i] == CurrentNameIndex)
	//	{
	//		FoundNameIndexItem = VisibleEnumNameIndices[i];
	//		break;
	//	}
	//}
	//if (FoundNameIndexItem.IsValid())
	//{
	//	bUpdatingSelectionInternally = true;
	//	SetSelectedItem(FoundNameIndexItem);
	//	bUpdatingSelectionInternally = false;
	//}
}

void FLGUIFontDataCustomization::OnPathTextChanged(const FString& InString, TSharedRef<IPropertyHandle> InPathProperty)
{
	InPathProperty->SetValue(InString);
}
void FLGUIFontDataCustomization::OnPathTextCommitted(const FString& InString, TSharedRef<IPropertyHandle> InPathProperty, IDetailLayoutBuilder* DetailBuilderPtr)
{
	FString pathString = InString;
	if (TargetScriptPtr->useRelativeFilePath)
	{
		if (pathString.StartsWith(FPaths::ProjectDir()))//is relative path
		{
			pathString.RemoveFromStart(FPaths::ProjectDir(), ESearchCase::CaseSensitive);
		}
	}
	InPathProperty->SetValue(InString);
}
FReply FLGUIFontDataCustomization::OnReloadButtonClicked(IDetailLayoutBuilder* DetailBuilderPtr)
{
	TargetScriptPtr->ReloadFont();
	DetailBuilderPtr->ForceRefreshDetails();
	return FReply::Handled();
}
void FLGUIFontDataCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilderPtr)
{
	if (DetailBuilderPtr)
	{
		DetailBuilderPtr->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE