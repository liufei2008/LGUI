// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabManager.h"
#include "LGUI.h"
#include "Engine/World.h"
#include "Core/LGUISettings.h"
#include "Engine/Engine.h"
#if WITH_EDITOR
#include "Editor.h"
#include "DrawDebugHelpers.h"
#include "Engine/Selection.h"
#include "EditorViewportClient.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "EngineUtils.h"
#endif

#define LOCTEXT_NAMESPACE "LGUIPrefabManagerObject"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

#if WITH_EDITOR
class FLGUIObjectCreateDeleteListener : public FUObjectArray::FUObjectCreateListener, public FUObjectArray::FUObjectDeleteListener
{
public:
	ULGUIPrefabManagerObject* Manager = nullptr;
	FLGUIObjectCreateDeleteListener(ULGUIPrefabManagerObject* InManager)
	{
		Manager = InManager;
		GUObjectArray.AddUObjectCreateListener(this);
		GUObjectArray.AddUObjectDeleteListener(this);
	}
	~FLGUIObjectCreateDeleteListener()
	{
		GUObjectArray.RemoveUObjectCreateListener(this);
		GUObjectArray.RemoveUObjectDeleteListener(this);
	}

	virtual void NotifyUObjectCreated(const class UObjectBase* Object, int32 Index)override
	{
		if (auto Comp = Cast<UActorComponent>((UObject*)Object))
		{
			if (Comp->IsVisualizationComponent())return;
			if (auto Actor = Comp->GetOwner())
			{
				Manager->OnComponentCreateDelete().Broadcast(true, Comp, Actor);
			}
		}
	}
	virtual void NotifyUObjectDeleted(const class UObjectBase* Object, int32 Index)override
	{
		if (auto Comp = Cast<UActorComponent>((UObject*)Object))
		{
			if (Comp->IsVisualizationComponent())return;
			if (auto Actor = Comp->GetOwner())
			{
				Manager->OnComponentCreateDelete().Broadcast(false, Comp, Actor);
			}
		}
	}
	virtual void OnUObjectArrayShutdown()override {};
};
#endif

ULGUIPrefabManagerObject* ULGUIPrefabManagerObject::Instance = nullptr;
ULGUIPrefabManagerObject::ULGUIPrefabManagerObject()
{

}
void ULGUIPrefabManagerObject::BeginDestroy()
{
#if WITH_EDITORONLY_DATA
	if (OnAssetReimportDelegateHandle.IsValid())
	{
		if (GEditor)
		{
			if (auto ImportSubsystem = GEditor->GetEditorSubsystem<UImportSubsystem>())
			{
				ImportSubsystem->OnAssetReimport.Remove(OnAssetReimportDelegateHandle);
			}
		}
	}
	if (OnMapOpenedDelegateHandle.IsValid())
	{
		FEditorDelegates::OnMapOpened.Remove(OnMapOpenedDelegateHandle);
	}
	if (OnPackageReloadedDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::OnPackageReloaded.Remove(OnPackageReloadedDelegateHandle);
	}
	if (OnBlueprintPreCompileDelegateHandle.IsValid())
	{
		if (GEditor)
		{
			GEditor->OnBlueprintPreCompile().Remove(OnBlueprintPreCompileDelegateHandle);
		}
	}
	if (OnBlueprintCompiledDelegateHandle.IsValid())
	{
		if (GEditor)
		{
			GEditor->OnBlueprintCompiled().Remove(OnBlueprintCompiledDelegateHandle);
		}
	}

	//cleanup preview world
	if (PreviewWorldForPrefabPackage && GEngine)
	{
		PreviewWorldForPrefabPackage->CleanupWorld();
		GEngine->DestroyWorldContext(PreviewWorldForPrefabPackage);
		PreviewWorldForPrefabPackage->ReleasePhysicsScene();
	}

	delete ObjectCreateDeleteListener;
	ObjectCreateDeleteListener = nullptr;
#endif
	Instance = nullptr;
	Super::BeginDestroy();
}

void ULGUIPrefabManagerObject::Tick(float DeltaTime)
{
#if WITH_EDITORONLY_DATA
	if (EditorTick.IsBound())
	{
		EditorTick.Broadcast(DeltaTime);
	}
	if (OneShotFunctionsToExecuteInTick.Num() > 0)
	{
		for (int i = 0; i < OneShotFunctionsToExecuteInTick.Num(); i++)
		{
			auto& Item = OneShotFunctionsToExecuteInTick[i];
			if (Item.Key <= 0)
			{
				Item.Value();
				OneShotFunctionsToExecuteInTick.RemoveAt(i);
				i--;
			}
			else
			{
				Item.Key--;
			}
		}
	}
#endif
#if WITH_EDITOR
	if (bShouldBroadcastLevelActorListChanged)
	{
		bShouldBroadcastLevelActorListChanged = false;
		if (IsValid(GEditor))
		{
			GEditor->BroadcastLevelActorListChanged();
		}
	}
#endif
}
TStatId ULGUIPrefabManagerObject::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULGUIPrefabManagerObject, STATGROUP_Tickables);
}

