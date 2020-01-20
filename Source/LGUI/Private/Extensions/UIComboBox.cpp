// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Extensions/UIComboBox.h"
#include "Extensions/UIComboBoxItem.h"
#include "Event/LGUIEventSystem.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUIBPLibrary.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "Core/ActorComponent/UIItem.h"

UUIComboBox::UUIComboBox()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIComboBox::BeginPlay()
{
	Super::BeginPlay();
}

void UUIComboBox::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UUIComboBox::CreateFromArray_Internal(const TArray<FString>& InItemNameArray, const int32& InSelectedItemIndex, const FUIComboBoxSelectDelegate& InCallback)
{
	if (ULGUIEventSystem::GetLGUIEventSystemInstance() != nullptr)
	{
		ULGUIEventSystem::GetLGUIEventSystemInstance()->SetSelectComponent(_RootUIActor->GetUIItem());
	}
	auto parentActor = _SrcItemActor->GetAttachParentActor();
	{
		auto script = _SrcItemActor->FindComponentByClass<UUIComboBoxItem>();
		script->Init(this, 0, InItemNameArray[0]);
		if (0 == InSelectedItemIndex)
		{
			script->SetSelectionState(true);
		}
		else
		{
			script->SetSelectionState(false);
		}
		_CreatedItemArray.Add(script);
	}
	int count = InItemNameArray.Num();
	for (int i = 1; i < count; i++)
	{
		auto copiedActor = ULGUIBPLibrary::DuplicateActor(_SrcItemActor, parentActor->GetRootComponent());
		auto script = copiedActor->FindComponentByClass<UUIComboBoxItem>();
		script->Init(this, i, InItemNameArray[i]);
		if (i == InSelectedItemIndex)
		{
			script->SetSelectionState(true);
		}
		else
		{
			script->SetSelectionState(false);
		}
		_CreatedItemArray.Add(script);
	}
	_SelectionChangeCallback = InCallback;
}
void UUIComboBox::CreateComboBoxFromArray(const TArray<FString>& InItemNameArray, const FUIComboBoxSelectDynamicDelegate& InCallback, class AUIBaseActor* InParentActor, int32 InSelectedItemIndex, EComboBoxPosition InPosition)
{
	CreateComboBoxFromArray(InItemNameArray, FUIComboBoxSelectDelegate::CreateLambda([=](int32 InSelectItemIndex, FString InSelectItem) {
		if (InCallback.IsBound())
			InCallback.Execute(InSelectItemIndex, InSelectItem);
	}), InParentActor, InSelectedItemIndex, InPosition);
}
void UUIComboBox::CreateComboBoxFromArray(const TArray<FString>& InItemNameArray, const FUIComboBoxSelectDelegate& InCallback, class AUIBaseActor* InParentActor, int32 InSelectedItemIndex, EComboBoxPosition InPosition)
{
	auto prefab = LoadObject<ULGUIPrefab>(NULL, TEXT("/LGUI/Prefabs/DefaultComboBox"));
	if (prefab == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[UIComboBox::CreateComboBoxFromArray]Load preset prefab error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
		return;
	}
	if (InItemNameArray.Num() == 0)
	{
		UE_LOG(LGUI, Error, TEXT("[UIComboBox/CreateFromArray]Input array count is 0!"));
		return;
	}
	auto loadedActor = ULGUIBPLibrary::LoadPrefab(InParentActor, prefab, InParentActor->GetRootComponent());
	auto script = loadedActor->FindComponentByClass<UUIComboBox>();
	auto rootCanvas = script->_RootUIActor->FindComponentByClass<ULGUICanvas>();
	rootCanvas->SetSortOrderToHighestOfHierarchy(true);
	auto rootUIItem = script->_RootUIActor->GetUIItem();
	FVector2D pivot(0.5f, 0);
	float anchorOffsetY = 0;
	switch (InPosition)
	{
	case EComboBoxPosition::Top:
	{
		pivot.Y = 0.0f;
		rootUIItem->SetAnchorVAlign(UIAnchorVerticalAlign::Top, false);
	}break;
	case EComboBoxPosition::Middle:
	{
		pivot.Y = 0.5f;
		rootUIItem->SetAnchorVAlign(UIAnchorVerticalAlign::Middle, false);
	}break;
	case EComboBoxPosition::Bottom:
	{
		pivot.Y = 1.0f;
		rootUIItem->SetAnchorVAlign(UIAnchorVerticalAlign::Bottom, false);
	}break;
	}
	rootUIItem->SetPivot(pivot);
	script->CreateFromArray_Internal(InItemNameArray, InSelectedItemIndex, InCallback);
}
void UUIComboBox::OnClickItem(const int32& InItemIndex, const FString& InItemName)
{
	_SelectionChangeCallback.ExecuteIfBound(InItemIndex, InItemName);
	ULGUIBPLibrary::DeleteActor(GetOwner());
}

bool UUIComboBox::OnPointerSelect_Implementation(const FLGUIPointerEventData& eventData)
{
	return false;
}
bool UUIComboBox::OnPointerDeselect_Implementation(const FLGUIPointerEventData& eventData)
{
	auto selectedComp = ULGUIEventSystem::GetLGUIEventSystemInstance()->GetCurrentSelectedComponent();
	if (selectedComp == nullptr || !selectedComp->IsAttachedTo(_RootUIActor->GetUIItem()) && selectedComp != _RootUIActor->GetUIItem())
	{
		ULGUIBPLibrary::DeleteActor(GetOwner());
	}
	return false;
}
