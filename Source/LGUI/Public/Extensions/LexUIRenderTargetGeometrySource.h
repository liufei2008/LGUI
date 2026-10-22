// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "LexUIComponentReference.h"
#include "Interfaces/Interface_CollisionDataProvider.h"
#include "DynamicMeshBuilder.h"
#include "LexUIRenderTargetInteraction.h"
#include "LexUIRenderTargetGeometrySource.generated.h"

class ULexCanvas;
class ULexWorldSpaceRaycasterSource;

UENUM(BlueprintType, Category = "LexUI")
enum class ELexUIRenderTargetGeometryMode : uint8
{
	Plane = 0,
	Cylinder = 1,

	/**
	 * RenderTarget mapped onto a static mesh. This component must attach to target StaticMeshComponent so we can find and use it.
	 * And in order to interact by LexUIRenderTargetInteraction, the 'Support UV From Hit Results' must be enabled in project settings.
	 */
	StaticMesh = 100,
};

/**
 * This component can generate a geometry to display LexUI's render target, and perform interaction source for LexUIRenderTargetInteraction component.
 */
UCLASS(ClassGroup = LGUI, Blueprintable, meta = (BlueprintSpawnableComponent), hidecategories = (Object, Activation, "Components|Activation"))
class LGUI_API ULexUIRenderTargetGeometrySource : public UMeshComponent, public IInterface_CollisionDataProvider, public ILexUIRenderTargetInteractionSourceInterface
{
	GENERATED_BODY()
	
public:	
	ULexUIRenderTargetGeometrySource();
	virtual void BeginPlay()override;
	virtual void EndPlay(EEndPlayReason::Type Reason)override;

private:
	UPROPERTY(EditAnywhere, Category = "LexUI")
		FLexUIComponentReference TargetWidgetPresenter;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		ELexUIRenderTargetGeometryMode GeometryMode = ELexUIRenderTargetGeometryMode::Plane;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		FVector2D Pivot = FVector2D(0.5f, 0.5f);
	/** Curvature of a cylindrical widget in degrees. */
	UPROPERTY(EditAnywhere, Category = "LexUI", meta = (ClampMin = -180.0f, ClampMax = 180.0f))
		float CylinderArcAngle = 45;
	/** Use this component's material for target static mesh component. */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool bOverrideStaticMeshMaterial = true;
	/** Enable backside interaction? Front side always interactable. */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool bEnableInteractOnBackside = false;
	/**
	 * Android GLES is flipped, so we flip it back. This just set the material property "FlipY".
	 * No need for UE5.1 and upward
	 */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool bFlipVerticalOnGLES = true;
	mutable TWeakObjectPtr<class UStaticMeshComponent> StaticMeshComp = nullptr;
	mutable TWeakObjectPtr<class ULexCanvas> TargetCanvasObject = nullptr;


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
	TWeakObjectPtr<class ULTweener> CheckRenderTargetTickTweener;
	void CheckRenderTargetTick();

	void UpdateLocalBounds();
	void UpdateCollision();
	void UpdateMeshData();
	friend class FLexUIRenderTargetGeometrySource_SceneProxy;
	TArray<FDynamicMeshVertex> Vertices;
	TArray<uint16> Triangles;
#if WITH_EDITOR
	bool bIsValidSceneProxy = false;
#endif
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

	// Begin ILGUIRenderTargetInteractionSourceInterface
	virtual ULexCanvas* GetTargetCanvas_Implementation()const override;
	virtual bool PerformLineTrace_Implementation(const int32& InHitFaceIndex, const FVector& InHitPoint, const FVector& InLineStart, const FVector& InLineEnd, FVector2D& OutHitUV)override;
	// End ILGUIRenderTargetInteractionSourceInterface
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	bool LineTraceHitUV(const int32& InHitFaceIndex, const FVector& InHitPoint, const FVector& InLineStart, const FVector& InLineEnd, FVector2D& OutHitUV)const;
	const TArray<FDynamicMeshVertex>& GetMeshVertices()const { return Vertices; }
	const TArray<uint16> GetMeshIndices()const { return Triangles; }

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		ULexCanvas* GetCanvas()const;
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		ELexUIRenderTargetGeometryMode GetGeometryMode()const { return GeometryMode; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		FVector2D GetPivot()const { return Pivot; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		float GetCylinderArcAngle()const { return CylinderArcAngle; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		bool GetOverrideStaticMeshMaterial()const { return bOverrideStaticMeshMaterial; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		bool GetEnableInteractOnBackside()const { return bEnableInteractOnBackside; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		bool GetFlipVerticalOnGLES()const { return bFlipVerticalOnGLES; }

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetCanvas(ULexCanvas* Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetGeometryMode(ELexUIRenderTargetGeometryMode Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetPivot(const FVector2D Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetCylinderArcAngle(float Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetEnableInteractOnBackside(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetFlipVerticalOnGLES(bool Value);

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		FIntPoint GetRenderTargetSize()const;

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		UMaterialInstanceDynamic* GetMaterialInstance()const;
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		UTextureRenderTarget2D* GetRenderTarget()const;
private:
};
