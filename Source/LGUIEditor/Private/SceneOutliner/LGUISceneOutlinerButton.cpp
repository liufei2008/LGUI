// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "SceneOutliner/LGUISceneOutlinerButton.h"
#include "LGUIEditorModule.h"
#include "Widgets/Input/SComboButton.h"
#include "Core/Actor/LGUIManager.h"

#define LOCTEXT_NAMESPACE "LGUISceneOutlinerButton"

FReply SLGUISceneOutlinerButton::OnButtonClicked()
{
	if (_TreeItemActor.IsValid(false))
	{
		GEditor->SelectNone(true, false);
		GEditor->SelectActor(_TreeItemActor.Get(), true, true, true);
		SetIsOpen(ShouldOpenDueToClick(), false);

		// If the menu is open, execute the related delegate.
		if (IsOpen() && OnComboBoxOpened.IsBound())
		{
			OnComboBoxOpened.Execute();
		}

		// Focusing any newly-created widgets must occur after they have been added to the UI root.
		FReply ButtonClickedReply = FReply::Handled();

		if (bIsFocusable)
		{
			TSharedPtr<SWidget> WidgetToFocus = WidgetToFocusPtr.Pin();
			if (!WidgetToFocus.IsValid())
			{
				// no explicitly focused AnchorData, try to focus the content
				WidgetToFocus = MenuContent;
			}

			if (!WidgetToFocus.IsValid())
			{
				// no content, so try to focus the original AnchorData set on construction
				WidgetToFocus = ContentWidgetPtr.Pin();
			}

			if (WidgetToFocus.IsValid())
			{
				ButtonClickedReply.SetUserFocus(WidgetToFocus.ToSharedRef(), EFocusCause::SetDirectly);
			}
		}

		return ButtonClickedReply;

	}
	return FReply::Handled();
}
FReply SLGUISceneOutlinerButton::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}
FReply SLGUISceneOutlinerButton::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		return OnButtonClicked();
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE