// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "LexUIManager.generated.h"

struct FLexUIHelperGizmoRenderParameter;
struct FLexUIHelperGizmoVertex;
class ULexEventSystem;
class ULexWidget;
class ULexVisualBatchMesh;
class ULexVisual;
class ULexCanvas;
class ULexBaseRaycaster;
class UUISelectable;
class ULexUIBehaviour;
class ULexBaseInputModule;

DECLARE_MULTICAST_DELEGATE_OneParam(FLexUIEditorTickMulticastDelegate, float);

/**
 * This manager is a single instance, mainly for manage LexUI in Editor
 */
UCLASS(NotBlueprintable, NotBlueprintType, Transient, NotPlaceable)
class LGUI_API ULexUIManagerObject :public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	ULexUIManagerObject();
	virtual void BeginDestroy()override;
public:
	//begin TickableEditorObject interface
	virtual void Tick(float DeltaTime)override;
	virtual bool IsTickable() const { return Instance == this; }
	virtual bool IsTickableInEditor()const { return Instance == this; }
	virtual TStatId GetStatId() const override;
	virtual bool IsEditorOnly()const override { return true; }
	//end TickableEditorObject interface
private:
	static ULexUIManagerObject* Instance;
#if WITH_EDITORONLY_DATA
	static bool bIsBlueprintCompiling;
	FLexUIEditorTickMulticastDelegate EditorTick;
	TArray<TTuple<int, TFunction<void()>>> OneShotFunctionsToExecuteInTick;
public:
	static void AddOneShotTickFunction(const TFunction<void()>& InFunction, int InDelayFrameCount = 0);
	FLexUIEditorTickMulticastDelegate& GetEditorTickDelegate();

#endif
#if WITH_EDITOR
	static bool GetIsBlueprintCompiling(){return bIsBlueprintCompiling;}
private:
	static bool InitCheck();
public:
	static ULexUIManagerObject* GetInstance(bool CreateIfNotValid = false);
private:
	FDelegateHandle OnBlueprintPreCompileDelegateHandle;
	FDelegateHandle OnBlueprintCompiledDelegateHandle;
	void OnBlueprintPreCompile(UBlueprint* InBlueprint);
	void OnBlueprintCompiled();
private:
	FDelegateHandle OnAssetReimportDelegateHandle;
	void OnAssetReimport(UObject* Asset);
	FDelegateHandle OnMapOpenedDelegateHandle;
	void OnMapOpened(const FString& FileName, bool AsTemplate);
	FDelegateHandle OnPackageReloadedDelegateHandle;
	void OnPackageReloaded(EPackageReloadPhase Phase, FPackageReloadedEvent* Event);
#endif
};

UCLASS(NotBlueprintable, NotBlueprintType, Transient)
class LGUI_API ULexUISelection : public UObject
{
	GENERATED_BODY()

public:
	static ULexUISelection* GetInstance(UWorld* InWorld);
	virtual bool IsEditorOnly() const override{return true;}
	void SelectWidget(ULexWidget* Widget);
	void SelectComponent(ULexUIBehaviour* Component);
	void ClearComponentSelection();
	void SelectNone();
	bool IsSelected(ULexWidget* Widget)const;
	TArray<TWeakObjectPtr<ULexWidget>> GetSelectedWidgets()const{return SelectedWidgetArray;}
	TArray<TWeakObjectPtr<ULexUIBehaviour>> GetSelectedComponents()const{return SelectedComponentArray;}
	FSimpleMulticastDelegate OnSelectionChanged;
private:
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	TArray<TWeakObjectPtr<ULexWidget>> SelectedWidgetArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	TArray<TWeakObjectPtr<ULexUIBehaviour>> SelectedComponentArray;
};

class ILexUICultureChangedInterface;
enum class ELexRenderMode : uint8;

class FLexUILayoutTree
{
public:
	TArray<TObjectPtr<ULexWidget>> WidgetArray;
};

UCLASS(NotBlueprintable, NotBlueprintType, Transient)
class LGUI_API ULexUIManagerWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:	
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual void Initialize(FSubsystemCollectionBase& Collection)override;
	virtual void PostInitialize()override;
	virtual void Deinitialize()override;
	virtual void BeginDestroy() override;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void OnWorldEndPlay(UWorld& InWorld) override;

	virtual TStatId GetStatId() const override;
	virtual bool IsTickableInEditor()const override{ return false; }//use Ticker in editor, because Ticker can also tick when drag vector2/3 value while normal tick can't
	virtual void Tick(float DeltaTime)override;
	void TickLexUI(float DeltaTime);
	void OnWorldPreSendAllEndOfFrameUpdates(UWorld* InWorld);
#if WITH_EDITOR
	void DrawHelperGizmo();
#endif
	void SubmitCanvasDrawCall();
	virtual bool IsTickableWhenPaused() const override;
	bool HasBegunPlay()const{return bHasCalledBeginPlay;}

	static ULexUIManagerWorldSubsystem* GetInstance(UWorld* InWorld);
#if WITH_EDITOR
	bool bShouldTickInEditor = false;
	ULexUISelection* GetSelection()const;
	FSimpleMulticastDelegate OnDeinitialize;
	FSimpleMulticastDelegate OnEndPlay;
	FSimpleMulticastDelegate OnLexUIWidgetOutlinerChanged;
	void MarkLexUIWidgetOutlinerChanged();
private:
	bool bLexUIWidgetOutlinerChanged = true;
#endif
	
private:
#if WITH_EDITOR
	static TArray<ULexUIManagerWorldSubsystem*> InstanceArray;
	FTSTicker::FDelegateHandle EditorTickDelegateHandle;
#endif

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	mutable TObjectPtr<ULexUISelection> Selection;
#endif
	
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	TArray<TWeakObjectPtr<ULexCanvas>> AllCanvasArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	TArray<TObjectPtr<ULexWidget>> AllWidgetArray;

	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<ULexBaseRaycaster>> AllRaycasterArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<UUISelectable>> AllSelectableArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<UObject>> AllCultureChangedArray;

	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	TMap<int, TWeakObjectPtr<ULexEventSystem>> MapUserIndexToEventSystem;

	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<ULexUIBehaviour>> LexUIBehavioursForTick;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<ULexUIBehaviour>> LexUIBehavioursForStart;

	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	TArray<TWeakObjectPtr<ULexWidget>> LayoutDirtyWidgetArray;
	
	TMap<TObjectPtr<ULexWidget>, FLexUILayoutTree> MapWidgetToLayoutTree;

	bool bIsExecutingStart = false;
	bool bIsExecutingTick = false;
	int32 CurrentExecutingTickIndex = -1;
	UPROPERTY(Transient) TArray<ULexUIBehaviour*> LexUIBehavioursNeedToRemoveFromTick;
#if WITH_EDITORONLY_DATA
	int32 PrevScreenSpaceOverlayCanvasCount = 1;
	TMap<FString, int> LayoutCalculationCounterMap;
#endif
	void OnCultureChanged();
	bool bShouldUpdateOnCultureChanged = false;
	FDelegateHandle OnCultureChangedDelegateHandle;

	TSharedPtr<class FLexUIRenderer, ESPMode::ThreadSafe> MainViewportViewExtension;
public:
#if WITH_EDITOR
	static void RefreshAllUI(UWorld* InWorld = nullptr);
	FSimpleMulticastDelegate EventOnOutlineChanged;
#endif
	
	const TArray<TWeakObjectPtr<ULexCanvas>>& GetAllCanvasArray()const{return AllCanvasArray;}
	void AddCanvas(ULexCanvas* InCanvas);
	void RemoveCanvas(ULexCanvas* InCanvas);
	TArray<ULexCanvas*> GetCanvasArrayByRenderMode(ELexRenderMode RenderMode)const;

	const TArray<TObjectPtr<ULexWidget>>& GetAllWidgetArray()const{return AllWidgetArray;}
	void AddWidget(ULexWidget* InWidget);
	void RemoveWidget(ULexWidget* InWidget);

	void AddLayoutDirtyWidget(ULexWidget* InWidget);
	void MarkRebuildLayoutTree(ULexWidget* InWidget);
	void MarkRebuildAllLayoutTree();
	void RebuildLayoutImmediately(ULexWidget* InWidget);
	void CalculateLayoutTree(ULexWidget* RootLayoutWidget);
