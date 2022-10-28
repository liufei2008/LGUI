﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
#if WITH_EDITORONLY_DATA
	//end TickableEditorObject interface
	FLGUIEditorTickMulticastDelegate EditorTick;
	FSimpleMulticastDelegate EditorViewportIndexAndKeyChange;
private:
	TMap<int32, uint32> EditorViewportIndexToKeyMap;
	int32 PrevEditorViewportCount = 0;
	int32 PrevScreenSpaceOverlayCanvasCount = 1;
public:
	int32 CurrentActiveViewportIndex = 0;
	uint32 CurrentActiveViewportKey = 0;
private:
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<ULGUICanvas>> AllCanvasArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<UUIItem>> AllRootUIItemArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<UUISelectableComponent>> AllSelectableArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TScriptInterface<ILGUILayoutInterface>> AllLayoutArray;

	bool bShouldSortLGUIRenderer = true;
	bool bShouldSortWorldSpaceCanvas = true;
	bool bShouldSortRenderTargetSpaceCanvas = true;

	bool bShouldBroadcastLevelActorListChanged = false;
#endif
#if WITH_EDITOR
private:
	static TArray<TTuple<int, TFunction<void()>>> OneShotFunctionsToExecuteInTick;
public:
	static void AddOneShotTickFunction(TFunction<void()> InFunction, int InDelayFrameCount = 0);
private:
	static bool InitCheck(UWorld* InWorld);
	void SortDrawcallOnRenderMode(ELGUIRenderMode InRenderMode);
	TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> ScreenSpaceOverlayViewExtension;
public:
	static ULGUIEditorManagerObject* GetInstance(UWorld* InWorld, bool CreateIfNotValid = false);
	static bool IsSelected(AActor* InObject);
	static bool AnySelectedIsChildOf(AActor* InObject);
	void CheckEditorViewportIndexAndKey();
	uint32 GetViewportKeyFromIndex(int32 InViewportIndex);

	const TArray<TWeakObjectPtr<UUIItem>>& GetAllRootUIItemArray();
public:
	static void AddRootUIItem(UUIItem* InItem);
	static void RemoveRootUIItem(UUIItem* InItem);

	const TArray<TWeakObjectPtr<UUISelectableComponent>>& GetAllSelectableArray() { return AllSelectableArray; }
	static void AddSelectable(UUISelectableComponent* InSelectable);
	static void RemoveSelectable(UUISelectableComponent* InSelectable);

	static void AddCanvas(ULGUICanvas* InCanvas);
	static void RemoveCanvas(ULGUICanvas* InCanvas);
	TArray<TWeakObjectPtr<ULGUICanvas>>& GetCanvasArray() { return AllCanvasArray; };
	void MarkSortLGUIRenderer();
	void MarkSortWorldSpaceCanvas();
	void MarkSortRenderTargetSpaceCanvas();

	const TArray<TScriptInterface<ILGUILayoutInterface>>& GetAllLayoutArray()const { return AllLayoutArray; }

	static TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> GetViewExtension(UWorld* InWorld, bool InCreateIfNotExist);

	static void RegisterLGUILayout(TScriptInterface<ILGUILayoutInterface> InItem);
	static void UnregisterLGUILayout(TScriptInterface<ILGUILayoutInterface> InItem);

	/**
	 * Editor raycast hit all visible UIBaseRenderable object.
	 * \param InWorld
	 * \param InUIItems
	 * \param LineStart
	 * \param LineEnd
	 * \param ResultSelectTarget
	 * \param InOutTargetIndexInHitArray	Pass in desired item index, and result selected item index. Default is -1, will use first one as result.
	 * \return 
	 */
	static bool RaycastHitUI(UWorld* InWorld, const TArray<UUIItem*>& InUIItems, const FVector& LineStart, const FVector& LineEnd
		, UUIBaseRenderable*& ResultSelectTarget, int& InOutTargetIndexInHitArray
	);
private:
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TSet<AActor*> AllActors_PrefabSystemProcessing;
	/** Functions that wait for prefab serialization complete then execute */
	TArray<TFunction<void()>> LateFunctionsAfterPrefabSerialization;
	FDelegateHandle OnBlueprintCompiledDelegateHandle;
#endif
	void RefreshOnBlueprintCompiled();
public:
	static void BeginPrefabSystemProcessingActor(UWorld* InWorld, AActor* InRootActor);
	static void EndPrefabSystemProcessingActor(UWorld* InWorld, AActor* InRootActor);
	static void AddActorForPrefabSystem(AActor* InActor);
	static void RemoveActorForPrefabSystem(AActor* InActor);
	static bool IsPrefabSystemProcessingActor(AActor* InActor);
	/**
	 * Add a function that execute after prefab system serialization and before Awake called
	 * @param	InPrefabActor	Current prefab system processing actor
	 * @param	InFunction		Function to call after prefab system serialization complete and before Awake called
	 */
	static void AddFunctionForPrefabSystemExecutionBeforeAwake(AActor* InPrefabActor, const TFunction<void()>& InFunction);

	static void RefreshAllUI();

	static void MarkBroadcastLevelActorListChanged();
