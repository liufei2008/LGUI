// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexVisualPostProcess.h"
#include "LexBackBufferCopy.generated.h"

/** 
 * UI element that can copy a back-buffer to a texture, so we can use it in our material
 * Use it in ScreenSpace or WorldSpace-LexUIRenderer.
 * If android OpenGL ES3.1, need to enable "ProjectSettings/Platforms/Android/Build/Support Backbuffer Sampling on OpenGL".
 */
UCLASS(ClassGroup = (LGUI), NotBlueprintable)
class LGUI_API ULexBackBufferCopy : public ULexVisualPostProcess
{
	GENERATED_BODY()

public:	
	ULexBackBufferCopy(const FObjectInitializer& ObjectInitializer);

private:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:

	virtual FLexVisualPostProcessRenderProxy* GetRenderProxy()override;
	virtual void MarkAllDirty()override;
private:
	virtual void SendRegionVertexDataToRenderProxy()override;
	void SendOthersDataToRenderProxy();
};
