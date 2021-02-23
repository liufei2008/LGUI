// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIEditorPCH.h"

#define LOCTEXT_NAMESPACE "UISpriteSheetTexturePlayerCustomization"

TSharedRef<IDetailCustomization> FUISpriteSheetTexturePlayerCustomization::MakeInstance()
{
	return MakeShareable(new FUISpriteSheetTexturePlayerCustomization);
}
void FUISpriteSheetTexturePlayerCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUISpriteSheetTexturePlayer>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[UISpriteSheetTexturePlayerCustomization]Get TargetScript is null"));
		return;
	}
	LGUIEditorUtils::ShowError_RequireComponent(&DetailBuilder, TargetScriptPtr.Get(), UUITexture::StaticClass());
}
#undef LOCTEXT_NAMESPACE