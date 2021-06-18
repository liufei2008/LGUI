// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"

#ifdef LGUI_USE_32BIT_INDEXBUFFER
typedef uint32 FLGUIIndexType;
const int MAX_TRIANGLE_COUNT = 2147483647;
typedef FDynamicMeshIndexBuffer32 FLGUIMeshIndexBuffer;
#else
typedef uint16 FLGUIIndexType;
const int MAX_TRIANGLE_COUNT = 65534;
typedef FDynamicMeshIndexBuffer16 FLGUIMeshIndexBuffer;
#endif
