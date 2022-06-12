// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIFreeTypeRenderFontDataCustomization.h"
#include "Misc/FileHelper.h"
#include "Core/LGUIFreeTypeRenderFontData.h"
#include "Widget/LGUIFileBrowser.h"
#include "Widgets/Input/STextComboBox.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "LGUIEditorTools.h"
#include "LGUIHeaders.h"

#define LOCTEXT_NAMESPACE "LGUIFreeTypeRenderFontDataCustomization"

TSharedRef<IDetailCustomization> FLGUIFreeTypeRenderFontDataCustomization::MakeInstance()
{
	return MakeShareable(new FLGUIFreeTypeRenderFontDataCustomization);
}

void FLGUIFreeTypeRenderFontDataCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULGUIFreeTypeRenderFontData>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
		return;
	}

	auto fontTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, fontType));
	fontTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateRaw(this, &FLGUIFreeTypeRenderFontDataCustomization::ForceRefresh, &DetailBuilder));
	uint8 fontTypeUint8;
	fontTypeHandle->GetValue(fontTypeUint8);
	auto fontType = (ELGUIDynamicFontDataType)fontTypeUint8;

	IDetailCategoryBuilder& lguiCategory = DetailBuilder.EditCategory("LGUI");
	lguiCategory.AddCustomRow(LOCTEXT("ReloadFont", "ReloadFont"))
	.WholeRowContent()
	[
		SNew(SButton)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &FLGUIFreeTypeRenderFontDataCustomization::OnReloadButtonClicked, &DetailBuilder)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReloadFont", "ReloadFont"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	]
	;
	lguiCategory.AddProperty(fontTypeHandle);
	TArray<FName> propertiesNeedToHide;
	if (fontType == ELGUIDynamicFontDataType::UnrealFont)
	{
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, fontFilePath));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, useRelativeFilePath));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, useExternalFileOrEmbedInToUAsset));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, fontFace));

		lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, unrealFont));
	}
	else
	{
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, fontFilePath));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, useRelativeFilePath));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, useExternalFileOrEmbedInToUAsset));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, unrealFont));
		propertiesNeedToHide.Add(GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, fontFace));

		auto fontFilePathHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, fontFilePath));
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
				.FolderPath(this, &FLGUIFreeTypeRenderFontDataCustomization::OnGetFontFilePath)
				.DialogTitle(TEXT("Browse for a font data file"))
				.DefaultFileName("font.ttf")
				.Filter(TEXT("Font file(*.ttf,*.ttc,*.otf)|*.ttf;*.ttc;*.otf|Any font file|*.*"))
				.OnFilePathChanged(this, &FLGUIFreeTypeRenderFontDataCustomization::OnPathTextChanged, fontFilePathHandle)
				.OnFilePathCommitted(this, &FLGUIFreeTypeRenderFontDataCustomization::OnPathTextCommitted, fontFilePathHandle, &DetailBuilder)
			]
			+SHorizontalBox::Slot()
			.MaxWidth(100)
			.Padding(FMargin(5, 0, 0, 0))
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([&]() {return TargetScriptPtr->useRelativeFilePath ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([&](ECheckBoxState State) 
					{
						TargetScriptPtr->useRelativeFilePath = State == ECheckBoxState::Checked ? true : false; 
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
		lguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, useExternalFileOrEmbedInToUAsset));
	}

	//faces
	FontFaceOptions.Empty();
	for (auto Face : TargetScriptPtr->subFaces)
	{
		FontFaceOptions.Add(MakeShareable(new FString(Face)));
	}
	auto fontFaceHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, fontFace));
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
					.Text(this, &FLGUIFreeTypeRenderFontDataCustomization::FontFaceOptions_GetCurrentFace)
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
					.OnGenerateRow(this, &FLGUIFreeTypeRenderFontDataCustomization::FontFaceOptions_GenerateComboItem, &DetailBuilder)
					.OnSelectionChanged(this, &FLGUIFreeTypeRenderFontDataCustomization::FontFaceOptions_OnComboChanged, fontFaceHandle, &DetailBuilder)
				]
			]
		]
		;
	//packing tag
	auto PackingTagProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIFreeTypeRenderFontData, packingTag));
	DetailBuilder.HideProperty(PackingTagProperty);
	RefreshNameList(nullptr);
	if (ULGUIAtlasManager::Instance != nullptr)
	{
		ULGUIAtlasManager::Instance->OnAtlasMapChanged.AddSP(this, &FLGUIFreeTypeRenderFontDataCustomization::RefreshNameList, &DetailBuilder);
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
						.OnTextCommitted(this, &FLGUIFreeTypeRenderFontDataCustomization::OnPackingTagTextCommited, PackingTagProperty, &DetailBuilder)
						.Text(this, &FLGUIFreeTypeRenderFontDataCustomization::GetPackingTagText, PackingTagProperty)
						.Font(DetailBuilder.GetDetailFont())
					]
				]
				.MenuContent()
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SListView<TSharedPtr<FName>>)
						.ListItemsSource(&PackingTagOptions)
						.OnGenerateRow(this, &FLGUIFreeTypeRenderFontDataCustomization::PackingTagOptions_GenerateComboItem, &DetailBuilder)
						.OnSelectionChanged(this, &FLGUIFreeTypeRenderFontDataCustomization::PackingTagOptions_OnComboChanged, PackingTagProperty, &DetailBuilder)
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

