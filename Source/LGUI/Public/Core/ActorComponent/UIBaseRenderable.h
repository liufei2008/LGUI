// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIItem.h"
#include "UIBaseRenderable.generated.h"

class UIGeometry;
class UMaterialInterface;
class ULGUICanvas;
class UUIDrawcall;

UENUM(BlueprintType, Category = LGUI)
enum class EUIRenderableType :uint8
{
	None,
	UIBatchGeometryRenderable,
	UIPostProcessRenderable,
	UIDirectMeshRenderable,
};
/** Base class of UI element that can be renderred by LGUICanvas */
UCLASS(Abstract, NotBlueprintable)
class LGUI_API UUIBaseRenderable : public UUIItem
{
	GENERATED_BODY()

public:	
	UUIBaseRenderable(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	EUIRenderableType uiRenderableType = EUIRenderableType::None;
public:

	virtual void ApplyUIActiveState() override;
	virtual void OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)override;
	virtual void DepthChanged()override;
	/** get UI renderable type */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		EUIRenderableType GetUIRenderableType()const { return uiRenderableType; }

	TSharedPtr<UUIDrawcall> drawcall = nullptr;//drawcall that response for this UI. @todo: use TWeakPtr
};
