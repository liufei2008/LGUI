// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIGeometryModifierBase.h"
#include "UIEffectPositionAsUV.generated.h"


UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIEffectPositionAsUV : public UUIGeometryModifierBase
{
	GENERATED_BODY()

public:	
	UUIEffectPositionAsUV();

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint8 uvChannel = 1;
public:
	virtual void ModifyUIGeometry(UIGeometry& InGeometry
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
