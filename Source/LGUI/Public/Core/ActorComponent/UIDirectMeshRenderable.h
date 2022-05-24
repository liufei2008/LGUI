// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseRenderable.h"
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

	virtual void UpdateGeometry()override;

	virtual void OnAnchorChange(bool InPivotChange, bool InSizeChange, bool InDiscardCache = true)override;

	void MarkVertexPositionDirty();

	virtual void MarkAllDirty()override;

	virtual bool LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)override;
public:
	/** Called by LGUICanvas when this UI element have valid mesh data. */
	virtual void OnMeshDataReady();
	virtual TWeakPtr<FLGUIMeshSection> GetMeshSection()const;
	virtual TWeakObjectPtr<ULGUIMeshComponent> GetUIMesh()const;
	virtual void ClearMeshData();
	virtual bool HaveValidData()const PURE_VIRTUAL(UUIDirectMeshRenderable::HaveValidData, return true;);
	virtual UMaterialInterface* GetMaterial()const PURE_VIRTUAL(UUIDirectMeshRenderable::GetMaterial, return nullptr;);

	virtual void SetClipType(ELGUICanvasClipType clipType) {};
	virtual void SetRectClipParameter(const FVector4& OffsetAndSize, const FVector4& Feather) {};
	virtual void SetTextureClipParameter(UTexture* ClipTex, const FVector4& OffsetAndSize) {};
protected:
	uint8 bLocalVertexPositionChanged : 1;
};
