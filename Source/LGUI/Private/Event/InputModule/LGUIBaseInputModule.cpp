// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Event/InputModule/LGUIBaseInputModule.h"
#include "LGUI.h"
#include "Core/LGUIManager.h"
#include "Engine/World.h"

ULGUIBaseInputModule::ULGUIBaseInputModule()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = true;
}

void ULGUIBaseInputModule::ActivateInputModule()
{
	ULGUIManagerWorldSubsystem::SetCurrentInputModule(this);
}
void ULGUIBaseInputModule::DeactivateInputModule()
{
	ULGUIManagerWorldSubsystem::ClearCurrentInputModule(this);
}
void ULGUIBaseInputModule::Activate(bool bReset)
{
	Super::Activate(bReset);
	if (this->GetWorld() == nullptr)return;
#if WITH_EDITOR
	if (this->GetWorld()->IsGameWorld())
#endif
	{
		ActivateInputModule();
	}
}
void ULGUIBaseInputModule::Deactivate()
{
	Super::Deactivate();
	DeactivateInputModule();
}
