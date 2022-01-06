// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "LTweenDelegateHandleWrapper.generated.h"

/** Just a wrapper for blueprint to store a delegate handle */
USTRUCT(BlueprintType)
struct LTWEEN_API FLTweenDelegateHandleWrapper
{
	GENERATED_BODY()
public:
	FLTweenDelegateHandleWrapper() {}
	FLTweenDelegateHandleWrapper(FDelegateHandle InDelegateHandle)
	{
		DelegateHandle = InDelegateHandle;
	}
	FDelegateHandle DelegateHandle;
};
