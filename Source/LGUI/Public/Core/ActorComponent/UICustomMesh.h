// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ActorComponent/UIBatchMeshRenderable.h"
#include "UICustomMesh.generated.h"

class ULGUICustomMesh;

/**
 * Render UI element with LGUICustomMesh.
 */
UCLASS(ClassGroup = LGUI, NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUICustomMesh : public UUIBatchMeshRenderable
{
	GENERATED_BODY()
	
public:	
	UUICustomMesh(const FObjectInitializer& ObjectInitializer);
protected:
	virtual void OnBeforeCreateOrUpdateGeometry()override;
	virtual UTexture* GetTextureToCreateGeometry()override;
	virtual void OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	virtual void BeginPlay()override;
	virtual void EndPlay(EEndPlayReason::Type Reason)override;

	/** Use a mesh generator to create your own mesh instead of a simple rect */
	UPROPERTY(EditAnywhere, Instanced, Category = LGUI)
		TObjectPtr<ULGUICustomMesh> CustomMesh = nullptr;

public:

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	UFUNCTION(BlueprintCallable, Category = LGUI)
	ULGUICustomMesh* GetCustomMesh()const { return CustomMesh; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetCustomMesh(ULGUICustomMesh* Value);
};
