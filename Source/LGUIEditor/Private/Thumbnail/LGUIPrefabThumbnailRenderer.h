// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "LGUIPrefabThumbnailScene.h"
#include "LGUIPrefabThumbnailRenderer.generated.h"

UCLASS()
class ULGUIPrefabThumbnailRenderer :public UDefaultSizedThumbnailRenderer
{
	GENERATED_BODY()
public:
	ULGUIPrefabThumbnailRenderer();

	virtual bool CanVisualizeAsset(UObject* Object)override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas, bool bAdditionalViewFamily)override;

	virtual void BeginDestroy()override;

private:
	FLGUIPrefabInstanceThumbnailScene ThumbnailScenes;
};