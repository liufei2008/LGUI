// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Tickable.h"
#include "Subsystems/WorldSubsystem.h"
#include "LGUIPrefabManager.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIEditorTickMulticastDelegate, float);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FLGUIEditorManagerOnComponentCreateDelete, bool, UActorComponent*, AActor*);

class ULGUIPrefab;

UCLASS(NotBlueprintable, NotBlueprintType, Transient, NotPlaceable)
class LGUI_API ULGUIPrefabManagerObject :public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	static ULGUIPrefabManagerObject* Instance;
	ULGUIPrefabManagerObject();
	virtual void BeginDestroy()override;
public:
	//begin TickableEditorObject interface
	virtual void Tick(float DeltaTime)override;
	virtual bool IsTickable() const { return Instance == this; }
	virtual bool IsTickableInEditor()const { return Instance == this; }
	virtual TStatId GetStatId() const override;
	virtual bool IsEditorOnly()const override { return true; }
	//end TickableEditorObject interface
#if WITH_EDITORONLY_DATA
private:
	int32 PrevEditorViewportCount = 0;
	FLGUIEditorTickMulticastDelegate EditorTick;
	UPROPERTY()UWorld* PreviewWorldForPrefabPackage = nullptr;
	bool bIsBlueprintCompiling = false;
	class FLGUIObjectCreateDeleteListener* ObjectCreateDeleteListener = nullptr;
private:
	bool bShouldBroadcastLevelActorListChanged = false;
#endif
#if WITH_EDITOR
private:
	TArray<TTuple<int, TFunction<void()>>> OneShotFunctionsToExecuteInTick;
	FLGUIEditorManagerOnComponentCreateDelete OnComponentCreateDeleteEvent;
public:
	static void AddOneShotTickFunction(const TFunction<void()>& InFunction, int InDelayFrameCount = 0);
	static FDelegateHandle RegisterEditorTickFunction(const TFunction<void(float)>& InFunction);
	static void UnregisterEditorTickFunction(const FDelegateHandle& InDelegateHandle);
	static FLGUIEditorManagerOnComponentCreateDelete& OnComponentCreateDelete() { InitCheck(); return Instance->OnComponentCreateDeleteEvent; }
private:
	static bool InitCheck();
public:
	static ULGUIPrefabManagerObject* GetInstance(bool CreateIfNotValid = false);
	static bool IsSelected(AActor* InObject);
	static bool AnySelectedIsChildOf(AActor* InObject);
	static UWorld* GetPreviewWorldForPrefabPackage();
	static bool GetIsBlueprintCompiling();
private:
	FDelegateHandle OnBlueprintPreCompileDelegateHandle;
	FDelegateHandle OnBlueprintCompiledDelegateHandle;
	void OnBlueprintPreCompile(UBlueprint* InBlueprint);
	void OnBlueprintCompiled();
public:
	static void MarkBroadcastLevelActorListChanged();
private:
	FDelegateHandle OnAssetReimportDelegateHandle;
	void OnAssetReimport(UObject* asset);
	FDelegateHandle OnMapOpenedDelegateHandle;
	void OnMapOpened(const FString& FileName, bool AsTemplate);
	FDelegateHandle OnPackageReloadedDelegateHandle;
	void OnPackageReloaded(EPackageReloadPhase Phase, FPackageReloadedEvent* Event);

public:
	DECLARE_DELEGATE_OneParam(FSortChildrenActors, TArray<AActor*>&);
	static FSortChildrenActors OnSortChildrenActors;
	DECLARE_DELEGATE_ThreeParams(FPrefabEditorViewport_MouseClick, const FVector&, const FVector&, AActor*&);
	static FPrefabEditorViewport_MouseClick OnPrefabEditorViewport_MouseClick;
	DECLARE_DELEGATE(FPrefabEditorViewport_MouseMove);
	static FPrefabEditorViewport_MouseMove OnPrefabEditorViewport_MouseMove;
	DECLARE_DELEGATE_ThreeParams(FPrefabEditor_CreateRootAgent, UClass*, ULGUIPrefab*, AActor*&);
	static FPrefabEditor_CreateRootAgent OnPrefabEditor_CreateRootAgent;
	DECLARE_DELEGATE_ThreeParams(FPrefabEditor_GetBounds, USceneComponent*, FBox&, bool&);
	static FPrefabEditor_GetBounds OnPrefabEditor_GetBounds;
	DECLARE_DELEGATE_TwoParams(FPrefabEditor_SavePrefab, AActor*, ULGUIPrefab*);
	static FPrefabEditor_SavePrefab OnPrefabEditor_SavePrefab;
#endif
};

UCLASS(NotBlueprintable, NotBlueprintType, Transient, NotPlaceable)
class LGUI_API ULGUIPrefabWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual void Initialize(FSubsystemCollectionBase& Collection)override {};
	virtual void Deinitialize()override {};

	static ULGUIPrefabWorldSubsystem* GetInstance(UWorld* World);
	DECLARE_EVENT_OneParam(ULGUIPrefabWorldSubsystem, FDeserializeSession, const FGuid&);
	FDeserializeSession OnBeginDeserializeSession;
	FDeserializeSession OnEndDeserializeSession;
private:
	/** Map actor to prefab-deserialize-settion-id */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	TMap<AActor*, FGuid> AllActors_PrefabSystemProcessing;
public:
	void BeginPrefabSystemProcessingActor(const FGuid& InSessionId);
	void EndPrefabSystemProcessingActor(const FGuid& InSessionId);
	void AddActorForPrefabSystem(AActor* InActor, const FGuid& InSessionId);
	void RemoveActorForPrefabSystem(AActor* InActor, const FGuid& InSessionId);
	FGuid GetPrefabSystemSessionIdForActor(AActor* InActor);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool IsPrefabSystemProcessingActor(AActor* InActor);
};
