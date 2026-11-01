// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputKeyEventArgs.h"
#include "Core/Components/LexWidget.h"
#include "Core/LexUIMesh/LexUIGizmoMesh.h"
#include "Core/LexUIManager.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "UnrealWidgetFwd.h"

class FLexUIPrefabEditorViewportClient;
class FScopedTransaction;
class FSceneViewFamilyContext;

/**
 * Move / rotate gizmo of the prefab editor viewport. Drawn with FLexUIGizmoMesh instead of the engine
 * transform widget, because prefab widgets are not level actors.
 */
class FLexUITransformWidget
{
public:
	FLexUITransformWidget(UWorld* InWorld, ULexWidget* InWidget, FLexUIPrefabEditorViewportClient* InViewportClient);
	~FLexUITransformWidget();

	void Tick();
	bool IsDragging()const { return bIsDragging; }
	bool HandleInputKey(const FInputKeyEventArgs& EventArgs);

private:
	void UpdateAxis();
	/** Coordinate system the gizmo is drawn and dragged in, follows the viewport's Ctrl+` toggle. */
	ECoordSystem GetCoordSystem()const;
	FVector PressAxisHitPoint = FVector::Zero();
	FVector PressAxisVector = FVector::ZeroVector;
	FTransform ThisTransformWhenPress = FTransform::Identity;
	FTransform ThisTransform = FTransform::Identity;
	FTransform RenderTransform = FTransform::Identity;
	float PressRenderScale = 1.0f;
	TWeakObjectPtr<ULexUIManagerWorldSubsystem> LexUIManager;
	TWeakObjectPtr<UWorld> World;
	TSharedPtr<FLexUIGizmoMesh> MoveAxisX;
	TSharedPtr<FLexUIGizmoMesh> MoveAxisY;
	TSharedPtr<FLexUIGizmoMesh> MoveAxisZ;
	TSharedPtr<FLexUIGizmoMesh> MovePlaneYZ;
	TSharedPtr<FLexUIGizmoMesh> MovePlaneZX;
	TSharedPtr<FLexUIGizmoMesh> MovePlaneXY;
	TSharedPtr<FLexUIGizmoMesh> RotateAxisX;
	TSharedPtr<FLexUIGizmoMesh> RotateAxisY;
	TSharedPtr<FLexUIGizmoMesh> RotateAxisZ;
	TWeakObjectPtr<UMaterialInterface> GizmoMaterial;
	TWeakObjectPtr<UMaterialInterface> RotateGizmoMaterial;
	FVector MovePlaneYZCenter;
	FVector MovePlaneZXCenter;
	FVector MovePlaneXYCenter;
	const float AxisLength = 100.0f;
	const float AxisPlaneSize = 30.0f;
	const float RotateAxisRadius = 100.0f;
	FColor ColorAxisX = FColor::Red, ColorAxisY = FColor::Green, ColorAxisZ = FColor::Blue;
	FColor HighlightColor = FColor::Yellow;
	bool bCanTick = false;
	enum class EMoveAxisType
	{
		None, X, Y, Z, YZ, ZX, XY,
	};
	EMoveAxisType MoveAxisType = EMoveAxisType::None;
	enum class ERotateAxisType
	{
		None, X, Y, Z,
	};
	ERotateAxisType RotateAxisType = ERotateAxisType::None;
	enum class ETransformType
	{
		None, Move, Rotate,
	};
	ETransformType TransformType = ETransformType::Move;
	bool bIsMousePressedAtThisFrame = false;
	bool bIsDragging = false;
	/** Coordinate system captured when the drag starts, so toggling space mid-drag cannot flip it under the mouse. */
	ECoordSystem DragCoordSystem = COORD_Local;
	TWeakObjectPtr<ULexWidget> SelectedWidget;
	FLexUIPrefabEditorViewportClient* ViewportClient = nullptr;
	TUniquePtr<FSceneViewFamilyContext> ViewFamily = nullptr;
	TUniquePtr<FScopedTransaction> Transaction = nullptr;
};
