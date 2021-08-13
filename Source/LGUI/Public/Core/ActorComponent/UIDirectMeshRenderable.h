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
	virtual void UpdateGeometry(const bool& parentLayoutChanged)override;

	virtual void WidthChanged()override;
	virtual void HeightChanged()override;
	virtual void PivotChanged()override;

	void MarkVertexPositionDirty();

	virtual void UpdateBasePrevData()override;
	virtual void UpdateCachedData()override;
	virtual void UpdateCachedDataBeforeGeometry()override;
	virtual void MarkAllDirtyRecursive()override;

	TWeakObjectPtr<UUIDrawcallMesh> UIDrawcallMesh = nullptr;
public:
	/** Canvas will create a UIDrawcallMesh for this UI element. */
	virtual void SetDrawcallMesh(UUIDrawcallMesh* InUIDrawcallMesh);
	virtual UUIDrawcallMesh* GetDrawcallMesh()const;
	virtual void ClearDrawcallMesh(bool InDestroyMesh);

	virtual void SetClipType(ELGUICanvasClipType clipType) {};
	virtual void SetRectClipParameter(const FVector4& OffsetAndSize, const FVector4& Feather) {};
	virtual void SetTextureClipParameter(UTexture* ClipTex, const FVector4& OffsetAndSize) {};
protected:
	uint8 bLocalVertexPositionChanged : 1;
	uint8 cacheForThisUpdate_LocalVertexPositionChanged : 1;
};
