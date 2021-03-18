// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabActor.h"
#include "LGUI.h"


// Sets default values
ALGUIPrefabActor::ALGUIPrefabActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PrefabComponent = CreateDefaultSubobject<ULGUIPrefabHelperComponent>(TEXT("LGUIPrefab"));
	RootComponent = PrefabComponent;
	bIsEditorOnlyActor = true;
}

// Called when the game starts or when spawned
void ALGUIPrefabActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALGUIPrefabActor::Tick(float DeltaTime)
{
	Super::Tick( DeltaTime );

}

void ALGUIPrefabActor::Destroyed()
{
	Super::Destroyed();
}