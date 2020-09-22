// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "DetailCustomization/UIEffectTextAnimationPropertyCustomization.h"
#include "GeometryModifier/UIEffectTextAnimation.h"
#include "LGUIEditorPCH.h"

#define LOCTEXT_NAMESPACE "UIEffectTextAnimationPropertyCustomization"

TSharedRef<IDetailCustomization> FUIEffectTextAnimationPropertyCustomization::MakeInstance()
{
	return MakeShareable(new FUIEffectTextAnimationPropertyCustomization);
}
void FUIEffectTextAnimationPropertyCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<UUIEffectTextAnimation_Property>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[UIEffectTextAnimationPropertyCustomization]Get TargetScript is null"));
		return;
	}

	TArray<FName> needToHidePropertyName;
	auto easeTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIEffectTextAnimation_Property, easeType));
	easeTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=, &DetailBuilder] {
		DetailBuilder.ForceRefreshDetails();
	}));
	if (TargetScriptPtr->GetEaseType() != LTweenEase::CurveFloat)
	{
		needToHidePropertyName.Add(GET_MEMBER_NAME_CHECKED(UUIEffectTextAnimation_Property, easeCurve));
	}

	for (auto item : needToHidePropertyName)
	{
		DetailBuilder.HideProperty(item);
	}
}
#undef LOCTEXT_NAMESPACE