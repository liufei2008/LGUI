// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FLGUIPrefabEditor;
struct FLGUIPrefabOverrideParameterData;

DECLARE_DELEGATE_TwoParams(FLGUIPrefabOverrideDataViewer_RevertPrefabWithParameterSet, UObject*, const TSet<FName>&);
DECLARE_DELEGATE_TwoParams(FLGUIPrefabOverrideDataViewer_RevertPrefabWithParameter, UObject*, const FName&);
DECLARE_DELEGATE(FLGUIPrefabOverrideDataViewer_RevertPrefabAllParameters);
//@todo: Apply single parameter
DECLARE_DELEGATE(FLGUIPrefabOverrideDataViewer_ApplyPrefabAllParameters);

class SLGUIPrefabOverrideDataViewer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGUIPrefabOverrideDataViewer) {}
		SLATE_EVENT(FLGUIPrefabOverrideDataViewer_RevertPrefabWithParameterSet, RevertPrefabWithParameterSet)
		SLATE_EVENT(FLGUIPrefabOverrideDataViewer_RevertPrefabWithParameter, RevertPrefabWithParameter)
		SLATE_EVENT(FLGUIPrefabOverrideDataViewer_RevertPrefabAllParameters, RevertPrefabAllParameters)
		SLATE_EVENT(FLGUIPrefabOverrideDataViewer_ApplyPrefabAllParameters, ApplyPrefabAllParameters)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void RefreshDataContent(const TArray<FLGUIPrefabOverrideParameterData>& ObjectOverrideParameterArray);
private:
	FLGUIPrefabOverrideDataViewer_RevertPrefabWithParameterSet RevertPrefabWithParameterSet;
	FLGUIPrefabOverrideDataViewer_RevertPrefabWithParameter RevertPrefabWithParameter;
	FLGUIPrefabOverrideDataViewer_RevertPrefabAllParameters RevertPrefabAllParameters;
	FLGUIPrefabOverrideDataViewer_ApplyPrefabAllParameters ApplyPrefabAllParameters;

	TSharedPtr<SVerticalBox> RootContentVerticalBox;
};
