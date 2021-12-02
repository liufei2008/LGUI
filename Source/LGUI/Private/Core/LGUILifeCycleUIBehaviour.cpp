// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/LGUILifeCycleUIBehaviour.h"
#include "LGUI.h"
#include "Core/Actor/LGUIManagerActor.h"

ULGUILifeCycleUIBehaviour::ULGUILifeCycleUIBehaviour()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void ULGUILifeCycleUIBehaviour::BeginPlay()
{
	UActorComponent::BeginPlay();//skip parent's OnComponentRenderStateDirty
	ALGUIManagerActor::AddLGUILifeCycleBehaviourForLifecycleEvent(this);
}

void ULGUILifeCycleUIBehaviour::OnRegister()
{
	Super::OnRegister();
	if (CheckRootUIComponent())
	{
		RootUIComp->AddLGUILifeCycleUIBehaviourComponent(this);
	}
}
void ULGUILifeCycleUIBehaviour::OnUnregister()
{
	Super::OnUnregister();
	if (RootUIComp.IsValid())
	{
		RootUIComp->RemoveLGUILifeCycleUIBehaviourComponent(this);
	}
}
#if WITH_EDITOR
void ULGUILifeCycleUIBehaviour::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RootUIComp = nullptr;
	CheckRootUIComponent();
}
#endif

bool ULGUILifeCycleUIBehaviour::CheckRootUIComponent() const
{
	if (this->GetWorld() == nullptr)return false;
	if (RootUIComp.IsValid())return true;
	if (auto Owner = GetOwner())
	{
		RootUIComp = Cast<UUIItem>(Owner->GetRootComponent());
		if(RootUIComp.IsValid())return true;
	}
	UE_LOG(LGUI, Error, TEXT("[ULGUILifeCycleUIBehaviour::CheckRootUIComponent]LGUILifeCycleUIBehaviour must attach to a UI actor!"));
	return false;
}

bool ULGUILifeCycleUIBehaviour::GetIsActiveAndEnable()const
{
	if (CheckRootUIComponent())
	{
		return RootUIComp->GetIsUIActiveInHierarchy() && enable;
	}
	else
	{
		return enable;
	}
}
bool ULGUILifeCycleUIBehaviour::IsAllowedToCallOnEnable()const
{
	return GetIsActiveAndEnable();
}

bool ULGUILifeCycleUIBehaviour::IsAllowedToCallAwake()const
{
	if (CheckRootUIComponent())
	{
		return RootUIComp->GetIsUIActiveInHierarchy();
	}
	else
	{
		return true;
	}
}

UUIItem* ULGUILifeCycleUIBehaviour::GetRootUIComponent() const
{
	if (CheckRootUIComponent())
	{
		return RootUIComp.Get();
	}
	return nullptr;
}

void ULGUILifeCycleUIBehaviour::OnUIActiveInHierachy(bool activeOrInactive) 
{ 
	if (activeOrInactive)
	{
		if (!bIsAwakeCalled)
		{
#if WITH_EDITOR
			if (!this->GetWorld()->IsGameWorld())//edit mode
			{

			}
			else
#endif
			{
				Awake();
			}
		}
	}
	SetActiveStateForEnableAndDisable(activeOrInactive);

	OnUIActiveInHierarchyBP(activeOrInactive);
}
