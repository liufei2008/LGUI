// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "ISceneOutliner.h"
#include "Widgets/Input/SComboButton.h"

class SLGUISceneOutlinerButton : public SComboButton
{
protected:
	virtual FReply OnButtonClicked()override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)override;
public:
	TWeakObjectPtr<AActor> _TreeItemActor;
};