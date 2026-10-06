// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LexTextCustomization.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Core/Components/LexText.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailGroup.h"
#include "MaterialDomain.h"
#include "Core/LexUIFontData_BaseObject.h"
#include "PropertyType/LexTextAlignmentCustomization.h"
#include "PropertyType/LexTextFontStyleCustomization.h"

#define LOCTEXT_NAMESPACE "UITextCustomization"
FLexTextCustomization::FLexTextCustomization()
{
}

FLexTextCustomization::~FLexTextCustomization()
{
}

TSharedRef<IDetailCustomization> FLexTextCustomization::MakeInstance()
{
	return MakeShareable(new FLexTextCustomization);
}
void FLexTextCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULexText>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[%s].%d Get TargetScript is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}
	
	IDetailCategoryBuilder& LGUICategory = DetailBuilder.EditCategory("LexUI");
	auto Font_PH = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexText, Font));
	Font_PH->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLexTextCustomization::ForceRefresh, &DetailBuilder));
	LGUICategory.AddProperty(Font_PH);
	LGUICategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULexText, Text));

	LGUICategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULexText, FontSize));
	LGUICategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULexText, FontSpace));

	//text alignment
	{
		DetailBuilder.GetDetailsViewSharedPtr()->RegisterInstancedCustomPropertyTypeLayout(TEXT("ELexUITextParagraphHorizontalAlign"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLexTextAlignmentCustomization::MakeInstance, true));
		DetailBuilder.GetDetailsViewSharedPtr()->RegisterInstancedCustomPropertyTypeLayout(TEXT("ELexUITextParagraphVerticalAlign"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLexTextAlignmentCustomization::MakeInstance, false));
		LGUICategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULexText, HAlign));
		LGUICategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULexText, VAlign));
	}
	//font style
	DetailBuilder.GetDetailsViewSharedPtr()->RegisterInstancedCustomPropertyTypeLayout(TEXT("ELexUITextFontStyle"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FLexTextFontStyleCustomization::MakeInstance));

	auto OverflowTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexText, OverflowType));
	OverflowTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLexTextCustomization::ForceRefresh, &DetailBuilder));
	LGUICategory.AddProperty(OverflowTypeHandle);
	
	TArray<FName> NeedToHidePropertyNames;
	auto RichText_PH = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexText, bRichText));
	RichText_PH->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLexTextCustomization::ForceRefresh, &DetailBuilder));
	bool bRichText = false;
	RichText_PH->GetValue(bRichText);
	if (bRichText)
	{
		IDetailGroup& RichTextGroup = LGUICategory.AddGroup(FName("RichText"), RichText_PH->GetPropertyDisplayName());
		RichTextGroup.HeaderProperty(RichText_PH);
		RichTextGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexText, RichTextTagFilterFlags)));
		RichTextGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexText, RichTextCustomStyleData)));
		RichTextGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexText, RichTextImageData)));
		RichTextGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexText, CreatedRichTextImageObjectArray)));
	}
	else
	{
		LGUICategory.AddProperty(RichText_PH);
		NeedToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULexText, RichTextCustomStyleData));
		NeedToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULexText, RichTextImageData));
		NeedToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULexText, CreatedRichTextImageObjectArray));
		NeedToHidePropertyNames.Add(GET_MEMBER_NAME_CHECKED(ULexText, RichTextTagFilterFlags));
	}

	for (auto item : NeedToHidePropertyNames)
	{
		DetailBuilder.HideProperty(item);
	}

	auto OverrideMaterial_PH = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexText, OverrideMaterial));
	OverrideMaterial_PH->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=, &DetailBuilder] {
		DetailBuilder.ForceRefreshDetails();
		}));
	LGUICategory.AddProperty(OverrideMaterial_PH);
	{
		ULexUIFontData_BaseObject* Font = nullptr;
		Font_PH->GetValue(*(UObject**)&Font);
		if (Font)
		{
			for (auto Item : Font->GetPresetMaterials())
			{
				if (IsValid(Item))
				{
					PresetMaterials.Add(Item);
				}
			}
		}
	}
	LGUICategory.AddCustomRow(LOCTEXT("PresetOverrideMaterialsRow", "PresetOverrideMaterials"))
	.Visibility(PresetMaterials.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed)
	.ValueContent()
	[
		SNew(SBox)
		.VAlign(VAlign_Center)
		[
			SNew(SComboButton)
			.HasDownArrow(true)
			.ButtonContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("PresetMaterials_PropertyName", "PresetMaterials"))
				.ToolTipText(LOCTEXT("PresetMaterials_ToolTip", "Here list PresetMaterials from LexUIFont, you can easily set these materials to OverrideMaterial."))
			]
			.MenuContent()
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SListView<TWeakObjectPtr<UMaterialInterface>>)
					.ListItemsSource(&PresetMaterials)
					.OnGenerateRow_Lambda([=](TWeakObjectPtr<UMaterialInterface> Item, const TSharedRef<STableViewBase>& OwnerTable)
					{
						return SNew(STableRow<TWeakObjectPtr<UMaterialInterface>>, OwnerTable)
							[
								SNew(SBox)
								.VAlign(VAlign_Center)
								.Padding(6, 4)
								[
									SNew(STextBlock)
									.Font(IDetailLayoutBuilder::GetDetailFont())
									.Text(FText::FromString(Item->GetName()))
									.ToolTipText(FText::FromString(Item->GetPathName()))
								]
							];
					})
					.OnSelectionChanged_Lambda([=](TWeakObjectPtr<UMaterialInterface> Item, ESelectInfo::Type SelectInfo)
					{
						if (auto MatItem = Item.Get())
						{
							OverrideMaterial_PH->SetValue(*(UObject**)&MatItem);
						}
					})
				]
			]
		]
	]
	;
	LGUICategory.AddCustomRow(LOCTEXT("MaterialDomainErrorTipRow", "MaterialDomainErrorTip"))
	.Visibility(TAttribute<EVisibility>::CreateSPLambda(this, [=]()
	{
		UMaterialInterface* OverrideMaterial = nullptr;
		OverrideMaterial_PH->GetValue((UObject*&)OverrideMaterial);
		if (!OverrideMaterial)
		{
			return EVisibility::Collapsed;
		}
		auto Mat = OverrideMaterial->GetMaterial();
		if (!Mat)
		{
			return EVisibility::Collapsed;
		}
		if (Mat->MaterialDomain == EMaterialDomain::MD_Surface)
		{
			return EVisibility::Collapsed;
		}
		return EVisibility::Visible;
	}))
	.WholeRowContent()
	.MinDesiredWidth(500)
	.VAlign(VAlign_Center)
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(LOCTEXT("MaterialDomainErrorTip", "OverrideMaterial should use Surface domain!"))
		.ColorAndOpacity(FLinearColor(FColor::Yellow))
		.AutoWrapText(true)
	]
	;
	LGUICategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULexText, ExpandMeshSize));
}
void FLexTextCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (auto Script = TargetScriptPtr.Get())
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE