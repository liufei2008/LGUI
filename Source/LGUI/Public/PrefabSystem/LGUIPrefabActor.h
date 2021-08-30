// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "LGUIPrefabActor.generated.h"

class ULGUIPrefabHelperComponent;

UCLASS(ConversionRoot, ComponentWrapperClass, NotBlueprintable, NotBlueprintType, HideCategories = (Rendering, Actor, Input))
class LGUI_API ALGUIPrefabActor : public AActor
{
	GENERATED_BODY()
	
	
public:	
	// Sets default values for this actor's properties
	ALGUIPrefabActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void Destroyed() override;

	ULGUIPrefabHelperComponent* GetPrefabComponent(){return PrefabComponent;}
private:
	UPROPERTY(Category = LGUIPrefab, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		ULGUIPrefabHelperComponent* PrefabComponent;
};
