// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutWithChildren.h"
#include "UIRoundedLayout.generated.h"

/**
 * Rounded layout, only affect children's position and angle, not affect size
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIRoundedLayout : public UUILayoutWithChildren
{
	GENERATED_BODY()

public:
	virtual void OnRebuildLayout()override;

	virtual bool GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const override;
protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float Radius = 100;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float StartAngle = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float EndAngle = 360;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bSetChildAngle = true;
};
