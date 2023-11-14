// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UIDirectMeshRenderable.h"
#include "Core/Actor/UIBaseActor.h"
#include "UIStaticMesh.generated.h"

USTRUCT()
struct FLGUIStaticMeshVertex
{
	GENERATED_BODY()

	FLGUIStaticMeshVertex()
		:
		Position(FVector::ZeroVector)
		, TangentX(FVector::RightVector)
		, TangentZ(FVector::UpVector)
		, Color(ForceInitToZero)
		, UV0(FVector2D::ZeroVector)
		, UV1(FVector2D::ZeroVector)
		, UV2(FVector2D::ZeroVector)
		, UV3(FVector2D::ZeroVector)
	{
	}

	FLGUIStaticMeshVertex(
		FVector InPos
		, FVector InTangentX
		, FVector InTangentZ
		, FColor InColor
		, FVector2D InUV0
		, FVector2D InUV1
		, FVector2D InUV2
		, FVector2D InUV3
	)
		: Position(InPos)
		, TangentX(InTangentX)
		, TangentZ(InTangentZ)
		, Color(InColor)
		, UV0(InUV0)
		, UV1(InUV1)
		, UV2(InUV2)
		, UV3(InUV3)
	{
	}

	UPROPERTY()
		FVector Position;
	UPROPERTY()
		FVector TangentX;
	UPROPERTY()
		FVector TangentZ;
	UPROPERTY()
		FColor Color;
	UPROPERTY()
		FVector2D UV0;
	UPROPERTY()
		FVector2D UV1;
	UPROPERTY()
		FVector2D UV2;
	UPROPERTY()
		FVector2D UV3;
};

/** Cache StaticMesh for use in LGUI's UIStaticMesh. Since we cannot read StaticMesh data in runtime, we must create this object and assign 'MeshAsset' property in editor. */
UCLASS()
class LGUI_API ULGUIStaticMeshCacheData : public UObject
{
	GENERATED_BODY()

public:
	/** Access the slate vertexes. */
	const TArray<FLGUIStaticMeshVertex>& GetVertexData() const;

	/** Access the indexes for the order in which to draw the vertexes. */
	const TArray<uint32>& GetIndexData() const;

	/** Material to be used with the specified vector art data. */
	UMaterialInterface* GetMaterial() const;

	/** Convert the static mesh data into slate vector art on demand. Does nothing in a cooked build. */
	void EnsureValidData();

#if WITH_EDITORONLY_DATA
	DECLARE_EVENT(ULGUIStaticMeshCacheData, FLGUIStaticMeshDataChangeEvent);
	FLGUIStaticMeshDataChangeEvent OnMeshDataChange;
#endif
private:
	// ~ UObject Interface
	virtual void PreSave(class FObjectPreSaveContext SaveContext) override;
	// ~ UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
	/** Does the actual work of converting mesh data into slate vector art */
	void InitFromStaticMesh(const UStaticMesh* InSourceMesh);
	void ClearMeshData();
#endif

#if WITH_EDITORONLY_DATA
	/** The mesh data asset from which the vector art is sourced */
	UPROPERTY(EditAnywhere, Category = "Vector Art")
		TObjectPtr<UStaticMesh> MeshAsset;

	/** The material which we are using, or the material from with the MIC was constructed. */
	UPROPERTY(Transient)
		TObjectPtr<UMaterialInterface> SourceMaterial;
#endif

	/** @see GetVertexData() */
	UPROPERTY()
		TArray<FLGUIStaticMeshVertex> VertexData;

	/** @see GetIndexData() */
	UPROPERTY()
		TArray<uint32> IndexData;

	/** @see GetMaterial() */
	UPROPERTY()
		TObjectPtr<UMaterialInterface> Material;
};

UENUM(BlueprintType, Category = LGUI)
enum class UIStaticMeshVertexColorType :uint8
{
	//Multiply mesh's vertex color with LGUI's color parameter
	MultiplyWithUIColor,
	//Replace mesh's vertex color by LGUI's color parameter
	ReplaceByUIColor,
	//Use mesh's vertex color only, not consider LGUI's color parameter
	NotAffectByUIColor,
};
/**
 * render a StaticMesh as UI element
 */
UCLASS(ClassGroup = (LGUI), Blueprintable, Experimental, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIStaticMesh : public UUIDirectMeshRenderable
{
	GENERATED_BODY()

public:	
	UUIStaticMesh(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TObjectPtr<ULGUIStaticMeshCacheData> meshCache;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UIStaticMeshVertexColorType vertexColorType = UIStaticMeshVertexColorType::NotAffectByUIColor;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TObjectPtr<UMaterialInterface> ReplaceMaterial;
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange)override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
	void OnStaticMeshDataChange();
#endif
#if WITH_EDITORONLY_DATA
	FDelegateHandle OnMeshDataChangeDelegateHandle;
#endif
	
	virtual void OnMeshDataReady()override;
	void CreateGeometry();
	virtual void UpdateGeometry()override;
	void UpdateMeshTransform(bool updateToDrawcallMesh);
	void UpdateMeshColor(bool updateToDrawcallMesh);
	virtual bool HaveValidData()const override;
	virtual UMaterialInterface* GetMaterial()const override;
public:
	/** return 'MeshCache' property */
	UFUNCTION(BlueprintCallable, Category = "LGUI") 
		ULGUIStaticMeshCacheData* GetMeshCache()const { return meshCache; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UIStaticMeshVertexColorType GetVertexColorType()const { return vertexColorType; }
	/** Get the 'ReplaceMaterial' property */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		class UMaterialInterface* GetReplaceMaterial()const { return ReplaceMaterial; }
	/** Get actual rendering material. If 'ReplaceMaterial' is valid then return 'ReplaceMaterial', or return the mesh's default material. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		class UMaterialInterface* GetRenderMaterial()const { return GetMaterial(); }
	/** return current rendering DynamicMaterialInstance, or create one if not valid. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		class UMaterialInstanceDynamic* GetOrCreateDynamicMaterialInstance();

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMesh(ULGUIStaticMeshCacheData* value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetVertexColorType(UIStaticMeshVertexColorType value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetReplaceMaterial(UMaterialInterface* value);
};

UCLASS(ClassGroup = LGUI)
class LGUI_API AUIStaticMeshActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()

public:
	AUIStaticMeshActor();

	virtual UUIItem* GetUIItem()const override { return UIStaticMesh; }
	virtual class UUIBaseRenderable* GetUIRenderable()const override { return UIStaticMesh; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIStaticMesh* GetUIStaticMesh()const { return UIStaticMesh; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UUIStaticMesh> UIStaticMesh;

};
