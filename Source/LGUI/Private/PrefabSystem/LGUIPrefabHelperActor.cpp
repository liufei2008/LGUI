// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "PrefabSystem/LGUIPrefabHelperComponent.h"
#include "LGUI.h"


// Sets default values
ALGUIPrefabHelperActor::ALGUIPrefabHelperActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PrefabComponent = CreateDefaultSubobject<ULGUIPrefabHelperComponent>(TEXT("LGUIPrefab"));
	RootComponent = PrefabComponent;
	bIsEditorOnlyActor = true;
}

// Called when the game starts or when spawned
void ALGUIPrefabHelperActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALGUIPrefabHelperActor::Tick(float DeltaTime)
{
	Super::Tick( DeltaTime );

}

void ALGUIPrefabHelperActor::Destroyed()
{
	Super::Destroyed();
}