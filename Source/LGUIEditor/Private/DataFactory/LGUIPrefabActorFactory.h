// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "ActorFactories/ActorFactory.h"
#include "LGUIPrefabActorFactory.generated.h"

/** Create a agent actor and use it to spawn the actual prefab. */
UCLASS()
class ULGUIPrefabActorFactory : public UActorFactory
{
	GENERATED_BODY()
public:
	ULGUIPrefabActorFactory();
	//~ Begin UActorFactory
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	virtual bool PreSpawnActor(UObject* Asset, FTransform& InOutLocation) override;
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual void PostCreateBlueprint(UObject* Asset, AActor* CDO) override;
	virtual UObject* GetAssetFromActorInstance(AActor* ActorInstance) override;
	//virtual FQuat AlignObjectToSurfaceNormal(const FVector& InSurfaceNormal, const FQuat& ActorRotation) const override;
	//~ End UActorFactory

};


class ULGUIPrefab;

UCLASS(ClassGroup = (LGUI), NotBlueprintable, NotBlueprintType, NotPlaceable, HideCategories = (Rendering, Actor, Input))
class ALGUIPrefabLoadHelperActor : public AActor
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	ALGUIPrefabLoadHelperActor();

	virtual void BeginPlay()override;
	virtual void Destroyed()override;
	virtual void BeginDestroy() override;

public:
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TObjectPtr<ULGUIPrefab> PrefabAsset = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TObjectPtr<AActor> LoadedRootActor = nullptr;
	void LoadPrefab(USceneComponent* InParent);
	void MoveActorToPrefabFolder();

public:
	bool bAutoDestroyLoadedActors = true;
};

