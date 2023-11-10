// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseActor.h"
#include "Core/ActorComponent/UIFrameCapture.h"
#include "UIFrameCaptureActor.generated.h"

/**
 * UI element that can capture screen as texture for further use.
 * Use it in ScreenSpace or WorldSpace-LGUIRenderer.
 * If android OpenGL ES3.1, need to enable "ProjectSettings/Platforms/Android/Build/Support Backbuffer Sampling on OpenGL".
 */
UCLASS(ClassGroup = LGUI)
class LGUI_API AUIFrameCaptureActor : public AUIBasePostProcessActor
{
	GENERATED_BODY()
	
public:	
	AUIFrameCaptureActor();

	virtual UUIItem* GetUIItem()const override { return UIFrameCapture; }
	virtual class UUIPostProcessRenderable* GetUIPostProcessRenderable()const override { return UIFrameCapture; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIFrameCapture* GetUIFrameCapture()const { return UIFrameCapture; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UUIFrameCapture> UIFrameCapture;
	
};
