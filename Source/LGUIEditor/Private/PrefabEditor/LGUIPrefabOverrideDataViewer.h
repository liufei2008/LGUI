// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FLGUIPrefabEditor;
struct FLGUIPrefabOverrideParameterData;
class ULGUIPrefabHelperObject;
class AActor;
class ULGUIPrefab;

DECLARE_DELEGATE_OneParam(FLGUIPrefabOverrideDataViewer_AfterRevertPrefab, ULGUIPrefab*);
DECLARE_DELEGATE_OneParam(FLGUIPrefabOverrideDataViewer_AfterApplyPrefab, ULGUIPrefab*);

class SLGUIPrefabOverrideDataViewer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGUIPrefabOverrideDataViewer) {}
		SLATE_EVENT(FLGUIPrefabOverrideDataViewer_AfterRevertPrefab, AfterRevertPrefab)
		SLATE_EVENT(FLGUIPrefabOverrideDataViewer_AfterApplyPrefab, AfterApplyPrefab)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, ULGUIPrefabHelperObject* InPrefabHelperObject);
	void RefreshDataContent(const TArray<FLGUIPrefabOverrideParameterData>& ObjectOverrideParameterArray);
	void SetPrefabHelperObject(ULGUIPrefabHelperObject* InPrefabHelperObject) { PrefabHelperObject = InPrefabHelperObject; }
private:
	FLGUIPrefabOverrideDataViewer_AfterRevertPrefab AfterRevertPrefab;
	FLGUIPrefabOverrideDataViewer_AfterApplyPrefab AfterApplyPrefab;

	TSharedPtr<SVerticalBox> RootContentVerticalBox;
	TWeakObjectPtr<ULGUIPrefabHelperObject> PrefabHelperObject;
};
