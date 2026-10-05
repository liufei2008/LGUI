// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexMeshModifierBase.h"
#include "LexMeshModifierExtendRect.generated.h"

/**
 * Extend rect mesh vertices, only support 4 vertices rectangle shape
 */
UCLASS(ClassGroup = (LGUI), Blueprintable, DisplayName="ExtendRect", meta = (BlueprintSpawnableComponent))
class LGUI_API ULexMeshModifierExtendRect : public ULexMeshModifierBase
{
	GENERATED_BODY()

public:	
	ULexMeshModifierExtendRect();

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FMargin Extend = FMargin(10, 10, 10, 10);
public:
	virtual void ModifyUIGeometry(FLexUIGeometry& InGeometry
		, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
	)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
	FMargin GetExtend()const { return Extend; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetExtend(FMargin Value);
};
