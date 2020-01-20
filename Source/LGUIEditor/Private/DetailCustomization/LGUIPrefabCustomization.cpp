// Copyright 2019 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIPrefabCustomization.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabSystem/ActorSerializer.h"
#include "Utils/LGUIUtils.h"

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

	//show prefab version
	DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIPrefab, EngineMajorVersion))->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIPrefab, EngineMinorVersion))->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	category.AddCustomRow(LOCTEXT("PrefabVersion", "PrefabVersion"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PrefabEngineVersion", "PrefabEngineVersion"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%d.%d"), TargetScriptPtr->EngineMajorVersion, TargetScriptPtr->EngineMinorVersion)))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		;

	auto useBuildDataHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIPrefab, UseBuildData));
	useBuildDataHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	DetailBuilder.HideProperty(useBuildDataHandle);
	category.AddCustomRow(LOCTEXT("UseBuildData", "UseBuildData"))
		.NameContent()
		[
			useBuildDataHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(500)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(110)
				[
				useBuildDataHandle->CreatePropertyValueWidget()
				]
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(10, 0, 0, 0))
			[
				SNew(SButton)
				.Text(FText::FromString("MakeAllTo TRUE"))
				.OnClicked(this, &FLGUIPrefabCustomization::OnClickUseBuildDataButton, true)
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString("MakeAllTo FALSE"))
				.OnClicked(this, &FLGUIPrefabCustomization::OnClickUseBuildDataButton, false)
			]
		]
		;

	auto dataCountForBuildHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULGUIPrefab, DataCountForBuild));
	DetailBuilder.HideProperty(dataCountForBuildHandle);
	category.AddCustomRow(LOCTEXT("",""))
		.NameContent()
		[
			dataCountForBuildHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(500)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(110)
				[
				dataCountForBuildHandle->CreatePropertyValueWidget()
				]
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(10, 0, 0, 0))
			[
				SNew(SButton)
				.Text(FText::FromString("RecreateThis"))
				.OnClicked(this, &FLGUIPrefabCustomization::OnClickRecreteButton)
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString("RecreateAllPrefab"))
				.OnClicked(this, &FLGUIPrefabCustomization::OnClickRecreteAllButton)
			]
		]
			;
}
FReply FLGUIPrefabCustomization::OnClickUseBuildDataButton(bool AllUseBuildData)
{
	for (TObjectIterator<ULGUIPrefab> PrefabItr; PrefabItr; ++PrefabItr)
	{
		if (PrefabItr->UseBuildData != AllUseBuildData)
		{
			PrefabItr->UseBuildData = AllUseBuildData;
			PrefabItr->MarkPackageDirty();
		}
	}
	return FReply::Handled();
}
FReply FLGUIPrefabCustomization::OnClickRecreteButton()
{
	if (auto script = TargetScriptPtr.Get())
	{
		ActorSerializer serializer(GWorld);
		auto loadedActor = ActorSerializer::LoadPrefabForEdit(GWorld, script, nullptr);
		serializer.SerializeActor(loadedActor, script);
		LGUIUtils::DeleteActor(loadedActor, true);
	}
	return FReply::Handled();
}
FReply FLGUIPrefabCustomization::OnClickRecreteAllButton()
{
	if (auto script = TargetScriptPtr.Get())
	{
		for (TObjectIterator<ULGUIPrefab> PrefabItr; PrefabItr; ++PrefabItr)
		{
			ActorSerializer serializer(GWorld);
			auto loadedActor = ActorSerializer::LoadPrefabForEdit(GWorld, *PrefabItr, nullptr);
			serializer.SerializeActor(loadedActor, *PrefabItr);
			LGUIUtils::DeleteActor(loadedActor, true);
		}
	}
	return FReply::Handled();
}
#undef LOCTEXT_NAMESPACE