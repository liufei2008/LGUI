// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once
#include "IDetailCustomization.h"
#include "LGUIEditorModule.h"
#include "IDetailPropertyRow.h"

DECLARE_DELEGATE_RetVal(FText, FLGUIArrowButtonDelegate);
class LGUIEditorUtils
{
private:
	static FName ErrorInfoCategory;
public:
	template<class T>
	static void ShowError_MultiComponentNotAllowed(IDetailLayoutBuilder* DetailBuilder, T* Component, const FString& ErrorMessage = "")
	{
		static_assert(TPointerIsConvertibleFromTo<T, const UActorComponent>::Value, "'T' template parameter to ShowWarning_MultiComponentNotAllowed must be derived from UActorComponent");
		if (auto actor = Component->GetOwner())
		{
			TArray<UActorComponent*> components;
			actor->GetComponents(T::StaticClass(), components);
			if (components.Num() > 1)
			{
				IDetailCategoryBuilder& lguiCategory = DetailBuilder->EditCategory(ErrorInfoCategory, FText::GetEmpty(), ECategoryPriority::Variable);
				lguiCategory.AddCustomRow(FText::FromString(FString(TEXT("MultiComponentNotAllowed_Tips"))))
					.WholeRowContent()
					[
						SNew(STextBlock)
						.Text(FText::FromString(ErrorMessage.Len() == 0 ? FString::Printf(TEXT("Multiple %s component in one actor is not allowed!"), *(T::StaticClass()->GetName())) : ErrorMessage))
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
				lguiCategory.AddCustomRow(FText::FromString(FString(TEXT("RequireComponent_Tips"))))
					.WholeRowContent()
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("This component require %s component on the same actor!"), *(RequireComponentType->GetName()))))
					.ColorAndOpacity(FLinearColor(FColor::Red))
					.AutoWrapText(true)
					]
				;
			}
		}
	}
	static void ShowError(IDetailLayoutBuilder* DetailBuilder, const FString& ErrorMessage)
	{
		IDetailCategoryBuilder& lguiCategory = DetailBuilder->EditCategory(ErrorInfoCategory, FText::GetEmpty(), ECategoryPriority::Variable);
		lguiCategory.AddCustomRow(FText::FromString(FString(TEXT("ErrorInfoText"))))
		.WholeRowContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(ErrorMessage))
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
	static void SetControlledByParentLayout(IDetailPropertyRow& prop, bool controlledByParentLayout)
	{
		prop.IsEnabled(!controlledByParentLayout);
		auto disabledByParentLayoutToolTip = FString(TEXT("This property is controlled by parent layout"));
		if (controlledByParentLayout) prop.ToolTip(FText::FromString(disabledByParentLayoutToolTip));
	}
	static void SetControlledBySelfLayout(IDetailPropertyRow& prop, bool controlledByThisLayout)
	{
		prop.IsEnabled(!controlledByThisLayout);
		auto disabledByParentLayoutToolTip = FString(TEXT("This property is controlled by self layout"));
		if (controlledByThisLayout) prop.ToolTip(FText::FromString(disabledByParentLayoutToolTip));
	}
	static TSharedRef<SWidget> GenerateArrowButtonContent(FText textContent)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(textContent)
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
};

FName LGUIEditorUtils::ErrorInfoCategory = TEXT("Error");