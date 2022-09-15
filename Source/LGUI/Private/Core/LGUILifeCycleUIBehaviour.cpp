// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/LGUILifeCycleUIBehaviour.h"
#include "LGUI.h"
#include "Core/Actor/LGUIManagerActor.h"

ULGUILifeCycleUIBehaviour::ULGUILifeCycleUIBehaviour()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	CallbacksBeforeAwake.SetNumZeroed((int)ECallbackFunctionType::COUNT);
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
				Call_Awake();
			}
		}
	}
	SetActiveStateForEnableAndDisable(activeOrInactive);
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnUIActiveInHierarchy(activeOrInactive);
	}
}

void ULGUILifeCycleUIBehaviour::Call_Awake()
{
	for (auto& CallbackFunc : CallbacksBeforeAwake)
	{
		if (CallbackFunc != nullptr)
		{
			CallbackFunc();
		}
	}
	CallbacksBeforeAwake.Empty();
	Super::Call_Awake();
}

void ULGUILifeCycleUIBehaviour::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnUIDimensionsChanged(positionChanged, sizeChanged);
	}
}
void ULGUILifeCycleUIBehaviour::OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged)
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnUIChildDimensionsChanged(child, positionChanged, sizeChanged);
	}
}
void ULGUILifeCycleUIBehaviour::OnUIChildAcitveInHierarchy(UUIItem* child, bool ativeOrInactive)
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnUIChildAcitveInHierarchy(child, ativeOrInactive);
	}
}
void ULGUILifeCycleUIBehaviour::OnUIAttachmentChanged()
{ 
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnUIAttachmentChanged();
	}
}
void ULGUILifeCycleUIBehaviour::OnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach) 
{ 
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnUIChildAttachmentChanged(child, attachOrDetach);
	}
}
void ULGUILifeCycleUIBehaviour::OnUIInteractionStateChanged(bool interactableOrNot)
{ 
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnUIInteractionStateChanged(interactableOrNot);
	}
}
void ULGUILifeCycleUIBehaviour::OnUIChildHierarchyIndexChanged(UUIItem* child)
{ 
	if (bCanExecuteBlueprintEvent)
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
			CallbacksBeforeAwake[(int)ECallbackFunctionType::Call_OnUIDimensionsChanged] = [=]() {
				if (ThisPtr.IsValid())
				{
					ThisPtr->OnUIDimensionsChanged(positionChanged, sizeChanged);
				}};
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
			CallbacksBeforeAwake[(int)ECallbackFunctionType::Call_OnUIChildDimensionsChanged] = [=]() {
				if (ThisPtr.IsValid() && ChildPtr.IsValid())
				{
					ThisPtr->OnUIChildDimensionsChanged(ChildPtr.Get(), positionChanged, sizeChanged);
				}};
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
			CallbacksBeforeAwake[(int)ECallbackFunctionType::Call_OnUIChildAcitveInHierarchy] = [=]() {
				if (ThisPtr.IsValid() && ChildPtr.IsValid())
				{
					ThisPtr->OnUIChildAcitveInHierarchy(ChildPtr.Get(), ativeOrInactive);
				}};
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
			CallbacksBeforeAwake[(int)ECallbackFunctionType::Call_OnUIAttachmentChanged] = [=]() {
				if (ThisPtr.IsValid())
				{
					ThisPtr->OnUIAttachmentChanged();
				}};
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
			CallbacksBeforeAwake[(int)ECallbackFunctionType::Call_OnUIChildAttachmentChanged] = [=]() {
				if (ThisPtr.IsValid() && ChildPtr.IsValid())
				{
					ThisPtr->OnUIChildAttachmentChanged(ChildPtr.Get(), attachOrDetach);
				}};
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
			CallbacksBeforeAwake[(int)ECallbackFunctionType::Call_OnUIInteractionStateChanged] = [=]() {
				if (ThisPtr.IsValid())
				{
					ThisPtr->OnUIInteractionStateChanged(interactableOrNot);
				}};
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
			CallbacksBeforeAwake[(int)ECallbackFunctionType::Call_OnUIChildHierarchyIndexChanged] = [=]() {
				if (ThisPtr.IsValid() && ChildPtr.IsValid())
				{
					ThisPtr->OnUIChildHierarchyIndexChanged(ChildPtr.Get());
				}};
		}
	}
}