#if WITH_EDITOR

void ULGUIPrefabManagerObject::AddOneShotTickFunction(const TFunction<void()>& InFunction, int InDelayFrameCount)
{
	InitCheck();
	InDelayFrameCount = FMath::Max(0, InDelayFrameCount);
	TTuple<int, TFunction<void()>> Item;
	Item.Key = InDelayFrameCount;
	Item.Value = InFunction;
	Instance->OneShotFunctionsToExecuteInTick.Add(Item);
}
FDelegateHandle ULGUIPrefabManagerObject::RegisterEditorTickFunction(const TFunction<void(float)>& InFunction)
{
	InitCheck();
	return Instance->EditorTick.AddLambda(InFunction);
}
void ULGUIPrefabManagerObject::UnregisterEditorTickFunction(const FDelegateHandle& InDelegateHandle)
{
	if (Instance != nullptr)
	{
		Instance->EditorTick.Remove(InDelegateHandle);
	}
}

ULGUIPrefabManagerObject* ULGUIPrefabManagerObject::GetInstance(bool CreateIfNotValid)
{
	if (CreateIfNotValid)
	{
		InitCheck();
	}
	return Instance;
}
bool ULGUIPrefabManagerObject::InitCheck()
{
	if (Instance == nullptr)
	{
		Instance = NewObject<ULGUIPrefabManagerObject>();
		Instance->AddToRoot();
		UE_LOG(LGUI, Log, TEXT("[ULGUIManagerObject::InitCheck]No Instance for LGUIManagerObject, create!"));
		//open map
		Instance->OnMapOpenedDelegateHandle = FEditorDelegates::OnMapOpened.AddUObject(Instance, &ULGUIPrefabManagerObject::OnMapOpened);
		Instance->OnPackageReloadedDelegateHandle = FCoreUObjectDelegates::OnPackageReloaded.AddUObject(Instance, &ULGUIPrefabManagerObject::OnPackageReloaded);
		if (GEditor)
		{
			//reimport asset
			Instance->OnAssetReimportDelegateHandle = GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetReimport.AddUObject(Instance, &ULGUIPrefabManagerObject::OnAssetReimport);
			//blueprint recompile
			Instance->OnBlueprintPreCompileDelegateHandle = GEditor->OnBlueprintPreCompile().AddUObject(Instance, &ULGUIPrefabManagerObject::OnBlueprintPreCompile);
			Instance->OnBlueprintCompiledDelegateHandle = GEditor->OnBlueprintCompiled().AddUObject(Instance, &ULGUIPrefabManagerObject::OnBlueprintCompiled);
		}
		Instance->ObjectCreateDeleteListener = new FLGUIObjectCreateDeleteListener(Instance);
	}
	return true;
}

void ULGUIPrefabManagerObject::OnBlueprintPreCompile(UBlueprint* InBlueprint)
{
	bIsBlueprintCompiling = true;
}
void ULGUIPrefabManagerObject::OnBlueprintCompiled()
{
	bIsBlueprintCompiling = true;
	AddOneShotTickFunction([this] {
		bIsBlueprintCompiling = false; 
		}, 2);
}

void ULGUIPrefabManagerObject::OnAssetReimport(UObject* asset)
{
	if (IsValid(asset))
	{
		auto textureAsset = Cast<UTexture2D>(asset);
		if (IsValid(textureAsset))
		{
			
		}
	}
}

void ULGUIPrefabManagerObject::OnMapOpened(const FString& FileName, bool AsTemplate)
{

}

void ULGUIPrefabManagerObject::OnPackageReloaded(EPackageReloadPhase Phase, FPackageReloadedEvent* Event)
{
	if (Phase == EPackageReloadPhase::PostBatchPostGC && Event != nullptr && Event->GetNewPackage() != nullptr)
	{
		auto Asset = Event->GetNewPackage()->FindAssetInPackage();
		if (auto PrefabAsset = Cast<ULGUIPrefab>(Asset))
		{
			
		}
	}
}

