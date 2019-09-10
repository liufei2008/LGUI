// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Components/MeshComponent.h"
#include "LGUIMeshComponent.generated.h"

struct FLGUIMeshSection
{
	TArray<FVector> vertices;
	TArray<uint16> triangles;
	TArray<FVector2D> uvs;
	TArray<FVector2D> uvs1;
	TArray<FVector2D> uvs2;
	TArray<FVector2D> uvs3;
	TArray<FColor> colors;
	TArray<FVector> normals;
	TArray<FVector> tangents;

	FBox SectionLocalBox;

	bool bSectionVisible;

	FLGUIMeshSection()
		: SectionLocalBox(ForceInit)
		, bSectionVisible(true)
	{}

	/** Reset this section, clear all mesh info. */
	void Reset()
	{
		vertices.Empty();
		uvs.Empty();
		uvs1.Empty();
		uvs2.Empty();
		uvs3.Empty();
		colors.Empty();
		triangles.Empty();
		normals.Empty();
		tangents.Empty();

		SectionLocalBox.Init();
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

	void SetToLGUIHud(TWeakPtr<FLGUIViewExtension, ESPMode::ThreadSafe> HudRenderer);
	void SetToLGUIWorld();

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
};


