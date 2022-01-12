// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/UILayoutBaseCustomization.h"
#include "LGUIEditorUtils.h"
#include "Layout/UILayoutBase.h"

#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

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
		if (auto world = TargetScriptPtr->GetWorld())
		{
			if (world->WorldType == EWorldType::Editor)
			{
				TargetScriptPtr->OnRebuildLayout();
			}
		}
	}
	else
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
	}
	LGUIEditorUtils::ShowError_MultiComponentNotAllowed(&DetailBuilder, TargetScriptPtr.Get(), LOCTEXT("MultipleLayoutComponentError", "Multiple layout components in one actor is not allowed!"));
}
#undef LOCTEXT_NAMESPACE