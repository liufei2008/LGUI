// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "ActorFactories/ActorFactory.h"
#include "LGUIPrefabActorFactory.generated.h"

UCLASS()
class ULGUIPrefabActorFactory : public UActorFactory
{
	GENERATED_BODY()
public:
	ULGUIPrefabActorFactory();
	//~ Begin UActorFactory
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	virtual bool PreSpawnActor(UObject* Asset, FTransform& InOutLocation) override;
	//virtual AActor* SpawnActor(UObject* Asset, ULevel* InLevel, const FTransform& Transform, EObjectFlags InObjectFlags, const FName Name);
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual void PostCreateBlueprint(UObject* Asset, AActor* CDO) override;
	virtual UObject* GetAssetFromActorInstance(AActor* ActorInstance) override;
	//virtual FQuat AlignObjectToSurfaceNormal(const FVector& InSurfaceNormal, const FQuat& ActorRotation) const override;
	//~ End UActorFactory

};
