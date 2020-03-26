// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/LGUIMesh/UIDrawcallMesh.h"
#include "LGUI.h"


UUIDrawcallMesh::UUIDrawcallMesh()
{
	PrimaryComponentTick.bCanEverTick = false;
	bSelectable = true;
	bCastStaticShadow = false;
	bAffectDynamicIndirectLighting = false;
	bReceivesDecals = false;
	bApplyImpulseOnDamage = false;
}

void UUIDrawcallMesh::BeginPlay()
{
	Super::BeginPlay();
}

void UUIDrawcallMesh::GenerateOrUpdateMesh(bool vertexPositionChanged, int8 AdditionalShaderChannelFlags)
{
	int vertexCount = MeshSection.vertices.Num();
	int indexCount = MeshSection.triangles.Num();

	if (prevIndexCount == indexCount && prevVertexCount == vertexCount)//if vertex count and triangle count not change, just update
	{
		UpdateMeshSection(vertexPositionChanged, AdditionalShaderChannelFlags);
	}
	else
	{
		CreateMeshSection();
	}

	prevVertexCount = vertexCount;
	prevIndexCount = indexCount;
}

#if WITH_EDITOR
void UUIDrawcallMesh::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif