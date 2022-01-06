// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "Core/LGUISpriteData.h"
#include "Widgets/Views/STileView.h"
#include "Widgets/Views/STableViewBase.h"
#pragma once

/**
 * 
 */
class SLGUIAtlasViewer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGUIAtlasViewer) {}
	
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<SDockTab> InOwnerTab);

private:
	void CloseTabCallback(TSharedRef<SDockTab> TabClosed);
};
