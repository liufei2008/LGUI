// Copyright 2019 LexLiu. All Rights Reserved.

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#pragma once


class FLGUIComponentListItem
{
public:
	FLGUIComponentListItem(FName InCompName, FString InCompDiscription)
	{
		ComponentName = InCompName;
		ComponentDiscription = InCompDiscription;
	}
	FName ComponentName;
	FString ComponentDiscription;
};
/**
 * 
 */
class SLGUIEventComponentSelector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGUIEventComponentSelector) {}
	
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<SDockTab> InOwnerTab);

	static class AActor* TargetActor;
	static class ILGUIDrawableEventCustomizationInterface* TargetCustomization;
	static int32 TargetItemIndex;
	static IPropertyHandleArray* EventListHandle;
private:
	TSharedPtr<SDockTab> OwnerTab;
	void CloseTabCallback(TSharedRef<SDockTab> TabClosed);
	
	FReply OnClickSelfButton();

	TSharedRef<ITableRow> OnGenerateTemplateTile(TSharedPtr<FLGUIComponentListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OnTemplateSelectionChanged(TSharedPtr<FLGUIComponentListItem> InItem, ESelectInfo::Type SelectInfo);
	TArray<TSharedPtr<FLGUIComponentListItem>> ItemSource;
};
