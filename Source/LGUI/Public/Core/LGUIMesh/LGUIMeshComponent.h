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

class FLGUIHudRenderer;
class ILGUIHudPrimitive;

//Generate dynamic mesh
UCLASS(ClassGroup = (LGUI), NotBlueprintable, Abstract)
class LGUI_API ULGUIMeshComponent : public UMeshComponent
{
	GENERATED_BODY()

public:
	virtual void CreateMeshSection();
	virtual void UpdateMeshSection(bool InVertexPositionChanged = true, int8 AdditionalShaderChannelFlags = 0);

	void ClearMesh();

	void SetUIMeshVisibility(bool bNewVisibility);
	bool IsMeshVisible() const;
	void SetColor(FColor InColor);
	FColor GetColor()const;

	void SetSupportScreenSpace(bool supportOrNot, TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> HudRenderer);
	void SetSupportWorldSpace(bool supportOrNot);

	void SetUITranslucentSortPriority(int32 NewTranslucentSortPriority);

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UMeshComponent Interface.
	virtual int32 GetNumMaterials() const override;
	//~ End UMeshComponent Interface.

	FLGUIMeshSection MeshSection;
private:
	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ Begin USceneComponent Interface.

	/** Update LocalBounds member from the local box of each section */
	void UpdateLocalBounds();

	friend class FLGUIMeshSceneProxy;

protected:
	TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> LGUIHudRenderer;
	bool IsSupportWorldSpace = true;
};


