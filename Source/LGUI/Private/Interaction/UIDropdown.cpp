// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UIDropdown.h"
#include "LGUI.h"
#include "Core/Components/LexCanvas.h"
#include "LexUIBPLibrary.h"
#include "Core/LexUIClipData.h"
#include "Core/Components/LexImage.h"
#include "Core/Components/LexWidget.h"
#include "Core/Components/LexText.h"
#include "Core/Components/LexVisualEmpty.h"
#include "Interaction/UIButton.h"



UUIDropdown::UUIDropdown()
{
}

void UUIDropdown::Awake()
{
	Super::Awake();
	if (ListRoot.IsValid())
	{
		ListRoot->SetWidgetActive(false);
		ListRoot->SetRenderOpacity(0);
		MaxHeight = ListRoot->GetHeight();
	}
	//set default display
	if (Options.Num() > 0)
	{
		auto tempValue = FMath::Clamp(Value, 0, Options.Num() - 1);
		if (CaptionText.IsValid())
		{
			CaptionText->SetText(Options[tempValue].Text);
		}
		if (CaptionImage.IsValid())
		{
			CaptionImage->SetBrush(Options[tempValue].ImageBrush);
		}
	}
}
#if WITH_EDITOR
void UUIDropdown::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (Options.Num() > 0)
	{
		auto TempValue = FMath::Clamp(Value, 0, Options.Num() - 1);
		if (CaptionText.IsValid())
		{
			CaptionText->SetText(Options[TempValue].Text);
		}
		if (CaptionImage.IsValid())
		{
			CaptionImage->SetBrush(Options[TempValue].ImageBrush);
		}
	}
}
#endif

