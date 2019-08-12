// Copyright 2019 LexLiu. All Rights Reserved.

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#pragma once

class FLGUIFunctionListItem
{
public:
	FLGUIFunctionListItem(FName InFunctionName, FString InParameterTypeString, TArray<LGUIDrawableEventParameterType> InParamTypeArray, bool InUseNativeParameter)
	{
		FunctionName = InFunctionName;
		ParameterTypeString = InParameterTypeString;
		ParamType = InParamTypeArray;
		UseNativeParameter = InUseNativeParameter;
	}
	FName FunctionName;
	FString ParameterTypeString;
	TArray<LGUIDrawableEventParameterType> ParamType;
	bool UseNativeParameter = false;
};
/**
 * 
 */
class SLGUIEventFunctionSelector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGUIEventFunctionSelector) {}
	
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<SDockTab> InOwnerTab);

	static TWeakObjectPtr<class UObject> TargetObject;//Actor or ActorComponent
	static class ILGUIDrawableEventCustomizationInterface* TargetCustomization;
	static int32 TargetItemIndex;
	static TArray<LGUIDrawableEventParameterType> NativeSupportParameter;//event native supported parameter
	static IPropertyHandleArray* EventListHandle;
private:
	TWeakPtr<SDockTab> OwnerTab;
	void CloseTabCallback(TSharedRef<SDockTab> TabClosed);

	TSharedRef<ITableRow> OnGenerateTemplateTile(TSharedPtr<FLGUIFunctionListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OnTemplateSelectionChanged(TSharedPtr<FLGUIFunctionListItem> InItem, ESelectInfo::Type SelectInfo);
	TArray<TSharedPtr<FLGUIFunctionListItem>> ItemSource;
};
