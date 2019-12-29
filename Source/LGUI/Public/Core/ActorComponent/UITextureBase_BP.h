// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "UITextureBase.h"
#include "UIRenderable_BP.h"
#include "UITextureBase_BP.generated.h"

//This is base class for create custom mesh based on UITexture. Just override OnCreateGeometry() and OnUpdateGeometry(...) to create or update your own geometry
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
	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnBeforeCreateOrUpdateGeometry"))
		void OnBeforeCreateOrUpdateGeometry_BP();
	//
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnCreateGeometry"))
		void OnCreateGeometry_BP(ULGUICreateGeometryHelper* InCreateGeometryHelper);
	//update geometry data. Do Not add or remove any vertex or triangles in this function
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnUpdateGeometry"))
		void OnUpdateGeometry_BP(ULGUIUpdateGeometryHelper* InUpdateGoemetryHelper, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);
public:
	//if vertex data change and vertex count not change.
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DisplayName = "MarkVertexChanged"))
		void MarkVertexChanged_BP();
	//if vertex count change or triangle count change, call this
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DisplayName = "MarkRebuildGeometry"))
		void MarkRebuildGeometry_BP();
private:
	UPROPERTY(Transient)ULGUICreateGeometryHelper* createGeometryHelper;
	UPROPERTY(Transient)ULGUIUpdateGeometryHelper* updateGeometryHelper;
};
