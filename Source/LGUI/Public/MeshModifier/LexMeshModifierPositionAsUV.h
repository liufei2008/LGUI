// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexMeshModifierBase.h"
#include "LexMeshModifierPositionAsUV.generated.h"


UCLASS(ClassGroup = (LGUI), Blueprintable, DisplayName="PositionAsUV", meta = (BlueprintSpawnableComponent))
class LGUI_API ULexMeshModifierPositionAsUV : public ULexMeshModifierBase
{
	GENERATED_BODY()

public:	
	ULexMeshModifierPositionAsUV();

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI", meta=(UIMin=0, UIMax=3))
	uint8 UVChannel = 1;
	UPROPERTY(EditAnywhere, Category = "LGUI")
	FVector2f Scale = FVector2f::One();
public:
	virtual void ModifyUIGeometry(FLexUIGeometry& InGeometry
		, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
	)override;
	virtual void ModifierWillChangeVertexData(bool& OutTriangleIndices, bool& OutVertexPosition, bool& OutUV, bool& OutColor)override
	{
		OutTriangleIndices = false;
		OutVertexPosition = false;
		OutUV = false;
		OutColor = false;
	};
};
