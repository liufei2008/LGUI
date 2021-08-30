// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIPrefabHelperComponentCustomization.h"
#include "LGUIEditorPCH.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabHelperComponentCustomization"

TSharedRef<IDetailCustomization> FLGUIPrefabHelperComponentCustomization::MakeInstance()
{
	return MakeShareable(new FLGUIPrefabHelperComponentCustomization);
}
void FLGUIPrefabHelperComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULGUIPrefabHelperComponent>(targetObjects[0].Get());
	if (TargetScriptPtr.IsValid())
	{
		
	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
		return;
	}
	if (IsValid(TargetScriptPtr->ParentActorForEditor))
	{
		LGUIEditorTools::MakeCurrentLevel(TargetScriptPtr->ParentActorForEditor);
	}
	TargetScriptPtr->LoadPrefab();
}
void FLGUIPrefabHelperComponentCustomization::ForceRefresh(IDetailLayoutBuilder* DetailBuilder)
{
	if (auto Script = TargetScriptPtr.Get())
	{
		DetailBuilder->ForceRefreshDetails();
	}
}
#undef LOCTEXT_NAMESPACE