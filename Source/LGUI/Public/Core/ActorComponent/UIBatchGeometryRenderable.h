﻿// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseRenderable.h"
#include "UIBatchGeometryRenderable.generated.h"

class UIGeometry;
class UMaterialInterface;
class ULGUICanvas;
class ULGUIMeshComponent;
/** UI element which have render geometry, and can be batched and renderred by LGUICanvas */
UCLASS(Abstract, NotBlueprintable)
class LGUI_API UUIBatchGeometryRenderable : public UUIBaseRenderable
{
	GENERATED_BODY()

public:	
	UUIBatchGeometryRenderable(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnUnregister()override;

	TSharedPtr<UIGeometry> geometry = nullptr;

	/** if have GeometryModifier component */
	bool HaveGeometryModifier();
	/** 
	 * use GeometryModifier to modify geometry 
	 * return: true if the modifier change the triangle count, else false
	 * @todo: return value contains triangle change and vertex change
	 */
	bool ApplyGeometryModifier(bool uvChanged, bool colorChanged, bool vertexPositionChanged, bool layoutChanged);
	TInlineComponentArray<class UUIGeometryModifierBase*> GeometryModifierComponentArray;

	virtual void ApplyUIActiveState() override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UMaterialInterface* GetCustomUIMaterial()const { return CustomUIMaterial; }
	/** 
	 * if inMat is a UMaterialInstanceDynamic, then it will directly use for render.
	 * if not, then a new MaterialInstanceDynamic will be created to render this UI item, and the created MaterialInstanceDynamic may shared with others UI items.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetCustomUIMaterial(UMaterialInterface* inMat);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetRaycastComplex() { return bRaycastComplex; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRaycastComplex(bool newValue) { bRaycastComplex = newValue; }
	/** 
	 * if CustomUIMaterial is a UMaterialInstanceDynamic, then will return it directly.
	 * if not, then return a created MaterialInstanceDynamic that renderring this UI item, may shared by other UI item. if this UI item is not renderred yet, then return nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UMaterialInstanceDynamic* GetMaterialInstanceDynamic()const;

	virtual void OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)override;
	virtual void WidthChanged()override;
	virtual void HeightChanged()override;
	virtual void PivotChanged()override;

	void MarkVertexPositionDirty();
	void MarkUVDirty();
	void MarkTriangleDirty();
	void MarkTextureDirty();
	void MarkMaterialDirty();

	void AddGeometryModifier(class UUIGeometryModifierBase* InModifier);
	void RemoveGeometryModifier(class UUIGeometryModifierBase* InModifier);

	virtual void UpdateBasePrevData()override;
	virtual void UpdateCachedData()override;
	virtual void UpdateCachedDataBeforeGeometry()override;
	virtual void MarkAllDirtyRecursive()override;
	TSharedPtr<UIGeometry> GetGeometry()const { return geometry; }

	virtual bool LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)override;
protected:
	friend class FUIGeometryRenderableCustomization;
	/** Use custom material to render this element */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayThumbnail = "false"))
		UMaterialInterface* CustomUIMaterial = nullptr;
	/** Only valid if RaycastTarget is true. true - linetrace hit real mesh triangles, false - linetrace hit widget rectangle */
	UPROPERTY(EditAnywhere, Category = "LGUI-Raycast")
		bool bRaycastComplex = false;

	/** do we have valid data to create geometry? */
	virtual bool HaveDataToCreateGeometry() { return true; }
	/** do we need texture to create geometry? */
	virtual bool NeedTextureToCreateGeometry() { return false; }
	/** if NeedTextureToCreateGeometry() is true, then we should provide this texture */
	virtual UTexture* GetTextureToCreateGeometry() { return nullptr; }

	/** do anything before acturally create or update geometry */
	virtual void OnBeforeCreateOrUpdateGeometry()PURE_VIRTUAL(UUIBatchGeometryRenderable::OnBeforeCreateOrUpdateGeometry, );
	/** create ui geometry */
	virtual void OnCreateGeometry()PURE_VIRTUAL(UUIBatchGeometryRenderable::OnCreateGeometry, );
	/** update ui geometry */
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)PURE_VIRTUAL(UUIBatchGeometryRenderable::OnUpdateGeometry, );

	bool CreateGeometry();
	virtual void UpdateLayout(bool& parentLayoutChanged, bool shouldUpdateLayout)override;
	virtual void UpdateGeometry()override final;
private:
	void UpdateGeometry_Implement();
	/** local vertex position changed */
	uint8 bLocalVertexPositionChanged : 1;
	/** vertex's uv change */
	uint8 bUVChanged:1;
	/** triangle index change */
	uint8 bTriangleChanged:1;

	uint8 cacheForThisUpdate_LocalVertexPositionChanged:1, cacheForThisUpdate_UVChanged:1, cacheForThisUpdate_TriangleChanged:1;
};
