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
		/**
		 * Refresh override parameter data.
		 * \param ObjectOverrideParameterArray All override parameter data in this sub prefab.
		 * \param InReferenceActor Pass in actor in sub prefab means only show override parameters of the actor or it's components. Pass in nullptr menas show all.
		 */
	void RefreshDataContent(TArray<FLGUIPrefabOverrideParameterData> ObjectOverrideParameterArray, AActor* InReferenceActor);
	void SetPrefabHelperObject(ULGUIPrefabHelperObject* InPrefabHelperObject);
private:
	FLGUIPrefabOverrideDataViewer_AfterRevertPrefab AfterRevertPrefab;
	FLGUIPrefabOverrideDataViewer_AfterApplyPrefab AfterApplyPrefab;

	TSharedPtr<SVerticalBox> RootContentVerticalBox;
	TWeakObjectPtr<ULGUIPrefabHelperObject> PrefabHelperObject;
};
