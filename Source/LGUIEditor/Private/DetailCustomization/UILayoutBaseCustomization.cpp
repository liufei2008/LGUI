// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DetailCustomization/UILayoutBaseCustomization.h"
#include "LGUIEditorPCH.h"

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

	bool anchorControlledByParentLayout = false;
	bool widthControlledByParentLayout = false;
	bool heightControlledByParentLayout = false;
	bool anchorHControlledBySelfLayout = false;
	bool anchorVControlledBySelfLayout = false;
	bool widthControlledBySelfLayout = false;
	bool heightControlledBySelfLayout = false;
	bool stretchLeftControlledBySelfLayout = false;
	bool stretchRightControlledBySelfLayout = false;
	bool stretchTopControlledBySelfLayout = false;
	bool stretchBottomControlledBySelfLayout = false;
	if (auto thisActor = TargetScriptPtr->GetOwner())
	{
		if (auto parentActor = thisActor->GetAttachParentActor())
		{
			bool ignoreParentLayout = false;
			if (auto thisLayoutElement = thisActor->FindComponentByClass<UUILayoutElement>())
			{
				ignoreParentLayout = thisLayoutElement->GetIgnoreLayout();
			}
			if (!ignoreParentLayout)
			{
				if (auto parentLayout = parentActor->FindComponentByClass<UUILayoutBase>())
				{
					anchorControlledByParentLayout = parentLayout->CanControlChildAnchor();
					widthControlledByParentLayout = parentLayout->CanControlChildWidth();
					heightControlledByParentLayout = parentLayout->CanControlChildHeight();
				}
			}
		}
		if (auto thisLayout = thisActor->FindComponentByClass<UUILayoutBase>())
		{
			anchorHControlledBySelfLayout = thisLayout->CanControlSelfHorizontalAnchor();
			anchorVControlledBySelfLayout = thisLayout->CanControlSelfVerticalAnchor();
			widthControlledBySelfLayout = thisLayout->CanControlSelfWidth();
			heightControlledBySelfLayout = thisLayout->CanControlSelfHeight();
			stretchLeftControlledBySelfLayout = thisLayout->CanControlSelfStrengthLeft();
			stretchRightControlledBySelfLayout = thisLayout->CanControlSelfStrengthRight();
			stretchTopControlledBySelfLayout = thisLayout->CanControlSelfStrengthTop();
			stretchBottomControlledBySelfLayout = thisLayout->CanControlSelfStrengthBottom();
		}
	}
	if (anchorControlledByParentLayout && (anchorHControlledBySelfLayout || anchorVControlledBySelfLayout))
	{
		LGUIEditorUtils::ShowWarning(&DetailBuilder, FString("Anchor is controlled by more than one UILayout component! This may cause issue!"));
	}
	if (widthControlledByParentLayout && widthControlledBySelfLayout)
	{
		LGUIEditorUtils::ShowWarning(&DetailBuilder, FString("Width is controlled by more than one UILayout component! This may cause issue!"));
	}
	if (heightControlledByParentLayout && heightControlledBySelfLayout)
	{
		LGUIEditorUtils::ShowWarning(&DetailBuilder, FString("Height is controlled by more than one UILayout component! This may cause issue!"));
	}
}
#undef LOCTEXT_NAMESPACE