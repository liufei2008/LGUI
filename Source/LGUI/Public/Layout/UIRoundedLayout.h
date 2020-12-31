// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutBase.h"
#include "UIRoundedLayout.generated.h"

//rounded layout, only affect position and size, not affect size
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIRoundedLayout : public UUILayoutBase
{
	GENERATED_BODY()

public:
	virtual void OnRebuildLayout()override;

	virtual bool CanControlChildAnchor()override;
	virtual bool CanControlChildWidth()override;
	virtual bool CanControlChildHeight()override;
	virtual bool CanControlSelfHorizontalAnchor()override;
	virtual bool CanControlSelfVerticalAnchor()override;
	virtual bool CanControlSelfWidth()override;
	virtual bool CanControlSelfHeight()override;
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
