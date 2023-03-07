// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComponentVisualizer.h"

class UUIItem;

enum class EUIItemVisualizerSelectorType
{
	Left, Right, Top, Bottom,
	LeftTop, RightTop, LeftBottom, RightBottom,
	Pivot,
};

struct HUIItemAnchorVisProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();
	HUIItemAnchorVisProxy(const UUIItem* InComponent, EUIItemVisualizerSelectorType InType);
	virtual EMouseCursor::Type GetMouseCursor()override
	{
		return EMouseCursor::CardinalCross;
	}
	EUIItemVisualizerSelectorType Type;
};

class FUIItemComponentVisualizer : public FComponentVisualizer
{
public:
	FUIItemComponentVisualizer();
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)override;
	virtual bool HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltalRotate, FVector& DeltaScale) override;
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	virtual bool GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const override;

private:
	EUIItemVisualizerSelectorType SelectorType;
	TWeakObjectPtr<UUIItem> TargetComp = nullptr;
};
