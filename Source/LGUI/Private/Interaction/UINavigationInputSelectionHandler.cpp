// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UINavigationInputSelectionHandler.h"
#include "LTweenBPLibrary.h"
#include "Core/Components/LexCanvas.h"
#include "Core/Components/LexWidget.h"

UUINavigationInputSelectionHandler::UUINavigationInputSelectionHandler()
{
}

void UUINavigationInputSelectionHandler::SelectWidget(ULexWidget* InSelected)
{
	if (bIsDestroyPending)
	{
		return;
	}
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveSelectWidget(InSelected);
		return;
	}
	auto Widget = GetWidget();
	if (!Widget)return;

	for (auto& Tweener : TweenerCollection)
	{
		ULTweenBPLibrary::KillIfIsTweening(this, Tweener.Get());
	}
	TweenerCollection.Reset();
	
	auto PrevSelected = CurrentSelected;
	CurrentSelected = InSelected;
	if (InSelected != nullptr && PrevSelected.IsValid())
	{
		Widget->SetParent(InSelected, true);
		auto Pos2D = InSelected->GetLocalSpaceCenter();
		auto Pos3D = FVector(0, Pos2D.X, Pos2D.Y);
		auto Tweener = Widget->LocalPositionTo(Pos3D, AnimDuration, 0, ELTweenEase::InOutSine);
		TweenerCollection.Add(Tweener);
		Tweener = Widget->SizeDeltaTo(InSelected->GetSize(), AnimDuration, 0, ELTweenEase::InOutSine);
		TweenerCollection.Add(Tweener);
		Tweener = Widget->LocalRotationQuaternionTo(FQuat::Identity, AnimDuration, 0, ELTweenEase::InOutSine);
		TweenerCollection.Add(Tweener);

		if (ThisCanvas.IsValid())
		{
			ThisCanvas->SetSortOrderToHighestOfHierarchy(false);
		}
	}
	else if (InSelected != nullptr)
	{
		auto Tweener = Widget->RenderOpacityTo(1.0f, AnimDuration, 0, ELTweenEase::Linear);
		TweenerCollection.Add(Tweener);
		Widget->SetParent(InSelected, true);
		auto Pos2D = InSelected->GetLocalSpaceCenter();
		auto Pos3D = FVector(0, Pos2D.X, Pos2D.Y);
		Widget->SetRelativeLocation(Pos3D);
		Widget->SetSizeDelta(InSelected->GetSize());
		Widget->SetRelativeRotation(FQuat::Identity);

		if (ThisCanvas.IsValid())
		{
			ThisCanvas->SetSortOrderToHighestOfHierarchy(false);
		}
	}
	else if (PrevSelected.IsValid())
	{
		auto Tweener = Widget->RenderOpacityTo(0.0f, AnimDuration, 0, ELTweenEase::Linear);
		TweenerCollection.Add(Tweener);
	}
}

void UUINavigationInputSelectionHandler::SelectNone()
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveSelectNone();
		return;
	}
	auto Widget = GetWidget();
	if (!Widget)return;
	if (!CurrentSelected.IsValid())return;

	// Mark as pending destroy to prevent any further SelectWidget() calls
	bIsDestroyPending = true;

	for (auto& Tweener : TweenerCollection)
	{
		ULTweenBPLibrary::KillIfIsTweening(this, Tweener.Get());
	}
	TweenerCollection.Reset();
	
	auto Tweener = Widget->RenderOpacityTo(0.0f, AnimDuration, 0, ELTweenEase::Linear)
	->OnComplete([=, this]()
	{
		if (auto W = this->GetWidget())
		{
			W->DestroyWidget();
		}
	});
	TweenerCollection.Add(Tweener);
	CurrentSelected = nullptr;
}
