// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Interaction/UIDropdownComponent.h"
#include "Event/LGUIEventSystem.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUIBPLibrary.h"
#include "Core/LGUISpriteData.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/ActorComponent/UISprite.h"
#include "Core/Actor/UIContainerActor.h"
#include "Core/Actor/UITextActor.h"
#include "Core/Actor/UISpriteActor.h"
#include "Interaction/UIButtonComponent.h"

UUIDropdownComponent::UUIDropdownComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ItemTemplate = FLGUIComponentReference(UUIDropdownItemComponent::StaticClass());
}

void UUIDropdownComponent::Awake()
{
	Super::Awake();
	if (ListRoot.IsValid())
	{
		ListRoot->GetUIItem()->SetUIActive(false);
		ListRoot->GetUIItem()->SetAlpha(0);
		MaxHeight = ListRoot->GetUIItem()->GetHeight();
	}
}
#if WITH_EDITOR
void UUIDropdownComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (Options.Num() > 0)
	{
		auto tempValue = FMath::Clamp(Value, 0, Options.Num() - 1);
		if (CaptionText.IsValid())
		{
			CaptionText->GetUIText()->SetText(Options[tempValue].Text);
		}
		if (CaptionSprite.IsValid() && IsValid(Options[tempValue].Sprite))
		{
			CaptionSprite->GetUISprite()->SetSprite(Options[tempValue].Sprite);
		}
	}
}
#endif

