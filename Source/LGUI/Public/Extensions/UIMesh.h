// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/Actor/UIBaseActor.h"
#include "UIMesh.generated.h"

UENUM(BlueprintType)
enum class UIMeshVertexColorType :uint8
{
	//Multiply mesh's vertex color with LGUI's color parameter
	MultiplyWithUIColor,
	//Replace mesh's vertex color by LGUI's color parameter
	ReplaceByUIColor,
	//Use mesh's vertex color only, not consider LGUI's color parameter
	NotAffectByUIColor,
};
//render a mesh as UI element
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIMesh : public UUIRenderable
{
	GENERATED_BODY()

public:	
	UUIMesh(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UStaticMesh* mesh;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UIMeshVertexColorType vertexColorType = UIMeshVertexColorType::MultiplyWithUIColor;
	
	virtual bool HaveDataToCreateGeometry()override;
	virtual bool NeedTextureToCreateGeometry()override { return false; }
	virtual void OnBeforeCreateOrUpdateGeometry()override {}
	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI") 
		UStaticMesh* GetMesh()const { return mesh; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UIMeshVertexColorType GetVertexColorType()const { return vertexColorType; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMesh(UStaticMesh* value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetVertexColorType(UIMeshVertexColorType value);
};

UCLASS()
class LGUI_API AUIMeshActor : public AUIBaseActor
{
	GENERATED_BODY()

public:
	AUIMeshActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UIMesh; }
	FORCEINLINE UUIMesh* GetUIMesh()const { return UIMesh; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUIMesh* UIMesh;

};
