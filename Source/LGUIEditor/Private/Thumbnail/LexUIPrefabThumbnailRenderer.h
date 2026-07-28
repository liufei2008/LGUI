// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexUIPrefabThumbnailScene.h"
#include "ThumbnailRendering/DefaultSizedThumbnailRenderer.h"
#include "LexUIPrefabThumbnailRenderer.generated.h"

UCLASS()
class ULexUIPrefabThumbnailRenderer :public UDefaultSizedThumbnailRenderer
{
	GENERATED_BODY()
public:
	ULexUIPrefabThumbnailRenderer();

	virtual bool CanVisualizeAsset(UObject* Object)override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas, bool bAdditionalViewFamily)override;

	virtual void BeginDestroy()override;

private:
	TUniquePtr<FLexUIPrefabThumbnailScene> ThumbnailScene;
};