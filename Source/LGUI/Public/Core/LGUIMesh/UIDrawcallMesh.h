// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Core/LGUIMesh/LGUIMeshComponent.h"
#include "UIDrawcallMesh.generated.h"

//render and show UI geometry
UCLASS(ClassGroup = (LGUI), Transient, NotBlueprintable, NotBlueprintType)
class LGUI_API UUIDrawcallMesh : public ULGUIMeshComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUIDrawcallMesh();

	// Called when the game starts
	virtual void BeginPlay() override;

public:
	void GenerateOrUpdateMesh(bool vertexPositionChanged = true);
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
private:
	friend class UUIPanel;
	//prev vertex and triangle index count, for us to tell which data should we update
	int prevVertexCount = 0;
	int prevIndexCount = 0;
};
