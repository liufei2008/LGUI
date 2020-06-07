// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/LGUIBehaviour.h"
#include "LGUI.h"
#include "Core/Actor/LGUIManagerActor.h"

ULGUIBehaviour::ULGUIBehaviour()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULGUIBehaviour::BeginPlay()
{
	Super::BeginPlay();
	ALGUIManagerActor::AddLGUIComponent(this);
}
void ULGUIBehaviour::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
void ULGUIBehaviour::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ALGUIManagerActor::RemoveLGUIComponent(this);

	if (!isOnDisableCalled)
	{
		OnDisable();
	}
	OnDestroy();
}
void ULGUIBehaviour::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}
void ULGUIBehaviour::OnRegister()
{
	Super::OnRegister();
	if (CheckRootUIComponent())
	{
		RootUIComp->AddUIBaseComponent(this);
	}
}
void ULGUIBehaviour::OnUnregister()
{
	Super::OnUnregister();
	if (CheckRootUIComponent())
	{
		RootUIComp->RemoveUIBaseComponent(this);
	}
}
#if WITH_EDITOR
void ULGUIBehaviour::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RootUIComp = nullptr;
	CheckRootUIComponent();
}
#endif

bool ULGUIBehaviour::CheckRootUIComponent()
{
	if (this->GetWorld() == nullptr)return false;
	if (RootUIComp != nullptr)return true;
	if (auto owner = GetOwner())
	{
		RootUIComp = Cast<UUIItem>(owner->GetRootComponent());
		if(RootUIComp != nullptr)return true;
	}
	return false;
}

bool ULGUIBehaviour::GetIsActiveAndEnable()
{
	return
		CheckRootUIComponent()
		&& RootUIComp->IsUIActiveInHierarchy()
		&& enable
		&& bRegistered
		;
}

void ULGUIBehaviour::SetEnable(bool value)
{
	if (enable != value)
	{
		enable = value;
	}
}

void ULGUIBehaviour::Awake()
{
	AwakeBP();
}
void ULGUIBehaviour::Start()
{
	StartBP();
}
void ULGUIBehaviour::Update(float DeltaTime)
{
	UpdateBP(DeltaTime);
}
void ULGUIBehaviour::OnDestroy()
{
	OnDestroyBP();
}
void ULGUIBehaviour::OnEnable()
{
	isOnDisableCalled = false;
	OnEnableBP();
}
void ULGUIBehaviour::OnDisable()
{
	isOnDisableCalled = true;
	OnDisableBP();
}

USceneComponent* ULGUIBehaviour::GetTransform()
{
	return GetOwner()->GetRootComponent();
}