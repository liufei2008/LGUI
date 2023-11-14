// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UICustomMesh.h"
#include "UICustomMeshActor.generated.h"

/**
 * Render UI element with LGUICustomMesh.
 */
UCLASS(ClassGroup = LGUI)
class LGUI_API AUICustomMeshActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()
	
public:	
	AUICustomMeshActor()
	{
		PrimaryActorTick.bCanEverTick = false;

		UICustomMesh = CreateDefaultSubobject<UUICustomMesh>(TEXT("UICustomMesh"));
		RootComponent = UICustomMesh;
	}

	virtual UUIItem* GetUIItem()const override { return UICustomMesh; }
	virtual class UUIBaseRenderable* GetUIRenderable()const override { return UICustomMesh; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUICustomMesh* GetUICustomMesh()const { return UICustomMesh; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UUICustomMesh> UICustomMesh;
	
};
