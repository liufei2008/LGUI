// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UITextureBase.h"
#include "UIBatchGeometryRenderable_BP.h"
#include "UITextureBase_BP.generated.h"

/** 
 * This is base class for create custom mesh based on UITexture. Just override OnCreateGeometry() and OnUpdateGeometry(...) to create or update your own geometry
 */
UCLASS(ClassGroup = (LGUI), Abstract, Blueprintable)
class LGUI_API UUITextureBase_BP : public UUITextureBase
{
	GENERATED_BODY()

public:	
	UUITextureBase_BP(const FObjectInitializer& ObjectInitializer);
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
protected:
	virtual void OnBeforeCreateOrUpdateGeometry()override;
	virtual void OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnBeforeCreateOrUpdateGeometry"))
		void ReceiveOnBeforeCreateOrUpdateGeometry();
	/** update geometry data. Do Not add or remove any vertex or triangles in this function */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnUpdateGeometry"))
		void ReceiveOnUpdateGeometry(ULGUICreateGeometryHelper* InGoemetryHelper, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);

protected:
	UPROPERTY(Transient)ULGUICreateGeometryHelper* createGeometryHelper;
};
