// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "PrefabScene.h"

class UStaticMeshComponent;
class UExponentialHeightFogComponent;
class ULGUIPrefab;
class AActor;

//Encapsulates a simple scene setup for Prefab Editor.
class FLGUIPrefabEditorScene : public FPrefabScene
{
public:
	FLGUIPrefabEditorScene(ConstructionValues CVS);

	static const FString RootAgentActorName;
	USceneComponent* GetParentComponentForPrefab(ULGUIPrefab* InPrefab);
	AActor* GetRootAgentActor()const { return RootAgentActor; }

	bool IsWorldDefaultActor(AActor* InActor)const;
private:

	/** Editor accessory components **/
	UStaticMeshComponent* m_EditorFloorComp;
	UStaticMeshComponent* m_EditorSkyComp;
	UExponentialHeightFogComponent* m_EditorHeightFogComponent;

	TArray<AActor*> WorldDefaultActors;

	AActor* RootAgentActor = nullptr;
};