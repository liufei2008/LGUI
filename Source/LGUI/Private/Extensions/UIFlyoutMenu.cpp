// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Extensions/UIFlyoutMenu.h"
#include "Extensions/UIFlyoutMenuItem.h"
#include "Core/Actor/UITextActor.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/Actor/UISpriteActor.h"
#include "Core/ActorComponent/UISprite.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Event/LGUIEventSystem.h"
#include "LGUIBPLibrary.h"

UUIFlyoutMenu::UUIFlyoutMenu()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIFlyoutMenu::BeginPlay()
{
	Super::BeginPlay();
}

void UUIFlyoutMenu::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UUIFlyoutMenu::CreateFromArray_Internal(const TArray<FString>& InItemNameArray, const FUIFlyoutMenuSelectDelegate& InCallback)
{
	if (ULGUIEventSystem::GetLGUIEventSystemInstance() != nullptr)
	{
		ULGUIEventSystem::GetLGUIEventSystemInstance()->SetSelectComponent(_RootUIActor->GetUIItem());
	}
	auto parentActor = _SrcItemActor->GetAttachParentActor();
	{
		auto script = _SrcItemActor->FindComponentByClass<UUIFlyoutMenuItem>();
		script->Init(this, 0, InItemNameArray[0]);
		_CreatedItemArray.Add(script);
	}
	int count = InItemNameArray.Num();
	for (int i = 1; i < count; i++)
	{
		auto copiedActor = ULGUIBPLibrary::DuplicateActor(_SrcItemActor, parentActor->GetRootComponent());
		auto script = copiedActor->FindComponentByClass<UUIFlyoutMenuItem>();
		script->Init(this, i, InItemNameArray[i]);
		_CreatedItemArray.Add(script);
	}
	_SelectionChangeCallback = InCallback;
}
void UUIFlyoutMenu::CreateFlyoutMenuFromArray(const TArray<FString>& InItemNameArray, const FUIFlyoutMenuSelectDynamicDelegate& InCallback, class AUIBaseActor* InParentActor, int32 InWidth, EFlyoutMenuVerticalPosition InVerticalPosition, EFlyoutMenuHorizontalAlignment InHorizontalAlign)
{
	CreateFlyoutMenuFromArray(InItemNameArray, FUIFlyoutMenuSelectDelegate::CreateLambda([=](int32 InSelectItemIndex, FString InSelectItem) {
		if (InCallback.IsBound())
			InCallback.Execute(InSelectItemIndex, InSelectItem);
	}), InParentActor, InWidth, InVerticalPosition, InHorizontalAlign);
}
void UUIFlyoutMenu::CreateFlyoutMenuFromArray(const TArray<FString>& InItemNameArray, const FUIFlyoutMenuSelectDelegate& InCallback, class AUIBaseActor* InParentActor, int32 InWidth, EFlyoutMenuVerticalPosition InVerticalPosition, EFlyoutMenuHorizontalAlignment InHorizontalAlign)
{
	auto prefab = LoadObject<ULGUIPrefab>(NULL, TEXT("/LGUI/Prefabs/DefaultFlyoutMenu"));
	if (prefab == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[UIFlyoutMenu::CreateFlyoutMenuFromArray]Load preset prefab error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
		return;
	}
	if (InItemNameArray.Num() == 0)
	{
		UE_LOG(LGUI, Error, TEXT("[UIFlyoutMenu/CreateFromArray]Input array count is 0!"));
		return;
	}
	auto loadedActor = ULGUIBPLibrary::LoadPrefab(InParentActor, prefab, InParentActor->GetRootComponent());
	auto script = loadedActor->FindComponentByClass<UUIFlyoutMenu>();
	auto rootCanvas = script->_RootUIActor->FindComponentByClass<ULGUICanvas>();
	rootCanvas->SetSortOrderToHighestOfHierarchy(true);
	auto rootUIItem = script->_RootUIActor->GetUIItem();
	FVector2D pivot(0.5f, 0);
	float anchorOffsetY = 0;
	switch (InVerticalPosition)
	{
	case EFlyoutMenuVerticalPosition::Top:
	{
		pivot.Y = 0.0f;
		rootUIItem->SetAnchorVAlign(UIAnchorVerticalAlign::Top, false);
	}break;
	case EFlyoutMenuVerticalPosition::Bottom:
	{
		pivot.Y = 1.0f;
		rootUIItem->SetAnchorVAlign(UIAnchorVerticalAlign::Bottom, false);
	}break;
	}
	switch (InHorizontalAlign)
	{
	case EFlyoutMenuHorizontalAlignment::Left:
	{
		pivot.X = 0.0f;
		rootUIItem->SetAnchorHAlign(UIAnchorHorizontalAlign::Left, false);
	}break;
	case EFlyoutMenuHorizontalAlignment::Center:
	{
		pivot.X = 0.5f;
		rootUIItem->SetAnchorHAlign(UIAnchorHorizontalAlign::Center, false);
	}break;
	case EFlyoutMenuHorizontalAlignment::Right:
	{
		pivot.X = 1.0f;
		rootUIItem->SetAnchorHAlign(UIAnchorHorizontalAlign::Right, false);
	}break;
	}
	rootUIItem->SetPivot(pivot);
	rootUIItem->SetWidth(InWidth);
	script->CreateFromArray_Internal(InItemNameArray, InCallback);
}
void UUIFlyoutMenu::OnClickItem(const int32& InItemIndex, const FString& InItemName)
{
	_SelectionChangeCallback.ExecuteIfBound(InItemIndex, InItemName);
	ULGUIBPLibrary::DeleteActor(GetOwner());
}

bool UUIFlyoutMenu::OnPointerSelect_Implementation(const FLGUIPointerEventData& eventData)
{
	return false;
}
bool UUIFlyoutMenu::OnPointerDeselect_Implementation(const FLGUIPointerEventData& eventData)
{
	auto selectedComp = ULGUIEventSystem::GetLGUIEventSystemInstance()->GetCurrentSelectedComponent();
	if (selectedComp == nullptr || !selectedComp->IsAttachedTo(_RootUIActor->GetUIItem()) && selectedComp != _RootUIActor->GetUIItem())
	{
		ULGUIBPLibrary::DeleteActor(GetOwner());
	}
	return false;
}