void FLGUIFreeTypeRenderFontDataCustomization::RefreshNameList(IDetailLayoutBuilder* DetailBuilder)
{
	PackingTagOptions.Reset();
	PackingTagOptions.Add(MakeShareable(new FName(NAME_None)));
	if (ULGUIAtlasManager::Instance != nullptr)
	{
		auto& AtlasMap = ULGUIAtlasManager::Instance->GetAtlasMap();
		for (auto KeyValue : AtlasMap)
		{
			PackingTagOptions.Add(TSharedPtr<FName>(new FName(KeyValue.Key)));
		}
	}
	if (DetailBuilder != nullptr)
	{
		DetailBuilder->ForceRefreshDetails();
	}
}

void FLGUIFreeTypeRenderFontDataCustomization::OnPackingTagTextCommited(const FText& InText, ETextCommit::Type CommitType, TSharedRef<IPropertyHandle> InProperty, IDetailLayoutBuilder* DetailBuilder)
{
	FName packingTag = FName(InText.ToString());
	InProperty->SetValue(packingTag);
	DetailBuilder->ForceRefreshDetails();
}

FText FLGUIFreeTypeRenderFontDataCustomization::FontFaceOptions_GetCurrentFace()const
{
	if (TargetScriptPtr->subFaces.Num() == 0 || TargetScriptPtr->fontFace >= TargetScriptPtr->subFaces.Num())
	{
		return LOCTEXT("NoFontFace", "(No Valid Face)");
	}
	return FText::FromString(TargetScriptPtr->subFaces[TargetScriptPtr->fontFace]);
}

TSharedRef<ITableRow> FLGUIFreeTypeRenderFontDataCustomization::FontFaceOptions_GenerateComboItem(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable, IDetailLayoutBuilder* DetailBuilder)
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

void FLGUIFreeTypeRenderFontDataCustomization::FontFaceOptions_OnComboChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> InProperty, IDetailLayoutBuilder* DetailBuilder)
{
	int FoundIndex = FontFaceOptions.IndexOfByKey(Item);
	if (FoundIndex != INDEX_NONE)
	{
		InProperty->SetValue(FoundIndex);
		DetailBuilder->ForceRefreshDetails();
	}
}

TSharedRef<ITableRow> FLGUIFreeTypeRenderFontDataCustomization::PackingTagOptions_GenerateComboItem(TSharedPtr<FName> InItem, const TSharedRef<STableViewBase>& OwnerTable, IDetailLayoutBuilder* DetailBuilder)
{
	return SNew(STableRow<TSharedPtr<FName>>, OwnerTable)
		[
			SNew(SBox)
			.Padding(FMargin(8, 2))
			[
				SNew(STextBlock)
				.Font(DetailBuilder->GetDetailFont())
				.Text(FText::FromName(*InItem))
			]
		];
}

void FLGUIFreeTypeRenderFontDataCustomization::PackingTagOptions_OnComboChanged(TSharedPtr<FName> Item, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> InProperty, IDetailLayoutBuilder* DetailBuilder)
{
	InProperty->SetValue(*Item.Get());
	DetailBuilder->ForceRefreshDetails();
}

FText FLGUIFreeTypeRenderFontDataCustomization::GetPackingTagText(TSharedRef<IPropertyHandle> InProperty)const
{
	FName packingTag;
	InProperty->GetValue(packingTag);
	return FText::FromName(packingTag);
}

FText FLGUIFreeTypeRenderFontDataCustomization::OnGetFontFilePath()const
{
	auto& fileManager = IFileManager::Get();
	return FText::FromString(TargetScriptPtr->fontFilePath.IsEmpty() ? fileManager.GetFilenameOnDisk(*FPaths::ProjectDir()) : TargetScriptPtr->fontFilePath);
}

FText FLGUIFreeTypeRenderFontDataCustomization::GetCurrentValue() const
{
	auto faceName = TargetScriptPtr->subFaces[TargetScriptPtr->fontFace];
	return FText::FromString(faceName);
}
void FLGUIFreeTypeRenderFontDataCustomization::OnFontFaceComboSelectionChanged(TSharedPtr<FString> InSelectedItem, ESelectInfo::Type SelectInfo, TSharedRef<IPropertyHandle> fontFaceHandle)
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
void FLGUIFreeTypeRenderFontDataCustomization::OnFontFaceComboMenuOpening()
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

void FLGUIFreeTypeRenderFontDataCustomization::OnPathTextChanged(const FString& InString, TSharedRef<IPropertyHandle> InPathProperty)
{
	InPathProperty->SetValue(InString);
}
void FLGUIFreeTypeRenderFontDataCustomization::OnPathTextCommitted(const FString& InString, TSharedRef<IPropertyHandle> InPathProperty, IDetailLayoutBuilder* DetailBuilderPtr)
{
	FString pathString = InString;
	if (TargetScriptPtr->useRelativeFilePath)
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
FReply FLGUIFreeTypeRenderFontDataCustomization::OnReloadButtonClicked(IDetailLayoutBuilder* DetailBuilderPtr)
{
	TargetScriptPtr->ReloadFont();
	DetailBuilderPtr->ForceRefreshDetails();
	return FReply::Handled();
}
void FLGUIFreeTypeRenderFontDataCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilderPtr)
{
	if (DetailBuilderPtr)
	{
		DetailBuilderPtr->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE