// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Components/LexVisualBatchMesh.h"
#include "LexCustomMesh.generated.h"

class ULexUICustomMeshSource;

/**
 * Render UI element with LGUICustomMesh.
 */
UCLASS(ClassGroup = LGUI, NotBlueprintable)
class LGUI_API ULexCustomMesh : public ULexVisualBatchMesh
{
	GENERATED_BODY()
	
public:	
	ULexCustomMesh(const FObjectInitializer& ObjectInitializer);
protected:
	virtual bool SupportDrawCallBatching()const override;
	virtual void OnBeforeCreateOrUpdateGeometry()override;
	virtual UTexture* GetTextureToCreateGeometry()override;
	virtual void OnUpdateGeometry(FLexUIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	/** Use a mesh generator to create your own mesh instead of a simple rect */
	UPROPERTY(EditAnywhere, Instanced, Category = "LexUI")
		TObjectPtr<ULexUICustomMeshSource> CustomMesh = nullptr;

public:

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	ULexUICustomMeshSource* GetCustomMesh()const { return CustomMesh; }

	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetCustomMesh(ULexUICustomMeshSource* Value);
};
