// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "DynamicMeshBuilder.h"

#ifdef LGUI_USE_32BIT_INDEXBUFFER
typedef uint32 FLGUIMeshIndexBufferType;
const int LGUI_MAX_VERTEX_COUNT = 2147483647;
typedef FDynamicMeshIndexBuffer32 FLGUIMeshIndexBuffer;
#else
typedef uint16 FLGUIMeshIndexBufferType;
const int LGUI_MAX_VERTEX_COUNT = 65535;
typedef FDynamicMeshIndexBuffer16 FLGUIMeshIndexBuffer;
#endif

#ifndef FLGUIIndexType
#define FLGUIIndexType UE_DEPRECATED_MACRO(5.0, "FLGUIIndexType has been renamed to FLGUIMeshIndexBufferType") FLGUIMeshIndexBufferType
#endif
