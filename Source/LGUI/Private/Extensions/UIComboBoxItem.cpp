// Copyright 2019 LexLiu. All Rights Reserved.

#include "Extensions/UIComboBoxItem.h"
#include "Extensions/UIComboBox.h"

UUIComboBoxItem::UUIComboBoxItem()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUIComboBoxItem::BeginPlay()
{
	Super::BeginPlay();
}

void UUIComboBoxItem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UUIComboBoxItem::Init(class UUIComboBox* InManager, const int32 InItemIndex, const FString& InItemName)
{
	_Manager = InManager;
	_TextActor->GetUIText()->SetText(InItemName);
	_Index = InItemIndex;
	_RootUIActor->GetUIItem()->SetHierarchyIndex(InItemIndex);
}
void UUIComboBoxItem::SetSelectionState(const bool& InSelect)
{
	_HighlightSpriteActor->GetUIItem()->SetAlpha(InSelect ? 1.0f : 0.0f);
}
bool UUIComboBoxItem::OnPointerClick_Implementation(const FLGUIPointerEventData& eventData)
{
	_Manager->OnClickItem(_Index, _TextActor->GetUIText()->GetText());
	return false;
}