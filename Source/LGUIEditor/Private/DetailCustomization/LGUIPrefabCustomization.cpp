// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIPrefabCustomization.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabSystem/ActorSerializer.h"
#include "LGUIEditorPCH.h"

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
				.Text(FText::FromString(TEXT("Fix it")))
				.OnClicked(this, &FLGUIPrefabCustomization::OnClickRecreteButton)
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
				.Text(FText::FromString(TEXT("Fix it")))
				.OnClicked(this, &FLGUIPrefabCustomization::OnClickRecreteButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.Visibility(this, &FLGUIPrefabCustomization::ShouldShowFixPrefabVersionButton)
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
			return FText::FromString(FString::Printf(TEXT("%d.%d (This prefab is made by a different engine version.)"), TargetScriptPtr->EngineMajorVersion, TargetScriptPtr->EngineMinorVersion));
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
		if (TargetScriptPtr->PrefabVersion == LGUI_PREFAB_VERSION)
		{
			return FText::FromString(FString::Printf(TEXT("%d"), TargetScriptPtr->PrefabVersion));
		}
		else
		{
			return FText::FromString(FString::Printf(TEXT("%d (This prefab is made by a different prefab system version.)"), TargetScriptPtr->PrefabVersion));
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
		if (TargetScriptPtr->PrefabVersion == LGUI_PREFAB_VERSION)
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
		if (TargetScriptPtr->PrefabVersion == LGUI_PREFAB_VERSION)
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
FReply FLGUIPrefabCustomization::OnClickRecreteButton()
{
	if (auto script = TargetScriptPtr.Get())
	{
		auto world = script->GetWorld();
		if (!IsValid(world))
		{
			world = GWorld;
		}
		if (IsValid(world))
		{
			LGUIPrefabSystem::ActorSerializer serializer(world);
			auto loadedActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(world, script, nullptr);
			serializer.SerializeActor(loadedActor, script);
			LGUIUtils::DestroyActorWithHierarchy(loadedActor, true);
		}
		else
		{
			UE_LOG(LGUIEditor, Error, TEXT("[FLGUIPrefabCustomization::OnClickRecreteButton]Can not get World! This is wired..."));
		}
	}
	return FReply::Handled();
}
FReply FLGUIPrefabCustomization::OnClickRecreteAllButton()
{
	if (auto script = TargetScriptPtr.Get())
	{
		for (TObjectIterator<ULGUIPrefab> PrefabItr; PrefabItr; ++PrefabItr)
		{
			auto world = script->GetWorld();
			if (!IsValid(world))
			{
				world = GWorld;
			}
			if (IsValid(world))
			{
				LGUIPrefabSystem::ActorSerializer serializer(world);
				auto loadedActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(world, *PrefabItr, nullptr);
				serializer.SerializeActor(loadedActor, *PrefabItr);
				LGUIUtils::DestroyActorWithHierarchy(loadedActor, true);
			}
			else
			{
				UE_LOG(LGUIEditor, Error, TEXT("[FLGUIPrefabCustomization::OnClickRecreteButton]Can not get World! This is wired..."));
			}
		}
	}
	return FReply::Handled();
}
FReply FLGUIPrefabCustomization::OnClickEditPrefabButton()
{
	LGUIEditorTools::SpawnPrefabForEdit(TargetScriptPtr.Get());
	return FReply::Handled();
}
#undef LOCTEXT_NAMESPACE