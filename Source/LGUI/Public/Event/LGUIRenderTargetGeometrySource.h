// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "LGUIComponentReference.h"
#include "LGUIRenderTargetGeometrySource.generated.h"

class ULGUICanvas;
class ULGUIWorldSpaceRaycasterSource;

/**
 * This defines how interaction point mapped to RenderTarget
 */
UENUM(BlueprintType, Category = LGUI)
enum class ELGUIRenderTargetGeometryMode : uint8
{
	Plane = 0,
	Cylinder = 1,

	/**
	 * RenderTarget mapped onto a static mesh. The component must attach to target StaticMeshComponent so we can find and use it.
	 * And in order to interact by LGUIRenderTargetRaycaster, the 'Support UV From Hit Results' must be enabled in project settings.
	 */
	StaticMesh = 100,
};

/**
 * 
 */
UCLASS(ClassGroup = LGUI, Blueprintable, Experimental, meta = (BlueprintSpawnableComponent), hidecategories = (Object, Activation, "Components|Activation", Sockets, Base, Lighting, LOD, Mesh))
class LGUI_API ULGUIRenderTargetGeometrySource : public UMeshComponent
{
	GENERATED_BODY()
	
public:	
	ULGUIRenderTargetGeometrySource();
	virtual void BeginPlay()override;

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
	TWeakObjectPtr<class UStaticMeshComponent> StaticMeshComp = nullptr;


	/** The body setup of the displayed quad */
	UPROPERTY(Transient, DuplicateTransient)
		class UBodySetup* BodySetup = nullptr;
	/** The dynamic instance of the material that the render target is attached to */
	UPROPERTY(Transient, DuplicateTransient)
		mutable UMaterialInstanceDynamic* MaterialInstance = nullptr;

	void UpdateBodySetup(bool bDrawSizeChanged = false);
	void UpdateMaterialInstance();
	void UpdateMaterialInstanceParameters();
	float ComputeComponentWidth() const;
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

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	bool LineTraceHit(const FVector& InStart, const FVector& InEnd, const FVector& InDir
		, FVector2D& OutHitUV, FVector& OutHitPoint, FVector& OutHitNormal, float& OutHitDistance
	)const;

	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUICanvas* GetCanvas()const;

	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUIRenderTargetGeometryMode GetGeometryMode()const { return GeometryMode; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FVector2D GetPivot()const { return Pivot; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetCylinderArcAngle()const { return CylinderArcAngle; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetGeometryMode(ELGUIRenderTargetGeometryMode Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetPivot(const FVector2D Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetCylinderArcAngle(float Value);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		FIntPoint GetRenderTargetSize()const;

	UFUNCTION(BlueprintCallable, Category = LGUI)
		UMaterialInstanceDynamic* GetMaterialInstance()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		UTextureRenderTarget2D* GetRenderTarget()const;
private:
};
