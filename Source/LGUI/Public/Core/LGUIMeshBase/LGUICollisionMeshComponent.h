// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "LGUIMeshComponent.h"
#include "PhysicsEngine/ConvexElem.h"
#include "PhysicsEngine/BodySetupEnums.h"
#include "Interfaces/Interface_CollisionDataProvider.h"
#include "LGUICollisionMeshComponent.generated.h"


UCLASS(NotBlueprintable, NotBlueprintType, Abstract)
class LGUI_API ULGUICollisionMeshComponent : public ULGUIMeshComponent, public IInterface_CollisionDataProvider
{
	GENERATED_BODY()

public:
	virtual void CreateMeshSection()override;
	virtual void UpdateMeshSection(bool InVertexPositionChanged = true)override;
#pragma region PhysicsDataProvider
	virtual bool GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData) override;
	virtual bool ContainsPhysicsTriMeshData(bool InUseAllTriData) const override;
	virtual bool WantsNegXTriMesh() override { return false; }
#pragma endregion
	virtual class UBodySetup* GetBodySetup() override;
protected:
	/** Ensure ProcMeshBodySetup is allocated and configured */
	void CreateProcMeshBodySetup();
	/** Mark collision data as dirty, and re-create on instance if necessary */
	void UpdateCollision();

	UPROPERTY(EditAnywhere, Category = "LGUI")TEnumAsByte<ECollisionTraceFlag> CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;
	/** Collision data */
	UPROPERTY(Instanced)class UBodySetup* ProcMeshBodySetup;
	/** Convex shapes used for simple collision */
	UPROPERTY() FKConvexElem CollisionConvexElems;
	/** Local space bounds of mesh */
	UPROPERTY() FBoxSphereBounds LocalBounds;
};