void UUIDropdown::Show()
{
	if (!ListRoot.IsValid())
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d ListRoot is not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}
	if (!IsValid(this->GetWidget()))return;
	if (!IsValid(this->GetWidget()->GetRootCanvas()))return;
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
	ListRoot->SetWidgetActive(true);
	ShowOrHideTweener = ListRoot->RenderOpacityTo(1, 0.3f, 0, ELTweenEase::OutCubic);
	auto CanvasOnListRoot = ListRoot->GetComponent<ULexCanvas>();
	if (!CanvasOnListRoot)
	{
		CanvasOnListRoot = ListRoot->AddComponent<ULexCanvas>();
	}

	bool bSortOrderSet = false;
	if (BlockerWidget.IsValid())
	{
		if (auto blockerCanvas = BlockerWidget->GetComponent<ULexCanvas>())
		{
			CanvasOnListRoot->SetSortOrder(blockerCanvas->GetSortOrder() + 1, true);
			bSortOrderSet = true;
		}
	}
	if(!bSortOrderSet)
	{
		CanvasOnListRoot->SetSortOrderToHighestOfHierarchy(true);
	}
	CanvasOnListRoot->SetOverrideSorting(true);

	//create list item as options
	if (!ItemTemplate.IsValid())
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d ItemTemplate is not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}
	if (bNeedRecreate)
	{
		bNeedRecreate = false;
		for (auto item : CreatedItemArray)
		{
			auto ItemWidget = item->GetWidget();
			ItemWidget->DestroyWidget();
		}
		CreatedItemArray.Reset();
		//create items
		CreateListItems();
	}

	//set position
	auto TempVerticalPosition = VerticalPosition;
	auto TempHorizontalPosition = HorizontalPosition;
	if (TempVerticalPosition == EUIDropdownVerticalPosition::Automatic
		|| TempHorizontalPosition == EUIDropdownHorizontalPosition::Automatic
		)
	{
		auto ThisWidget = GetWidget();
		if (ThisWidget->GetClipData().IsValid())//have valid ClipData, then use ClipData to tell if the Dropdown list visible
		{
			auto ClipData = ThisWidget->GetClipData().Pin();
			if (TempVerticalPosition == EUIDropdownVerticalPosition::Automatic)
			{
				FVector ListBottomWorldSpace;
				if (VerticalOverlap)
				{
					auto SelfTop = ThisWidget->GetLocalSpaceTop();
					auto ListBottomInSelfSpace = SelfTop - ListRoot->GetHeight();
					ListBottomWorldSpace = ThisWidget->GetWorldTransform().TransformPosition(FVector(0, 0, ListBottomInSelfSpace));
				}
				else
				{
					auto SelfBottom = ThisWidget->GetLocalSpaceBottom();
					auto ListBottomInSelfSpace = SelfBottom - ListRoot->GetHeight();
					ListBottomWorldSpace = ThisWidget->GetWorldTransform().TransformPosition(FVector(0, 0, ListBottomInSelfSpace));
				}
				if (!ClipData->IsPointVisible(ListBottomWorldSpace))
				{
					TempVerticalPosition = EUIDropdownVerticalPosition::Top;
				}
				else
				{
					TempVerticalPosition = EUIDropdownVerticalPosition::Bottom;//default is bottom
				}
			}
			if (TempHorizontalPosition == EUIDropdownHorizontalPosition::Automatic)
			{
				auto SelfRight = ThisWidget->GetLocalSpaceRight();
				auto ListRightWorldSpace = ThisWidget->GetWorldTransform().TransformPosition(FVector(0, SelfRight + ListRoot->GetWidth(), 0));
				if (!ClipData->IsPointVisible(ListRightWorldSpace))
				{
					TempHorizontalPosition = EUIDropdownHorizontalPosition::Left;
				}
				else
				{
					TempHorizontalPosition = EUIDropdownHorizontalPosition::Right;//default is right
				}
			}
		}
		else//no valid ClipData, then use RootCanvas
		{
			auto RootCanvasWidget = ThisWidget->GetRootCanvas()->GetWidget();
			FTransform SelfToCanvasSpaceTf;
			auto InverseCanvasSpaceTf = RootCanvasWidget->GetWorldTransform().Inverse();
			FTransform::Multiply(&SelfToCanvasSpaceTf, &ThisWidget->GetWorldTransform(), &InverseCanvasSpaceTf);
			if (TempVerticalPosition == EUIDropdownVerticalPosition::Automatic)
			{
				//convert top point position from drop-down's self to root ui space, and tell if it is inside root rect
				FVector ListBottomInClipSpace;
				if (VerticalOverlap)
				{
					auto SelfTop = ThisWidget->GetLocalSpaceTop();
					auto ListBottomInSelfSpace = SelfTop - ListRoot->GetHeight();
					ListBottomInClipSpace = SelfToCanvasSpaceTf.TransformPosition(FVector(0, 0, ListBottomInSelfSpace));
				}
				else
				{
					auto SelfBottom = ThisWidget->GetLocalSpaceBottom();
					auto ListBottomInSelfSpace = SelfBottom - ListRoot->GetHeight();
					ListBottomInClipSpace = SelfToCanvasSpaceTf.TransformPosition(FVector(0, 0, ListBottomInSelfSpace));
				}
				if (ListBottomInClipSpace.Z < RootCanvasWidget->GetLocalSpaceBottom())
				{
					TempVerticalPosition = EUIDropdownVerticalPosition::Top;
				}
				else
				{
					TempVerticalPosition = EUIDropdownVerticalPosition::Bottom;//default is bottom
				}
			}
			if (TempHorizontalPosition == EUIDropdownHorizontalPosition::Automatic)
			{
				auto SelfRight = ThisWidget->GetLocalSpaceRight();
				auto ListRightInCanvasSpace = SelfToCanvasSpaceTf.TransformPosition(FVector(0, SelfRight + ListRoot->GetWidth(), 0));
				if (ListRightInCanvasSpace.Y > RootCanvasWidget->GetLocalSpaceRight())
				{
					TempHorizontalPosition = EUIDropdownHorizontalPosition::Left;
				}
				else
				{
					TempHorizontalPosition = EUIDropdownHorizontalPosition::Right;//default is right
				}
			}
		}
	}

	FVector2D Pivot(0.5f, 0);
	switch (TempVerticalPosition)
	{
	case EUIDropdownVerticalPosition::Top:
	{
		Pivot.Y = 0.0f;
		if (VerticalOverlap)
		{
			ListRoot->SetVerticalAnchorMinMax(FVector2D(0.0f, 0.0f), true);
		}
		else
		{
			ListRoot->SetVerticalAnchorMinMax(FVector2D(1.0f, 1.0f), true);
		}
	}break;
	case EUIDropdownVerticalPosition::Middle:
	{
		Pivot.Y = 0.5f;
		ListRoot->SetVerticalAnchorMinMax(FVector2D(0.5f, 0.5f), true);
	}break;
	case EUIDropdownVerticalPosition::Bottom:
	{
		Pivot.Y = 1.0f;
		if (VerticalOverlap)
		{
			ListRoot->SetVerticalAnchorMinMax(FVector2D(1.0f, 1.0f), true);
		}
		else
		{
			ListRoot->SetVerticalAnchorMinMax(FVector2D(0.0f, 0.0f), true);
		}
	}break;
	}
	ListRoot->SetVerticalAnchoredPosition(0);

	switch (TempHorizontalPosition)
	{
	case EUIDropdownHorizontalPosition::Left:
	{
		Pivot.X = 1.0f;
		ListRoot->SetHorizontalAnchorMinMax(FVector2D(0.0f, 0.0f), true);
	}break;
	case EUIDropdownHorizontalPosition::Center:
	{
		Pivot.X = 0.5f;
		ListRoot->SetHorizontalAnchorMinMax(FVector2D(0.5f, 0.5f), true);
	}break;
	case EUIDropdownHorizontalPosition::Right:
	{
		Pivot.X = 0.0f;
		ListRoot->SetHorizontalAnchorMinMax(FVector2D(1.0f, 1.0f), true);
	}break;
	}
	ListRoot->SetHorizontalAnchoredPosition(0);

	ListRoot->SetPivot(Pivot);
}
void UUIDropdown::Hide()
{
	if (!ListRoot.IsValid())
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d ListRoot is not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}
	if (!bIsShow)return;
	bIsShow = false;
	if (ShowOrHideTweener.IsValid())
	{
		ShowOrHideTweener->Kill();
	}
	
	ShowOrHideTweener = ListRoot->RenderOpacityTo(0, 0.3f, 0, ELTweenEase::InCubic)->OnComplete(
		FSimpleDelegate::CreateWeakLambda(ListRoot.Get(), [=, this] {
		ListRoot->SetWidgetActive(false);
		}));

	if (BlockerWidget.IsValid())
	{
		BlockerWidget->DestroyWidget();
		BlockerWidget = nullptr;
	}
}
void UUIDropdown::CreateBlocker()
{
	BlockerWidget = NewObject<ULexWidget>(this->GetWidget()->GetOuter());
	BlockerWidget->SetDisplayName(TEXT("UIDropdown_Blocker"));
	BlockerWidget->SetParent(this->GetWidget()->GetRootCanvas()->GetWidget(), false);
	BlockerWidget->SetSizeDelta(FVector2D::ZeroVector);
	BlockerWidget->SetAnchorMin(FVector2D(0.0f, 0.0f));
	BlockerWidget->SetAnchorMax(FVector2D(1.0f, 1.0f));
	BlockerWidget->CreateNewVisual<ULexVisualEmpty>();//Need visual to do raycast
	auto BlockerCanvas = BlockerWidget->AddComponent<ULexCanvas>();
	BlockerCanvas->SetOverrideSorting(true);
	BlockerCanvas->SetSortOrderToHighestOfHierarchy();
	BlockerCanvas->SetTraceChannel(this->GetWidget()->GetRootCanvas()->GetTraceChannel());
	auto BlockerButton = BlockerWidget->AddComponent<UUIButton>();
	BlockerButton->GetOnClickEvent().AddWeakLambda(this, [this] {
		this->Hide();
		});
}
void UUIDropdown::CreateListItems()
{
	auto ItemTemplateWidget = ItemTemplate->GetWidget();
	if (!IsValid(ItemTemplateWidget))
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d ItemTemplate must be a LexWidget!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}
	ItemTemplateWidget->SetWidgetActive(true);
	auto ScrollViewContentWidget = ItemTemplateWidget->GetParent();
	for (int i = 0, count = Options.Num(); i < count; i++)
	{
		auto CopiedItemWidget = ULexUIBPLibrary::DuplicateWidget(this->GetOuter()->GetWorld(), ItemTemplateWidget, ScrollViewContentWidget);
		CopiedItemWidget->SetDisplayName(FString::Printf(TEXT("Item_%d"), i));
		auto script = CopiedItemWidget->GetComponent<UUIDropdownItemComponent>();
		int index = i;
		script->Init(i, Options[i], [=, this]() {
			this->OnSelectItem(index);
			});
		script->SetSelectionState(i == Value);
		OnSetItemCustomDataFunction.ExecuteIfBound(i, script, CopiedItemWidget);
		CreatedItemArray.Add(script);
	}
	ItemTemplateWidget->SetWidgetActive(false);

	ULexWidget::RebuildLayoutImmediately(ScrollViewContentWidget);
	float HeightOffset = 0;
	if (auto ViewportWidget = ScrollViewContentWidget->GetParent())
	{
		HeightOffset = ListRoot->GetHeight() - ViewportWidget->GetHeight();
	}
	//if content is larger smaller than MaxHeight, then make the ListRoot smaller too
	if (ScrollViewContentWidget->GetHeight() + HeightOffset < MaxHeight)
	{
		ListRoot->SetHeight(ScrollViewContentWidget->GetHeight() + HeightOffset);
	}
	//if content is bigger than MaxHeight, then make the ListRoot as MaxHeight, so the scroll-view will work
	else if (ScrollViewContentWidget->GetHeight() + HeightOffset > MaxHeight)
	{
		ListRoot->SetHeight(MaxHeight + HeightOffset);
	}
}
FUIDropdownOptionData UUIDropdown::GetOption(int index)const
{
	if (index >= Options.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d index: %d out of range: %d!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, index, Options.Num());
		return FUIDropdownOptionData();
	}
	return Options[index];
}
FUIDropdownOptionData UUIDropdown::GetCurrentOption()const
{
	if (Value >= Options.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[%s]Value: %d out of range: %d!"), ANSI_TO_TCHAR(__FUNCTION__), Value, Options.Num());
		return FUIDropdownOptionData();
	}
	return Options[Value];
}
void UUIDropdown::SetValue(int InValue, bool FireEvent)
{
	if (Value != InValue)
	{
		Value = InValue;
		if (FireEvent)
		{
			OnValueChangedCPP.Broadcast(Value);
			OnValueChangedBP.Broadcast(Value);
			OnValueChanged.FireEvent(Value);
		}
		ApplyValueToVisual();
	}
}

