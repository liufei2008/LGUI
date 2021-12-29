// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
	LGUIEditorUtils::ShowError_MultiComponentNotAllowed(&DetailBuilder, TargetScriptPtr.Get(), TEXT("Multiple layout components in one actor is not allowed!"));

	bool anchorControlledByParentLayout = false;
	bool widthControlledByParentLayout = false;
	bool heightControlledByParentLayout = false;
	bool anchorControlledBySelfLayout = false;
	bool widthControlledBySelfLayout = false;
	bool heightControlledBySelfLayout = false;
	if (auto thisActor = TargetScriptPtr->GetOwner())
	{
		if (auto parentActor = thisActor->GetAttachParentActor())
		{
			bool ignoreParentLayout = false;
			auto ThisLayoutElements = thisActor->GetComponentsByInterface(ULGUILayoutElementInterface::StaticClass());
			if (ThisLayoutElements.Num() > 0)
			{
				ignoreParentLayout = ILGUILayoutElementInterface::Execute_GetIgnoreLayout(ThisLayoutElements[0]);
			}
			if (!ignoreParentLayout)
			{
				auto ParentLayouts = parentActor->GetComponentsByInterface(ULGUILayoutInterface::StaticClass());
				if (ParentLayouts.Num() > 0)
				{
					anchorControlledByParentLayout = ILGUILayoutInterface::Execute_CanControlChildAnchor(ParentLayouts[0]);
					widthControlledByParentLayout = ILGUILayoutInterface::Execute_CanControlChildWidth(ParentLayouts[0]);
					heightControlledByParentLayout = ILGUILayoutInterface::Execute_CanControlChildHeight(ParentLayouts[0]);
				}
			}
		}
		auto ThisLayouts = thisActor->GetComponentsByInterface(ULGUILayoutInterface::StaticClass());
		if (ThisLayouts.Num() > 0)
		{
			anchorControlledBySelfLayout = ILGUILayoutInterface::Execute_CanControlSelfAnchor(ThisLayouts[0]);
			widthControlledBySelfLayout = ILGUILayoutInterface::Execute_CanControlSelfWidth(ThisLayouts[0]);
			heightControlledBySelfLayout = ILGUILayoutInterface::Execute_CanControlSelfHeight(ThisLayouts[0]);
		}
	}
	if (anchorControlledByParentLayout && anchorControlledBySelfLayout)
	{
		LGUIEditorUtils::ShowWarning(&DetailBuilder, FString("Anchor is controlled by multiple layout components! This may cause issue!"));
	}
	if (widthControlledByParentLayout && widthControlledBySelfLayout)
	{
		LGUIEditorUtils::ShowWarning(&DetailBuilder, FString("Width is controlled by multiple layout components! This may cause issue!"));
	}
	if (heightControlledByParentLayout && heightControlledBySelfLayout)
	{
		LGUIEditorUtils::ShowWarning(&DetailBuilder, FString("Height is controlled by multiple layout components! This may cause issue!"));
	}
}
#undef LOCTEXT_NAMESPACE