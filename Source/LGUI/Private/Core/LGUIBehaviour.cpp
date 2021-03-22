// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/LGUIBehaviour.h"
#include "LGUI.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "PrefabSystem/ActorCopier.h"
#include "PrefabSystem/ActorSerializer.h"

ULGUIBehaviour::ULGUIBehaviour()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void ULGUIBehaviour::BeginPlay()
{
	Super::BeginPlay();
	ALGUIManagerActor::AddLGUIComponentForLifecycleEvent(this);
}
void ULGUIBehaviour::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
void ULGUIBehaviour::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetIsActiveAndEnable())
	{
		if (isEnableCalled)
		{
			OnDisable();
		}
	}
	if (isAwakeCalled)
	{
		OnDestroy();
	}
	Super::EndPlay(EndPlayReason);
}
void ULGUIBehaviour::OnRegister()
{
	Super::OnRegister();
	if (CheckRootUIComponent())
	{
		RootUIComp->AddUIBaseComponent(this);
	}
#if WITH_EDITOR
	if (this->GetWorld())
	{
		if (!this->GetWorld()->IsGameWorld())//edit mode
		{
			bool isInWorld = true;
			if (this->GetWorld()->GetPathName().StartsWith(TEXT("/Engine/Transient.")))
			{
				isInWorld = false;
			}
			if (isInWorld)
			{
				if (executeInEditMode)
				{
					if (auto Instance = ULGUIEditorManagerObject::GetInstance(this->GetWorld(), true))
					{
						EditorTickDelegateHandle = Instance->EditorTick.AddUObject(this, &ULGUIBehaviour::Update);
					}
				}
			}
		}
	}
#endif
}
void ULGUIBehaviour::OnUnregister()
{
	Super::OnUnregister();
	if (CheckRootUIComponent())
	{
		RootUIComp->RemoveUIBaseComponent(this);
	}
#if WITH_EDITOR
	if (this->GetWorld())
	{
		if (!this->GetWorld()->IsGameWorld())//edit mode
		{
			if (EditorTickDelegateHandle.IsValid())
			{
				if (ULGUIEditorManagerObject::Instance != nullptr)
				{
					ULGUIEditorManagerObject::Instance->EditorTick.Remove(EditorTickDelegateHandle);
				}
				EditorTickDelegateHandle.Reset();
			}
		}
	}
#endif
}
#if WITH_EDITOR
void ULGUIBehaviour::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RootUIComp = nullptr;
	CheckRootUIComponent();
	if (auto Property = PropertyChangedEvent.Property)
	{
		if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(ULGUIBehaviour, enable))
		{
			if (this->GetWorld()->IsGameWorld())//only execute in edit mode
			{
				if (enable)
				{
					OnEnable();
				}
				else
				{
					OnDisable();
				}
			}
		}
	}
}
#endif

bool ULGUIBehaviour::CheckRootUIComponent() const
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

bool ULGUIBehaviour::GetIsActiveAndEnable()const
{
	if (CheckRootUIComponent())
	{
		return RootUIComp->IsUIActiveInHierarchy() && enable;
	}
	else
	{
		return enable;
	}
}
bool ULGUIBehaviour::GetIsRootComponentActive()const
{
	if (CheckRootUIComponent())
	{
		return RootUIComp->IsUIActiveInHierarchy();
	}
	else
	{
		return true;
	}
}

void ULGUIBehaviour::SetEnable(bool value)
{
	if (enable != value)
	{
		enable = value;

		if (GetIsRootComponentActive())
		{
			if (enable)
			{
				OnEnable();
			}
			else
			{
				OnDisable();
			}
		}
	}
}

void ULGUIBehaviour::Awake()
{
	isAwakeCalled = true;
	AwakeBP();
}
void ULGUIBehaviour::Start()
{
	isStartCalled = true;
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
	isEnableCalled = true;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{

	}
	else//handle update in game mode
#endif
	{
		ALGUIManagerActor::AddLGUIBehavioursForUpdate(this);
	}
	OnEnableBP();
}
void ULGUIBehaviour::OnDisable()
{
	isEnableCalled = false;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{

	}
	else//handle update in game mode
#endif
	{
		ALGUIManagerActor::RemoveLGUIBehavioursFromUpdate(this);
	}
	OnDisableBP();
}

UUIItem* ULGUIBehaviour::GetRootComponent() const
{
	if (CheckRootUIComponent())
	{
		return RootUIComp;
	}
	return nullptr;
}

USceneComponent* ULGUIBehaviour::GetRootSceneComponent()const
{
	if (auto owner = GetOwner())
	{
		return owner->GetRootComponent();
	}
	return nullptr;
}

AActor* ULGUIBehaviour::InstantiateActor(AActor* OriginObject, USceneComponent* Parent)
{
	return LGUIPrefabSystem::ActorCopier::DuplicateActor(OriginObject, Parent);
}
AActor* ULGUIBehaviour::InstantiatePrefab(class ULGUIPrefab* OriginObject, USceneComponent* Parent)
{
	return LGUIPrefabSystem::ActorSerializer::LoadPrefab(this->GetWorld(), OriginObject, Parent, false);
}
AActor* ULGUIBehaviour::InstantiatePrefabWithTransform(class ULGUIPrefab* OriginObject, USceneComponent* Parent, FVector Location, FRotator Rotation, FVector Scale)
{
	return LGUIPrefabSystem::ActorSerializer::LoadPrefab(this->GetWorld(), OriginObject, Parent, Location, Rotation.Quaternion(), Scale);
}

void ULGUIBehaviour::OnUIActiveInHierachy(bool activeOrInactive) 
{ 
	if (activeOrInactive)
	{
		if (!isAwakeCalled)
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
		if (enable)
		{
#if WITH_EDITOR
			if (!this->GetWorld()->IsGameWorld())//edit mode
			{

			}
			else
#endif
			{
				OnEnable();
			}
		}
	}
	else
	{
		if (enable)
		{
			if (isEnableCalled)
			{
#if WITH_EDITOR
				if (!this->GetWorld()->IsGameWorld())//edit mode
				{

				}
				else
#endif
				{
					OnDisable();
				}
			}
		}
	}
	OnUIActiveInHierarchyBP(activeOrInactive);
}