void UUIDropdownComponent::Show()
{
	if (!ListRoot.IsValid())
	{
		UE_LOG(LGUI, Error, TEXT("[UUIDropdownComponent::Show]ListRoot is not valid!"));
		return;
	}
	if (!IsValid(this->GetRootComponent()))return;
	if (!IsValid(this->GetRootComponent()->GetRootCanvas()))return;
	if (IsShow)return;
	IsShow = true;
	if (ShowOrHideTweener.IsValid())
	{
		ShowOrHideTweener->Kill();
	}

	//create blocker
	CreateBlocker();
	//show list
	ListRoot->GetUIItem()->SetUIActive(true);
	auto listRootUIItem = ListRoot->GetUIItem();
	ShowOrHideTweener = ULGUIBPLibrary::AlphaTo(listRootUIItem, 1, 0.3f, 0, LTweenEase::OutCubic);
	auto canvasOnListRoot = ListRoot->FindComponentByClass<ULGUICanvas>();
	if (!IsValid(canvasOnListRoot))
	{
		canvasOnListRoot = NewObject<ULGUICanvas>(ListRoot.Get());
		canvasOnListRoot->RegisterComponent();
	}

	bool sortOrderSet = false;
	if (BlockerActor.IsValid())
	{
		if (auto blockerCanvas =BlockerActor->FindComponentByClass<ULGUICanvas>())
		{
			canvasOnListRoot->SetSortOrder(blockerCanvas->GetSortOrder() + 1, true);
			sortOrderSet = true;
		}
	}
	if(!sortOrderSet)
	{
		canvasOnListRoot->SetSortOrderToHighestOfHierarchy(true);
	}

	//create list item as options
	if (!ItemTemplate.IsValid())
	{
		UE_LOG(LGUI, Error, TEXT("[UUIDropdownComponent::Show]ItemTemplate is not valid!"));
		return;
	}
	if (NeedRecreate)
	{
		NeedRecreate = false;
		for (auto item : CreatedItemArray)
		{
			auto itemActor = item->GetOwner();
			ULGUIBPLibrary::DestroyActorWithHierarchy(itemActor, true);
		}
		CreatedItemArray.Reset();
		//create items
		CreateListItems();
	}

	//set position
	auto tempVerticalPosition = VerticalPosition;
	auto tempHorizontalPosition = HorizontalPosition;
	if (tempVerticalPosition == EUIDropdownVerticalPosition::Automatic
		|| tempHorizontalPosition == EUIDropdownHorizontalPosition::Automatic
		)
	{
		auto rootUIItem = GetRootComponent()->GetRootCanvas()->GetUIItem();
		auto rootBottom = rootUIItem->GetLocalSpaceBottom();
		auto rootLeft = rootUIItem->GetLocalSpaceLeft();

		FTransform selfToRootTf;
		auto inverseCanvasTf = rootUIItem->GetComponentTransform().Inverse();
		FTransform::Multiply(&selfToRootTf, &GetRootComponent()->GetComponentTransform(), &inverseCanvasTf);
		if (tempVerticalPosition == EUIDropdownVerticalPosition::Automatic)
		{
			//convert top point position from dropdown's self to root ui space, and tell if it is inside root rect
			FVector listBottomInRootSpace;
			if (VerticalOverlap)
			{
				auto selfTop = GetRootComponent()->GetLocalSpaceTop();
				auto listTopInSelfSpace = selfTop - listRootUIItem->GetHeight();
				listBottomInRootSpace = selfToRootTf.TransformPosition(FVector(0, listTopInSelfSpace, 0));
			}
			else
			{
				auto selfBottom = GetRootComponent()->GetLocalSpaceBottom();
				auto listBottomInSelfSpace = selfBottom - listRootUIItem->GetHeight();
				listBottomInRootSpace = selfToRootTf.TransformPosition(FVector(0, listBottomInSelfSpace, 0));
			}
			if (listBottomInRootSpace.Y < rootBottom)
			{
				tempVerticalPosition = EUIDropdownVerticalPosition::Top;
			}
			else
			{
				tempVerticalPosition = EUIDropdownVerticalPosition::Bottom;
			}
		}
		if (tempHorizontalPosition == EUIDropdownHorizontalPosition::Automatic)
		{
			auto selfLeft = GetRootComponent()->GetLocalSpaceLeft();
			auto listLeftInRootSpace = selfToRootTf.TransformPosition(FVector(selfLeft - listRootUIItem->GetWidth(), 0, 0));
			if (listLeftInRootSpace.X > rootLeft)
			{
				tempHorizontalPosition = EUIDropdownHorizontalPosition::Left;
			}
			else
			{
				tempHorizontalPosition = EUIDropdownHorizontalPosition::Right;
			}
		}
	}

	FVector2D pivot(0.5f, 0);
	switch (tempVerticalPosition)
	{
	case EUIDropdownVerticalPosition::Top:
	{
		pivot.Y = 0.0f;
		if (VerticalOverlap)
		{
			listRootUIItem->SetAnchorVAlign(UIAnchorVerticalAlign::Bottom, false);
		}
		else
		{
			listRootUIItem->SetAnchorVAlign(UIAnchorVerticalAlign::Top, false);
		}
	}break;
	case EUIDropdownVerticalPosition::Middle:
	{
		pivot.Y = 0.5f;
		listRootUIItem->SetAnchorVAlign(UIAnchorVerticalAlign::Middle, false);
	}break;
	case EUIDropdownVerticalPosition::Bottom:
	{
		pivot.Y = 1.0f;
		if (VerticalOverlap)
		{
			listRootUIItem->SetAnchorVAlign(UIAnchorVerticalAlign::Top, false);
		}
		else
		{
			listRootUIItem->SetAnchorVAlign(UIAnchorVerticalAlign::Bottom, false);
		}
	}break;
	}
	switch (tempHorizontalPosition)
	{
	case EUIDropdownHorizontalPosition::Left:
	{
		pivot.X = 1.0f;
		listRootUIItem->SetAnchorHAlign(UIAnchorHorizontalAlign::Left, false);
	}break;
	case EUIDropdownHorizontalPosition::Center:
	{
		pivot.X = 0.5f;
		listRootUIItem->SetAnchorHAlign(UIAnchorHorizontalAlign::Center, false);
	}break;
	case EUIDropdownHorizontalPosition::Right:
	{
		pivot.X = 0.0f;
		listRootUIItem->SetAnchorHAlign(UIAnchorHorizontalAlign::Right, false);
	}break;
	}
	listRootUIItem->SetPivot(pivot);
}
void UUIDropdownComponent::Hide()
{
	if (!ListRoot.IsValid())
	{
		UE_LOG(LGUI, Error, TEXT("[UUIDropdownComponent::Show]ListRoot is not valid!"));
		return;
	}
	if (!IsShow)return;
	IsShow = false;
	if (ShowOrHideTweener.IsValid())
	{
		ShowOrHideTweener->Kill();
	}

	auto listRootUIItem = ListRoot->GetUIItem();
	ShowOrHideTweener = ULGUIBPLibrary::AlphaTo(listRootUIItem, 0, 0.3f, 0, LTweenEase::InCubic)->OnComplete(FSimpleDelegate::CreateWeakLambda(listRootUIItem, [listRootUIItem] {
		listRootUIItem->SetUIActive(false);
		}));

	if (BlockerActor.IsValid())
	{
		BlockerActor->Destroy();
		BlockerActor.Reset();
	}
}
void UUIDropdownComponent::CreateBlocker()
{
	AUIContainerActor* blocker = this->GetWorld()->SpawnActor<AUIContainerActor>();
#if WITH_EDITOR
	blocker->SetActorLabel(TEXT("UIDropdown_Blocker"));
#endif
	auto blockerUIItem = blocker->GetUIItem();
	blockerUIItem->SetRaycastTarget(true);
	blockerUIItem->SetTraceChannel(this->GetRootComponent()->GetTraceChannel());
	blockerUIItem->AttachToComponent(this->GetRootComponent()->GetRootCanvas()->GetUIItem(), FAttachmentTransformRules::KeepRelativeTransform);
	blockerUIItem->SetAnchorHAlign(UIAnchorHorizontalAlign::Stretch);
	blockerUIItem->SetAnchorVAlign(UIAnchorVerticalAlign::Stretch);
	blockerUIItem->SetHorizontalStretch(FVector2D::ZeroVector);
	blockerUIItem->SetVerticalStretch(FVector2D::ZeroVector);
	auto blockerCanvas = NewObject<ULGUICanvas>(blocker);
	blockerCanvas->RegisterComponent();
	blocker->AddInstanceComponent(blockerCanvas);
	blockerCanvas->SetSortOrderToHighestOfHierarchy();
	auto blockerButton = NewObject<UUIButtonComponent>(blocker);
	blockerButton->RegisterComponent();
	blocker->AddInstanceComponent(blockerButton);
	blockerButton->RegisterClickEvent([this] {
		this->Hide();
		});
	BlockerActor = blocker;
}
void UUIDropdownComponent::CreateListItems()
{
	auto templateUIItem = Cast<UUIItem>(ItemTemplate.GetActor()->GetRootComponent());
	if (!IsValid(templateUIItem))
	{
		UE_LOG(LGUI, Error, TEXT("[UUIDropdownComponent::Show]ItemTemplate must be a UIItem!"));
		return;
	}
	templateUIItem->SetUIActive(true);
	auto contentUIItem = templateUIItem->GetParentAsUIItem();
	for (int i = 0, count = Options.Num(); i < count; i++)
	{
		auto copiedItemActor = ULGUIBPLibrary::DuplicateActor(ItemTemplate.GetActor(), contentUIItem);
#if WITH_EDITOR
		copiedItemActor->SetActorLabel(FString::Printf(TEXT("Item_%d"), i));
#endif
		auto script = copiedItemActor->FindComponentByClass<UUIDropdownItemComponent>();
		int index = i;
		script->Init(Options[i], [=]() {
			this->OnSelectItem(index);
			});
		script->SetSelectionState(i == Value);
		CreatedItemArray.Add(script);
	}
	templateUIItem->SetUIActive(false);
	float heightOffset = 0;
	if (auto viewportUIItem = contentUIItem->GetParentAsUIItem())
	{
		heightOffset = ListRoot->GetUIItem()->GetHeight() - viewportUIItem->GetHeight();
	}
	if (contentUIItem->GetHeight() + heightOffset < MaxHeight)
	{
		ListRoot->GetUIItem()->SetHeight(contentUIItem->GetHeight() + heightOffset);
	}
}
FUIDropdownOptionData UUIDropdownComponent::GetOption(int index)const
{
	if (index >= Options.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[UUIDropdownComponent::GetOption]index: %d out of range: %d!"), index, Options.Num());
		return FUIDropdownOptionData();
	}
	return Options[index];
}
FUIDropdownOptionData UUIDropdownComponent::GetCurrentOption()const
{
	if (Value >= Options.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[UUIDropdownComponent::GetCurrentOption]Value: %d out of range: %d!"), Value, Options.Num());
		return FUIDropdownOptionData();
	}
	return Options[Value];
}
void UUIDropdownComponent::SetValue(int newValue, bool fireEvent)
{
	if (Value != newValue)
	{
		if (Value >= Options.Num())
		{
			UE_LOG(LGUI, Error, TEXT("[UUIDropdownComponent::SetValue]Value: %d out of range: %d!"), newValue, Options.Num());
			return;
		}
		Value = newValue;
		if (fireEvent)
		{
			if (OnSelectionChangeCPP.IsBound())OnSelectionChangeCPP.Broadcast(Value);
			OnSelectionChange.FireEvent(Value);
		}
		ApplyValueToUI();
	}
}
void UUIDropdownComponent::SetVerticalPosition(EUIDropdownVerticalPosition InValue)
{
	if (VerticalPosition != InValue)
	{
		VerticalPosition = InValue;
	}
}
void UUIDropdownComponent::SetHorizontalPosition(EUIDropdownHorizontalPosition InValue)
{
	if (HorizontalPosition != InValue)
	{
		HorizontalPosition = InValue;
	}
}
void UUIDropdownComponent::SetVerticalOverlap(bool newValue)
{
	if (VerticalOverlap != newValue)
	{
		VerticalOverlap = newValue;
	}
}
void UUIDropdownComponent::SetOptions(const TArray<FUIDropdownOptionData>& InOptions)
{
	NeedRecreate = true;
	Options = InOptions;
	ApplyValueToUI();
}
void UUIDropdownComponent::AddOptions(const TArray<FUIDropdownOptionData>& InOptions)
{
	NeedRecreate = true;
	Options.SetNumUninitialized(Options.Num() + InOptions.Num());
	for (int i = 0; i < InOptions.Num(); i++)
	{
		Options.Add(InOptions[i]);
	}
	ApplyValueToUI();
}

