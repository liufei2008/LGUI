// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DetailCustomization/UIEffectTextAnimationCustomization.h"
#include "LGUIEditorPCH.h"

#define LOCTEXT_NAMESPACE "UIEffectTextAnimationCustomization"

TSharedRef<IDetailCustomization> FUIEffectTextAnimationCustomization::MakeInstance()
{
	return MakeShareable(new FUIEffectTextAnimationCustomization);
}
void FUIEffectTextAnimationCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUIEffectTextAnimation>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
		return;
	}
	LGUIEditorUtils::ShowError_RequireComponent(&DetailBuilder, TargetScriptPtr.Get(), UUIText::StaticClass());
}
#undef LOCTEXT_NAMESPACE