// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
	virtual void ModifyUIGeometry(
		TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged,
		bool uvChanged, bool colorChanged, bool vertexPositionChanged, bool layoutChanged
		)override;
};
