// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseRenderable.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "UIDirectMeshRenderable.generated.h"

struct FLGUIMeshSection;
class ULGUIMeshComponent;
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

	virtual void OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)override;
	virtual void UpdateGeometry()override;

	virtual void OnAnchorChange(bool InPivotChange, bool InSizeChange, bool InDiscardCache = true)override;

	void MarkVertexPositionDirty();

	virtual void MarkAllDirtyRecursive()override;

	virtual bool LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)override;

	TWeakObjectPtr<ULGUIMeshComponent> UIMesh = nullptr;
	TWeakPtr<FLGUIMeshSection> MeshSection = nullptr;
public:
	/** Canvas will create a UIDrawcallMesh for this UI element. */
	virtual void SetMeshData(TWeakObjectPtr<ULGUIMeshComponent> InUIMesh, TWeakPtr<FLGUIMeshSection> InMeshSection);
	virtual TWeakPtr<FLGUIMeshSection> GetMeshSection()const;
	virtual TWeakObjectPtr<ULGUIMeshComponent> GetUIMesh()const;
	virtual void ClearMeshData();

	virtual void SetClipType(ELGUICanvasClipType clipType) {};
	virtual void SetRectClipParameter(const FVector4& OffsetAndSize, const FVector4& Feather) {};
	virtual void SetTextureClipParameter(UTexture* ClipTex, const FVector4& OffsetAndSize) {};
protected:
	uint8 bLocalVertexPositionChanged : 1;
};
