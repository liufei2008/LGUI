// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneManagement.h"

class FLGUIViewExtension;

class ILGUIHudPrimitive
{
public:
	virtual ~ILGUIHudPrimitive() {}
	virtual FMeshBatch GetMeshElement(class FMeshElementCollector* Collector) = 0;
	virtual int GetRenderPriority() const = 0;
	virtual bool CanRender() const = 0;
	virtual class FPrimitiveSceneProxy* GetPrimitiveSceneProxy() = 0;
};
