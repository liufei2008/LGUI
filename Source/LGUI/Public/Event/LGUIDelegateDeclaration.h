// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "Engine/Engine.h"
#include "CoreMinimal.h"
#include "Event/LGUIPointerEventData.h"

DECLARE_DELEGATE_OneParam(FLGUIBoolDelegate, bool);
DECLARE_DELEGATE_OneParam(FLGUIFloatDelegate, float);
DECLARE_DELEGATE_OneParam(FLGUIVector2Delegate, FVector2D);
DECLARE_DELEGATE_OneParam(FLGUIStringDelegate, const FString&);
DECLARE_DELEGATE_OneParam(FLGUIInt32Delegate, int32);

DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIMulticastBoolDelegate, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIMulticastFloatDelegate, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIMulticastVector2Delegate, FVector2D);
DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIMulticastStringDelegate, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIMulticastInt32Delegate, int32);

DECLARE_DELEGATE_ThreeParams(FLGUIHitDelegate, bool, const FHitResult&, USceneComponent*);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FLGUIMulticastHitDelegate, bool, const FHitResult&, USceneComponent*);
DECLARE_DELEGATE_OneParam(FLGUIPointerEventDelegate, ULGUIPointerEventData*);
DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIMulticastPointerEventDelegate, ULGUIPointerEventData*);
DECLARE_DELEGATE_OneParam(FLGUIBaseEventDelegate, ULGUIBaseEventData*);
DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIMulticastBaseEventDelegate, ULGUIBaseEventData*);