// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/UISpriteSequencePlayerCustomization.h"
#include "Extensions/UISpriteSequencePlayer.h"
#include "LGUIEditorUtils.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "UISpriteSequencePlayerCustomization"

TSharedRef<IDetailCustomization> FUISpriteSequencePlayerCustomization::MakeInstance()
{
	return MakeShareable(new FUISpriteSequencePlayerCustomization);
}
void FUISpriteSequencePlayerCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUISpriteSequencePlayer>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[UISpriteSequencePlayerCustomization]Get TargetScript is null"));
		return;
	}
	LGUIEditorUtils::ShowError_RequireComponent(&DetailBuilder, TargetScriptPtr.Get(), UUISpriteBase::StaticClass());
}
#undef LOCTEXT_NAMESPACE