// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUILifeCycleBehaviour.h"
#include "LGUI.h"
#include "Core/Actor/LGUIManager.h"
#include "PrefabSystem/LGUIPrefabManager.h"
#include "PrefabSystem/LGUIPrefab.h"
#include LGUIPREFAB_SERIALIZER_NEWEST_INCLUDE
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
	bIsSerializedFromLGUIPrefab = false;

	bCanExecuteBlueprintEvent = GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native);
}

void ULGUILifeCycleBehaviour::BeginPlay()
{
	Super::BeginPlay();
	ULGUIManagerWorldSubsystem::AddLGUILifeCycleBehaviourForLifecycleEvent(this);
	if (GetRootSceneComponent())
	{
		if (auto RootUIComp = Cast<UUIItem>(RootComp.Get()))
		{
			UIActiveInHierarchyStateChangedDelegateHandle = RootUIComp->RegisterUIActiveStateChanged(FUIItemActiveInHierarchyStateChangedDelegate::CreateUObject(this, &ULGUILifeCycleBehaviour::OnUIActiveInHierarchyStateChanged));
		}
		else
		{
			bPrevIsRootComponentVisible = RootComp->GetVisibleFlag();
			ComponentRenderStateDirtyDelegateHandle = UActorComponent::MarkRenderStateDirtyEvent.AddUObject(this, &ULGUILifeCycleBehaviour::OnComponentRenderStateDirty);
		}
	}
}
void ULGUILifeCycleBehaviour::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
void ULGUILifeCycleBehaviour::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bIsEnableCalled)
	{
		Call_OnDisable();
	}
	if (bIsAwakeCalled)
	{
		OnDestroy();
	}
	if (UIActiveInHierarchyStateChangedDelegateHandle.IsValid())
	{
		if (RootComp.IsValid())
		{
			if (auto RootUIComp = Cast<UUIItem>(RootComp.Get()))
			{
				RootUIComp->UnregisterUIActiveStateChanged(UIActiveInHierarchyStateChangedDelegateHandle);
			}
		}
	}
	if (ComponentRenderStateDirtyDelegateHandle.IsValid())
	{
		UActorComponent::MarkRenderStateDirtyEvent.Remove(ComponentRenderStateDirtyDelegateHandle);
	}
	Super::EndPlay(EndPlayReason);
}
void ULGUILifeCycleBehaviour::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITOR
	if (GetWorld() && !GetWorld()->IsGameWorld())
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
				EditorTickDelegateHandle = ULGUIPrefabManagerObject::RegisterEditorTickFunction([=](float deltaTime) {this->Update(deltaTime); });
			}
		}
	}
#endif
}
void ULGUILifeCycleBehaviour::OnUnregister()
{
	Super::OnUnregister();
#if WITH_EDITOR
	if (EditorTickDelegateHandle.IsValid())
	{
		ULGUIPrefabManagerObject::UnregisterEditorTickFunction(EditorTickDelegateHandle);
		EditorTickDelegateHandle.Reset();
	}
#endif
}

void ULGUILifeCycleBehaviour::OnUIActiveInHierarchyStateChanged(bool InState)
{
	SetActiveStateForEnableAndDisable(InState);
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
						Call_OnEnable();
					}
					else
					{
						Call_OnDisable();
					}
				}
			}
		}
	}
}
#endif

bool ULGUILifeCycleBehaviour::IsAllowedToCallAwake()const
{
	if (GetRootSceneComponent())
	{
		if (auto RootUIComp = Cast<UUIItem>(RootComp.Get()))
		{
			return RootUIComp->GetIsUIActiveInHierarchy();
		}
		else
		{
			return true;
		}
	}
	return true;
}
bool ULGUILifeCycleBehaviour::GetIsActiveAndEnable()const
{
	if (GetRootSceneComponent())
	{
		if (auto RootUIComp = Cast<UUIItem>(RootComp.Get()))
		{
			return RootUIComp->GetIsUIActiveInHierarchy() && this->GetEnable();
		}
		else
		{
			return RootComp->GetVisibleFlag();
		}
	}
	else
	{
		return this->GetEnable();
	}
}

