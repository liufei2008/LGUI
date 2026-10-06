// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LexUIPrefabCustomization.h"
#include "PrefabSystem/LexUIPrefab.h"
#include "PrefabSystem/LexUIPrefabHelperObject.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabCustomization"

TSharedRef<IDetailCustomization> FLexUIPrefabCustomization::MakeInstance()
{
	return MakeShareable(new FLexUIPrefabCustomization);
}

void FLexUIPrefabCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULexUIPrefab>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[%s].%d Get TargetScript is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LexUI");

	//category.AddCustomRow(LOCTEXT("Edit prefab", "Edit prefab"))
	//	.NameContent()
	//	[
	//		SNew(SButton)
	//		.Text(LOCTEXT("Edit prefab", "Edit prefab"))
	//		.ToolTipText(LOCTEXT("EditPrefab_Tooltip", "Edit this prefab in level editor, use selected actor as parent."))
	//		.OnClicked(this, &FLGUIPrefabCustomization::OnClickEditPrefabButton)
	//	]
	//	;

	//show prefab version
	DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexUIPrefab, EngineMajorVersion))->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexUIPrefab, EngineMinorVersion))->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULexUIPrefab, PrefabVersion))->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	category.AddCustomRow(LOCTEXT("EngineVersion", "Engine Version"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("EngineVersion", "Engine Version"))
			.ToolTipText(LOCTEXT("EngineVersionTooltip", "Engine's version when creating this prefab."))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		.MinDesiredWidth(500)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.Padding(FMargin(4, 2))
				[
					SNew(STextBlock)
					.Text(this, &FLexUIPrefabCustomization::GetEngineVersionText)
					.ToolTipText(LOCTEXT("EngineVersionTooltip", "Engine's version when creating this prefab."))
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ColorAndOpacity(this, &FLexUIPrefabCustomization::GetEngineVersionTextColorAndOpacity)
					.AutoWrapText(true)
				]
			]
			+SHorizontalBox::Slot()
			.MaxWidth(80)
			[
				SNew(SButton)
				.Text(LOCTEXT("FixEngineVersion", "Fix it"))
				.OnClicked(this, &FLexUIPrefabCustomization::OnClickRecreteButton)
				.Visibility(this, &FLexUIPrefabCustomization::ShouldShowFixEngineVersionButton)
			]
		]
		;
	category.AddCustomRow(LOCTEXT("PrefabVersion", "Prefab Version"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PrefabVersion", "Prefab Version"))
			.ToolTipText(LOCTEXT("PrefabVersionTooltip", "LGUIPrefab system's version when creating this prefab."))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		.MinDesiredWidth(500)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.Padding(FMargin(4, 2))
				[
					SNew(STextBlock)
					.Text(this, &FLexUIPrefabCustomization::GetPrefabVersionText)
					.ToolTipText(LOCTEXT("PrefabVersionTooltip", "LGUIPrefab system's version when creating this prefab."))
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ColorAndOpacity(this, &FLexUIPrefabCustomization::GetPrefabVersionTextColorAndOpacity)
					.AutoWrapText(true)
				]
			]
			+SHorizontalBox::Slot()
			.MaxWidth(80)
			[
				SNew(SButton)
				.Text(LOCTEXT("FixPrefabVersion", "Fix it"))
				.OnClicked(this, &FLexUIPrefabCustomization::OnClickRecreteButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.Visibility(this, &FLexUIPrefabCustomization::ShouldShowFixPrefabVersionButton)
			]
		]
		;

	category.AddCustomRow(LOCTEXT("AdditionalButton", "Additional Button"), true)
		.WholeRowContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("RecreateThis", "Recreate this prefab"))
			.OnClicked(this, &FLexUIPrefabCustomization::OnClickRecreteButton)
			.HAlign(EHorizontalAlignment::HAlign_Center)
		]
		;
}
FText FLexUIPrefabCustomization::GetEngineVersionText()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->EngineMajorVersion == ENGINE_MAJOR_VERSION && TargetScriptPtr->EngineMinorVersion == ENGINE_MINOR_VERSION)
		{
			return FText::FromString(FString::Printf(TEXT("%d.%d"), TargetScriptPtr->EngineMajorVersion, TargetScriptPtr->EngineMinorVersion));
		}
		else
		{
			return FText::Format(LOCTEXT("PrefabEngineVersionError", "{0}.{1} (This prefab is made by a different engine version.)"), TargetScriptPtr->EngineMajorVersion, TargetScriptPtr->EngineMinorVersion);
		}
	}
	else
	{
		return LOCTEXT("Error", "Error");
	}
}
FText FLexUIPrefabCustomization::GetPrefabVersionText()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->PrefabVersion == LEXUI_CURRENT_PREFAB_VERSION)
		{
			return FText::FromString(FString::Printf(TEXT("%d"), TargetScriptPtr->PrefabVersion));
		}
		else
		{
			return FText::Format(LOCTEXT("PrefabSystemVersionError", "{0} (This prefab is made by a different prefab system version.)"), TargetScriptPtr->PrefabVersion);
		}
	}
	else
	{
		return LOCTEXT("Error", "Error");
	}
}
EVisibility FLexUIPrefabCustomization::ShouldShowFixEngineVersionButton()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->EngineMajorVersion == ENGINE_MAJOR_VERSION && TargetScriptPtr->EngineMinorVersion == ENGINE_MINOR_VERSION)
		{
			return EVisibility::Hidden;
		}
		else
		{
			return EVisibility::Visible;
		}
	}
	else
	{
		return EVisibility::Hidden;
	}
}
FSlateColor FLexUIPrefabCustomization::GetEngineVersionTextColorAndOpacity()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->EngineMajorVersion == ENGINE_MAJOR_VERSION && TargetScriptPtr->EngineMinorVersion == ENGINE_MINOR_VERSION)
		{
			return FSlateColor::UseForeground();
		}
		else
		{
			return FLinearColor::Yellow;
		}
	}
	else
	{
		return FSlateColor::UseForeground();
	}
}
FSlateColor FLexUIPrefabCustomization::GetPrefabVersionTextColorAndOpacity()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->PrefabVersion == LEXUI_CURRENT_PREFAB_VERSION)
		{
			return FSlateColor::UseForeground();
		}
		else
		{
			return FLinearColor::Yellow;
		}
	}
	else
	{
		return FSlateColor::UseForeground();
	}
}
EVisibility FLexUIPrefabCustomization::ShouldShowFixPrefabVersionButton()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->PrefabVersion == LEXUI_CURRENT_PREFAB_VERSION)
		{
			return EVisibility::Hidden;
		}
		else
		{
			return EVisibility::Visible;
		}
	}
	else
	{
		return EVisibility::Hidden;
	}
}

FReply FLexUIPrefabCustomization::OnClickRecreteButton()
{
	if (auto Prefab = TargetScriptPtr.Get())
	{
		Prefab->RecreatePrefab();
	}
	return FReply::Handled();
}
FReply FLexUIPrefabCustomization::OnClickEditPrefabButton()
{
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE