// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "ThumbnailHelpers.h"

class ULexWidget;
class ULexUIPrefab;

class FLexUIPrefabThumbnailScene :public FThumbnailPreviewScene
{
public:
	FLexUIPrefabThumbnailScene();
	bool IsValidForVisualization()const;
	void SetPrefab(ULexUIPrefab* Prefab);
protected:
	virtual void GetViewMatrixParameters(const float InFOVDegrees, FVector& OutOrigin, float& OutOrbitPitch, float& OutOrbitYaw, float& OutOrbitZoom)const override;
	virtual USceneThumbnailInfo* GetSceneThumbnailInfo(const float TargetDistance)const;
	void SpawnPreviewWidget();
	void GetBoundsRecursive(ULexWidget* RootWidget, FBoxSphereBounds& OutBounds)const;
private:
	void ClearOldWidgets();
private:
	int32 NumStartingActors;
	TWeakObjectPtr<ULexWidget> RootAgentWidget;
	TWeakObjectPtr<ULexUIPrefab> CurrentPrefab;
	FText CachedPrefabContent;
	FBoxSphereBounds PreviewBounds;
};
