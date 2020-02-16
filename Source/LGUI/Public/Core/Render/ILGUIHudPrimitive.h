// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneManagement.h"
#include "Core/Render/LGUIRenderer.h"

class FLGUIViewExtension;

class ILGUIHudPrimitive
{
public:
	virtual ~ILGUIHudPrimitive() {}
	virtual FMeshBatch GetMeshElement(class FMeshElementCollector* Collector) = 0;
	virtual int GetRenderPriority() const = 0;
	virtual bool CanRender() const = 0;
	virtual class FPrimitiveSceneProxy* GetPrimitiveSceneProxy() = 0;
	virtual FRHIVertexBuffer* GetVertexBufferRHI() = 0;
	virtual uint32 GetNumVerts() = 0;
	virtual bool GetIsPostProcess() = 0;
	virtual TWeakObjectPtr<UUIPostProcess> GetPostProcessObject() = 0;
	//virtual BeforeRenderPostProcessFunction GetBeforeRenderPostProcess() = 0;
	//virtual RenderPostProcessFunction GetRenderPostProcess() = 0;
};
