// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIPrefabCustomization.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Utils/LGUIUtils.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "LGUIEditorTools.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabCustomization"

TSharedRef<IDetailCustomization> FLGUIPrefabCustomization::MakeInstance()
{
	return MakeShareable(new FLGUIPrefabCustomization);
}

void FLGUIPrefabCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULGUIPrefab>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
		return;
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LGUI");

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
	DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIPrefab, EngineMajorVersion))->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIPrefab, EngineMinorVersion))->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIPrefab, PrefabVersion))->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
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
					.Text(this, &FLGUIPrefabCustomization::GetEngineVersionText)
					.ToolTipText(LOCTEXT("EngineVersionTooltip", "Engine's version when creating this prefab."))
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ColorAndOpacity(this, &FLGUIPrefabCustomization::GetEngineVersionTextColorAndOpacity)
					.AutoWrapText(true)
				]
			]
			+SHorizontalBox::Slot()
			.MaxWidth(80)
			[
				SNew(SButton)
				.Text(LOCTEXT("FixEngineVersion", "Fix it"))
				.OnClicked(this, &FLGUIPrefabCustomization::OnClickRecreteButton)
				.Visibility(this, &FLGUIPrefabCustomization::ShouldShowFixEngineVersionButton)
			]
			+SHorizontalBox::Slot()
			.MaxWidth(80)
			[
				SNew(SButton)
				.Text(LOCTEXT("FixAllEngineVersion", "Fix all"))
				.OnClicked(this, &FLGUIPrefabCustomization::OnClickRecreteAllButton)
				.Visibility(this, &FLGUIPrefabCustomization::ShouldShowFixEngineVersionButton)
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
					.Text(this, &FLGUIPrefabCustomization::GetPrefabVersionText)
					.ToolTipText(LOCTEXT("PrefabVersionTooltip", "LGUIPrefab system's version when creating this prefab."))
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ColorAndOpacity(this, &FLGUIPrefabCustomization::GetPrefabVersionTextColorAndOpacity)
					.AutoWrapText(true)
				]
			]
			+SHorizontalBox::Slot()
			.MaxWidth(80)
			[
				SNew(SButton)
				.Text(LOCTEXT("FixPrefabVersion", "Fix it"))
				.OnClicked(this, &FLGUIPrefabCustomization::OnClickRecreteButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.Visibility(this, &FLGUIPrefabCustomization::ShouldShowFixPrefabVersionButton)
			]
			+SHorizontalBox::Slot()
			.MaxWidth(80)
			[
				SNew(SButton)
				.Text(LOCTEXT("FixAllPrefabVersion", "Fix all"))
				.OnClicked(this, &FLGUIPrefabCustomization::OnClickRecreteAllButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.Visibility(this, &FLGUIPrefabCustomization::ShouldShowFixPrefabVersionButton)
			]
		]
		;

	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULGUIPrefab, PrefabHelperObject));
	auto PrefabHelperObjectProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIPrefab, PrefabHelperObject));
	category.AddCustomRow(LOCTEXT("PrefabHelperObject", "PrefabHelperObject"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("AgentObjectsWidgetName", "AgentObjects"))
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
					.Text(this, &FLGUIPrefabCustomization::AgentObjectText)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("FixPrefabHelperObject", "Fix"))
				.ToolTipText(LOCTEXT("FixAgentRootActor_Tooltip", "Missing agent objects! This may cause cook & package fail. Click to fix it. Because we can't fix it in cook thread, so you need to do it manually."))
				.OnClicked(this, &FLGUIPrefabCustomization::OnClickRecreateAgentObjects)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.Visibility(this, &FLGUIPrefabCustomization::ShouldShowFixAgentObjectsButton)
			]
		]
	;
	category.AddCustomRow(LOCTEXT("AdditionalButton", "Additional Button"), true)
		.WholeRowContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("RecreateThis", "Recreate this prefab"))
				.OnClicked(this, &FLGUIPrefabCustomization::OnClickRecreteButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("RecreateAll", "Recreate all prefabs"))
				.OnClicked(this, &FLGUIPrefabCustomization::OnClickRecreteAllButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
			]
		]
		;
}
FText FLGUIPrefabCustomization::GetEngineVersionText()const
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
FText FLGUIPrefabCustomization::GetPrefabVersionText()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->PrefabVersion == LGUI_CURRENT_PREFAB_VERSION)
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
EVisibility FLGUIPrefabCustomization::ShouldShowFixEngineVersionButton()const
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
FSlateColor FLGUIPrefabCustomization::GetEngineVersionTextColorAndOpacity()const
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
FSlateColor FLGUIPrefabCustomization::GetPrefabVersionTextColorAndOpacity()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->PrefabVersion == LGUI_CURRENT_PREFAB_VERSION)
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
EVisibility FLGUIPrefabCustomization::ShouldShowFixPrefabVersionButton()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->PrefabVersion == LGUI_CURRENT_PREFAB_VERSION)
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
EVisibility FLGUIPrefabCustomization::ShouldShowFixAgentObjectsButton()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->PrefabVersion >= (uint16)ELGUIPrefabVersion::BuildinFArchive
			&& (!IsValid(TargetScriptPtr->PrefabHelperObject) || !IsValid(TargetScriptPtr->PrefabHelperObject->LoadedRootActor))
			)
		{
			return EVisibility::Visible;
		}
		return EVisibility::Hidden;
	}
	else
	{
		return EVisibility::Hidden;
	}
}

FText FLGUIPrefabCustomization::AgentObjectText()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->PrefabVersion >= (uint16)ELGUIPrefabVersion::BuildinFArchive
			&& (!IsValid(TargetScriptPtr->PrefabHelperObject) || !IsValid(TargetScriptPtr->PrefabHelperObject->LoadedRootActor))
			)
		{
			return LOCTEXT("AgentObjectNotValid", "NotValid");
		}
	}
	return LOCTEXT("AgentObjectValid", "Valid");
}

FReply FLGUIPrefabCustomization::OnClickRecreteButton()
{
	if (auto Prefab = TargetScriptPtr.Get())
	{
		Prefab->RecreatePrefab();
	}
	return FReply::Handled();
}
FReply FLGUIPrefabCustomization::OnClickRecreteAllButton()
{
	auto World = ULGUIEditorManagerObject::GetPreviewWorldForPrefabPackage();
	if (!IsValid(World))
	{
		UE_LOG(LGUIEditor, Error, TEXT("[FLGUIPrefabCustomization::OnClickRecreteButton]Can not get World! This is wired..."));
	}
	else
	{
		auto AllPrefabs = LGUIEditorTools::GetAllPrefabArray();
		for (auto Prefab : AllPrefabs)
		{
			if (
				Prefab->EngineMajorVersion != ENGINE_MAJOR_VERSION || Prefab->EngineMinorVersion != ENGINE_MINOR_VERSION
				|| Prefab->PrefabVersion != LGUI_CURRENT_PREFAB_VERSION
				)
			{
				Prefab->RecreatePrefab();
			}
		}
	}
	return FReply::Handled();
}
FReply FLGUIPrefabCustomization::OnClickEditPrefabButton()
{
	return FReply::Handled();
}
FReply FLGUIPrefabCustomization::OnClickRecreateAgentObjects()
{
	auto AllPrefabs = LGUIEditorTools::GetAllPrefabArray();
	for (auto Prefab : AllPrefabs)
	{
		Prefab->MakeAgentObjectsInPreviewWorld();
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE