// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Core/LGUISpriteData.h"
#include "Tickable.h"
#include "LGUIManagerActor.generated.h"

class UUIItem;
class ULGUICanvas;
class ULGUIBaseRaycaster;
class UUISelectableComponent;
class ULGUIBehaviour;

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
	//end TickableEditorObject interface
	FLGUIEditorTickMulticastDelegate EditorTick;
protected:
	//collection of all UIItem from current level
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<UUIItem*> allUIItem;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<ULGUICanvas*> allCanvas;
private:
	static bool InitCheck(UWorld* InWorld);
#if WITH_EDITOR
public:
	static bool IsSelected(AActor* InObject);
#endif
public:
	static void AddUIItem(UUIItem* InItem);
	static void RemoveUIItem(UUIItem* InItem);
	FORCEINLINE const TArray<UUIItem*>& GetAllUIItem(){ return allUIItem; }

	static void AddCanvas(ULGUICanvas* InCanvas);
	static void SortCanvasOnOrder();
	static void RemoveCanvas(ULGUICanvas* InCanvas);
	FORCEINLINE const TArray<ULGUICanvas*>& GetAllCanvas(){ return allCanvas; }
};

UCLASS(NotBlueprintable, NotBlueprintType, notplaceable)
class LGUI_API ALGUIManagerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	static ALGUIManagerActor* Instance;
	ALGUIManagerActor();
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
		TArray<UUISelectableComponent*> allSelectableArray;

	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<ULGUIBehaviour*> uiComponentsForAwake;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<ULGUIBehaviour*> uiComponentsForEnable;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<ULGUIBehaviour*> uiComponentsForStart;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<ULGUIBehaviour*> uiComponentsForUpdate;
private:
	static bool InitCheck(UWorld* InWorld);
	
public:
	static void AddUIItem(UUIItem* InItem);
	static void RemoveUIItem(UUIItem* InItem);
	FORCEINLINE const TArray<UUIItem*>& GetAllUIItem(){ return allUIItem; }

	static void AddCanvas(ULGUICanvas* InCanvas);
	static void SortCanvasOnOrder();
	static void RemoveCanvas(ULGUICanvas* InCanvas);
	FORCEINLINE const TArray<ULGUICanvas*>& GetAllCanvas(){ return allCanvas; }

	FORCEINLINE const TArray<ULGUIBaseRaycaster*>& GetRaycasters(){ return raycasterArray; }
	static void AddRaycaster(ULGUIBaseRaycaster* InRaycaster);
	static void RemoveRaycaster(ULGUIBaseRaycaster* InRaycaster);

	FORCEINLINE const TArray<UUISelectableComponent*>& GetSelectables() { return allSelectableArray; }
	static void AddSelectable(UUISelectableComponent* InSelectable);
	static void RemoveSelectable(UUISelectableComponent* InSelectable);

	static void AddLGUIComponent(ULGUIBehaviour* InComp);
	static void RemoveLGUIComponent(ULGUIBehaviour* InComp);

	void Tick_PrePhysics();
	void Tick_DuringPhysics(float deltaTime);
};
UCLASS(ClassGroup=(LGUI), NotBlueprintable)
class LGUI_API ULGUIManagerComponent_PrePhysics : public UActorComponent
{
	GENERATED_BODY()
public:
	ULGUIManagerComponent_PrePhysics();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		ALGUIManagerActor* ManagerActor;
};
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

class LGUI_API LGUIManager
{
public:
	static bool IsManagerValid(UWorld* InWorld);

	static void AddUIItem(UUIItem* InItem);
	static void RemoveUIItem(UUIItem* InItem);
	static const TArray<UUIItem*>& GetAllUIItem(UWorld* InWorld);

	static void AddCanvas(ULGUICanvas* InCanvas);
	static void SortCanvasOnOrder(UWorld* InWorld);
	static void RemoveCanvas(ULGUICanvas* InCanvas);
	static const TArray<ULGUICanvas*>& GetAllCanvas(UWorld* InWorld);
#if WITH_EDITOR
	static bool IsSelected_Editor(AActor* InItem);
#endif
};
