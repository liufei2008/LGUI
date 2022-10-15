// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
#include "Layout/UILayoutBase.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

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
		ListRoot->GetUIItem()->SetIsUIActive(false);
		CanvasGroupOnListRoot = ListRoot->GetUIItem()->GetCanvasGroup();
		if (CanvasGroupOnListRoot != nullptr)
		{
			CanvasGroupOnListRoot->SetAlpha(0);
		}
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
	if (!IsValid(this->GetRootUIComponent()))return;
	if (!IsValid(this->GetRootUIComponent()->GetRootCanvas()))return;
	if (bIsShow)return;
	bIsShow = true;
	if (ShowOrHideTweener.IsValid())
	{
		ShowOrHideTweener->Kill();
	}

	//create blocker
	if (bUseInteractionBlock)
	{
		CreateBlocker();
	}
	//show list
	ListRoot->GetUIItem()->SetIsUIActive(true);
	if (CanvasGroupOnListRoot.IsValid())
	{
		ShowOrHideTweener = CanvasGroupOnListRoot->AlphaTo(1, 0.3f, 0, LTweenEase::OutCubic);
	}
	auto canvasOnListRoot = ListRoot->FindComponentByClass<ULGUICanvas>();
	if (!IsValid(canvasOnListRoot))
	{
		canvasOnListRoot = NewObject<ULGUICanvas>(ListRoot.Get());
		canvasOnListRoot->RegisterComponent();
	}

	bool sortOrderSet = false;
	if (BlockerActor.IsValid())
	{
		if (auto blockerCanvas = BlockerActor->FindComponentByClass<ULGUICanvas>())
		{
			canvasOnListRoot->SetSortOrder(blockerCanvas->GetSortOrder() + 1, true);
			sortOrderSet = true;
		}
	}
	if(!sortOrderSet)
	{
		canvasOnListRoot->SetSortOrderToHighestOfHierarchy(true);
	}
	canvasOnListRoot->SetOverrideSorting(true);

	//create list item as options
	if (!ItemTemplate.IsValidComponentReference())
	{
		UE_LOG(LGUI, Error, TEXT("[UUIDropdownComponent::Show]ItemTemplate is not valid!"));
		return;
	}
	if (bNeedRecreate)
	{
		bNeedRecreate = false;
		for (auto item : CreatedItemArray)
		{
			auto itemActor = item->GetOwner();
			ULGUIBPLibrary::DestroyActorWithHierarchy(itemActor, true);
		}
		CreatedItemArray.Reset();
		//create items
		CreateListItems();
	}

	auto ListRootUIItem = ListRoot->GetUIItem();
	//set position
	auto tempVerticalPosition = VerticalPosition;
	auto tempHorizontalPosition = HorizontalPosition;
	if (tempVerticalPosition == EUIDropdownVerticalPosition::Automatic
		|| tempHorizontalPosition == EUIDropdownHorizontalPosition::Automatic
		)
	{
		//search up til find clipped canvas, or root canvas
		ULGUICanvas* canvas = GetRootUIComponent()->GetRenderCanvas();
		while (true)
		{
			if (canvas->GetActualClipType() == ELGUICanvasClipType::Rect)
			{
				break;
			}
			else
			{
				auto upperCanvas = canvas->GetParentCanvas();
				if (!upperCanvas.IsValid())
				{
					break;
				}
				else
				{
					canvas = upperCanvas.Get();
				}
			}
		}

		auto canvasUIItem = canvas->GetUIItem();

		FTransform selfToCanvasTf;
		auto inverseCanvasTf = canvasUIItem->GetComponentTransform().Inverse();
		FTransform::Multiply(&selfToCanvasTf, &GetRootUIComponent()->GetComponentTransform(), &inverseCanvasTf);
		if (tempVerticalPosition == EUIDropdownVerticalPosition::Automatic)
		{
			//convert top point position from dropdown's self to root ui space, and tell if it is inside root rect
			FVector listBottomInCanvasSpace;
			if (VerticalOverlap)
			{
				auto selfTop = GetRootUIComponent()->GetLocalSpaceTop();
				auto listBottomInSelfSpace = selfTop - ListRootUIItem->GetHeight();
				listBottomInCanvasSpace = selfToCanvasTf.TransformPosition(FVector(0, 0, listBottomInSelfSpace));
			}
			else
			{
				auto selfBottom = GetRootUIComponent()->GetLocalSpaceBottom();
				auto listBottomInSelfSpace = selfBottom - ListRootUIItem->GetHeight();
				listBottomInCanvasSpace = selfToCanvasTf.TransformPosition(FVector(0, 0, listBottomInSelfSpace));
			}
			if (listBottomInCanvasSpace.Z < canvas->GetClipRectMin().Y)
			{
				tempVerticalPosition = EUIDropdownVerticalPosition::Top;
			}
			else
			{
				tempVerticalPosition = EUIDropdownVerticalPosition::Bottom;//default is bottom
			}
		}
		if (tempHorizontalPosition == EUIDropdownHorizontalPosition::Automatic)
		{
			auto selfRight = GetRootUIComponent()->GetLocalSpaceRight();
			auto listRightInCanvasSpace = selfToCanvasTf.TransformPosition(FVector(0, selfRight + ListRootUIItem->GetWidth(), 0));
			if (listRightInCanvasSpace.Y > canvas->GetClipRectMax().X)
			{
				tempHorizontalPosition = EUIDropdownHorizontalPosition::Left;
			}
			else
			{
				tempHorizontalPosition = EUIDropdownHorizontalPosition::Right;//default is right
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
			ListRootUIItem->SetVerticalAnchorMinMax(FVector2D(0.0f, 0.0f), true);
		}
		else
		{
			ListRootUIItem->SetVerticalAnchorMinMax(FVector2D(1.0f, 1.0f), true);
		}
	}break;
	case EUIDropdownVerticalPosition::Middle:
	{
		pivot.Y = 0.5f;
		ListRootUIItem->SetVerticalAnchorMinMax(FVector2D(0.5f, 0.5f), true);
	}break;
	case EUIDropdownVerticalPosition::Bottom:
	{
		pivot.Y = 1.0f;
		if (VerticalOverlap)
		{
			ListRootUIItem->SetVerticalAnchorMinMax(FVector2D(1.0f, 1.0f), true);
		}
		else
		{
			ListRootUIItem->SetVerticalAnchorMinMax(FVector2D(0.0f, 0.0f), true);
		}
	}break;
	}
	ListRootUIItem->SetVerticalAnchoredPosition(0);

	switch (tempHorizontalPosition)
	{
	case EUIDropdownHorizontalPosition::Left:
	{
		pivot.X = 1.0f;
		ListRootUIItem->SetHorizontalAnchorMinMax(FVector2D(0.0f, 0.0f), true);
	}break;
	case EUIDropdownHorizontalPosition::Center:
	{
		pivot.X = 0.5f;
		ListRootUIItem->SetHorizontalAnchorMinMax(FVector2D(0.5f, 0.5f), true);
	}break;
	case EUIDropdownHorizontalPosition::Right:
	{
		pivot.X = 0.0f;
		ListRootUIItem->SetHorizontalAnchorMinMax(FVector2D(1.0f, 1.0f), true);
	}break;
	}
	ListRootUIItem->SetHorizontalAnchoredPosition(0);

	ListRootUIItem->SetPivot(pivot);
}
void UUIDropdownComponent::Hide()
{
	if (!ListRoot.IsValid())
	{
		UE_LOG(LGUI, Error, TEXT("[UUIDropdownComponent::Show]ListRoot is not valid!"));
		return;
	}
	if (!bIsShow)return;
	bIsShow = false;
	if (ShowOrHideTweener.IsValid())
	{
		ShowOrHideTweener->Kill();
	}

	if (CanvasGroupOnListRoot.IsValid())
	{
		auto ListRootUIItem = ListRoot->GetUIItem();
		ShowOrHideTweener = CanvasGroupOnListRoot->AlphaTo(0, 0.3f, 0, LTweenEase::InCubic)->OnComplete(FSimpleDelegate::CreateWeakLambda(CanvasGroupOnListRoot.Get(), [ListRootUIItem] {
			ListRootUIItem->SetIsUIActive(false);
			}));
	}

	if (BlockerActor.IsValid())
	{
		BlockerActor->Destroy();
		BlockerActor = nullptr;
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
	blockerUIItem->SetTraceChannel(this->GetRootUIComponent()->GetTraceChannel());
	blockerUIItem->AttachToComponent(this->GetRootUIComponent()->GetRootCanvas()->GetUIItem(), FAttachmentTransformRules::KeepRelativeTransform);
	FUIAnchorData AnchorData;
	AnchorData.SizeDelta = FVector2D::ZeroVector;
	AnchorData.AnchorMin = FVector2D(0.0f, 0.0f);
	AnchorData.AnchorMax = FVector2D(1.0f, 1.0f);
	blockerUIItem->SetAnchorData(AnchorData);
	auto blockerCanvas = NewObject<ULGUICanvas>(blocker);
	blockerCanvas->RegisterComponent();
	blocker->AddInstanceComponent(blockerCanvas);
	blockerCanvas->SetOverrideSorting(true);
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
	templateUIItem->SetIsUIActive(true);
	auto contentUIItem = templateUIItem->GetParentUIItem();
	for (int i = 0, count = Options.Num(); i < count; i++)
	{
		auto copiedItemActor = ULGUIBPLibrary::DuplicateActor(ItemTemplate.GetActor(), contentUIItem);
#if WITH_EDITOR
		copiedItemActor->SetActorLabel(FString::Printf(TEXT("Item_%d"), i));
#endif
		auto script = copiedItemActor->FindComponentByClass<UUIDropdownItemComponent>();
		int index = i;
		script->Init(i, Options[i], [=]() {
			this->OnSelectItem(index);
			});
		script->SetSelectionState(i == Value);
		OnSetItemCustomDataFunction.ExecuteIfBound(i, script, copiedItemActor);
		CreatedItemArray.Add(script);
	}
	templateUIItem->SetIsUIActive(false);
	if (auto contentLayout = contentUIItem->GetOwner()->FindComponentByClass<UUILayoutBase>())
	{
		contentLayout->OnRebuildLayout();
	}
	float heightOffset = 0;
	if (auto viewportUIItem = contentUIItem->GetParentUIItem())
	{
		heightOffset = ListRoot->GetUIItem()->GetHeight() - viewportUIItem->GetHeight();
	}
	//if content is larger smaller than MaxHeight, then make the ListRoot smaller too
	if (contentUIItem->GetHeight() + heightOffset < MaxHeight)
	{
		ListRoot->GetUIItem()->SetHeight(contentUIItem->GetHeight() + heightOffset);
	}
	//if content is bigger than MaxHeight, then make the ListRoot as MaxHeight, so the scollview will work
	else if (contentUIItem->GetHeight() + heightOffset > MaxHeight)
	{
		ListRoot->GetUIItem()->SetHeight(MaxHeight + heightOffset);
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
		Value = newValue;
		if (fireEvent)
		{
			OnSelectionChangeCPP.Broadcast(Value);
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
	bNeedRecreate = true;
	Options = InOptions;
	ApplyValueToUI();
}
void UUIDropdownComponent::AddOptions(const TArray<FUIDropdownOptionData>& InOptions)
{
	bNeedRecreate = true;
	Options.SetNumUninitialized(Options.Num() + InOptions.Num());
	for (int i = 0; i < InOptions.Num(); i++)
	{
		Options.Add(InOptions[i]);
	}
	ApplyValueToUI();
}
void UUIDropdownComponent::SetUseInteractionBlock(bool InValue)
{
	if (bUseInteractionBlock != InValue)
	{
		bUseInteractionBlock = true;
		if (!bUseInteractionBlock)
		{
			if (BlockerActor.IsValid())
			{
				BlockerActor->Destroy();
				BlockerActor = nullptr;
			}
		}
	}
}

void UUIDropdownComponent::OnSelectItem(int index)
{
	SetValue(index, true);
	Hide();
}
void UUIDropdownComponent::ApplyValueToUI()
{
	if (!Options.IsValidIndex(Value))return;

	if (CaptionText.IsValid())
	{
		CaptionText->GetUIText()->SetText(Options[Value].Text);
	}
	if (CaptionSprite.IsValid() && IsValid(Options[Value].Sprite))
	{
		CaptionSprite->GetUISprite()->SetSprite(Options[Value].Sprite);
	}

	//apply to options
	for (int i = 0; i < Options.Num() && i < CreatedItemArray.Num(); i++)
	{
		auto script = CreatedItemArray[i];
		if (script.IsValid())
		{
			script->SetSelectionState(i == Value);
		}
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
		if (!eventData->selectedComponent->IsAttachedTo(this->GetRootUIComponent()))
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
		InDelegate.ExecuteIfBound(InSelection);
		});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUIDropdownComponent::UnregisterSelectionChangeEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnSelectionChangeCPP.Remove(InDelegateHandle.DelegateHandle);
}

void UUIDropdownComponent::SetItemCustomDataFunction(const FUIDropdownComponentDelegate_SetItemCustomData& InFunction)
{
	OnSetItemCustomDataFunction = InFunction;
}
void UUIDropdownComponent::SetItemCustomDataFunction(const TFunction<void(int, class UUIDropdownItemComponent*, AActor*)>& InFunction)
{
	OnSetItemCustomDataFunction.BindLambda(InFunction);
}
void UUIDropdownComponent::SetItemCustomDataFunction(const FUIDropdownComponentDynamicDelegate_SetItemCustomData& InFunction)
{
	OnSetItemCustomDataFunction.BindLambda([InFunction](int InItemIndex, UUIDropdownItemComponent* InItemScript, AActor* InItemActor) {
		if (InFunction.IsBound())
		{
			InFunction.Execute(InItemIndex, InItemScript, InItemActor);
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d OnSetItemCustomDataFunction function not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		}
		});
}
void UUIDropdownComponent::ClearItemCustomDataFunction()
{
	OnSetItemCustomDataFunction = FUIDropdownComponentDelegate_SetItemCustomData();
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

void UUIDropdownItemComponent::Init(int32 Index, const FUIDropdownOptionData& Data, const TFunction<void()>& OnSelect)
{
	if (TextActor.IsValid())
	{
		TextActor->GetUIText()->SetText(Data.Text);
	}
	if (SpriteActor.IsValid() && IsValid(Data.Sprite))
	{
		SpriteActor->GetUISprite()->SetSprite(Data.Sprite);
	}
	if (Toggle.IsValidComponentReference())
	{
		auto toggleComp = Toggle.GetComponent<UUIToggleComponent>();
		toggleComp->RegisterToggleEvent([OnSelect](bool select){
			OnSelect();
		});
	}
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		OnSelectDynamic.BindDynamic(this, &UUIDropdownItemComponent::DynamicDelegate_OnSelect);
		OnSelectCPP.BindLambda(OnSelect);
		ReceiveInit(Index, Data, OnSelectDynamic);
	}
}
void UUIDropdownItemComponent::SetSelectionState(const bool& InSelect)
{
	if (Toggle.IsValidComponentReference())
	{
		Toggle.GetComponent<UUIToggleComponent>()->SetValue(InSelect, false);
	}
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveSetSelectionState(InSelect);
	}
}
bool UUIDropdownItemComponent::OnPointerClick_Implementation(ULGUIPointerEventData* eventData)
{
	return false;
}
UUIToggleComponent* UUIDropdownItemComponent::GetToggle()const
{
	if (Toggle.IsValidComponentReference())
	{
		return Toggle.GetComponent<UUIToggleComponent>();
	}
	return nullptr;
}

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
