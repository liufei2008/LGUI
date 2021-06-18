// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/LGUIMesh/UIDrawcallMesh.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Core/LGUIIndexBuffer.h"

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
#if WITH_EDITOR
		if (indexCount >= MAX_TRIANGLE_COUNT)
		{
			auto errorMsg = FString::Printf(TEXT("[LGUIDrwcall] Too many triangles in single drawcall! This will cause issue!"));
			LGUIUtils::EditorNotification(FText::FromString(errorMsg), 10);
			UE_LOG(LGUI, Error, TEXT("%s"), *errorMsg);
		}
#endif
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