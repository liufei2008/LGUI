// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComponentVisualizer.h"
#include "UIFlexibleGridLayoutComponentVisualizer.generated.h"

class UUIFlexibleGridLayout;

struct HUIFlexibleGridLayoutSpliterVisProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();
	HUIFlexibleGridLayoutSpliterVisProxy(const UUIFlexibleGridLayout* InComponent, bool InHorizontalOrVertical, bool InFirstOrSecond, int32 InSpliterIndex);
	virtual EMouseCursor::Type GetMouseCursor()override
	{
		return EMouseCursor::CardinalCross;
	}
	bool bHorizontalOrVertical = true;
	int32 SpliterIndex = 0;
	bool bFirstOrSecond = true;
};

struct HUIFlexibleGridLayoutFrameLineVisProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();
	enum class EFrameType
	{
		Left, Top, Right, Bottom
	};
	HUIFlexibleGridLayoutFrameLineVisProxy(const UUIFlexibleGridLayout* InComponent, EFrameType InFrameType);
	virtual EMouseCursor::Type GetMouseCursor()override
	{
		return EMouseCursor::Crosshairs;
	}
	EFrameType FrameType = EFrameType::Left;
};

UCLASS(Transient)
class LGUICOMPONENTVISUALIZER_API UUIFlexibleGridLayoutVisualizerSelectionState : public UObject
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

	HUIFlexibleGridLayoutFrameLineVisProxy::EFrameType FrameType = HUIFlexibleGridLayoutFrameLineVisProxy::EFrameType::Left;
	FVector CurrentClickPoint = FVector::ZeroVector;
};

class FUIFlexibleGridLayoutComponentVisualizer : public FComponentVisualizer
{
public:
	FUIFlexibleGridLayoutComponentVisualizer();
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
	UUIFlexibleGridLayoutVisualizerSelectionState* SelectionState = nullptr;
	TWeakObjectPtr<UUIFlexibleGridLayout> TargetComp = nullptr;
};
