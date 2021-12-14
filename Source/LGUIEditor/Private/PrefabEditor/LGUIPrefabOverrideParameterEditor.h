// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class ULGUIPrefabOverrideParameterObject;
class FLGUIPrefabEditor;

class SLGUIPrefabOverrideParameterEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGUIPrefabOverrideParameterEditor) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FLGUIPrefabEditor> InPrefabEditorPtr);
	void SetTargetObject(ULGUIPrefabOverrideParameterObject* InTargetObject, AActor* InActor);
	void SetTipText(const FString& InTipText);
private:
	FReply OnClickApplyParameterButton();
	TWeakPtr<FLGUIPrefabEditor> PrefabEditorPtr;
	TSharedPtr<IDetailsView> DescriptorDetailView;
	TSharedPtr<STextBlock> TipText;
	TWeakObjectPtr<AActor> PrefabRootActor;
	TWeakObjectPtr<ULGUIPrefabOverrideParameterObject> TargetObject;
	EVisibility ApplyChangeButtonVisibility()const;
};