void UUIDropdownComponent::OnSelectItem(int index)
{
	SetValue(index, true);
	Hide();
	//apply to options
	for (int i = 0, count = Options.Num(); i < count; i++)
	{
		auto script = CreatedItemArray[i];
		if (script.IsValid())
		{
			script->SetSelectionState(i == Value);
		}
	}
}
void UUIDropdownComponent::ApplyValueToUI()
{
	if (Value >= Options.Num())return;

	if (CaptionText.IsValid())
	{
		CaptionText->GetUIText()->SetText(Options[Value].Text);
	}
	if (CaptionSprite.IsValid() && IsValid(Options[Value].Sprite))
	{
		CaptionSprite->GetUISprite()->SetSprite(Options[Value].Sprite);
	}
}
bool UUIDropdownComponent::OnPointerClick_Implementation(ULGUIPointerEventData* eventData)
{
	Show();
	return AllowEventBubbleUp;
}
bool UUIDropdownComponent::OnPointerDeselect_Implementation(ULGUIBaseEventData* eventData)
{
	if (IsValid(eventData->selectedComponent))
	{
		if (!eventData->selectedComponent->IsAttachedTo(this->GetRootComponent()))
		{
			Hide();
		}
	}
	return AllowEventBubbleUp;
}

FDelegateHandle UUIDropdownComponent::RegisterSelectionChangeEvent(const FLGUIInt32Delegate& InDelegate)
{
	return OnSelectionChangeCPP.Add(InDelegate);
}
FDelegateHandle UUIDropdownComponent::RegisterSelectionChangeEvent(const TFunction<void(int)>& InFunction)
{
	return OnSelectionChangeCPP.AddLambda(InFunction);
}
void UUIDropdownComponent::UnregisterSelectionChangeEvent(const FDelegateHandle& InHandle)
{
	OnSelectionChangeCPP.Remove(InHandle);
}
FLGUIDelegateHandleWrapper UUIDropdownComponent::RegisterSelectionChangeEvent(const FUIDropdownComponentDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnSelectionChangeCPP.AddLambda([InDelegate](int InSelection) {
		if (InDelegate.IsBound()) InDelegate.Execute(InSelection);
		});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUIDropdownComponent::UnregisterSelectionChangeEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnSelectionChangeCPP.Remove(InDelegateHandle.DelegateHandle);
}







