// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutBase.h"
#include "UISizeControlByAspectRatio.generated.h"


UENUM(BlueprintType)
enum class EUISizeControlByAspectRatioMode :uint8
{
	None,
	WidthControlHeight,
	HeightControlWidth,
	FitInParent,
	EnvelopeParent,
};
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUISizeControlByAspectRatio : public UUILayoutBase
{
	GENERATED_BODY()

public:	
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		EUISizeControlByAspectRatioMode GetControlMode() { return ControlMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAspectRatio() { return AspectRatio; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetControlMode(EUISizeControlByAspectRatioMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAspectRatio(float value);

	virtual bool CanControlChildAnchor()override;
	virtual bool CanControlChildWidth()override;
	virtual bool CanControlChildHeight()override;
	virtual bool CanControlSelfHorizontalAnchor()override;
	virtual bool CanControlSelfVerticalAnchor()override;
	virtual bool CanControlSelfWidth()override;
	virtual bool CanControlSelfHeight()override;
	virtual bool CanControlSelfStrengthLeft()override;
	virtual bool CanControlSelfStrengthRight()override;
	virtual bool CanControlSelfStrengthTop()override;
	virtual bool CanControlSelfStrengthBottom()override;
protected:
	virtual void OnRebuildLayout()override;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUISizeControlByAspectRatioMode ControlMode = EUISizeControlByAspectRatioMode::None;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float AspectRatio = 1.0f;

	//these will not affect this Layout
	virtual void OnUIChildAcitveInHierarchy(UUIItem* InChild, bool InUIActive)override {}
	virtual void OnUIChildAttachmentChanged(UUIItem* InChild, bool attachOrDetach)override {}
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* InChild)override {}
};
