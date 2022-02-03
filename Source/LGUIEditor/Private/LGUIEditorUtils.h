// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "IDetailCustomization.h"
#include "LGUIEditorModule.h"
#include "IDetailPropertyRow.h"

#define LOCTEXT_NAMESPACE "LGUIEditorUtils"
DECLARE_DELEGATE_RetVal(FText, FLGUIArrowButtonDelegate);
class LGUIEditorUtils
{
#define ErrorInfoCategory TEXT("Error")
public:
	template<class T>
	static void ShowError_MultiComponentNotAllowed(IDetailLayoutBuilder* DetailBuilder, T* Component)
	{
		auto ErrorMessage = FText::Format(LOCTEXT("MultipleComponentInOneActorNotAllowed", "Multiple {0} component in one actor is not allowed!"), FText::FromName(T::StaticClass()->GetFName()));
		ShowError_MultiComponentNotAllowed(DetailBuilder, Component, ErrorMessage);
	}
	template<class T>
	static void ShowError_MultiComponentNotAllowed(IDetailLayoutBuilder* DetailBuilder, T* Component, const FText& ErrorMessage)
	{
		static_assert(TPointerIsConvertibleFromTo<T, const UActorComponent>::Value, "'T' template parameter to ShowWarning_MultiComponentNotAllowed must be derived from UActorComponent");
		if (auto actor = Component->GetOwner())
		{
			TArray<UActorComponent*> components;
			actor->GetComponents(T::StaticClass(), components);
			if (components.Num() > 1)
			{
				IDetailCategoryBuilder& lguiCategory = DetailBuilder->EditCategory(ErrorInfoCategory, FText::GetEmpty(), ECategoryPriority::Variable);
				lguiCategory.AddCustomRow(LOCTEXT("MultiComponentNotAllowed_Tips", "MultiComponentNotAllowed_Tips"))
					.WholeRowContent()
					[
						SNew(STextBlock)
						.Text(ErrorMessage)
						.ColorAndOpacity(FLinearColor(FColor::Red))
						.AutoWrapText(true)
					]
				;
			}
		}
	}
	static void ShowError_RequireComponent(IDetailLayoutBuilder* DetailBuilder, UActorComponent* Component, TSubclassOf<UActorComponent> RequireComponentType)
	{
		if (auto actor = Component->GetOwner())
		{
			TArray<UActorComponent*> Components;
			actor->GetComponents(RequireComponentType, Components);
			if (Components.Num() == 0)
			{
				IDetailCategoryBuilder& lguiCategory = DetailBuilder->EditCategory(ErrorInfoCategory, FText::GetEmpty(), ECategoryPriority::Variable);
				lguiCategory.AddCustomRow(LOCTEXT("RequireComponentRow", "RequireComponent"))
					.WholeRowContent()
					[
						SNew(STextBlock)
						.Text(FText::Format(LOCTEXT("RequireComponentTip", "This component require {0} component on actor!"), FText::FromName(RequireComponentType->GetFName())))
						.ColorAndOpacity(FLinearColor(FColor::Red))
						.AutoWrapText(true)
					]
				;
			}
		}
	}
	static void ShowError(IDetailLayoutBuilder* LayoutBuilder, const FText& ErrorMessage)
	{
		IDetailCategoryBuilder& category = LayoutBuilder->EditCategory(ErrorInfoCategory, FText::GetEmpty(), ECategoryPriority::Variable);
		category.AddCustomRow(LOCTEXT("ErrorInfoTextRow", "ErrorInfoText"))
		.WholeRowContent()
		[
			SNew(STextBlock)
			.Text(ErrorMessage)
			.ColorAndOpacity(FLinearColor(FColor::Red))
			.AutoWrapText(true)
		]
		;
	}
	static void ShowWarning(IDetailLayoutBuilder* LayoutBuilder, const FText& ErrorMessage)
	{
		IDetailCategoryBuilder& category = LayoutBuilder->EditCategory(ErrorInfoCategory, FText::GetEmpty(), ECategoryPriority::Variable);
		category.AddCustomRow(LOCTEXT("WarningInfoTextRow", "WarningInfoText"))
			.WholeRowContent()
			[
				SNew(STextBlock)
				.Text(ErrorMessage)
				.ColorAndOpacity(FLinearColor(FColor::Yellow))
				.AutoWrapText(true)
			]
		;
	}
	static void ShowError(IDetailCategoryBuilder* CategoryBuilder, const FText& ErrorMessage)
	{
		CategoryBuilder->AddCustomRow(LOCTEXT("ErrorInfoTextRow", "ErrorInfoText"))
			.WholeRowContent()
			[
				SNew(STextBlock)
				.Text(ErrorMessage)
				.ColorAndOpacity(FLinearColor(FColor::Red))
				.AutoWrapText(true)
			]
		;
	}
	static IDetailPropertyRow& CreateSubDetail(IDetailCategoryBuilder* category, IDetailLayoutBuilder* DetailBuilder, TSharedRef<IPropertyHandle> handle)
	{
		DetailBuilder->HideProperty(handle);
		IDetailPropertyRow& prop = category->AddProperty(handle);
		auto name = handle->GetPropertyDisplayName().ToString();
		name = "    " + name;
		prop.DisplayName(FText::FromString(name));
		return prop;
	}
	static TSharedRef<SWidget> GenerateArrowButtonContent(FText textContent)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(textContent)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.AutoWidth()
			.Padding(2, 4)
			[
				SNew(SImage)
				.Image(FEditorStyle::GetBrush("ComboButton.Arrow"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			];
	}
	static TSharedRef<SWidget> GenerateArrowButtonContent(const FLGUIArrowButtonDelegate& getTextFunction)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text_Lambda([=] {
					if(getTextFunction.IsBound())
						return getTextFunction.Execute();
					return FText();
				})
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.AutoWidth()
			.Padding(2, 4)
			[
				SNew(SImage)
				.Image(FEditorStyle::GetBrush("ComboButton.Arrow"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			];
	}
	static bool IsEnabledOnProperty(TSharedRef<IPropertyHandle> PropertyHandle)
	{
		return PropertyHandle->IsEditable();
	}
};
#undef LOCTEXT_NAMESPACE
