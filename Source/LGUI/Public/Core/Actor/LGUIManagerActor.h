// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Core/LGUISpriteData.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Tickable.h"
#include "LGUIManagerActor.generated.h"

class UUIItem;
class UUIText;
class UUIBatchGeometryRenderable;
class UUIBaseRenderable;
class ULGUICanvas;
class ULGUIBaseRaycaster;
class UUISelectableComponent;
class ULGUILifeCycleBehaviour;
class ULGUIBaseInputModule;
class ILGUILayoutInterface;

DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIEditorTickMulticastDelegate, float);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FLGUIEditorManagerOnComponentCreateDelete, bool, UActorComponent*, AActor*);

UCLASS(NotBlueprintable, NotBlueprintType, Transient, NotPlaceable)
class LGUI_API ULGUIEditorManagerObject :public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	static ULGUIEditorManagerObject* Instance;
	ULGUIEditorManagerObject();
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
	TMap<int32, uint32> EditorViewportIndexToKeyMap;
	int32 PrevEditorViewportCount = 0;
	int32 PrevScreenSpaceOverlayCanvasCount = 1;
	FLGUIEditorTickMulticastDelegate EditorTick;
	FSimpleMulticastDelegate EditorViewportIndexAndKeyChange;
	UPROPERTY()UWorld* PreviewWorldForPrefabPackage = nullptr;
	bool bIsBlueprintCompiling = false;
	class FLGUIObjectCreateDeleteListener* ObjectCreateDeleteListener = nullptr;
public:
	int32 CurrentActiveViewportIndex = 0;
	uint32 CurrentActiveViewportKey = 0;
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
	static FDelegateHandle RegisterEditorViewportIndexAndKeyChange(const TFunction<void()>& InFunction);
	static void UnregisterEditorViewportIndexAndKeyChange(const FDelegateHandle& InDelegateHandle);
	static FLGUIEditorManagerOnComponentCreateDelete& OnComponentCreateDelete() { InitCheck(); return Instance->OnComponentCreateDeleteEvent; }
private:
	static bool InitCheck();
public:
	static ULGUIEditorManagerObject* GetInstance(bool CreateIfNotValid = false);
	static bool IsSelected(AActor* InObject);
	static bool AnySelectedIsChildOf(AActor* InObject);
	void CheckEditorViewportIndexAndKey();
	uint32 GetViewportKeyFromIndex(int32 InViewportIndex);
	static UWorld* GetPreviewWorldForPrefabPackage();
	static bool GetIsBlueprintCompiling();
public:
	/**
	 * Editor raycast hit all visible UIBaseRenderable object.
	 * @param InWorld
	 * @param InUIItems
	 * @param LineStart
	 * @param LineEnd
	 * @param ResultSelectTarget
	 * @param InOutTargetIndexInHitArray	Pass in desired item index, and result selected item index. Default is -1, will use first one as result.
	 * \return 
	 */
	static bool RaycastHitUI(UWorld* InWorld, const TArray<UUIItem*>& InUIItems, const FVector& LineStart, const FVector& LineEnd
		, UUIBaseRenderable*& ResultSelectTarget, int& InOutTargetIndexInHitArray
	);
private:
	FDelegateHandle OnBlueprintPreCompileDelegateHandle;
	FDelegateHandle OnBlueprintCompiledDelegateHandle;
	void OnBlueprintPreCompile(UBlueprint* InBlueprint);
	void OnBlueprintCompiled();
	void RefreshOnBlueprintCompiled();
public:
	static void MarkBroadcastLevelActorListChanged();
private:
	FDelegateHandle OnAssetReimportDelegateHandle;
	void OnAssetReimport(UObject* asset);
	FDelegateHandle OnActorLabelChangedDelegateHandle;
	void OnActorLabelChanged(AActor* actor);
	FDelegateHandle OnMapOpenedDelegateHandle;
	void OnMapOpened(const FString& FileName, bool AsTemplate);
	FDelegateHandle OnPackageReloadedDelegateHandle;
	void OnPackageReloaded(EPackageReloadPhase Phase, FPackageReloadedEvent* Event);
#endif
};

struct FLGUILifeCycleBehaviourArrayContainer
{
	TArray<TTuple<TWeakObjectPtr<ULGUILifeCycleBehaviour>, int32>> LGUILifeCycleBehaviourArray;
	/** Functions that wait for prefab serialization complete then execute */
	TArray<TFunction<void()>> Functions;
};

