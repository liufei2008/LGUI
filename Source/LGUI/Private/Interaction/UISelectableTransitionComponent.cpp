// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Interaction/UISelectableTransitionComponent.h"
#include "LTweenBPLibrary.h"


void UUISelectableTransitionComponent::StopTransition() 
{ 
	for (auto tweener : TweenerCollection)
	{
		ULTweenBPLibrary::KillIfIsTweening(tweener);
	}
	TweenerCollection.Reset();
}
void UUISelectableTransitionComponent::CollectTweener(ULTweener* InItem)
{
	TweenerCollection.Add(InItem);
}