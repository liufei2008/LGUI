// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/UObjectGlobals.h"
#include "HitProxies.h"

//this file mostly reference from "UnrealEd/public/LevelViewportClickHandlers.h"

class AActor;
class ABrush;
class FLGUIPrefabEditorViewportClient;
class UModel;
struct FTypedElementHandle;
struct FViewportClick;
struct HActor;

namespace LGUIPrefabViewportClickHandlers
{
	bool ClickViewport(FLGUIPrefabEditorViewportClient* ViewportClient, const FViewportClick& Click);

	bool ClickElement(FLGUIPrefabEditorViewportClient* ViewportClient, const FTypedElementHandle& HitElement, const FViewportClick& Click);

	bool ClickActor(FLGUIPrefabEditorViewportClient* ViewportClient,AActor* Actor,const FViewportClick& Click,bool bAllowSelectionChange);

	bool ClickComponent(FLGUIPrefabEditorViewportClient* ViewportClient, HActor* ActorHitProxy, const FViewportClick& Click);

	void ClickBrushVertex(FLGUIPrefabEditorViewportClient* ViewportClient,ABrush* InBrush,FVector* InVertex,const FViewportClick& Click);

	void ClickStaticMeshVertex(FLGUIPrefabEditorViewportClient* ViewportClient,AActor* InActor,FVector& InVertex,const FViewportClick& Click);
	
	void ClickSurface(FLGUIPrefabEditorViewportClient* ViewportClient, UModel* Model, int32 iSurf, const FViewportClick& Click);

	void ClickBackdrop(FLGUIPrefabEditorViewportClient* ViewportClient,const FViewportClick& Click);

	void ClickLevelSocket(FLGUIPrefabEditorViewportClient* ViewportClient, HHitProxy* HitProxy, const FViewportClick& Click);
};


