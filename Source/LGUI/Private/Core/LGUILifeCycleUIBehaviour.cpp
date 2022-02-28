// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/LGUILifeCycleUIBehaviour.h"
#include "LGUI.h"
#include "Core/Actor/LGUIManagerActor.h"

ULGUILifeCycleUIBehaviour::ULGUILifeCycleUIBehaviour()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
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
	CheckRootUIComponent();
}
#endif

bool ULGUILifeCycleUIBehaviour::CheckRootUIComponent() const
{
	if (RootUIComp.IsValid())return true;
	if (this->GetWorld() == nullptr)return false;
	if (auto Owner = GetOwner())
	{
		RootUIComp = Cast<UUIItem>(Owner->GetRootComponent());
		if(RootUIComp.IsValid())return true;
	}
	UE_LOG(LGUI, Error, TEXT("[ULGUILifeCycleUIBehaviour::CheckRootUIComponent]LGUILifeCycleUIBehaviour must attach to a UI actor!"));
	return false;
}

UUIItem* ULGUILifeCycleUIBehaviour::GetRootUIComponent() const
{
	if (RootUIComp.IsValid())
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
				Call_Awake();
			}
		}
	}
	SetActiveStateForEnableAndDisable(activeOrInactive);
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnUIActiveInHierarchy(activeOrInactive);
	}
}

void ULGUILifeCycleUIBehaviour::Call_Awake()
{
	Super::Call_Awake();
	for (auto CallbackFunc : CallbacksBeforeAwake)
	{
		CallbackFunc();
	}
	CallbacksBeforeAwake.Empty();
}

void ULGUILifeCycleUIBehaviour::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnUIDimensionsChanged(positionChanged, sizeChanged);
	}
}
void ULGUILifeCycleUIBehaviour::OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnUIChildDimensionsChanged(child, positionChanged, sizeChanged);
	}
}
void ULGUILifeCycleUIBehaviour::OnUIChildAcitveInHierarchy(UUIItem* child, bool ativeOrInactive)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnUIChildAcitveInHierarchy(child, ativeOrInactive);
	}
}
void ULGUILifeCycleUIBehaviour::OnUIAttachmentChanged()
{ 
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnUIAttachmentChanged();
	}
}
void ULGUILifeCycleUIBehaviour::OnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach) 
{ 
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnUIChildAttachmentChanged(child, attachOrDetach);
	}
}
void ULGUILifeCycleUIBehaviour::OnUIInteractionStateChanged(bool interactableOrNot)
{ 
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnUIInteractionStateChanged(interactableOrNot);
	}
}
void ULGUILifeCycleUIBehaviour::OnUIChildHierarchyIndexChanged(UUIItem* child)
{ 
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnUIChildHierarchyIndexChanged(child);
	}
}


void ULGUILifeCycleUIBehaviour::Call_OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		OnUIDimensionsChanged(positionChanged, sizeChanged);
	}
	else
#endif
	{
		if (bIsAwakeCalled)
		{
			OnUIDimensionsChanged(positionChanged, sizeChanged);
		}
		else
		{
			auto ThisPtr = MakeWeakObjectPtr(this);
			CallbacksBeforeAwake.Add([=]() {
				if (ThisPtr.IsValid())
				{
					ThisPtr->OnUIDimensionsChanged(positionChanged, sizeChanged);
				}});
		}
	}
}
void ULGUILifeCycleUIBehaviour::Call_OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged)
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		OnUIChildDimensionsChanged(child, positionChanged, sizeChanged);
	}
	else
#endif
	{
		if (bIsAwakeCalled)
		{
			OnUIChildDimensionsChanged(child, positionChanged, sizeChanged);
		}
		else
		{
			auto ThisPtr = MakeWeakObjectPtr(this);
			auto ChildPtr = MakeWeakObjectPtr(child);
			CallbacksBeforeAwake.Add([=]() {
				if (ThisPtr.IsValid() && ChildPtr.IsValid())
				{
					ThisPtr->OnUIChildDimensionsChanged(ChildPtr.Get(), positionChanged, sizeChanged);
				}});
		}
	}
}
void ULGUILifeCycleUIBehaviour::Call_OnUIChildAcitveInHierarchy(UUIItem* child, bool ativeOrInactive)
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		OnUIChildAcitveInHierarchy(child, ativeOrInactive);
	}
	else
#endif
	{
		if (bIsAwakeCalled)
		{
			OnUIChildAcitveInHierarchy(child, ativeOrInactive);
		}
		else
		{
			auto ThisPtr = MakeWeakObjectPtr(this);
			auto ChildPtr = MakeWeakObjectPtr(child);
			CallbacksBeforeAwake.Add([=]() {
				if (ThisPtr.IsValid() && ChildPtr.IsValid())
				{
					ThisPtr->OnUIChildAcitveInHierarchy(ChildPtr.Get(), ativeOrInactive);
				}});
		}
	}
}
void ULGUILifeCycleUIBehaviour::Call_OnUIAttachmentChanged()
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		OnUIAttachmentChanged();
	}
	else
#endif
	{
		if (bIsAwakeCalled)
		{
			OnUIAttachmentChanged();
		}
		else
		{
			auto ThisPtr = MakeWeakObjectPtr(this);
			CallbacksBeforeAwake.Add([=]() {
				if (ThisPtr.IsValid())
				{
					ThisPtr->OnUIAttachmentChanged();
				}});
		}
	}
}
void ULGUILifeCycleUIBehaviour::Call_OnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach)
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		OnUIChildAttachmentChanged(child, attachOrDetach);
	}
	else
#endif
	{
		if (bIsAwakeCalled)
		{
			OnUIChildAttachmentChanged(child, attachOrDetach);
		}
		else
		{
			auto ThisPtr = MakeWeakObjectPtr(this);
			auto ChildPtr = MakeWeakObjectPtr(child);
			CallbacksBeforeAwake.Add([=]() {
				if (ThisPtr.IsValid() && ChildPtr.IsValid())
				{
					ThisPtr->OnUIChildAttachmentChanged(ChildPtr.Get(), attachOrDetach);
				}});
		}
	}
}
void ULGUILifeCycleUIBehaviour::Call_OnUIInteractionStateChanged(bool interactableOrNot)
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		OnUIInteractionStateChanged(interactableOrNot);
	}
	else
#endif
	{
		if (bIsAwakeCalled)
		{
			OnUIInteractionStateChanged(interactableOrNot);
		}
		else
		{
			auto ThisPtr = MakeWeakObjectPtr(this);
			CallbacksBeforeAwake.Add([=]() {
				if (ThisPtr.IsValid())
				{
					ThisPtr->OnUIInteractionStateChanged(interactableOrNot);
				}});
		}
	}
}
void ULGUILifeCycleUIBehaviour::Call_OnUIChildHierarchyIndexChanged(UUIItem* child)
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		OnUIChildHierarchyIndexChanged(child);
	}
	else
#endif
	{
		if (bIsAwakeCalled)
		{
			OnUIChildHierarchyIndexChanged(child);
		}
		else
		{
			auto ThisPtr = MakeWeakObjectPtr(this);
			auto ChildPtr = MakeWeakObjectPtr(child);
			CallbacksBeforeAwake.Add([=]() {
				if (ThisPtr.IsValid() && ChildPtr.IsValid())
				{
					ThisPtr->OnUIChildHierarchyIndexChanged(ChildPtr.Get());
				}});
		}
	}
}
