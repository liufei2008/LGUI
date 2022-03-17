// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "DynamicMeshBuilder.h"

#ifdef LGUI_USE_32BIT_INDEXBUFFER
typedef uint32 FLGUIIndexType;
const int LGUI_MAX_VERTEX_COUNT = 2147483647;
typedef FDynamicMeshIndexBuffer32 FLGUIMeshIndexBuffer;
#else
typedef uint16 FLGUIIndexType;
const int LGUI_MAX_VERTEX_COUNT = 65535;
typedef FDynamicMeshIndexBuffer16 FLGUIMeshIndexBuffer;
#endif
