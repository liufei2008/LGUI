// Copyright 2019 LexLiu. All Rights Reserved.

#include "DetailCustomization/EditorToolsCustomization.h"
#include "Window/LGUIEditorTools.h"
#include "LGUIEditorModule.h"
#include "LGUIEditorUtils.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/Actor/UIContainerActor.h"
#include "Core/Actor/UIPanelActor.h"
#include "Core/Actor/UISpriteActor.h"
#include "Core/Actor/UITextActor.h"
#include "Core/Actor/UITextureActor.h"

#define LOCTEXT_NAMESPACE "EditorToolsCustomization"

TSharedRef<IDetailCustomization> FEditorToolsCustomization::MakeInstance()
{
	return MakeShareable(new FEditorToolsCustomization);
}
void FEditorToolsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	auto TargetScriptPtr = Cast<ULGUIEditorToolsAgentObject>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{

	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}
	
	IDetailCategoryBuilder& createBaseCategory = DetailBuilder.EditCategory("UI Create Base Element");
	createBaseCategory.AddProperty("UIPanel");
	createBaseCategory.AddProperty("UIContainer");
	createBaseCategory.AddProperty("UISprite");
	createBaseCategory.AddProperty("UIText");
	createBaseCategory.AddProperty("UITexture");
	
	auto createExtensionVerticalBoxWidget = SNew(SVerticalBox);
	auto createBPExtensionVerticalBoxWidget = SNew(SVerticalBox);
	for (TObjectIterator<UClass> ClassItr; ClassItr; ++ClassItr)
	{
		if (ClassItr->IsChildOf(AUIBaseActor::StaticClass()))
		{
			if (*ClassItr != AUIContainerActor::StaticClass()
				&& *ClassItr != AUIPanelActor::StaticClass()
				&& *ClassItr != AUISpriteActor::StaticClass()
				&& *ClassItr != AUITextActor::StaticClass()
				&& *ClassItr != AUITextureActor::StaticClass()
				&& *ClassItr != AUIBaseActor::StaticClass())
			{
				bool isBlueprint = ClassItr->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
				if (isBlueprint)
				{
					if (ClassItr->GetName().StartsWith(TEXT("SKEL_")))
					{
						continue;
					}
				}
				TSharedRef<SVerticalBox> targetWidget = isBlueprint ? createBPExtensionVerticalBoxWidget : createExtensionVerticalBoxWidget;
				targetWidget->AddSlot()
				[
					SNew(SBox)
					.Padding(FMargin(0, 2))
					[
					SNew(SButton)
					.Text(FText::FromString(ClassItr->GetName()))
					.ToolTipText(FText::FromString(ClassItr->GetPathName()))
					.HAlign(EHorizontalAlignment::HAlign_Center)
					.OnClicked_Lambda([=]{
					ULGUIEditorToolsAgentObject::CreateUIItemActor(*ClassItr);
					return FReply::Handled();
					})
					]
				];
			}
		}
	}
	IDetailCategoryBuilder& createExtensionCategory = DetailBuilder.EditCategory("UI Create Extension Element");
	createExtensionCategory.AddCustomRow(LOCTEXT("CreateUIExtentionButtons","CreateUIExtentionButtons"))
		.WholeRowContent()
		[
			createExtensionVerticalBoxWidget
		]
	;

	//IDetailCategoryBuilder& createBPExtensionCategory = DetailBuilder.EditCategory("UI Create Blueprint Extension Element");
	//createBPExtensionCategory.AddCustomRow(LOCTEXT("CreateUIBPExtentionButtons", "CreateUIBlueprintExtentionButtons"))
	//	.WholeRowContent()
	//	[
	//		createBPExtensionVerticalBoxWidget
	//	]
	//;



	auto changeTraceChannelVerticalBoxWidget = SNew(SVerticalBox);
	auto CollisionProfile = UCollisionProfile::Get();
	for (int i = 0, count = (int)ETraceTypeQuery::TraceTypeQuery_MAX; i < count; i++)
	{
		auto collisionChannel = UEngineTypes::ConvertToCollisionChannel((ETraceTypeQuery)i);
		auto channelName = CollisionProfile->ReturnChannelNameFromContainerIndex(collisionChannel).ToString();
		if (channelName != TEXT("MAX"))
		{
			changeTraceChannelVerticalBoxWidget->AddSlot()
				[
					SNew(SBox)
					.Padding(FMargin(0, 2))
					[
					SNew(SButton)
					.Text(FText::FromString(channelName))
					.ToolTipText(FText::FromString(FString::Printf(TEXT("Change selected UI item actor's trace channel to %s"), *channelName)))
					.HAlign(EHorizontalAlignment::HAlign_Center)
					.OnClicked_Lambda([=]{
					ULGUIEditorToolsAgentObject::ChangeTraceChannel_Impl((ETraceTypeQuery)i);
					return FReply::Handled();
					})
					]
				];
		}
	}
	IDetailCategoryBuilder& changeTraceChannelCategory = DetailBuilder.EditCategory("UI Change Trace Channel");
	changeTraceChannelCategory.AddCustomRow(LOCTEXT("ChangeTraceChannel","Change Trace Channel Buttons"))
		.WholeRowContent()
		[
			changeTraceChannelVerticalBoxWidget
		]
	;
}
#undef LOCTEXT_NAMESPACE