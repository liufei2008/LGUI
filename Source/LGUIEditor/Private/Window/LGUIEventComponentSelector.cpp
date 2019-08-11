// Copyright 2019 LexLiu. All Rights Reserved.

#include "Window/LGUIEventComponentSelector.h"
#include "DetailCustomization/LGUIDrawableEventCustomization.h"

#define LOCTEXT_NAMESPACE "LGUIEventComponentSelector"

AActor* SLGUIEventComponentSelector::TargetActor = nullptr;
ILGUIDrawableEventCustomizationInterface* SLGUIEventComponentSelector::TargetCustomization = nullptr;
int32 SLGUIEventComponentSelector::TargetItemIndex = 0;
IPropertyHandleArray* SLGUIEventComponentSelector::EventListHandle = nullptr;

void SLGUIEventComponentSelector::Construct(const FArguments& Args, TSharedPtr<SDockTab> InOwnerTab)
{
	OwnerTab = InOwnerTab;
	InOwnerTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateSP(this, &SLGUIEventComponentSelector::CloseTabCallback));
	TSharedPtr<SWidget> ContentWidget;
	if (TargetActor == nullptr)
	{
		ContentWidget = SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock).Text(FText::FromString("You must select a component first!"))
			];
	}
	else
	{
		auto TempContent = SNew(SVerticalBox);
		ContentWidget = TempContent;
		//add actor self
		auto SelfItem = TSharedRef<FLGUIComponentListItem>(new FLGUIComponentListItem(LGUIEventActorSelfName, TargetActor->GetClass()->GetName()));
		ItemSource.Add(SelfItem);
		//add components button
		auto Components = TargetActor->GetComponents();
		for (auto Comp : Components)
		{
			if(Comp->HasAnyFlags(EObjectFlags::RF_Transient))continue;//skip transient component
			auto Item = TSharedRef<FLGUIComponentListItem>(new FLGUIComponentListItem(Comp->GetFName(), Comp->GetClass()->GetName()));
			ItemSource.Add(Item);
		}
		TempContent->AddSlot()
			.Padding(2)
			[
				SNew(SListView<TSharedPtr<FLGUIComponentListItem>>)
				.SelectionMode(ESelectionMode::Single)
				.ListItemsSource(&ItemSource)
				.OnGenerateRow(this, &SLGUIEventComponentSelector::OnGenerateTemplateTile)
				.OnSelectionChanged(this, &SLGUIEventComponentSelector::OnTemplateSelectionChanged)
				.ItemHeight(50)
			];
	}

	ChildSlot
		[
			ContentWidget.ToSharedRef()
		];
}
void SLGUIEventComponentSelector::CloseTabCallback(TSharedRef<SDockTab> TabClosed)
{
	
}
FReply SLGUIEventComponentSelector::OnClickSelfButton()
{
	TargetCustomization->OnSelectActorSelf(EventListHandle, TargetItemIndex);
	OwnerTab.Pin()->RequestCloseTab();
	return FReply::Handled();
}
TSharedRef<ITableRow> SLGUIEventComponentSelector::OnGenerateTemplateTile(TSharedPtr<FLGUIComponentListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FLGUIComponentListItem>>, OwnerTable)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("NoBorder"))
			.Padding(FMargin(5, 5))
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(FText::FromString(InItem->ComponentName.ToString()))
				]
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				[
					SNew(STextBlock)
					.Text(FText::FromString(InItem->ComponentDiscription))
				]
			]
		];
}
void SLGUIEventComponentSelector::OnTemplateSelectionChanged(TSharedPtr<FLGUIComponentListItem> InItem, ESelectInfo::Type SelectInfo)
{
	if (InItem->ComponentName == LGUIEventActorSelfName && InItem->ComponentDiscription == TargetActor->GetClass()->GetName())//actor self selected
	{
		TargetCustomization->OnSelectActorSelf(EventListHandle, TargetItemIndex);
	}
	else
	{
		TargetCustomization->OnSelectComponent(EventListHandle, InItem->ComponentName, TargetItemIndex);
	}
	OwnerTab.Pin()->RequestCloseTab();
}
#undef LOCTEXT_NAMESPACE