// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/Actor/UIBaseActor.h"
#include "UIStaticMesh.generated.h"

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
class LGUI_API UUIStaticMesh : public UUIBatchGeometryRenderable
{
	GENERATED_BODY()

public:	
	UUIStaticMesh(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UStaticMesh* mesh;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UIStaticMeshVertexColorType vertexColorType = UIStaticMeshVertexColorType::NotAffectByUIColor;
	
	virtual bool HaveDataToCreateGeometry()override;
	virtual bool NeedTextureToCreateGeometry()override { return false; }
	virtual void OnBeforeCreateOrUpdateGeometry()override {}
	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
	virtual void UpdateSelfRenderMaterial(bool textureChange, bool materialChange)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI") 
		UStaticMesh* GetMesh()const { return mesh; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UIStaticMeshVertexColorType GetVertexColorType()const { return vertexColorType; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMesh(UStaticMesh* value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetVertexColorType(UIStaticMeshVertexColorType value);
};

UCLASS()
class LGUI_API AUIStaticMeshActor : public AUIBaseActor
{
	GENERATED_BODY()

public:
	AUIStaticMeshActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UIStaticMesh; }
	FORCEINLINE UUIStaticMesh* GetUIStaticMesh()const { return UIStaticMesh; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUIStaticMesh* UIStaticMesh;

};
