// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/UIComponentBase.h"
#include "LGUI.h"

UUIComponentBase::UUIComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIComponentBase::BeginPlay()
{
	Super::BeginPlay();

}

void UUIComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
}
void UUIComponentBase::OnRegister()
{
	Super::OnRegister();
	if (CheckRootUIComponent())
	{
		RootUIComp->AddUIBaseComponent(this);
	}
}
void UUIComponentBase::OnUnregister()
{
	Super::OnUnregister();
	if (CheckRootUIComponent())
	{
		RootUIComp->RemoveUIBaseComponent(this);
	}
}
#if WITH_EDITOR
void UUIComponentBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RootUIComp = nullptr;
	CheckRootUIComponent();
}
#endif

bool UUIComponentBase::CheckRootUIComponent()
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