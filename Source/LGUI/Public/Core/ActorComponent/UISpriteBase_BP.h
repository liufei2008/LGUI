// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "Core/LGUISpriteData.h"
#include "UISpriteBase.h"
#include "UIBatchGeometryRenderable_BP.h"
#include "UISpriteBase_BP.generated.h"

/** 
 * This is base class for create custom mesh based on UISprite. Just override OnCreateGeometry() and OnUpdateGeometry(...) to create or update your own geometry
 */
UCLASS(ClassGroup = (LGUI), Abstract, Blueprintable)
class LGUI_API UUISpriteBase_BP : public UUISpriteBase
{
	GENERATED_BODY()

public:	
	UUISpriteBase_BP(const FObjectInitializer& ObjectInitializer);
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
protected:
	virtual void OnBeforeCreateOrUpdateGeometry()override;
	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnBeforeCreateOrUpdateGeometry"))
		void ReceiveOnBeforeCreateOrUpdateGeometry();
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnCreateGeometry"))
		void ReceiveOnCreateGeometry(ULGUICreateGeometryHelper* InCreateGeometryHelper, ULGUISpriteData_BaseObject* InSpriteData);
	/** update geometry data. Do Not add or remove any vertex or triangles in this function */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnUpdateGeometry"))
		void ReceiveOnUpdateGeometry(ULGUIUpdateGeometryHelper* InUpdateGoemetryHelper, ULGUISpriteData_BaseObject* InSpriteData, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);
public:
	/** if vertex data change and vertex count not change. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void MarkVertexChanged();
	/** if vertex count change or triangle count change, call this */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void MarkRebuildGeometry();
protected:
	UPROPERTY(Transient)ULGUICreateGeometryHelper* createGeometryHelper;
	UPROPERTY(Transient)ULGUIUpdateGeometryHelper* updateGeometryHelper;
};
