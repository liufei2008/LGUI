// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "LGUIComponentReference.h"
#include "Interfaces/Interface_CollisionDataProvider.h"
#include "DynamicMeshBuilder.h"
#include "LGUIRenderTargetGeometrySource.generated.h"

class ULGUICanvas;
class ULGUIWorldSpaceRaycasterSource;

UENUM(BlueprintType, Category = LGUI)
enum class ELGUIRenderTargetGeometryMode : uint8
{
	Plane = 0,
	Cylinder = 1,

	/**
	 * RenderTarget mapped onto a static mesh. This component must attach to target StaticMeshComponent so we can find and use it.
	 * And in order to interact by LGUIRenderTargetInteraction, the 'Support UV From Hit Results' must be enabled in project settings.
	 */
	StaticMesh = 100,
};

/**
 * This component can generate a geometry to display LGUI's render target, and perform interaction source for LGUIRenderTargetInteraction component.
 */
UCLASS(ClassGroup = LGUI, Blueprintable, meta = (BlueprintSpawnableComponent), hidecategories = (Object, Activation, "Components|Activation", Sockets, Base, Lighting, LOD, Mesh))
class LGUI_API ULGUIRenderTargetGeometrySource : public UMeshComponent, public IInterface_CollisionDataProvider
{
	GENERATED_BODY()
	
public:	
	ULGUIRenderTargetGeometrySource();
	virtual void BeginPlay()override;
	virtual void EndPlay(EEndPlayReason::Type Reason)override;

private:
	UPROPERTY(EditAnywhere, Category = LGUI)
		FLGUIComponentReference TargetCanvas;
	UPROPERTY(EditAnywhere, Category = LGUI)
		ELGUIRenderTargetGeometryMode GeometryMode = ELGUIRenderTargetGeometryMode::Plane;
	UPROPERTY(EditAnywhere, Category = LGUI)
		FVector2D Pivot = FVector2D(0.5f, 0.5f);
	/** Curvature of a cylindrical widget in degrees. */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (ClampMin = -180.0f, ClampMax = 180.0f))
		float CylinderArcAngle = 45;
	/** Use this component's material for target static mesh component. */
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bOverrideStaticMeshMaterial = true;
	/** Enable backside interaction? Front side always interactable. */
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bEnableInteractOnBackside = false;
	/**
	 * Android GLES is flipped, so we flip it back. This just set the material property "FlipY".
	 * No need for UE5.1 and upward
	 */
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bFlipVerticalOnGLES = true;
	mutable TWeakObjectPtr<class UStaticMeshComponent> StaticMeshComp = nullptr;
	mutable TWeakObjectPtr<class ULGUICanvas> TargetCanvasObject = nullptr;


	/** The body setup of the displayed quad */
	UPROPERTY(Transient, DuplicateTransient)
		TObjectPtr<class UBodySetup> BodySetup = nullptr;
	/** The dynamic instance of the material that the render target is attached to */
	UPROPERTY(Transient, DuplicateTransient)
		mutable TObjectPtr<UMaterialInstanceDynamic> MaterialInstance = nullptr;

	void UpdateBodySetup(bool bIsDirty = true);
	void UpdateMaterialInstance();
	void UpdateMaterialInstanceParameters();
	UMaterialInterface* GetPresetMaterial()const;
	float ComputeComponentWidth() const;
	float ComputeComponentHeight() const;
	float ComputeComponentThickness() const;
	bool CheckStaticMesh()const;

	void BeginCheckRenderTarget();
	void EndCheckRenderTarget();
	FDelegateHandle CheckRenderTargetTickDelegate;
	void CheckRenderTargetTick();

	void UpdateLocalBounds();
	void UpdateCollision();
	void UpdateMeshData();
	TArray<FDynamicMeshVertex> Vertices;
	TArray<uint32> Triangles;
public:
	/* UPrimitiveComponent Interface */
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual UBodySetup* GetBodySetup() override;
	virtual FCollisionShape GetCollisionShape(float Inflation) const override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void DestroyComponent(bool bPromoteChildren = false) override;
	virtual UMaterialInterface* GetMaterial(int32 MaterialIndex) const override;
	virtual void SetMaterial(int32 ElementIndex, UMaterialInterface* Material) override;
	virtual int32 GetNumMaterials() const override;
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const override;

	//~ Begin Interface_CollisionDataProvider Interface
	virtual bool GetTriMeshSizeEstimates(struct FTriMeshCollisionDataEstimates& OutTriMeshEstimates, bool bInUseAllTriData) const override;
	virtual bool GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData) override;
	virtual bool ContainsPhysicsTriMeshData(bool InUseAllTriData) const override;
	virtual bool WantsNegXTriMesh() override;
	//~ End Interface_CollisionDataProvider Interface
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	bool LineTraceHitUV(const int32& InHitFaceIndex, const FVector& InHitPoint, const FVector& InLineStart, const FVector& InLineEnd, FVector2D& OutHitUV)const;
	const TArray<FDynamicMeshVertex>& GetMeshVertices()const { return Vertices; }
	const TArray<uint32> GetMeshIndices()const { return Triangles; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUICanvas* GetCanvas()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUIRenderTargetGeometryMode GetGeometryMode()const { return GeometryMode; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FVector2D GetPivot()const { return Pivot; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetCylinderArcAngle()const { return CylinderArcAngle; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetOverrideStaticMeshMaterial()const { return bOverrideStaticMeshMaterial; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetEnableInteractOnBackside()const { return bEnableInteractOnBackside; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetFlipVerticalOnGLES()const { return bFlipVerticalOnGLES; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetCanvas(ULGUICanvas* Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetGeometryMode(ELGUIRenderTargetGeometryMode Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetPivot(const FVector2D Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetCylinderArcAngle(float Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetEnableInteractOnBackside(bool Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetFlipVerticalOnGLES(bool Value);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		FIntPoint GetRenderTargetSize()const;

	UFUNCTION(BlueprintCallable, Category = LGUI)
		UMaterialInstanceDynamic* GetMaterialInstance()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		UTextureRenderTarget2D* GetRenderTarget()const;
private:
};
