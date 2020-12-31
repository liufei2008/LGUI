// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "Components/MeshComponent.h"
#include "LGUIMeshComponent.generated.h"

struct FLGUIMeshSection
{
	TArray<uint16> triangles;
	TArray<FDynamicMeshVertex> vertices;

	bool bSectionVisible;

	FLGUIMeshSection()
		: bSectionVisible(true)
	{}

	/** Reset this section, clear all mesh info. */
	void Reset()
	{
		vertices.Empty();
		triangles.Empty();

		bSectionVisible = true;
	}
};

class FLGUIViewExtension;
class ILGUIHudPrimitive;

//Generate dynamic mesh
UCLASS(NotBlueprintable, NotBlueprintType, Abstract)
class LGUI_API ULGUIMeshComponent : public UMeshComponent
{
	GENERATED_BODY()

public:
	virtual void CreateMeshSection();
	virtual void UpdateMeshSection(bool InVertexPositionChanged = true, int8 AdditionalShaderChannelFlags = 0);

	void ClearMesh();

	void SetMeshVisible(bool bNewVisibility);
	bool IsMeshVisible() const;
	void SetColor(FColor InColor);
	FColor GetColor()const;

	void SetSupportScreenSpace(bool supportOrNot, TWeakPtr<FLGUIViewExtension, ESPMode::ThreadSafe> HudRenderer);
	void SetSupportWorldSpace(bool supportOrNot);
	void SetToPostProcess(class UUIPostProcess* InPostProcessObject);

	void SetUITranslucentSortPriority(int32 NewTranslucentSortPriority);

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UMeshComponent Interface.
	virtual int32 GetNumMaterials() const override;
	//~ End UMeshComponent Interface.

private:
	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ Begin USceneComponent Interface.

	/** Update LocalBounds member from the local box of each section */
	void UpdateLocalBounds();

	friend class FLGUIMeshSceneProxy;

protected:
	FLGUIMeshSection MeshSection;
	TWeakPtr<FLGUIViewExtension, ESPMode::ThreadSafe> LGUIHudRenderer;
	bool IsSupportWorldSpace = true;
	UPROPERTY(Transient) class UUIPostProcess* PostProcessObject;
};


