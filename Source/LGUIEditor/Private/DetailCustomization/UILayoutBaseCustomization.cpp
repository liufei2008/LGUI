// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "DetailCustomization/UILayoutBaseCustomization.h"
#include "LGUIEditorUtils.h"

#define LOCTEXT_NAMESPACE "UILayoutBaseCustomization"

TSharedRef<IDetailCustomization> FUILayoutBaseCustomization::MakeInstance()
{
	return MakeShareable(new FUILayoutBaseCustomization);
}
void FUILayoutBaseCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUILayoutBase>(targetObjects[0].Get());
	if (TargetScriptPtr != nullptr)
	{
		if (TargetScriptPtr->GetWorld()->WorldType == EWorldType::Editor)
		{
			TargetScriptPtr->OnRebuildLayout();
		}
	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}
	LGUIEditorUtils::ShowError_MultiComponentNotAllowed(&DetailBuilder, TargetScriptPtr.Get(), TEXT("Multiple UILayout component in one actor is not allowed!"));
}
#undef LOCTEXT_NAMESPACE