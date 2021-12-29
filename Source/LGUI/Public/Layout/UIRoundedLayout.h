// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutBase.h"
#include "UIRoundedLayout.generated.h"

/**
 * Rounded layout, only affect children's position and angle, not affect size
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIRoundedLayout : public UUILayoutBase
{
	GENERATED_BODY()

public:
	virtual void OnRebuildLayout()override;

	virtual bool CanControlChildAnchor_Implementation()const override;
	virtual bool CanControlChildHorizontalAnchoredPosition_Implementation()const override;
	virtual bool CanControlChildVerticalAnchoredPosition_Implementation()const override;
	virtual bool CanControlChildWidth_Implementation()const override;
	virtual bool CanControlChildHeight_Implementation()const override;
	virtual bool CanControlChildAnchorLeft_Implementation()const override;
	virtual bool CanControlChildAnchorRight_Implementation()const override;
	virtual bool CanControlChildAnchorBottom_Implementation()const override;
	virtual bool CanControlChildAnchorTop_Implementation()const override;

	virtual bool CanControlSelfAnchor_Implementation()const override;
	virtual bool CanControlSelfHorizontalAnchoredPosition_Implementation()const override;
	virtual bool CanControlSelfVerticalAnchoredPosition_Implementation()const override;
	virtual bool CanControlSelfWidth_Implementation()const override;
	virtual bool CanControlSelfHeight_Implementation()const override;
	virtual bool CanControlSelfAnchorLeft_Implementation()const override;
	virtual bool CanControlSelfAnchorRight_Implementation()const override;
	virtual bool CanControlSelfAnchorBottom_Implementation()const override;
	virtual bool CanControlSelfAnchorTop_Implementation()const override;
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