void UUIDropdown::SetValue(int InValue)
{
	SetValue(InValue, true);
}

void UUIDropdown::SetValueWithoutNotify(int InValue)
{
	SetValue(InValue, false);
}

void UUIDropdown::SetVerticalPosition(EUIDropdownVerticalPosition InValue)
{
	if (VerticalPosition != InValue)
	{
		VerticalPosition = InValue;
	}
}
void UUIDropdown::SetHorizontalPosition(EUIDropdownHorizontalPosition InValue)
{
	if (HorizontalPosition != InValue)
	{
		HorizontalPosition = InValue;
	}
}
void UUIDropdown::SetVerticalOverlap(bool newValue)
{
	if (VerticalOverlap != newValue)
	{
		VerticalOverlap = newValue;
	}
}
void UUIDropdown::SetOptions(const TArray<FUIDropdownOptionData>& InOptions)
{
	bNeedRecreate = true;
	Options = InOptions;
	ApplyValueToVisual();
}
void UUIDropdown::AddOptions(const TArray<FUIDropdownOptionData>& InOptions)
{
	bNeedRecreate = true;
	Options.SetNumUninitialized(Options.Num() + InOptions.Num());
	for (int i = 0; i < InOptions.Num(); i++)
	{
		Options.Add(InOptions[i]);
	}
	ApplyValueToVisual();
}
void UUIDropdown::SetUseInteractionBlock(bool InValue)
{
	if (bUseInteractionBlock != InValue)
	{
		bUseInteractionBlock = true;
		if (!bUseInteractionBlock)
		{
			if (BlockerWidget.IsValid())
			{
				BlockerWidget->DestroyWidget();
				BlockerWidget = nullptr;
			}
		}
	}
}