bool ULGUILifeCycleBehaviour::IsAllowedToCallOnEnable()const
{
	if (GetRootSceneComponent())
	{
		if (auto RootUIComp = Cast<UUIItem>(RootComp.Get()))
		{
			return RootUIComp->GetIsUIActiveInHierarchy();
		}
		else
		{
			return RootComp->GetVisibleFlag();
		}
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
						Call_OnEnable();
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
						Call_OnDisable();
					}
				}
			}
		}
	}
	else//not call awake, should be the first time that get IsUIActive:true
	{
		if (activeOrInactive)
		{
			if (!bIsAwakeCalled)
			{
				Call_Awake();
			}
			if (enable)
			{
				if (!bIsEnableCalled)
				{
					Call_OnEnable();
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
					Call_OnEnable();
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
					Call_OnDisable();
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
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveAwake();
	}
}
void ULGUILifeCycleBehaviour::Start()
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveStart();
	}
}
void ULGUILifeCycleBehaviour::Update(float DeltaTime)
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveUpdate(DeltaTime);
	}
}
void ULGUILifeCycleBehaviour::OnDestroy()
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnDestroy();
	}
}
void ULGUILifeCycleBehaviour::OnEnable()
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnEnable();
	}
}
void ULGUILifeCycleBehaviour::OnDisable()
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveOnDisable();
	}
}

void ULGUILifeCycleBehaviour::Call_Awake()
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Should never reach this point!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
#if !UE_BUILD_SHIPPING
	if (bIsAwakeCalled)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Awake already executed!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
	bIsAwakeCalled = true;
	Awake();
	if (GetRootSceneComponent())
	{
		bPrevIsRootComponentVisible = RootComp->GetVisibleFlag();//get bPrevIsRootComponentVisible in Awake, incase SetVisibility is called inside Awake, which will not trigger OnComponentRenderStateDirty callback, because RenderState is already dirty when Awake/BeginPlay
	}
}

void ULGUILifeCycleBehaviour::Call_Start()
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Should never reach this point!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
#if !UE_BUILD_SHIPPING
	if (bIsStartCalled)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Start already executed!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
	bIsStartCalled = true;
	Start();
}

void ULGUILifeCycleBehaviour::Call_OnEnable()
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Should never reach this point!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
#if !UE_BUILD_SHIPPING
	if (bIsEnableCalled)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d OnEnable already executed!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
	bIsEnableCalled = true;

	OnEnable();
	if (!bIsStartCalled)
	{
		ULGUIManagerWorldSubsystem::AddLGUILifeCycleBehavioursForStart(this);
	}
	else
	{
		if (bCanExecuteUpdate && !bIsAddedToUpdate)
		{
			bIsAddedToUpdate = true;
			ULGUIManagerWorldSubsystem::AddLGUILifeCycleBehavioursForUpdate(this);
		}
	}
}

void ULGUILifeCycleBehaviour::Call_OnDisable()
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//edit mode
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Should never reach this point!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
#if !UE_BUILD_SHIPPING
	if (!bIsEnableCalled)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d OnEnable not executed!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
#endif
	bIsEnableCalled = false;

	OnDisable();
	if (!bIsStartCalled)
	{
		ULGUIManagerWorldSubsystem::RemoveLGUILifeCycleBehavioursFromStart(this);
	}
	else
	{
		if (bIsAddedToUpdate)
		{
			bIsAddedToUpdate = false;
			ULGUIManagerWorldSubsystem::RemoveLGUILifeCycleBehavioursFromUpdate(this);
		}
	}
}

USceneComponent* ULGUILifeCycleBehaviour::GetRootSceneComponent()const
{
	if (!RootComp.IsValid())
	{
		if (auto Owner = GetOwner())
		{
			if (auto TempRootComp = Owner->GetRootComponent())
			{
				RootComp = TWeakObjectPtr<USceneComponent>(TempRootComp);
			}
			else
			{
				UE_LOG(LGUI, Error, TEXT("[%s].%d RootComponent not exist in owner actor!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			}
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d Owner is not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		}
	}
	return RootComp.Get();
}

AActor* ULGUILifeCycleBehaviour::InstantiateActor(AActor* OriginObject, USceneComponent* Parent)
{
	return LGUIPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::DuplicateActor(OriginObject, Parent);
}
AActor* ULGUILifeCycleBehaviour::InstantiatePrefab(class ULGUIPrefab* OriginObject, USceneComponent* Parent)
{
	return OriginObject->LoadPrefab(this->GetWorld(), Parent, false);
}
AActor* ULGUILifeCycleBehaviour::InstantiatePrefabWithTransform(class ULGUIPrefab* OriginObject, USceneComponent* Parent, FVector Location, FRotator Rotation, FVector Scale)
{
	return OriginObject->LoadPrefabWithTransform(this, Parent, Location, Rotation.Quaternion(), Scale);
}