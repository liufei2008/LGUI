// Copyright 2025-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LexVisualBatchMesh.h"
#include "LexVisualEmpty.generated.h"

/**
 * LexVisualEmpty is just an empty visual, it will not render, but can handle raycast event
 */
UCLASS(ClassGroup = (LGUI), BlueprintType, Blueprintable)
class LGUI_API ULexVisualEmpty : public ULexVisual
{
	GENERATED_BODY()
public:
	ULexVisualEmpty(const FObjectInitializer& ObjectInitializer);

protected:
	
	virtual void PostInitProperties() override;
	virtual void BeginDestroy() override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:
	
};
