// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseRenderable.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "UIDirectMeshRenderable.generated.h"

class UUIDrawcallMesh;
/** 
 * UI element that use mesh to render directly
 */
UCLASS(Abstract, NotBlueprintable)
class LGUI_API UUIDirectMeshRenderable : public UUIBaseRenderable
{
	GENERATED_BODY()

public:	
	UUIDirectMeshRenderable(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnUnregister()override;
	virtual void ApplyUIActiveState() override;

	virtual void OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)override;
	virtual void UpdateGeometry(const bool& parentLayoutChanged)override final;
public:
	virtual void SetDrawcallMesh(UUIDrawcallMesh* InUIDrawcallMesh) {};
};
