// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "IDetailCustomization.h"
#include "LGUIEditorModule.h"
#include "IDetailPropertyRow.h"

#define LOCTEXT_NAMESPACE "LGUIEditorUtils"
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
	static bool IsEnabledOnProperty(TSharedRef<IPropertyHandle> PropertyHandle)
	{
		return PropertyHandle->IsEditable();
	}
	static void DrawThumbnailIcon(const FString& IconPath, int32 X, int32 Y, uint32 Width, uint32 Height, FCanvas* Canvas);
private:
	static TMap<FString, UTexture2D*> IconPathToTextureMap;
};
#undef LOCTEXT_NAMESPACE
