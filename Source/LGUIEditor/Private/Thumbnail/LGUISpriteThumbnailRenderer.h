// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "LGUIPrefabThumbnailScene.h"
#include "LGUISpriteThumbnailRenderer.generated.h"

UCLASS()
class ULGUISpriteThumbnailRenderer :public UDefaultSizedThumbnailRenderer
{
	GENERATED_BODY()
public:
	ULGUISpriteThumbnailRenderer();

	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas)override;

	virtual void BeginDestroy()override;

protected:
	void DrawFrame(class ULGUISpriteData* Sprite, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas, FBoxSphereBounds* OverrideRenderBounds);
	void DrawGrid(int32 X, int32 Y, uint32 Width, uint32 Height, FCanvas* Canvas);
};