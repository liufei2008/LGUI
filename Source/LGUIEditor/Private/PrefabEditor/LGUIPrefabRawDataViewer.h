// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FLGUIPrefabEditor;
class ULGUIPrefab;

class SLGUIPrefabRawDataViewer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGUIPrefabRawDataViewer) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FLGUIPrefabEditor> InPrefabEditorPtr, ULGUIPrefab* InPrefab);
private:
	TWeakPtr<FLGUIPrefabEditor> PrefabEditorPtr;
	TSharedPtr<IDetailsView> DescriptorDetailView;
};
