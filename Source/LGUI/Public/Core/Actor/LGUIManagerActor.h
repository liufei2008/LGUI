// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "Core/LGUISpriteData.h"
#include "Tickable.h"
#include "LGUIManagerActor.generated.h"

class UUIItem;
class ULGUICanvas;
class ULGUIBaseRaycaster;

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
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<ULGUIBaseRaycaster*> raycasterArray;
private:
	static bool InitCheck(UWorld* InWorld);
#if WITH_EDITOR
public:
	static bool IsSelected(AActor* InObject);
#endif
public:
	FORCEINLINE static void AddUIItem(UUIItem* InItem);
	FORCEINLINE static void RemoveUIItem(UUIItem* InItem);
	FORCEINLINE const TArray<UUIItem*>& GetAllUIItem();

	FORCEINLINE static void AddCanvas(ULGUICanvas* InCanvas);
	FORCEINLINE static void SortCanvasOnOrder();
	FORCEINLINE static void RemoveCanvas(ULGUICanvas* InCanvas);
	FORCEINLINE const TArray<ULGUICanvas*>& GetAllCanvas();
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
private:
	static bool InitCheck(UWorld* InWorld);
	
public:
	FORCEINLINE static void AddUIItem(UUIItem* InItem);
	FORCEINLINE static void RemoveUIItem(UUIItem* InItem);
	FORCEINLINE const TArray<UUIItem*>& GetAllUIItem();

	FORCEINLINE static void AddCanvas(ULGUICanvas* InCanvas);
	FORCEINLINE static void SortCanvasOnOrder();
	FORCEINLINE static void RemoveCanvas(ULGUICanvas* InCanvas);
	FORCEINLINE const TArray<ULGUICanvas*>& GetAllCanvas();

	FORCEINLINE const TArray<ULGUIBaseRaycaster*>& GetRaycasters();
	FORCEINLINE static void AddRaycaster(ULGUIBaseRaycaster* InRaycaster);
	FORCEINLINE static void RemoveRaycaster(ULGUIBaseRaycaster* InRaycaster);
};

class LGUI_API LGUIManager
{
public:
	static bool IsManagerValid(UWorld* InWorld);

	FORCEINLINE static void AddUIItem(UUIItem* InItem);
	FORCEINLINE static void RemoveUIItem(UUIItem* InItem);
	FORCEINLINE static const TArray<UUIItem*>& GetAllUIItem(UWorld* InWorld);

	FORCEINLINE static void AddCanvas(ULGUICanvas* InCanvas);
	FORCEINLINE static void SortCanvasOnOrder(UWorld* InWorld);
	FORCEINLINE static void RemoveCanvas(ULGUICanvas* InCanvas);
	static const TArray<ULGUICanvas*>& GetAllCanvas(UWorld* InWorld);
#if WITH_EDITOR
	static bool IsSelected_Editor(AActor* InItem);
#endif
};
