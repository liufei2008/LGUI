// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutBase.h"
#include "UISizeControlByAspectRatio.generated.h"


UENUM(BlueprintType, Category = LGUI)
enum class EUISizeControlByAspectRatioMode :uint8
{
	None,
	WidthControlHeight,
	HeightControlWidth,
	FitInParent,
	EnvelopeParent,
};

/**
 * Use aspect ratio to control with and height.
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUISizeControlByAspectRatio : public UUILayoutBase
{
	GENERATED_BODY()

public:	
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		EUISizeControlByAspectRatioMode GetControlMode()const { return ControlMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAspectRatio()const { return AspectRatio; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetControlMode(EUISizeControlByAspectRatioMode value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAspectRatio(float value);

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
