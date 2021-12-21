// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/LGUILifeCycleBehaviour.h"
#include "LGUI.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "PrefabSystem/ActorSerializer3.h"
#include "Components/SceneComponent.h"

ULGUILifeCycleBehaviour::ULGUILifeCycleBehaviour()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	bIsAwakeCalled = false;
	bIsStartCalled = false;
	bIsEnableCalled = false;
	bCanExecuteUpdate = true;
	bIsAddedToUpdate = false;
	bPrevIsRootComponentVisible = false;
}

void ULGUILifeCycleBehaviour::BeginPlay()
{
	Super::BeginPlay();
	ALGUIManagerActor::AddLGUILifeCycleBehaviourForLifecycleEvent(this);
	if (GetRootSceneComponent())
	{
		bPrevIsRootComponentVisible = RootComp->GetVisibleFlag();
	}
	ComponentRenderStateDirtyDelegateHandle = UActorComponent::MarkRenderStateDirtyEvent.AddUObject(this, &ULGUILifeCycleBehaviour::OnComponentRenderStateDirty);
}
void ULGUILifeCycleBehaviour::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
void ULGUILifeCycleBehaviour::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bIsEnableCalled)
	{
		OnDisable();
	}
	if (bIsAwakeCalled)
	{
		OnDestroy();
	}
	UActorComponent::MarkRenderStateDirtyEvent.Remove(ComponentRenderStateDirtyDelegateHandle);
	Super::EndPlay(EndPlayReason);
}
void ULGUILifeCycleBehaviour::OnRegister()
{
	Super::OnRegister();
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
						EditorTickDelegateHandle = Instance->EditorTick.AddUObject(this, &ULGUILifeCycleBehaviour::Update);
					}
				}
			}
		}
	}
#endif
}
void ULGUILifeCycleBehaviour::OnUnregister()
{
	Super::OnUnregister();
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

DECLARE_CYCLE_STAT(TEXT("LGUILifeCycleBehaviour Callback_OnComponentRenderStateDirty"), STAT_OnComponentRenderStateDirty, STATGROUP_LGUI);
void ULGUILifeCycleBehaviour::OnComponentRenderStateDirty(UActorComponent& InComp)
{
	SCOPE_CYCLE_COUNTER(STAT_OnComponentRenderStateDirty);
	if (&InComp == RootComp.Get())
	{
		auto NewVisibleFlag = RootComp->GetVisibleFlag();
		if (bPrevIsRootComponentVisible != NewVisibleFlag)
		{
			bPrevIsRootComponentVisible = NewVisibleFlag;

			SetActiveStateForEnableAndDisable(NewVisibleFlag);
		}
	}
}
#if WITH_EDITOR
void ULGUILifeCycleBehaviour::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(ULGUILifeCycleBehaviour, enable))
		{
			if (this->GetWorld())
			{
				if (this->GetWorld()->IsGameWorld())
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
}
#endif

bool ULGUILifeCycleBehaviour::GetIsActiveAndEnable()const
{
	return enable;
}

bool ULGUILifeCycleBehaviour::IsAllowedToCallOnEnable()const
{
	if (GetRootSceneComponent())
	{
		return RootComp->GetVisibleFlag();
	}
	return true;
}

void ULGUILifeCycleBehaviour::SetActiveStateForEnableAndDisable(bool activeOrInactive)
{
	if (bIsAwakeCalled)
	{
		if (activeOrInactive)
		{
			if (enable)
			{
				if (!bIsEnableCalled)
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
		}
		else
		{
			if (enable)
			{
				if (bIsEnableCalled)
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
	}
}


void ULGUILifeCycleBehaviour::SetEnable(bool value)
{
	if (enable != value)
	{
		if (IsAllowedToCallOnEnable())
		{
			enable = value;
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
			else
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
		else
		{
			enable = value;
		}
	}
}

void ULGUILifeCycleBehaviour::SetCanExecuteUpdate(bool value)
{
	if (bCanExecuteUpdate != value)
	{
		bCanExecuteUpdate = value;
	}
}

void ULGUILifeCycleBehaviour::Awake()
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		check(0);
	}
#endif
	bIsAwakeCalled = true;
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		AwakeBP();
	}
}
void ULGUILifeCycleBehaviour::Start()
{
	bIsStartCalled = true;
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		StartBP();
	}
}
void ULGUILifeCycleBehaviour::Update(float DeltaTime)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		UpdateBP(DeltaTime);
	}
}
void ULGUILifeCycleBehaviour::OnDestroy()
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		OnDestroyBP();
	}
}
void ULGUILifeCycleBehaviour::OnEnable()
{
	bIsEnableCalled = true;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		check(0);
	}
	else//handle update in game mode
#endif
	{
		if (!bIsStartCalled)
		{
			ALGUIManagerActor::AddLGUILifeCycleBehavioursForStart(this);
		}
		else
		{
			if (bCanExecuteUpdate && !bIsAddedToUpdate)
			{
				bIsAddedToUpdate = true;
				ALGUIManagerActor::AddLGUILifeCycleBehavioursForUpdate(this);
			}
		}
	}
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		OnEnableBP();
	}
}
void ULGUILifeCycleBehaviour::OnDisable()
{
	bIsEnableCalled = false;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		check(0);
	}
	else//handle update in game mode
#endif
	{
		if (!bIsStartCalled)
		{
			ALGUIManagerActor::RemoveLGUILifeCycleBehavioursFromStart(this);
		}
		else
		{
			if (bIsAddedToUpdate)
			{
				bIsAddedToUpdate = false;
				ALGUIManagerActor::RemoveLGUILifeCycleBehavioursFromUpdate(this);
			}
		}
	}
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		OnDisableBP();
	}
}

USceneComponent* ULGUILifeCycleBehaviour::GetRootSceneComponent()const
{
	if (RootComp == nullptr)
	{
		if (auto Owner = GetOwner())
		{
			if (auto TempRootComp = Owner->GetRootComponent())
			{
				RootComp = TWeakObjectPtr<USceneComponent>(TempRootComp);
			}
			else
			{
				UE_LOG(LGUI, Error, TEXT("[ULGUILifeCycleBehaviour::GetRootSceneComponent]RootComponent is not valid!"));
			}
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[ULGUILifeCycleBehaviour::GetRootSceneComponent]Owner is not valid!"));
		}
	}
	return RootComp.Get();
}

AActor* ULGUILifeCycleBehaviour::InstantiateActor(AActor* OriginObject, USceneComponent* Parent)
{
	return LGUIPrefabSystem3::ActorSerializer3::DuplicateActor(OriginObject, Parent);
}
AActor* ULGUILifeCycleBehaviour::InstantiatePrefab(class ULGUIPrefab* OriginObject, USceneComponent* Parent)
{
	return OriginObject->LoadPrefab(this->GetWorld(), Parent, false);
}
AActor* ULGUILifeCycleBehaviour::InstantiatePrefabWithTransform(class ULGUIPrefab* OriginObject, USceneComponent* Parent, FVector Location, FRotator Rotation, FVector Scale)
{
	return OriginObject->LoadPrefabWithTransform(this->GetWorld(), Parent, Location, Rotation.Quaternion(), Scale);
}