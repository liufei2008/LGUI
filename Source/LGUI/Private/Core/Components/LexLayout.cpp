// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexLayout.h"

#include "LTweenBPLibrary.h"
#include "Core/Components/LexWidget.h"

#if WITH_EDITOR
void ULexLayout::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif


ULexLayoutAnimation::ULexLayoutAnimation()
{
	bCanExecuteBlueprintEvent = GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native);
}

void ULexLayoutAnimation::OnApplyLayoutResults(const TArray<FLayoutAnimationSnapshotData>& SnapshotDataArray, int32 TweenId)
{
	if (bCanExecuteBlueprintEvent)
	{
		auto LayoutContainer = GetLayoutContainer();
		for (int i = 0; i < SnapshotDataArray.Num(); i++)
		{
			auto& SnapshotData = SnapshotDataArray[i];
			if (SnapshotData.Widget == LayoutContainer->GetWidget())
			{
				ReceiveOnApplyLayoutResultForSelfWidget(SnapshotData.Widget, SnapshotData.Position, SnapshotData.Size, TweenId);
			}
			else
			{
				ReceiveOnApplyLayoutResultForChildWidget(i, SnapshotData.Widget, SnapshotData.Position, SnapshotData.Size, TweenId);
			}
		}
	}
}

ULexLayoutContainer* ULexLayoutAnimation::GetLayoutContainer()const
{
	if (!IsValid(OwnerLayoutContainer))
	{
		OwnerLayoutContainer = this->GetTypedOuter<ULexLayoutContainer>();
	}
	return OwnerLayoutContainer;
}

ULTweener* ULexLayoutAnimation::AnimPosition2D(ULexWidget* Widget, FVector2D StartPosition, FVector2D EndPosition, float Duration, float Delay, ELTweenEase Ease)
{
	Widget->SetPositionForLayoutAnimation(StartPosition);
	auto Tweener = ULTweenManager::To(Widget
		, FLTweenFloatGetterFunction::CreateLambda([=]()
		{
			return 0;
		}), FLTweenFloatSetterFunction::CreateLambda([=](float Value)
		{
			auto Pos = FMath::Lerp(StartPosition, EndPosition, Value);
			Widget->SetPositionForLayoutAnimation(Pos);
		}), 1.0f, Duration)
	->SetDelay(Delay)->SetEase(Ease);
	return Tweener;
}

ULTweener* ULexLayoutAnimation::AnimSize(ULexWidget* Widget, FVector2D StartSize, FVector2D EndSize, float Duration, float Delay, ELTweenEase Ease)
{
	Widget->SetSizeForLayoutAnimation(StartSize);
	auto Tweener = ULTweenManager::To(Widget
		, FLTweenFloatGetterFunction::CreateLambda([=]()
		{
			return 0;
		}), FLTweenFloatSetterFunction::CreateLambda([=](float Value)
		{
			auto Size = FMath::Lerp(StartSize, EndSize, Value);
			Widget->SetSizeForLayoutAnimation(Size);
		}), 1.0f, Duration)
	->SetDelay(Delay)->SetEase(Ease);
	return Tweener;
}

ULTweener* ULexLayoutAnimation::AnimScale(ULexWidget* Widget, FVector StartScale, FVector EndScale, float Duration, float Delay, ELTweenEase Ease)
{
	Widget->SetRelativeScale(StartScale);
	auto Tweener = ULTweenManager::To(Widget
		, FLTweenFloatGetterFunction::CreateLambda([=]()
		{
			return 0;
		}), FLTweenFloatSetterFunction::CreateLambda([=](float Value)
		{
			auto Scale = FMath::Lerp(StartScale, EndScale, Value);
			Widget->SetRelativeScale(Scale);
		}), 1.0f, Duration)
	->SetDelay(Delay)->SetEase(Ease);
	return Tweener;
}

ULTweener* ULexLayoutAnimation::AnimRotation(ULexWidget* Widget, FRotator StartRotation, FRotator EndRotation, float Duration, float Delay, ELTweenEase Ease)
{
	Widget->SetRelativeRotation(StartRotation.Quaternion());
	auto Tweener = ULTweenManager::To(Widget
		, FLTweenFloatGetterFunction::CreateLambda([=]()
		{
			return 0;
		}), FLTweenFloatSetterFunction::CreateLambda([=](float Value)
		{
			auto Rotator = FMath::Lerp(StartRotation, EndRotation, Value);
			Widget->SetRelativeRotation(Rotator.Quaternion());
		}), 1.0f, Duration)
	->SetDelay(Delay)->SetEase(Ease);
	return Tweener;
}

ULTweener* ULexLayoutAnimation::AnimRenderOpacity(ULexWidget* Widget, float StartOpacity, float EndOpacity, float Duration, float Delay, ELTweenEase Ease)
{
	Widget->SetRenderOpacity(StartOpacity);
	auto Tweener = ULTweenManager::To(Widget
		, FLTweenFloatGetterFunction::CreateLambda([=]()
		{
			return StartOpacity;
		}), FLTweenFloatSetterFunction::CreateLambda([=](float Value)
		{
			Widget->SetRenderOpacity(Value);
		}), EndOpacity, Duration)
	->SetDelay(Delay)->SetEase(Ease);
	return Tweener;
}

