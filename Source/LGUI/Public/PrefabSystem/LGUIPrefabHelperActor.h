// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "LGUIPrefabHelperActor.generated.h"

class ULGUIPrefabHelperObject;

UCLASS(NotBlueprintable, NotBlueprintType, HideCategories = (Rendering, Actor, Input))
class LGUI_API ALGUIPrefabHelperActor : public AActor
{
	GENERATED_BODY()
	
	
public:	
	// Sets default values for this actor's properties
	ALGUIPrefabHelperActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Destroyed() override;

public:
#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void LoadPrefab(USceneComponent* InParent);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SavePrefab();
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void RevertPrefab();
	//delete this prefab actor
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void DeleteThisInstance();
	void MoveActorToPrefabFolder();
#endif

#if WITH_EDITORONLY_DATA
public:
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		ULGUIPrefabHelperObject* PrefabHelperObject;
	FColor IdentityColor = FColor::Black;
	bool IsRandomColor = true;
	bool AutoDestroyLoadedActors = true;
	static TArray<FColor> AllColors;
private:
	static FName PrefabFolderName;
#endif
};