void UUIDropdown::OnSelectItem(int Index)
{
	SetValue(Index, true);
	Hide();
}
void UUIDropdown::ApplyValueToVisual()
{
	if (!Options.IsValidIndex(Value))return;

	if (CaptionText.IsValid())
	{
		CaptionText->SetText(Options[Value].Text);
	}
	if (CaptionImage.IsValid())
	{
		CaptionImage->SetBrush(Options[Value].ImageBrush);
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
bool UUIDropdown::OnPointerClick_Implementation(ULexPointerEventData* EventData)
{
	Show();
	return AllowEventBubbleUp;
}
bool UUIDropdown::OnPointerDeselect_Implementation(ULexBaseEventData* EventData)
{
	if (IsValid(EventData->SelectedComponent))
	{
		if (!EventData->SelectedComponent->IsChildOf(this->GetWidget()))
		{
			Hide();
		}
	}
	return AllowEventBubbleUp;
}

void UUIDropdown::SetItemCustomDataFunction(const FUIDropdownComponentDelegate_SetItemCustomData& InFunction)
{
	OnSetItemCustomDataFunction = InFunction;
}
void UUIDropdown::SetItemCustomDataFunction(const TFunction<void(int, class UUIDropdownItemComponent*, ULexWidget*)>& InFunction)
{
	OnSetItemCustomDataFunction.BindLambda(InFunction);
}
void UUIDropdown::SetItemCustomDataFunction(const FUIDropdownComponentDynamicDelegate_SetItemCustomData& InFunction)
{
	OnSetItemCustomDataFunction.BindLambda([InFunction](int InItemIndex, UUIDropdownItemComponent* InItemScript, ULexWidget* InItemWidget) {
		if (InFunction.IsBound())
		{
			InFunction.Execute(InItemIndex, InItemScript, InItemWidget);
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d OnSetItemCustomDataFunction function not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		}
		});
}
void UUIDropdown::ClearItemCustomDataFunction()
{
	OnSetItemCustomDataFunction = FUIDropdownComponentDelegate_SetItemCustomData();
}


#include "Interaction/UIToggle.h"

UUIDropdownItemComponent::UUIDropdownItemComponent()
{
}

void UUIDropdownItemComponent::Awake()
{
	Super::Awake();
	this->SetCanExecuteTick(false);
}

void UUIDropdownItemComponent::Init(int32 Index, const FUIDropdownOptionData& Data, const TFunction<void()>& OnSelect)
{
	if (Text.IsValid())
	{
		Text->SetText(Data.Text);
	}
	if (Image.IsValid())
	{
		Image->SetBrush(Data.ImageBrush);
	}
	if (Toggle.IsValid())
	{
		Toggle->GetOnValueChangedEvent().AddWeakLambda(this, [OnSelect](bool select){
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
	if (Toggle.IsValid())
	{
		Toggle->SetValueWithoutNotify(InSelect);
	}
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveSetSelectionState(InSelect);
	}
}
bool UUIDropdownItemComponent::OnPointerClick_Implementation(ULexPointerEventData* EventData)
{
	return false;
}
UUIToggle* UUIDropdownItemComponent::GetToggle()const
{
	return Toggle.Get();
}


