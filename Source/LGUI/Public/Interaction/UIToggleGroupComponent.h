// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "UIToggleGroupComponent.generated.h"

class UUIToggleComponent;

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIToggleGroupComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UUIToggleGroupComponent();

	// Called when the game starts
	virtual void BeginPlay() override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	UPROPERTY(Transient) UUIToggleComponent* LastSelect = nullptr;
	UPROPERTY(EditAnywhere, Category = "LGUI-ToggleGroup")
		bool bAllowNoneSelected = true;
public:
	void SelectItem(UUIToggleComponent* item);
	void SelectNone();
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		void SetSelection(UUIToggleComponent* Target);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		void ClearSelection();
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		UUIToggleComponent* GetSelectedItem();

	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		bool GetAllowNoneSelected() { return bAllowNoneSelected; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		void SetAllowNoneSelected(bool InBool) { bAllowNoneSelected = InBool; }
};