void ULexLayoutAnimation_CommonTween::OnApplyLayoutResults(const TArray<FLayoutAnimationSnapshotData>& SnapshotDataArray, int32 TweenId)
{
	for (auto& SnapshotData : SnapshotDataArray)
	{
		auto NewPos = SnapshotData.Widget->GetAnchoredPosition();
		auto NewSize = SnapshotData.Widget->GetSizeDelta();
		auto OldPos = SnapshotData.Position;
		auto OldSize = SnapshotData.Size;
		SnapshotData.Widget->SetPositionAndSizeForLayoutAnimation(OldPos, OldSize);

		auto Tweener = ULTweenManager::To(this
		, FLTweenFloatGetterFunction::CreateLambda([=]()
		{
			return 0;
		}), FLTweenFloatSetterFunction::CreateLambda([=](float Value)
		{
			auto Pos = FMath::Lerp(OldPos, NewPos, Value);
			auto Size = FMath::Lerp(OldSize, NewSize, Value);
			SnapshotData.Widget->SetPositionAndSizeForLayoutAnimation(Pos, Size);
		}), 1.0f, Duration)
		->SetEase(Ease)->SetId(TweenId);
		if (Ease == ELTweenEase::CurveFloat)
		{
			Tweener->SetRuntimeFloatCurve(EaseCurve);
		}
	}
}

void ULexLayoutContainer::PostReinitProperties()
{
	Super::PostReinitProperties();
#if WITH_EDITOR
	if (!this->GetName().StartsWith("Default__"))
	{
		if (auto Widget = GetWidget())
		{
			if (auto World = Widget->GetWorld())
			{
				if (!World->IsGameWorld())
				{
					ULexWidget::MarkLayoutForRebuild(Widget);
				}
			}
		}
	}
#endif
}

void ULexLayoutContainer::OnRegister()
{
	Super::OnRegister();
	if (auto Widget = GetWidget())
	{
		ULexWidget::MarkLayoutForRebuild(Widget);
	}
}

void ULexLayoutContainer::SnapshotLayout()
{
	RefreshChildren();
	if (!bUseAnimation || !AnimationHandler)return;//snapshot just for animation
	if (bIsInitialLayout)
	{
		bIsInitialLayout = false;
		if (bSkipAnimationForInitialLayout)
			return;//initial layout no need snapshot
	}
	ULTweenManager::KillAllTweensById(this, (int32)GetTypeHash(this));
	auto Widget = GetWidget();
	LayoutAnimSnapshotDataArray.Reset();
	for (auto& Child : Children)
	{
		FLayoutAnimationSnapshotData SnapshotData;
		SnapshotData.Position = Child->GetAnchoredPosition();
		SnapshotData.Size = Child->GetSize();
		SnapshotData.Widget = Child;
		LayoutAnimSnapshotDataArray.Add(SnapshotData);
	}
	if (auto Parent = Widget->GetParent())
	{
		if (!Parent->GetLayoutContainer() && Widget->GetLayoutSelf())// if parent is a layout-container and this is a layout-self, then we need to add self to snapshot data for later animation
		{
			FLayoutAnimationSnapshotData SnapshotData;
			SnapshotData.Position = Widget->GetAnchoredPosition();
			SnapshotData.Size = Widget->GetSize();
			SnapshotData.Widget = Widget;
			LayoutAnimSnapshotDataArray.Add(SnapshotData);
		}
	}
}
void ULexLayoutContainer::ApplyLayoutResult()
{
	if (!bUseAnimation || !AnimationHandler)return;
#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())//editor mode not use animation
	{
		LayoutAnimSnapshotDataArray.Reset();
		return;
	}
#endif
	AnimationHandler->OnApplyLayoutResults(LayoutAnimSnapshotDataArray, (int32)GetTypeHash(this));
	LayoutAnimSnapshotDataArray.Reset();
}

void ULexLayoutContainer::SetLayoutAnimation(ULexLayoutAnimation* Value)
{
	AnimationHandler = Value;
}

ULexLayoutAnimation* ULexLayoutContainer::CreateNewLayoutAnimation(TSubclassOf<ULexLayoutAnimation> Class)
{
	auto NewAnimationHandler = NewObject<ULexLayoutAnimation>(this, Class, NAME_None, RF_Public | RF_Transactional);
	AnimationHandler = NewAnimationHandler;
	return AnimationHandler;
}

void ULexLayout::MarkLayoutDirty()
{
	bIsLayoutDirty = true;
}

ULexLayoutContainer::ULexLayoutContainer()
{
}

void ULexLayoutContainer::RefreshChildren()
{
	auto Widget = GetWidget();
	Children.Empty();
	for (auto& ChildWidget : Widget->GetChildren())
	{
		if (!ChildWidget->GetWidgetActiveInHierarchy())continue;
		if (ChildWidget->GetIgnoreLayout())continue;
		Children.Add(ChildWidget);

		auto AnchorMin = ChildWidget->GetAnchorMin();
		auto AnchorMax = ChildWidget->GetAnchorMax();
		if (AnchorMin.X != AnchorMax.X)//custom anchor not support
		{
			ChildWidget->SetHorizontalAnchorMinMax(FVector2D(0.5, 0.5), true, true);
		}
		if (AnchorMin.Y != AnchorMax.Y)
		{
			ChildWidget->SetVerticalAnchorMinMax(FVector2D(0.5, 0.5), true, true);
		}
	}
}

#if WITH_EDITOR
void ULexLayoutContainer::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);
	if (!this->GetName().StartsWith("Default__"))
	{
		ULexWidget::MarkLayoutForRebuild(GetWidget());
	}
}

void ULexLayoutSelf::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);
	if (!this->GetName().StartsWith("Default__"))
	{
		ULexWidget::MarkLayoutForRebuild(GetWidget());
	}
}
#endif

void ULexLayoutSelf::PostReinitProperties()
{
	Super::PostReinitProperties();
#if WITH_EDITOR
	if (!this->GetName().StartsWith("Default__"))
	{
		if (auto Widget = GetWidget())
		{
			if (auto World = Widget->GetWorld())
			{
				if (!World->IsGameWorld())
				{
					ULexWidget::MarkLayoutForRebuild(Widget);
				}
			}
		}
	}
#endif
}
