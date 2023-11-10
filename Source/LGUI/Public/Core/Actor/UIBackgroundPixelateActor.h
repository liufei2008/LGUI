// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UIBackgroundPixelate.h"
#include "UIBackgroundPixelateActor.generated.h"

/**
 * UI element that can make the background look pixelated
 * Use it in ScreenSpace or WorldSpace-LGUIRenderer.
 * If android OpenGL ES3.1, need to enable "ProjectSettings/Platforms/Android/Build/Support Backbuffer Sampling on OpenGL".
 */
UCLASS(ClassGroup = LGUI)
class LGUI_API AUIBackgroundPixelateActor : public AUIBasePostProcessActor
{
	GENERATED_BODY()
	
public:	
	AUIBackgroundPixelateActor();

	virtual UUIItem* GetUIItem()const override { return UIBackgroundPixelate; }
	virtual class UUIPostProcessRenderable* GetUIPostProcessRenderable()const override { return UIBackgroundPixelate; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIBackgroundPixelate* GetUIBackgroundPixelate()const { return UIBackgroundPixelate; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UUIBackgroundPixelate> UIBackgroundPixelate;
	
};
