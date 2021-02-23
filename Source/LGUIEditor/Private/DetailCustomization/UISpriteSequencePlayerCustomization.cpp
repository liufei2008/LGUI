// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DetailCustomization/UISpriteSequencePlayerCustomization.h"
#include "LGUIEditorPCH.h"

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