UWorld* ULGUIPrefabManagerObject::GetPreviewWorldForPrefabPackage()
{
	InitCheck();
	auto& PreviewWorldForPrefabPackage = Instance->PreviewWorldForPrefabPackage;
	if (PreviewWorldForPrefabPackage == nullptr)
	{
		FName UniqueWorldName = MakeUniqueObjectName(Instance, UWorld::StaticClass(), FName("LGUI_PreviewWorldForPrefabPackage"));
		PreviewWorldForPrefabPackage = NewObject<UWorld>(Instance, UniqueWorldName);
		PreviewWorldForPrefabPackage->AddToRoot();
		PreviewWorldForPrefabPackage->WorldType = EWorldType::EditorPreview;

		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(PreviewWorldForPrefabPackage->WorldType);
		WorldContext.SetCurrentWorld(PreviewWorldForPrefabPackage);

		PreviewWorldForPrefabPackage->InitializeNewWorld(UWorld::InitializationValues()
			.AllowAudioPlayback(false)
			.CreatePhysicsScene(false)
			.RequiresHitProxies(false)
			.CreateNavigation(false)
			.CreateAISystem(false)
			.ShouldSimulatePhysics(false)
			.SetTransactional(false));
	}
	return PreviewWorldForPrefabPackage;
}
bool ULGUIPrefabManagerObject::GetIsBlueprintCompiling()
{
	if (InitCheck())
	{
		return Instance->bIsBlueprintCompiling;
	}
	return false;
}

void ULGUIPrefabManagerObject::MarkBroadcastLevelActorListChanged()
{
	if (Instance != nullptr)
	{
		Instance->bShouldBroadcastLevelActorListChanged = true;
	}
}

bool ULGUIPrefabManagerObject::IsSelected(AActor* InObject)
{
	if (!IsValid(GEditor))return false;
	return GEditor->GetSelectedActors()->IsSelected(InObject);
}

bool ULGUIPrefabManagerObject::AnySelectedIsChildOf(AActor* InObject)
{
	if (!IsValid(GEditor))return false;
	for (FSelectionIterator itr(GEditor->GetSelectedActorIterator()); itr; ++itr)
	{
		auto itrActor = Cast<AActor>(*itr);
		if (IsValid(itrActor) && itrActor->IsAttachedTo(InObject))
		{
			return true;
		}
	}
	return false;
}

ULGUIPrefabManagerObject::FSortChildrenActors ULGUIPrefabManagerObject::OnSortChildrenActors;
ULGUIPrefabManagerObject::FPrefabEditorViewport_MouseClick ULGUIPrefabManagerObject::OnPrefabEditorViewport_MouseClick;
ULGUIPrefabManagerObject::FPrefabEditorViewport_MouseMove ULGUIPrefabManagerObject::OnPrefabEditorViewport_MouseMove;
ULGUIPrefabManagerObject::FPrefabEditor_CreateRootAgent ULGUIPrefabManagerObject::OnPrefabEditor_CreateRootAgent;
ULGUIPrefabManagerObject::FPrefabEditor_GetBounds ULGUIPrefabManagerObject::OnPrefabEditor_GetBounds;
ULGUIPrefabManagerObject::FPrefabEditor_SavePrefab ULGUIPrefabManagerObject::OnPrefabEditor_SavePrefab;
#endif


ULGUIPrefabWorldSubsystem* ULGUIPrefabWorldSubsystem::GetInstance(UWorld* World)
{
	return World->GetSubsystem<ULGUIPrefabWorldSubsystem>();
}
void ULGUIPrefabWorldSubsystem::BeginPrefabSystemProcessingActor(const FGuid& InSessionId)
{
	OnBeginDeserializeSession.Broadcast(InSessionId);
}
void ULGUIPrefabWorldSubsystem::EndPrefabSystemProcessingActor(const FGuid& InSessionId)
{
	OnEndDeserializeSession.Broadcast(InSessionId);
}
void ULGUIPrefabWorldSubsystem::AddActorForPrefabSystem(AActor* InActor, const FGuid& InSessionId)
{
	AllActors_PrefabSystemProcessing.Add(InActor, InSessionId);
}
void ULGUIPrefabWorldSubsystem::RemoveActorForPrefabSystem(AActor* InActor, const FGuid& InSessionId)
{
	AllActors_PrefabSystemProcessing.Remove(InActor);
}
FGuid ULGUIPrefabWorldSubsystem::GetPrefabSystemSessionIdForActor(AActor* InActor)
{
	if (auto FoundPtr = AllActors_PrefabSystemProcessing.Find(InActor))
	{
		return *FoundPtr;
	}
	return FGuid();
}

bool ULGUIPrefabWorldSubsystem::IsPrefabSystemProcessingActor(AActor* InActor)
{
	return AllActors_PrefabSystemProcessing.Contains(InActor);
}
#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
#undef LOCTEXT_NAMESPACE