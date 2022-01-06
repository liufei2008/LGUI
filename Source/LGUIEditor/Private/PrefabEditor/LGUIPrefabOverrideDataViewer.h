// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FLGUIPrefabEditor;

class SLGUIPrefabOverrideDataViewer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGUIPrefabOverrideDataViewer) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FLGUIPrefabEditor> InPrefabEditorPtr);
	void RefreshDataContent(AActor* InSubPrefabActor);
private:
	TWeakPtr<FLGUIPrefabEditor> PrefabEditorPtr;
	TSharedPtr<SVerticalBox> RootContentVerticalBox;
};
