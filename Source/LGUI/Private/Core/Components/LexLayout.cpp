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

void ULexLayoutAnimation::OnApplyLayoutResults(const TArray<FLayoutAnimationSnapshotData>& SnapshotDataArray,
	TArray<TWeakObjectPtr<ULTweener>>& ResultTweenerArray)
{
	if (bCanExecuteBlueprintEvent)
	{
		TArray<ULTweener*> TempResultTweenerArray;
		ReceiveOnApplyLayoutResults(SnapshotDataArray, TempResultTweenerArray);
		for (auto& Tweener : TempResultTweenerArray)
		{
			ResultTweenerArray.Add(Tweener);
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

void ULexLayoutAnimation_CommonTween::OnApplyLayoutResults(const TArray<FLayoutAnimationSnapshotData>& SnapshotDataArray, TArray<TWeakObjectPtr<ULTweener>>& ResultTweenerArray)
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
		->SetEase(Ease);
		if (Ease == ELTweenEase::CurveFloat)
		{
			Tweener->SetRuntimeFloatCurve(EaseCurve);
		}
		ResultTweenerArray.Add(Tweener);
	}
}

void ULexLayoutAnimation_SlideIn::OnApplyLayoutResults(const TArray<FLayoutAnimationSnapshotData>& SnapshotDataArray,
	TArray<TWeakObjectPtr<ULTweener>>& ResultTweenerArray)
{
	auto LayoutWidget = GetLayoutContainer()->GetWidget();
	for (auto& SnapshotData : SnapshotDataArray)
	{
		if (SnapshotData.Widget == LayoutWidget)continue;
		auto NewPos = SnapshotData.Widget->GetAnchoredPosition();
		auto NewSize = SnapshotData.Widget->GetSizeDelta();
		auto NewOpacity = SnapshotData.Widget->GetRenderOpacity();
		auto OldPos = NewPos + PositionOffset;
		auto OldSize = NewSize + SizeOffset;
		auto OldOpacity = NewOpacity + OpacityOffset;
		SnapshotData.Widget->SetPositionAndSizeForLayoutAnimation(OldPos, OldSize);
		SnapshotData.Widget->SetRenderOpacity(OldOpacity);

		auto Tweener = ULTweenManager::To(this
		, FLTweenFloatGetterFunction::CreateLambda([=]()
		{
			return 0;
		}), FLTweenFloatSetterFunction::CreateLambda([=](float Value)
		{
			auto Pos = FMath::Lerp(OldPos, NewPos, Value);
			auto Size = FMath::Lerp(OldSize, NewSize, Value);
			SnapshotData.Widget->SetPositionAndSizeForLayoutAnimation(Pos, Size);
			SnapshotData.Widget->SetRenderOpacity(FMath::Lerp(OldOpacity, NewOpacity, Value));
		}), 1.0f, Duration)
		->SetEase(Ease);
		if (Ease == ELTweenEase::CurveFloat)
		{
			Tweener->SetRuntimeFloatCurve(EaseCurve);
		}
		ResultTweenerArray.Add(Tweener);
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
	if (LayoutAnimTweenerArray.Num() > 0)
	{
		ULTweenBPLibrary::ArrayKillIfIsTweening(this, LayoutAnimTweenerArray);
		LayoutAnimTweenerArray.Reset();
	}
	auto Widget = GetWidget();
	LayoutAnimSnapshotDataArray.Reset();
	for (auto& Child : Widget->GetChildren())
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
	AnimationHandler->OnApplyLayoutResults(LayoutAnimSnapshotDataArray, LayoutAnimTweenerArray);
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

FVector2f ULexLayoutSelf::GetLayoutFinalSize()
{
	auto Widget = GetWidget();
	return FVector2f(Widget->GetWidth(), Widget->GetHeight());
}
