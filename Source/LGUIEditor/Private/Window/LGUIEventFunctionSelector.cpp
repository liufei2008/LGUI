// Copyright 2019 LexLiu. All Rights Reserved.

#include "Window/LGUIEventFunctionSelector.h"
#include "DetailCustomization/LGUIDrawableEventCustomization.h"

#define LOCTEXT_NAMESPACE "LGUIEventFunctionSelector"

TWeakObjectPtr<UObject> SLGUIEventFunctionSelector::TargetObject = nullptr;
ILGUIDrawableEventCustomizationInterface* SLGUIEventFunctionSelector::TargetCustomization = nullptr;
int32 SLGUIEventFunctionSelector::TargetItemIndex = 0;
TArray<LGUIDrawableEventParameterType> SLGUIEventFunctionSelector::NativeSupportParameter;
IPropertyHandleArray* SLGUIEventFunctionSelector::EventListHandle = nullptr;

void SLGUIEventFunctionSelector::Construct(const FArguments& Args, TSharedPtr<SDockTab> InOwnerTab)
{
	OwnerTab = InOwnerTab;
	InOwnerTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateSP(this, &SLGUIEventFunctionSelector::CloseTabCallback));
	TSharedPtr<SWidget> ContentWidget;
	if (!TargetObject.IsValid())
	{
		ContentWidget = SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock).Text(FText::FromString("Target object is null"))
			];
	}
	else
	{
		auto TempContent = SNew(SVerticalBox);
		ContentWidget = TempContent;
		//add function button
		auto FunctionField = TFieldRange<UFunction>(TargetObject->GetClass());
		for (auto Func : FunctionField)
		{
			TArray<LGUIDrawableEventParameterType> paramTypeArray;
			if (ULGUIDrawableEventParameterHelper::IsSupportedFunction(Func, paramTypeArray) && (paramTypeArray.Num() == NativeSupportParameter.Num() || paramTypeArray.Num() == 0))//show only supported type
			{
				if (paramTypeArray.Num() == 0)//empty parameter
				{
					FString ParamTypeString = ULGUIDrawableEventParameterHelper::ParameterTypeToName({ }, Func);
					auto FuncItem = TSharedRef<FLGUIFunctionListItem>(new FLGUIFunctionListItem(Func->GetFName(), ParamTypeString, { }, false));
					ItemSource.Add(FuncItem);
				}
				else//
				{
					FString ParamTypeString(TEXT(""));
					for (auto item : paramTypeArray)
					{
						ParamTypeString += ULGUIDrawableEventParameterHelper::ParameterTypeToName(item, Func);
						ParamTypeString.AppendChar(',');
					}
					ParamTypeString.RemoveAt(ParamTypeString.Len() - 1);
					auto FuncItem = TSharedRef<FLGUIFunctionListItem>(new FLGUIFunctionListItem(Func->GetFName(), ParamTypeString, paramTypeArray, false));
					ItemSource.Add(FuncItem);
					if (paramTypeArray == NativeSupportParameter)//if function support native parameter, then draw another button, and show as native parameter
					{
						auto NativeFuncName = TSharedRef<FLGUIFunctionListItem>(new FLGUIFunctionListItem(Func->GetFName(), "(NativeParameter)", paramTypeArray, true));
						ItemSource.Add(NativeFuncName);
					}
				}
			}
		}

		TempContent->AddSlot()
			.Padding(2)
			[
				SNew(SListView<TSharedPtr<FLGUIFunctionListItem>>)
				.SelectionMode(ESelectionMode::Single)
				.ListItemsSource(&ItemSource)
				.OnGenerateRow(this, &SLGUIEventFunctionSelector::OnGenerateTemplateTile)
				.OnSelectionChanged(this, &SLGUIEventFunctionSelector::OnTemplateSelectionChanged)
				.ItemHeight(50)
			];
	}

	ChildSlot
		[
			ContentWidget.ToSharedRef()
		];
}
void SLGUIEventFunctionSelector::CloseTabCallback(TSharedRef<SDockTab> TabClosed)
{
	
}


TSharedRef<ITableRow> SLGUIEventFunctionSelector::OnGenerateTemplateTile(TSharedPtr<FLGUIFunctionListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FLGUIFunctionListItem>>, OwnerTable)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("NoBorder"))
			.Padding(FMargin(5, 5))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(FText::FromString(InItem->FunctionName.ToString()))
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				[
					SNew(STextBlock)
					.Text(FText::FromString(InItem->ParameterTypeString))
				]
			]
		];
}
void SLGUIEventFunctionSelector::OnTemplateSelectionChanged(TSharedPtr<FLGUIFunctionListItem> InItem, ESelectInfo::Type SelectInfo)
{
	if (TargetCustomization != nullptr)
	{
		TargetCustomization->OnSelectFunction(EventListHandle, InItem->FunctionName, TargetItemIndex, InItem->ParamType, InItem->UseNativeParameter);
	}
	OwnerTab.Pin()->RequestCloseTab();
}
#undef LOCTEXT_NAMESPACE