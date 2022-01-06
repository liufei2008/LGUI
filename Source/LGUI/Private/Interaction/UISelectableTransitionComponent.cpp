// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Interaction/UISelectableTransitionComponent.h"
#include "LTweenBPLibrary.h"


void UUISelectableTransitionComponent::Awake()
{
	Super::Awake();
	this->SetCanExecuteUpdate(false);
}
void UUISelectableTransitionComponent::StopTransition() 
{ 
	for (auto tweener : TweenerCollection)
	{
		ULTweenBPLibrary::KillIfIsTweening(this, tweener);
	}
	TweenerCollection.Reset();
}
void UUISelectableTransitionComponent::CollectTweener(ULTweener* InItem)
{
	TweenerCollection.Add(InItem);
}
void UUISelectableTransitionComponent::CollectTweeners(const TSet<ULTweener*>& InItems)
{
	TweenerCollection.Reserve(TweenerCollection.Num() + InItems.Num());
	for (auto item : InItems)
	{
		TweenerCollection.Add(item);
	}
}

void UUISelectableTransitionComponent::OnNormal(bool InImmediateSet)
{ 
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnNormal(InImmediateSet);
	}
}
void UUISelectableTransitionComponent::OnHighlighted(bool InImmediateSet)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnHighlighted(InImmediateSet);
	}
}
void UUISelectableTransitionComponent::OnPressed(bool InImmediateSet)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnPressed(InImmediateSet);
	}
}
void UUISelectableTransitionComponent::OnDisabled(bool InImmediateSet)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnDisabled(InImmediateSet);
	}
}
void UUISelectableTransitionComponent::OnStartCustomTransition(FName InTransitionName, bool InImmediateSet)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnStartCustomTransition(InTransitionName, InImmediateSet);
	}
}