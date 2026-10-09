// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LexWorldSpaceRaycasterBase.h"
#include "LexWorldSpaceRaycaster.generated.h"

/**
 * Raycast on world space UI, need LexCanvas component on same actor
 */
UCLASS(ClassGroup = LGUI, meta=(BlueprintSpawnableComponent))
class LGUI_API ULexWorldSpaceRaycaster : public ULexWorldSpaceRaycasterBase
{
	GENERATED_BODY()

public:
	ULexWorldSpaceRaycaster();

protected:
	TWeakObjectPtr<ULexCanvas> RootCanvas;
	bool CheckRootCanvas();
	
	virtual void BeginPlay() override;
	virtual void Raycast(ULexPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, TArray<FLexUIHitResult>& OutHitResultArray)override;
};