private:
	FDelegateHandle OnAssetReimportDelegateHandle;
	void OnAssetReimport(UObject* asset);
	FDelegateHandle OnActorLabelChangedDelegateHandle;
	void OnActorLabelChanged(AActor* actor);
	FDelegateHandle OnMapOpenedDelegateHandle;
	void OnMapOpened(const FString& FileName, bool AsTemplate);
	static UWorld* PreviewWorldForPrefabPackage;
public:
	static UWorld* GetPreviewWorldForPrefabPackage();
#endif
};

struct FLGUILifeCycleBehaviourArrayContainer
{
	TArray<TTuple<TWeakObjectPtr<ULGUILifeCycleBehaviour>, int32>> LGUILifeCycleBehaviourArray;
	/** Functions that wait for prefab serialization complete then execute */
	TArray<TFunction<void()>> Functions;
};

class ILGUICultureChangedInterface;

UCLASS(NotBlueprintable, NotBlueprintType, NotPlaceable)
class LGUI_API ALGUIManagerActor : public AActor
{
	GENERATED_BODY()
private:
	static TMap<UWorld*, ALGUIManagerActor*> WorldToInstanceMap;
	bool bExistInInstanceMap = false;
#if WITH_EDITORONLY_DATA
	bool bIsPlaying = false;
#endif
public:	
	static ALGUIManagerActor* GetLGUIManagerActorInstance(UObject* WorldContextObject);
	static ALGUIManagerActor* GetInstance(UWorld* InWorld, bool CreateIfNotValid = false);
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
		TArray<TWeakObjectPtr<ULGUIBaseRaycaster>> AllRaycasterArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TWeakObjectPtr<ULGUIBaseInputModule> CurrentInputModule = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<UUISelectableComponent>> AllSelectableArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TScriptInterface<ILGUILayoutInterface>> AllLayoutArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TScriptInterface<ILGUICultureChangedInterface>> AllCultureChangedArray;

	bool bShouldSortLGUIRenderer = true;
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

	void SortDrawcallOnRenderMode(ELGUIRenderMode InRenderMode);
	TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> ScreenSpaceOverlayViewExtension;

	void UpdateLayout();
	bool bNeedUpdateLayout = false;
public:

	UFUNCTION(BlueprintCallable, Category = "LGUI")
	static void RegisterLGUICultureChangedEvent(TScriptInterface<ILGUICultureChangedInterface> InItem);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	static void UnregisterLGUICultureChangedEvent(TScriptInterface<ILGUICultureChangedInterface> InItem);

	/** Force LGUI to update layout immediately */
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta=(WorldContext = "WorldContextObject"))
	static void ForceUpdateLayout(UObject* WorldContextObject);

	static void AddCanvas(ULGUICanvas* InCanvas);
	static void RemoveCanvas(ULGUICanvas* InCanvas);
	TArray<TWeakObjectPtr<ULGUICanvas>>& GetCanvasArray() { return AllCanvasArray; };
	void MarkSortLGUIRenderer();
	void MarkSortWorldSpaceCanvas();
	void MarkSortRenderTargetSpaceCanvas();

	const TArray<TScriptInterface<ILGUILayoutInterface>>& GetAllLayoutArray()const { return AllLayoutArray; }

	static TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> GetViewExtension(UWorld* InWorld, bool InCreateIfNotExist);

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
	static bool GetIsPlaying(UWorld* InWorld);
	static void DrawFrameOnUIItem(UUIItem* InItem, bool IsScreenSpace = false);
	static void DrawNavigationArrow(UWorld* InWorld, const TArray<FVector>& InControlPoints, const FVector& InArrowPointA, const FVector& InArrowPointB, FColor const& InColor, bool IsScreenSpace = false);
	static void DrawNavigationVisualizerOnUISelectable(UWorld* InWorld, UUISelectableComponent* InSelectable, bool IsScreenSpace = false);
private:
	static void DrawDebugBoxOnScreenSpace(UWorld* InWorld, FVector const& Center, FVector const& Box, const FQuat& Rotation, FColor const& Color);
#endif
private:
	/** Map actor to prefab's root actor */
	TMap<AActor*, TTuple<AActor*, int32>> AllActors_PrefabSystemProcessing;
	/** Map root actor to LGUILifeCycleBehaviour array */
	TMap<AActor*, FLGUILifeCycleBehaviourArrayContainer> LGUILifeCycleBehaviours_PrefabSystemProcessing;
public:
	void BeginPrefabSystemProcessingActor(AActor* InRootActor);
	void EndPrefabSystemProcessingActor(AActor* InRootActor);
	void AddActorForPrefabSystem(AActor* InActor, AActor* InRootActor, int32 InActorIndex);
	void RemoveActorForPrefabSystem(AActor* InActor, AActor* InRootActor);
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
