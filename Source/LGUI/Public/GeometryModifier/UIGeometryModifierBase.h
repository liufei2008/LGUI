// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "Core/UIGeometry.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "Components/ActorComponent.h"
#include "UIGeometryModifierBase.generated.h"

class UUIBatchGeometryRenderable;

/** 
 * For modify ui geometry, act like a filter.
 * Need UIBatchGeometryRenderable component.
 */
UCLASS(Abstract)
class LGUI_API UUIGeometryModifierBase : public UActorComponent
{
	GENERATED_BODY()

public:	
	UUIGeometryModifierBase();

protected:
	virtual void BeginPlay()override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnRegister()override;
	virtual void OnUnregister()override;

	/** Execute order of this effect in actor.The greater executeOrder is the later this effect execute */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int executeOrder = 0;
	/** 
	 * If there are multiple UIBatchGeometryRenderable components, then select one of them by name.
	 * Leave it empty if only one UIBatchGeometryRenderable component.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FName componentName;

	UUIBatchGeometryRenderable* GetRenderableUIItem();
private:
	TWeakObjectPtr<UUIBatchGeometryRenderable> renderableUIItem;
	void RemoveFromUIBatchGeometry();
	void AddToUIBatchGeometry();
public:
	FORCEINLINE int GetExecuteOrder()const { return executeOrder; }
	/**
	 * Modify UI geometry's vertex and triangle.
	 * @param	InOutOriginVerticesCount: orign vertex count; after modify, new vertex count must be set to this
	 * @param	InOutOriginTriangleIndicesCount: orign triangle indices count; after modify, new triangle indices count must be set to this
	 * @param	OutTriangleChanged: if this modifier affect triangle, then set this to true
	 */
	virtual void ModifyUIGeometry(
		TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged,
		bool uvChanged, bool colorChanged, bool vertexPositionChanged, bool layoutChanged
		) PURE_VIRTUAL(UUIGeometryModifierBase::ModifyUIGeometry,);
};
