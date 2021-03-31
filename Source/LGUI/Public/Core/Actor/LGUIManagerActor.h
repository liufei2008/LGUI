// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Core/LGUISpriteData.h"
#include "Tickable.h"
#include "LGUIManagerActor.generated.h"

class UUIItem;
class UUIRenderable;
class ULGUICanvas;
class ULGUIBaseRaycaster;
class UUISelectableComponent;
class ULGUIBehaviour;
class ULGUIBaseInputModule;

DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIEditorTickMulticastDelegate, float);

UCLASS(NotBlueprintable, NotBlueprintType, Transient)
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
	static bool CanExecuteSelectionConvert;
private:
	TMap<int32, uint32> EditorViewportIndexToKeyMap;
	int32 PrevEditorViewportCount = 0;
public:
	int32 CurrentActiveViewportIndex = 0;
	uint32 CurrentActiveViewportKey = 0;
protected:
	//collection of all UIItem from current level
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<UUIItem>> allUIItem;
	/** Collect all canvas, and sort by order. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<ULGUICanvas>> allCanvas;
	TArray<UUIItem*> tempUIItemArray;
	TArray<ULGUICanvas*> tempCanvasArray;
#endif
#if WITH_EDITOR
private:
	static bool InitCheck(UWorld* InWorld);
public:
	static ULGUIEditorManagerObject* GetInstance(UWorld* InWorld, bool CreateIfNotValid = false);
	static bool IsSelected(AActor* InObject);
	static bool AnySelectedIsChildOf(AActor* InObject);
	void CheckEditorViewportIndexAndKey();
	uint32 GetViewportKeyFromIndex(int32 InViewportIndex);
public:
	static void AddUIItem(UUIItem* InItem);
	static void RemoveUIItem(UUIItem* InItem);
	const TArray<UUIItem*>& GetAllUIItem();

	static void AddCanvas(ULGUICanvas* InCanvas);
	static void SortCanvasOnOrder();
	static void RemoveCanvas(ULGUICanvas* InCanvas);
	const TArray<ULGUICanvas*>& GetAllCanvas();

private:
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<AActor>> AllActors_PrefabSystemProcessing;
#endif
public:
	static void BeginPrefabSystemProcessingActor(UWorld* InWorld);
	static void EndPrefabSystemProcessingActor();
	static void AddActorForPrefabSystem(AActor* InActor);
	static void RemoveActorForPrefabSystem(AActor* InActor);
	static bool IsPrefabSystemProcessingActor(AActor* InActor);

	static void RefreshAllUI();
private:
	bool IsCalculatingSelection = false;
	FDelegateHandle OnSelectionChangedDelegateHandle;
	TArray<FHitResult> CacheHitResultArray;
	TWeakObjectPtr<UUIRenderable> LastSelectTarget;
	TWeakObjectPtr<AActor> LastSelectedActor;
	void OnSelectionChanged(UObject* newSelection);
	FDelegateHandle OnAssetReimportDelegateHandle;
	void OnAssetReimport(UObject* asset);
	FDelegateHandle OnActorLabelChangedDelegateHandle;
	void OnActorLabelChanged(AActor* actor);
#if 0
	void LogObjectFlags(UObject* obj);
#endif
#endif
};

USTRUCT()
struct FLGUIBehaviourArrayContainer
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = LGUI)
		TArray<TWeakObjectPtr<ULGUIBehaviour>> LGUIBehaviourArray;
};

UCLASS(NotBlueprintable, NotBlueprintType, notplaceable)
class LGUI_API ALGUIManagerActor : public AActor
{
	GENERATED_BODY()
private:
	static TMap<UWorld*, ALGUIManagerActor*> WorldToInstanceMap;
	bool existInInstanceMap = false;
public:	
	static ALGUIManagerActor* GetLGUIManagerActorInstance(UObject* WorldContextObject);
#if WITH_EDITORONLY_DATA
	static bool IsPlaying;//@todo: this should relate to world
#endif
	ALGUIManagerActor();
	virtual void BeginPlay()override;
	virtual void BeginDestroy()override;
	virtual void Tick(float DeltaTime)override;
protected:
	//collection of all UIItem from current level
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<UUIItem*> allUIItem;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<ULGUICanvas*> allCanvas;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<ULGUIBaseRaycaster*> raycasterArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		ULGUIBaseInputModule* currentInputModule = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<UUISelectableComponent*> allSelectableArray;

	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<ULGUIBehaviour>> LGUIBehavioursForUpdate;

	bool firstAwakeExecuted = false;
private:
	static ALGUIManagerActor* GetInstance(UWorld* InWorld, bool CreateIfNotValid = false);
public:
	static void AddUIItem(UUIItem* InItem);
	static void RemoveUIItem(UUIItem* InItem);
	FORCEINLINE const TArray<UUIItem*>& GetAllUIItem(){ return allUIItem; }

	static void AddCanvas(ULGUICanvas* InCanvas);
	static void SortCanvasOnOrder(ULGUICanvas* InCanvas);
	static void RemoveCanvas(ULGUICanvas* InCanvas);
	FORCEINLINE const TArray<ULGUICanvas*>& GetAllCanvas(){ return allCanvas; }

	FORCEINLINE const TArray<ULGUIBaseRaycaster*>& GetRaycasters(){ return raycasterArray; }
	static void AddRaycaster(ULGUIBaseRaycaster* InRaycaster);
	static void RemoveRaycaster(ULGUIBaseRaycaster* InRaycaster);

	FORCEINLINE ULGUIBaseInputModule* GetInputModule() { return currentInputModule; }
	static void SetInputModule(ULGUIBaseInputModule* InInputModule);
	static void ClearInputModule(ULGUIBaseInputModule* InInputModule);

	FORCEINLINE const TArray<UUISelectableComponent*>& GetSelectables() { return allSelectableArray; }
	static void AddSelectable(UUISelectableComponent* InSelectable);
	static void RemoveSelectable(UUISelectableComponent* InSelectable);

private:
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<AActor*> AllActors_PrefabSystemProcessing;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<FLGUIBehaviourArrayContainer> LGUIBehaviours_PrefabSystemProcessing;
	int PrefabSystemProcessing_CurrentArrayIndex = -1;
	void EndPrefabSystemProcessingActor_Implement();
public:
	static void BeginPrefabSystemProcessingActor(UWorld* InWorld);
	static void EndPrefabSystemProcessingActor(UWorld* InWorld);
	static void AddActorForPrefabSystem(AActor* InActor);
	static void RemoveActorForPrefabSystem(AActor* InActor);
	static bool IsPrefabSystemProcessingActor(AActor* InActor);

	static void AddLGUIComponentForLifecycleEvent(ULGUIBehaviour* InComp);
	static void AddLGUIBehavioursForUpdate(ULGUIBehaviour* InComp);
	static void RemoveLGUIBehavioursFromUpdate(ULGUIBehaviour* InComp);
	static void ProcessLGUIComponentLifecycleEvent(ULGUIBehaviour* InComp);

	void Tick_PrePhysics();
	void Tick_DuringPhysics(float deltaTime);
};
//UCLASS(ClassGroup=(LGUI), NotBlueprintable)
//class LGUI_API ULGUIManagerComponent_PrePhysics : public UActorComponent
//{
//	GENERATED_BODY()
//public:
//	ULGUIManagerComponent_PrePhysics();
//	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
//	UPROPERTY(VisibleAnywhere, Category = "LGUI")
//		ALGUIManagerActor* ManagerActor;
//};
UCLASS(ClassGroup = (LGUI), NotBlueprintable)
class LGUI_API ULGUIManagerComponent_DuringPhysics : public UActorComponent
{
	GENERATED_BODY()
public:
	ULGUIManagerComponent_DuringPhysics();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		ALGUIManagerActor* ManagerActor;
};
