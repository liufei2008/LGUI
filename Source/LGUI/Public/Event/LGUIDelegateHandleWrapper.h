// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "LGUIDelegateHandleWrapper.generated.h"

/**
 *Just a wrapper for blueprint to store a delegate handle
 */
USTRUCT(BlueprintType)
struct LGUI_API FLGUIDelegateHandleWrapper
{
	GENERATED_BODY()
public:
	FLGUIDelegateHandleWrapper() {}
	FLGUIDelegateHandleWrapper(FDelegateHandle InDelegateHandle)
	{
		DelegateHandle = InDelegateHandle;
	}
	FDelegateHandle DelegateHandle;
};
