// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "UIItem.h"
#include "UIRenderable.generated.h"

class UIGeometry;
class UMaterialInterface;
class ULGUICanvas;
//UI element which have render geometry, and can be renderred by LGUICanvas
UCLASS(Abstract, NotBlueprintable)
class LGUI_API UUIRenderable : public UUIItem
{
	GENERATED_BODY()

public:	
	UUIRenderable(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	TSharedPtr<UIGeometry> geometry = nullptr;

	//if have GeometryModifier component
	bool HaveGeometryModifier();
	/*use GeometryModifier to modify geometry 
	 return: true if the modifier change the triangle count, else false
	*/
	bool ApplyGeometryModifier();
	TInlineComponentArray<class UUIGeometryModifierBase*> GeometryModifierComponentArray;

	virtual void ApplyUIActiveState() override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UMaterialInterface* GetCustomUIMaterial()const { return CustomUIMaterial; }
	//if inMat is a UMaterialInstanceDynamic, then it will directly use for render.
	//if not, then a new MaterialInstanceDynamic will be created to render this UI item, and the created MaterialInstanceDynamic may shared with others UI items.
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetCustomUIMaterial(UMaterialInterface* inMat);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetRaycastComplex() { return bRaycastComplex; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRaycastComplex(bool newValue) { bRaycastComplex = newValue; }
	//if CustomUIMaterial is a UMaterialInstanceDynamic, then will return it directly.
	//if not, then return a created MaterialInstanceDynamic that renderring this UI item, may shared by other UI item. if this UI item is not renderred yet, then return nullptr
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UMaterialInstanceDynamic* GetMaterialInstanceDynamic();
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetIsPostProcess() { return bIsPostProcess; }

	virtual void OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)override;

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
	TSharedPtr<UIGeometry> GetGeometry() { return geometry; }

	virtual bool LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)override;
protected:
	friend class FUIRenderableCustomization;
	//Use custom material to render this element
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UMaterialInterface* CustomUIMaterial = nullptr;
	//Only valid if RaycastTarget is true. true - linetrace hit real mesh triangles, false - linetrace hit widget rectangle
	UPROPERTY(EditAnywhere, Category = "LGUI", AdvancedDisplay)
		bool bRaycastComplex = false;

	//do we have valid data to create geometry?
	virtual bool HaveDataToCreateGeometry() { return true; }
	//do we need texture create geometry?
	virtual bool NeedTextureToCreateGeometry() { return false; }
	//if NeedTextureToCreateGeometry() is true, then we should provide this texture
	virtual UTexture* GetTextureToCreateGeometry() { return nullptr; }

	//do anything before acturally create or update geometry
	virtual void OnBeforeCreateOrUpdateGeometry();
	//create ui geometry
	virtual void OnCreateGeometry();
	//update ui geometry
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);

	void CreateGeometry();
	virtual void UpdateGeometry(const bool& parentTransformChanged)override final;

protected:
	uint8 bIsPostProcess : 1;//post process item
private:
	uint8 bUVChanged:1;//vertex's uv change
	uint8 bTriangleChanged:1;//triangle index change
	uint8 bTextureChanged:1;//texture change
	uint8 bMaterialChanged:1;//custom material change

	uint8 cacheForThisUpdate_UVChanged:1, cacheForThisUpdate_TriangleChanged:1, cacheForThisUpdate_TextureChanged:1, cacheForThisUpdate_MaterialChanged:1;
};