class ILGUICultureChangedInterface;
enum class ELGUIRenderMode : uint8;

UCLASS(NotBlueprintable, NotBlueprintType, Transient, NotPlaceable)
class LGUI_API ALGUIManagerActor : public AActor
{
	GENERATED_BODY()
private:
	static TMap<UWorld*, ALGUIManagerActor*> WorldToInstanceMap;
	bool bExistInInstanceMap = false;
public:	
	static ALGUIManagerActor* GetInstance(UWorld* InWorld, bool CreateIfNotValid = false);
	static const TMap<UWorld*, ALGUIManagerActor*>& GetWorldToInstanceMap() { return WorldToInstanceMap; }
	ALGUIManagerActor();
	virtual void BeginPlay()override;
	virtual void BeginDestroy()override;
	virtual void Tick(float DeltaTime)override;
private:
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<UUIItem>> AllRootUIItemArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<ULGUICanvas>> AllCanvasArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<ULGUICanvas>> ScreenSpaceCanvasArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<ULGUICanvas>> WorldSpaceUECanvasArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<ULGUICanvas>> WorldSpaceLGUICanvasArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<ULGUICanvas>> RenderTargetSpaceLGUICanvasArray;

	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<ULGUIBaseRaycaster>> AllRaycasterArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TWeakObjectPtr<ULGUIBaseInputModule> CurrentInputModule = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<UUISelectableComponent>> AllSelectableArray;
	//ILGUILayoutInterface
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<UObject>> AllLayoutArray;
	//ILGUICultureChangedInterface
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<UObject>> AllCultureChangedArray;

	bool bShouldSortScreenSpaceCanvas = true;
	bool bShouldSortWorldSpaceLGUICanvas = true;
	bool bShouldSortWorldSpaceCanvas = true;
	bool bShouldSortRenderTargetSpaceCanvas = true;

	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<ULGUILifeCycleBehaviour>> LGUILifeCycleBehavioursForUpdate;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<ULGUILifeCycleBehaviour>> LGUILifeCycleBehavioursForStart;
	bool bIsExecutingStart = false;
	bool bIsExecutingUpdate = false;
	int32 CurrentExecutingUpdateIndex = -1;
	TArray<ULGUILifeCycleBehaviour*> LGUILifeCycleBehavioursNeedToRemoveFromUpdate;
#if WITH_EDITORONLY_DATA
	int32 PrevScreenSpaceOverlayCanvasCount = 1;
#endif
	void OnCultureChanged();
	bool bShouldUpdateOnCultureChanged = false;
	FDelegateHandle onCultureChangedDelegateHandle;

	TSharedPtr<class FLGUIRenderer, ESPMode::ThreadSafe> ScreenSpaceOverlayViewExtension;

	void UpdateLayout();
	bool bNeedUpdateLayout = false;
public:
#if WITH_EDITOR
	static void RefreshAllUI(UWorld* InWorld = nullptr);
#endif
	static void AddRootUIItem(UUIItem* InItem);
	static void RemoveRootUIItem(UUIItem* InItem);
	const TArray<TWeakObjectPtr<UUIItem>>& GetAllRootUIItemArray()const { return AllRootUIItemArray; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
	static void RegisterLGUICultureChangedEvent(TScriptInterface<ILGUICultureChangedInterface> InItem);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	static void UnregisterLGUICultureChangedEvent(TScriptInterface<ILGUICultureChangedInterface> InItem);

	/** Force LGUI to update layout immediately */
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta=(WorldContext = "WorldContextObject"))
	static void ForceUpdateLayout(UObject* WorldContextObject);

	static void AddCanvas(ULGUICanvas* InCanvas, ELGUIRenderMode InCurrentRenderMode);
	static void RemoveCanvas(ULGUICanvas* InCanvas, ELGUIRenderMode InCurrentRenderMode);
	static void CanvasRenderModeChange(ULGUICanvas* InCanvas, ELGUIRenderMode InOldRenderMode, ELGUIRenderMode InNewRenderMode);
	TArray<TWeakObjectPtr<ULGUICanvas>>& GetCanvasArray() { return AllCanvasArray; };
	void MarkSortScreenSpaceCanvas();
	void MarkSortWorldSpaceLGUICanvas();
	void MarkSortWorldSpaceCanvas();
	void MarkSortRenderTargetSpaceCanvas();
	static void SortDrawcallOnRenderMode(ELGUIRenderMode InRenderMode, const TArray<TWeakObjectPtr<ULGUICanvas>>& InCanvasArray);

