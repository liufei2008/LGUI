// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Tickable.h"
#include "Subsystems/WorldSubsystem.h"
#include "LGUIPrefabManager.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIEditorTickMulticastDelegate, float);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FLGUIEditorManagerOnComponentCreateDelete, bool, UActorComponent*, AActor*);

class ULGUIPrefab;
class ULGUIPrefabHelperObject;

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
	DECLARE_DELEGATE_OneParam(FSerialize_SortChildrenActors, TArray<AActor*>&);
	static FSerialize_SortChildrenActors OnSerialize_SortChildrenActors;
	DECLARE_DELEGATE_OneParam(FDeserialize_Components, const TArray<UActorComponent*>&);
	static FDeserialize_Components OnDeserialize_ProcessComponentsBeforeRerunConstructionScript;
	DECLARE_DELEGATE_FourParams(FPrefabEditorViewport_MouseClick, UWorld*, const FVector&, const FVector&, AActor*&);
	static FPrefabEditorViewport_MouseClick OnPrefabEditorViewport_MouseClick;
	DECLARE_DELEGATE_OneParam(FPrefabEditorViewport_MouseMove, UWorld*);
	static FPrefabEditorViewport_MouseMove OnPrefabEditorViewport_MouseMove;
	DECLARE_DELEGATE_FourParams(FPrefabEditor_CreateRootAgent, UWorld*, UClass*, ULGUIPrefab*, AActor*&);
	static FPrefabEditor_CreateRootAgent OnPrefabEditor_CreateRootAgent;
	DECLARE_DELEGATE_ThreeParams(FPrefabEditor_GetBounds, USceneComponent*, FBox&, bool&);
	static FPrefabEditor_GetBounds OnPrefabEditor_GetBounds;
	DECLARE_DELEGATE_TwoParams(FPrefabEditor_SavePrefab, AActor*, ULGUIPrefab*);
	static FPrefabEditor_SavePrefab OnPrefabEditor_SavePrefab;
	DECLARE_DELEGATE(FPrefabEditor_Refresh);
	static FPrefabEditor_Refresh OnPrefabEditor_Refresh;
	DECLARE_DELEGATE_ThreeParams(FPrefabEditor_ReplaceObjectPropertyForApplyOrRevert, ULGUIPrefabHelperObject*, UObject*, FName&);
	static FPrefabEditor_ReplaceObjectPropertyForApplyOrRevert OnPrefabEditor_ReplaceObjectPropertyForApplyOrRevert;
	DECLARE_DELEGATE_ThreeParams(FPrefabEditor_AfterObjectPropertyApplyOrRevert, ULGUIPrefabHelperObject*, UObject*, FName);
	static FPrefabEditor_AfterObjectPropertyApplyOrRevert OnPrefabEditor_AfterObjectPropertyApplyOrRevert;
	DECLARE_DELEGATE_TwoParams(FPrefabEditor_AfterMakePrefabAsSubPrefab, ULGUIPrefabHelperObject*, AActor*);
	static FPrefabEditor_AfterMakePrefabAsSubPrefab OnPrefabEditor_AfterMakePrefabAsSubPrefab;
	DECLARE_DELEGATE_ThreeParams(FPrefabEditor_AfterCollectPropertyToOverride, ULGUIPrefabHelperObject*, UObject*, FName);
	static FPrefabEditor_AfterCollectPropertyToOverride OnPrefabEditor_AfterCollectPropertyToOverride;
	DECLARE_DELEGATE_ThreeParams(FPrefabEditor_CopyRootObjectParentAnchorData, ULGUIPrefabHelperObject*, UObject*, UObject*);
	static FPrefabEditor_CopyRootObjectParentAnchorData OnPrefabEditor_CopyRootObjectParentAnchorData;
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

	/**
	 * Tell if PrefabSystem is deserializing the actor, can use this function in BeginPlay, if this return true then means properties are not ready yet, then you should use ILGUIPrefabInterface and implement Awake instead of BeginPlay.
	 * PrefabSystem is deserializing actor during LoadPrefab or DuplicateActor.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool IsPrefabSystemProcessingActor(AActor* InActor);
};