#include "Interaction/UIDropdownComponent.h"
#include "Core/Actor/UITextActor.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/Actor/UISpriteActor.h"
#include "Core/ActorComponent/UISprite.h"
#include "Interaction/UIToggleComponent.h"

UUIDropdownItemComponent::UUIDropdownItemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Toggle = FLGUIComponentReference(UUIToggleComponent::StaticClass());
}

void UUIDropdownItemComponent::Init(const FUIDropdownOptionData& Data, const TFunction<void()>& OnSelect)
{
	if (TextActor.IsValid())
	{
		TextActor->GetUIText()->SetText(Data.Text);
	}
	if (SpriteActor.IsValid())
	{
		SpriteActor->GetUISprite()->SetSprite(Data.Sprite);
	}
	if (Toggle.IsValid())
	{
		auto toggleComp = Toggle.GetComponent<UUIToggleComponent>();
		toggleComp->RegisterToggleEvent([OnSelect, toggleComp](bool select){
			OnSelect();
		});
	}
}
void UUIDropdownItemComponent::SetSelectionState(const bool& InSelect)
{
	if (Toggle.IsValid())
	{
		Toggle.GetComponent<UUIToggleComponent>()->SetValue(InSelect, false);
	}
}
bool UUIDropdownItemComponent::OnPointerClick_Implementation(ULGUIPointerEventData* eventData)
{
	
	return false;
}