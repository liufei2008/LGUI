// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComponentVisualizer.h"
#include "UIPanelLayoutFlexibleGridComponentVisualizer.generated.h"

class UUIPanelLayout_FlexibleGrid;

struct HUIPanelLayoutFlexibleGridSpliterVisProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();
	HUIPanelLayoutFlexibleGridSpliterVisProxy(const UUIPanelLayout_FlexibleGrid* InComponent, bool InHorizontalOrVertical, bool InFirstOrSecond, int32 InSpliterIndex);
	virtual EMouseCursor::Type GetMouseCursor()override
	{
		return EMouseCursor::CardinalCross;
	}
	bool bHorizontalOrVertical = true;
	int32 SpliterIndex = 0;
	bool bFirstOrSecond = true;
};

struct HUIPanelLayoutFlexibleGridFrameLineVisProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();
	enum class EFrameType
	{
		Left, Top, Right, Bottom
	};
	HUIPanelLayoutFlexibleGridFrameLineVisProxy(const UUIPanelLayout_FlexibleGrid* InComponent, EFrameType InFrameType);
	virtual EMouseCursor::Type GetMouseCursor()override
	{
		return EMouseCursor::Crosshairs;
	}
	EFrameType FrameType = EFrameType::Left;
};

UCLASS(Transient)
class LGUICOMPONENTVISUALIZER_API UUIPanelLayoutFlexibleGridVisualizerSelectionState : public UObject
{
	GENERATED_BODY()
public:
	enum class EHitProxyType
	{
		Frame, Spliter,
	};
	EHitProxyType ProxyType = EHitProxyType::Spliter;

	bool bHorizontalOrVertical = true;
	bool bFirstOrSecond = true;
	int32 SelectedSpliterIndex = -1;

	HUIPanelLayoutFlexibleGridFrameLineVisProxy::EFrameType FrameType = HUIPanelLayoutFlexibleGridFrameLineVisProxy::EFrameType::Left;
	FVector CurrentClickPoint = FVector::ZeroVector;
};

class FUIPanelLayoutFlexibleGridComponentVisualizer : public FComponentVisualizer
{
public:
	FUIPanelLayoutFlexibleGridComponentVisualizer();
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltalRotate, FVector& DeltaScale) override;
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	virtual TSharedPtr<SWidget> GenerateContextMenu() const override;

private:
	void AddSpliter();
	void RemoveSpliter();
	void ChangeSpliterType();
	void LayoutChangeSizeToPixelRelevant(bool InHorizontalOrVertical);
	UUIPanelLayoutFlexibleGridVisualizerSelectionState* SelectionState = nullptr;
	TWeakObjectPtr<UUIPanelLayout_FlexibleGrid> TargetComp = nullptr;
};
