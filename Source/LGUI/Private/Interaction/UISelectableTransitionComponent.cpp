// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Interaction/UISelectableTransitionComponent.h"
#include "LTweenBPLibrary.h"


UUISelectableTransitionComponent::UUISelectableTransitionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UUISelectableTransitionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UUISelectableTransitionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UUISelectableTransitionComponent::StopTransition() 
{ 
	for (auto tweener : TweenerCollection)
	{
		ULTweenBPLibrary::KillIfIsTweening(tweener);
	}
	TweenerCollection.Empty();
}