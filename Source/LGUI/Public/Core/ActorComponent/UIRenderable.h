// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "UIItem.h"
#include "UIRenderable.generated.h"

class UIGeometry;
//UI element which have render geometry, and can be renderred by LGUICanvas
UCLASS(Abstract)
class LGUI_API UUIRenderable : public UUIItem
{
	GENERATED_BODY()

public:	
	UUIRenderable();

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
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetCustomUIMaterial(UMaterialInterface* inMat);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetRaycastComplex() { return bRaycastComplex; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRaycastComplex(bool newValue) { bRaycastComplex = newValue; }
	//return created MaterialInstanceDynamic that renderring this UI item, may shared by other UI item. if this UI item is not renderred yet, then return nullptr
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UMaterialInstanceDynamic* GetMaterialInstanceDynamic();

	FORCEINLINE void MarkUVDirty();
	FORCEINLINE void MarkTriangleDirty();
	FORCEINLINE void MarkTextureDirty();
	FORCEINLINE void MarkMaterialDirty();
	FORCEINLINE virtual bool ShouldRenderPixelPerfect();

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

	bool bUVChanged = true;//vertex's uv change
	bool bTriangleChanged = true;//triangle index change
	bool bTextureChanged = true;//texture change
	bool bMaterialChanged = true;//custom material change

	bool cacheForThisUpdate_UVChanged, cacheForThisUpdate_TriangleChanged, cacheForThisUpdate_TextureChanged, cacheForThisUpdate_MaterialChanged;
};
