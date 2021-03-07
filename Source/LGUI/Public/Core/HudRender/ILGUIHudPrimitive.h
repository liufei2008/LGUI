﻿// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneManagement.h"
#include "Core/HudRender/LGUIRenderer.h"

class FLGUIViewExtension;
class UUIPostProcess;

class ILGUIHudPrimitive
{
public:
	virtual ~ILGUIHudPrimitive() {}

	virtual bool CanRender() const = 0;
	virtual int GetRenderPriority() const = 0;

	//begin mesh interface
	virtual FMeshBatch GetMeshElement(class FMeshElementCollector* Collector) = 0;
	virtual FRHIVertexBuffer* GetVertexBufferRHI() = 0;
	virtual uint32 GetNumVerts() = 0;
	//end mesh interface

	//begin post process interface
	virtual bool GetIsPostProcess() = 0;
	virtual TWeakObjectPtr<UUIPostProcess> GetPostProcessObject() = 0;
	//end post process interface
};
