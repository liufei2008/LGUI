// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Extensions/UIFlyoutMenuItem.h"
#include "Extensions/UIFlyoutMenu.h"
#include "Core/Actor/UITextActor.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/Actor/UISpriteActor.h"
#include "Core/ActorComponent/UISprite.h"

UUIFlyoutMenuItem::UUIFlyoutMenuItem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIFlyoutMenuItem::BeginPlay()
{
	Super::BeginPlay();
}

void UUIFlyoutMenuItem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UUIFlyoutMenuItem::Init(class UUIFlyoutMenu* InManager, const int32 InItemIndex, const FString& InItemName)
{
	_Manager = InManager;
	_TextActor->GetUIText()->SetText(InItemName);
	_Index = InItemIndex;
	_RootUIActor->GetUIItem()->SetHierarchyIndex(InItemIndex);
}
void UUIFlyoutMenuItem::SetSelectionState(const bool& InSelect)
{
	_HighlightSpriteActor->GetUIItem()->SetAlpha(InSelect ? 1.0f : 0.0f);
}
bool UUIFlyoutMenuItem::OnPointerClick_Implementation(ULGUIPointerEventData* eventData)
{
	_Manager->OnClickItem(_Index, _TextActor->GetUIText()->GetText());
	return false;
}