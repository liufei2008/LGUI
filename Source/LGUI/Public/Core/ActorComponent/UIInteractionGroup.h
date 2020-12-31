// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UIItem.h"
#include "Components/ActorComponent.h"
#include "UIInteractionGroup.generated.h"

/*
This component controls interaction state of all children UI elements, include self
One actor can only have one UIInteractionGroup component
*/
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIInteractionGroup : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	UUIInteractionGroup();
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	bool CheckUIItem();

	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bInteractable = true;
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bIgnoreParentGroup = false;
	UPROPERTY(Transient) UUIItem* CacheUIItem = nullptr;
public:
	UFUNCTION(Category = LGUI) bool GetInteractable() const { return bInteractable; }
	UFUNCTION(Category = LGUI) bool GetIgnoreParentGroup() const { return bIgnoreParentGroup; }

	UFUNCTION(Category = LGUI) void SetInteractable(const bool& InBool);
	UFUNCTION(Category = LGUI) void SetIgnoreParentGroup(const bool& InBool);
};