#if WITH_EDITOR
	int IncreateLayoutCalculationCounter(const FString& InPathName);
#endif

	UFUNCTION(BlueprintCallable, Category = "LGUI")
	static void RegisterLexUICultureChangedEvent(TScriptInterface<ILexUICultureChangedInterface> InItem);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	static void UnregisterLexUICultureChangedEvent(TScriptInterface<ILexUICultureChangedInterface> InItem);

	static TSharedPtr<class FLexUIRenderer, ESPMode::ThreadSafe> GetViewExtension(UWorld* InWorld, bool InCreateIfNotExist);

	const TArray<TWeakObjectPtr<ULexBaseRaycaster>>& GetAllRaycasterArray(){ return AllRaycasterArray; }
	static void AddRaycaster(ULexBaseRaycaster* InRaycaster);
	static void RemoveRaycaster(ULexBaseRaycaster* InRaycaster);

	const TArray<TWeakObjectPtr<UUISelectable>>& GetAllSelectableArray() { return AllSelectableArray; }
	static void AddSelectable(UUISelectable* InSelectable);
	static void RemoveSelectable(UUISelectable* InSelectable);

	const TMap<int, TWeakObjectPtr<ULexEventSystem>>& GetMapUserIndexToEventSystem() { return MapUserIndexToEventSystem; }
	ULexEventSystem* GetEventSystemByUserIndex(int UserIndex = 0);
	void AddEventSystem(ULexEventSystem* InEventSystem);
	void RemoveEventSystem(ULexEventSystem* InEventSystem);
	
#if WITH_EDITOR
	/**
	 * Editor raycast hit all visible UIBaseRenderable object.
	 * @param InWorld
	 * @param InWidgets
	 * @param LineStart
	 * @param LineEnd
	 * @param ResultSelectTarget
	 * @param InOutTargetIndexInHitArray	Pass in desired item index, and result selected item index. Default is -1, will use first one as result.
	 * \return 
	 */
	static bool RaycastHitUI(UWorld* InWorld, const TArray<ULexWidget*>& InWidgets, const FVector& LineStart, const FVector& LineEnd
		, ULexWidget*& ResultSelectTarget, int& InOutTargetIndexInHitArray
	);
	void DrawFrameOnWidget(ULexWidget* InItem, bool ScreenOrWorld = false);
	void DrawNavigationArrow(UWorld* InWorld, const TArray<FVector>& InControlPoints, const FVector& InArrowPointA, const FVector& InArrowPointB, FColor const& InColor, void* Object, const FString& DebugName, bool ScreenOrWorld = false);
	void DrawNavigationVisualizerOnUISelectable(UWorld* InWorld, UUISelectable* InSelectable, bool IsScreenSpace = false);
	FEditorViewportClient* GetEditorViewportClient();
	static UMaterialInterface* GetDefaultGizmoMaterial();
	
	static void DrawDebugRect(UWorld* InWorld, const FVector& Center, const FMatrix& LocalToWorld, FVector2D const& Rect, FColor const& Color, void* Object, const FString& DebugName, bool ScreenOrWorld);
	static void DrawDebugBox(UWorld* InWorld, const FVector& Center, const FMatrix& LocalToWorld, FVector const& Box, FColor const& Color, void* Object, const FString& DebugName, bool ScreenOrWorld);
	static void DrawDebugLine(UWorld* InWorld, const FMatrix& LocalToWorld, const TArray<FVector3f>& LinePoints, FColor const& Color, void* Object, const FString& DebugName, bool ScreenOrWorld);
private:
	//this is cached when call GetEditorViewportClient
	FEditorViewportClient* CacheViewportClient = nullptr;
	void OnEndOfFrame();
	void OnEnginePreExit();
#endif
public:

	static void AddLexUIBehavioursForTick(ULexUIBehaviour* InComp);
	static void RemoveLexUIBehavioursFromTick(ULexUIBehaviour* InComp);
	static void AddLexUIBehavioursForStart(ULexUIBehaviour* InComp);
	static void RemoveLexUIBehavioursFromStart(ULexUIBehaviour* InComp);
};
