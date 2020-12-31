// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutBase.h"
#include "Core/LGUIBehaviour.h"
#include "UISizeControlByOther.generated.h"

//this is only a helper component for UISizeControlByOther, no need to be serialized
UCLASS(ClassGroup=(LGUI), Transient)
class LGUI_API UUISizeControlByOtherHelper :public ULGUIBehaviour
{
	GENERATED_BODY()

private:
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)override;
	friend class UUISizeControlByOther;
	UPROPERTY(Transient)class UUISizeControlByOther* TargetComp;
};

UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUISizeControlByOther : public UUILayoutBase
{
	GENERATED_BODY()

public:	
	virtual void Awake()override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		class AUIBaseActor* GetTargetActor() { return TargetActor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetControlWidth() { return ControlWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAdditionalWidth() { return AdditionalWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetControlHeight() { return ControlHeight; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAdditionalHeight() { return AdditionalHeight; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetControlWidth(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAdditionalWidth(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetControlHeight(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAdditionalHeight(float value);

	virtual bool CanControlChildAnchor()override;
	virtual bool CanControlChildWidth()override;
	virtual bool CanControlChildHeight()override;
	virtual bool CanControlSelfHorizontalAnchor()override;
	virtual bool CanControlSelfVerticalAnchor()override;
	virtual bool CanControlSelfWidth()override;
	virtual bool CanControlSelfHeight()override;
protected:
	friend class UUISizeControlByOtherHelper;
	virtual void OnRebuildLayout()override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
#endif

	//Target object's size control this object's size
	UPROPERTY(EditAnywhere, Category = "LGUI")
		class AUIBaseActor* TargetActor;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ControlWidth = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float AdditionalWidth = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ControlHeight = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float AdditionalHeight = 0;

	UPROPERTY(Transient) class UUIItem* TargetUIItem = nullptr;
	UPROPERTY(Transient) class UUISizeControlByOtherHelper* HelperComp = nullptr;
	bool CheckTargetUIItem();

	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)override;
	virtual void OnUIAttachmentChanged()override;

	//these will not affect this Layout
	virtual void OnUIChildAcitveInHierarchy(UUIItem* InChild, bool InUIActive)override {}
	virtual void OnUIChildAttachmentChanged(UUIItem* InChild, bool attachOrDetach)override {}
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* InChild)override {}
};
