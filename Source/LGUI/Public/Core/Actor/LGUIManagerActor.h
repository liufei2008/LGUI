// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "Core/LGUISpriteData.h"
#include "Tickable.h"
#include "LGUIManagerActor.generated.h"

class UUIItem;
class ULGUICanvas;
class ULGUIBaseRaycaster;
class UUISelectableComponent;

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
	FORCEINLINE static void AddRaycaster(ULGUIBaseRaycaster* InRaycaster);
	FORCEINLINE static void RemoveRaycaster(ULGUIBaseRaycaster* InRaycaster);

	FORCEINLINE const TArray<UUISelectableComponent*>& GetSelectables() { return allSelectableArray; }
	static void AddSelectable(UUISelectableComponent* InSelectable);
	static void RemoveSelectable(UUISelectableComponent* InSelectable);
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
