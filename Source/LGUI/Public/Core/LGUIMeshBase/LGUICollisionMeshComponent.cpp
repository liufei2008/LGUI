// Copyright 2019 LexLiu. All Rights Reserved.

#include "LGUICollisionMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodySetup.h"


void ULGUICollisionMeshComponent::CreateMeshSection()
{
	Super::CreateMeshSection();
#if WITH_EDITOR
	if (GetWorld() && GetWorld()->WorldType != EWorldType::Editor)
#endif
	{
		UpdateCollision();
	}
}
void ULGUICollisionMeshComponent::UpdateMeshSection(bool InVertexPositionChanged)
{
	Super::UpdateMeshSection(InVertexPositionChanged);
	if (InVertexPositionChanged)
	{
		auto& vertices = MeshSection.vertices;
		int NumVerts = vertices.Num();
		TArray<FVector> CollisionPositions;
		for (int32 VertIdx = 0; VertIdx < NumVerts; VertIdx++)
		{
			CollisionPositions.Add(vertices[VertIdx]);
		}
		BodyInstance.UpdateTriMeshVertices(CollisionPositions);
#if WITH_EDITOR
		if (GetWorld() && GetWorld()->WorldType != EWorldType::Editor)
#endif
		{
			UpdateCollision();
		}
	}
}
#pragma region PhysicsDataProvider
bool ULGUICollisionMeshComponent::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{
	// See if we should copy UVs
	bool bCopyUVs = UPhysicsSettings::Get()->bSupportUVFromHitResults;
	if (bCopyUVs)
	{
		CollisionData->UVs.AddZeroed(1); // only one UV channel
	}

	// Copy vert data
	auto& vertices = MeshSection.vertices;
	auto& uvs = MeshSection.uvs;
	auto& TargetVertices = CollisionData->Vertices;
	int VertCount = vertices.Num();
	TargetVertices.Reserve(VertCount);
	if (bCopyUVs)
	{
		CollisionData->UVs[0].Reserve(VertCount);
	}
	for (int32 VertIdx = 0; VertIdx < VertCount; VertIdx++)
	{
		TargetVertices.Add(vertices[VertIdx]);

		// Copy UV if desired
		if (bCopyUVs)
		{
			CollisionData->UVs[0].Add(uvs[VertIdx]);
		}
	}

	// Copy triangle data
	auto& triangles = MeshSection.triangles;
	const int32 NumTriangles = triangles.Num() / 3;
	CollisionData->Indices.Reserve(NumTriangles);
	CollisionData->MaterialIndices.Reserve(NumTriangles);
	for (int32 TriIdx = 0; TriIdx < NumTriangles; TriIdx++)
	{
		FTriIndices Triangle;
		Triangle.v0 = triangles[(TriIdx * 3) + 0];
		Triangle.v1 = triangles[(TriIdx * 3) + 1];
		Triangle.v2 = triangles[(TriIdx * 3) + 2];
		CollisionData->Indices.Add(Triangle);

		// Also store material info
		CollisionData->MaterialIndices.Add(0);
	}

	CollisionData->bFlipNormals = true;
	CollisionData->bDeformableMesh = true;
	CollisionData->bFastCook = true;

	return true;
}

bool ULGUICollisionMeshComponent::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
	if (MeshSection.triangles.Num() >= 3)
	{
		return true;
	}

	return false;
}
#pragma endregion

UBodySetup* ULGUICollisionMeshComponent::GetBodySetup()
{
	CreateProcMeshBodySetup();
	return ProcMeshBodySetup;
}
void ULGUICollisionMeshComponent::CreateProcMeshBodySetup()
{
	if (ProcMeshBodySetup == NULL)
	{
		// The body setup in a template needs to be public since the property is Tnstanced and thus is the archetype of the instance meaning there is a direct reference
		ProcMeshBodySetup = NewObject<UBodySetup>(this, NAME_None, (IsTemplate() ? RF_Public : RF_NoFlags));
		ProcMeshBodySetup->BodySetupGuid = FGuid::NewGuid();

		ProcMeshBodySetup->bGenerateMirroredCollision = false;
		ProcMeshBodySetup->bDoubleSidedGeometry = false;
		ProcMeshBodySetup->CollisionTraceFlag = CollisionTraceFlag;
	}
}

void ULGUICollisionMeshComponent::UpdateCollision()
{
	bool bCreatePhysState = false; // Should we create physics state at the end of this function?

								   // If its created, shut it down now
	if (bPhysicsStateCreated)
	{
		DestroyPhysicsState();
		bCreatePhysState = true;
	}

	// Ensure we have a BodySetup
	CreateProcMeshBodySetup();

	// Fill in simple collision convex elements
	if (ProcMeshBodySetup->AggGeom.ConvexElems.Num() == 0)
	{
		ProcMeshBodySetup->AggGeom.ConvexElems.Add(CollisionConvexElems);
	}
	else
	{
		ProcMeshBodySetup->AggGeom.ConvexElems[0] = CollisionConvexElems;
	}

	// Set trace flag
	ProcMeshBodySetup->CollisionTraceFlag = CollisionTraceFlag;

	// New GUID as collision has changed
	ProcMeshBodySetup->BodySetupGuid = FGuid::NewGuid();
	// Also we want cooked data for this
	ProcMeshBodySetup->bHasCookedCollisionData = true;
	// Clear current mesh data
	ProcMeshBodySetup->InvalidatePhysicsData();
	// Create new mesh data
	ProcMeshBodySetup->CreatePhysicsMeshes();

	// Create new instance state if desired
	if (bCreatePhysState)
	{
		CreatePhysicsState();
	}
}