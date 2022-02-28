// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "WorkflowOrientedApp/WorkflowUObjectDocuments.h"


class FBlueprintEditor;
class SLGUIPrefabSequenceEditorWidgetImpl;
class ULGUIPrefabSequence;
class ULGUIPrefabSequenceComponent;


class SLGUIPrefabSequenceEditorWidget
	: public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SLGUIPrefabSequenceEditorWidget){}
	SLATE_END_ARGS();

	void Construct(const FArguments&, TWeakPtr<FBlueprintEditor> InBlueprintEditor);
	void AssignSequence(ULGUIPrefabSequence* NewLGUIPrefabSequence);
	ULGUIPrefabSequence* GetSequence() const;
	FText GetDisplayLabel() const;

private:

	TWeakPtr<SLGUIPrefabSequenceEditorWidgetImpl> Impl;
};