	const TArray<TWeakObjectPtr<UObject>>& GetAllLayoutArray()const { return AllLayoutArray; }

	static TSharedPtr<class FLGUIRenderer, ESPMode::ThreadSafe> GetViewExtension(UWorld* InWorld, bool InCreateIfNotExist);

	const TArray<TWeakObjectPtr<ULGUIBaseRaycaster>>& GetAllRaycasterArray(){ return AllRaycasterArray; }
	static void AddRaycaster(ULGUIBaseRaycaster* InRaycaster);
	static void RemoveRaycaster(ULGUIBaseRaycaster* InRaycaster);

	TWeakObjectPtr<ULGUIBaseInputModule> GetCurrentInputModule() { return CurrentInputModule; }
	static void SetCurrentInputModule(ULGUIBaseInputModule* InInputModule);
	static void ClearCurrentInputModule(ULGUIBaseInputModule* InInputModule);

	const TArray<TWeakObjectPtr<UUISelectableComponent>>& GetAllSelectableArray() { return AllSelectableArray; }
	static void AddSelectable(UUISelectableComponent* InSelectable);
	static void RemoveSelectable(UUISelectableComponent* InSelectable);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
	static void RegisterLGUILayout(TScriptInterface<ILGUILayoutInterface> InItem);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	static void UnregisterLGUILayout(TScriptInterface<ILGUILayoutInterface> InItem);
	static void MarkUpdateLayout(UWorld* InWorld);
#if WITH_EDITOR
	static void DrawFrameOnUIItem(UUIItem* InItem, bool IsScreenSpace = false);
	static void DrawNavigationArrow(UWorld* InWorld, const TArray<FVector>& InControlPoints, const FVector& InArrowPointA, const FVector& InArrowPointB, FColor const& InColor, bool IsScreenSpace = false);
	static void DrawNavigationVisualizerOnUISelectable(UWorld* InWorld, UUISelectableComponent* InSelectable, bool IsScreenSpace = false);
private:
	static void DrawDebugBoxOnScreenSpace(UWorld* InWorld, FVector const& Center, FVector const& Box, const FQuat& Rotation, FColor const& Color);
	static void DrawDebugRectOnScreenSpace(UWorld* InWorld, FVector const& Center, FVector const& Box, const FQuat& Rotation, FColor const& Color);
#endif
private:
	/** Map actor to prefab-deserialize-settion-id and actor index */
	TMap<AActor*, TTuple<FGuid, int32>> AllActors_PrefabSystemProcessing;
	/** Map prefab-deserialize-settion-id to LGUILifeCycleBehaviour array */
	TMap<FGuid, FLGUILifeCycleBehaviourArrayContainer> LGUILifeCycleBehaviours_PrefabSystemProcessing;
public:
	void BeginPrefabSystemProcessingActor(const FGuid& InSessionId);
	void EndPrefabSystemProcessingActor(const FGuid& InSessionId);
	void AddActorForPrefabSystem(AActor* InActor, const FGuid& InSessionId, int32 InActorIndex);
	void RemoveActorForPrefabSystem(AActor* InActor, const FGuid& InSessionId);
	bool IsPrefabSystemProcessingActor(AActor* InActor);
	/**
	 * Add a function that execute after prefab system serialization and before Awake called
	 * @param	InPrefabActor	Current prefab system processing actor
	 * @param	InFunction		Function to call after prefab system serialization complete and before Awake called
	 */
	void AddFunctionForPrefabSystemExecutionBeforeAwake(AActor* InPrefabActor, const TFunction<void()>& InFunction);

	static void AddLGUILifeCycleBehaviourForLifecycleEvent(ULGUILifeCycleBehaviour* InComp);
	static void AddLGUILifeCycleBehavioursForUpdate(ULGUILifeCycleBehaviour* InComp);
	static void RemoveLGUILifeCycleBehavioursFromUpdate(ULGUILifeCycleBehaviour* InComp);
	static void AddLGUILifeCycleBehavioursForStart(ULGUILifeCycleBehaviour* InComp);
	static void RemoveLGUILifeCycleBehavioursFromStart(ULGUILifeCycleBehaviour* InComp);
	static void ProcessLGUILifecycleEvent(ULGUILifeCycleBehaviour* InComp);
};
