// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Layout/LGUICanvasScaler.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#if WITH_EDITOR
#include "Core/Actor/LGUIManagerActor.h"
#include "DrawDebugHelpers.h"
#include "Editor.h"
#endif
#include "Engine/TextureRenderTarget2D.h"


ULGUICanvasScaler::ULGUICanvasScaler()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULGUICanvasScaler::BeginPlay()
{
	Super::BeginPlay();
	CheckCanvas();
	SetCanvasProperties();
	if (CheckCanvas())
	{
		if (Canvas->IsRootCanvas())
		{
			if (Canvas->GetRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay
				|| Canvas->GetRenderMode() == ELGUIRenderMode::RenderTarget
				)
			{
				CheckAndApplyViewportParameter();

				if (Canvas->GetRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay)
				{
					if (auto world = GetWorld())
					{
						if (auto gameViewport = world->GetGameViewport())
						{
							if (auto viewport = gameViewport->Viewport)
							{
								_ViewportResizeDelegateHandle = viewport->ViewportResizedEvent.AddUObject(this, &ULGUICanvasScaler::OnViewportResized);
							}
						}
					}
				}
			}
		}
	}
}



void ULGUICanvasScaler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void ULGUICanvasScaler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (_ViewportResizeDelegateHandle.IsValid())
	{
		if (auto world = GetWorld())
		{
			if (auto gameViewport = world->GetGameViewport())
			{
				if (auto viewport = gameViewport->Viewport)
				{
					viewport->ViewportResizedEvent.Remove(_ViewportResizeDelegateHandle);
				}
			}
		}
	}
}


void ULGUICanvasScaler::CheckAndApplyViewportParameter()
{
	switch (Canvas->GetRenderMode())
	{
	case ELGUIRenderMode::ScreenSpaceOverlay:
	{
		ViewportSize = Canvas->GetViewportSize();
		OnViewportParameterChanged();
	}
	break;
	case ELGUIRenderMode::RenderTarget:
	{
		auto renderTarget = Canvas->GetRenderTarget();
		ViewportSize.X = renderTarget->SizeX;
		ViewportSize.Y = renderTarget->SizeY;
		OnViewportParameterChanged();
	}
	break;
	}
}
void ULGUICanvasScaler::OnViewportResized(FViewport* viewport, uint32)
{
	ViewportSize = Canvas->GetViewportSize();//why not just get the viewport size from "viewport" parameter? because assets editor's viewport(ie. material, texture editor viewport) can fire the same event, and size is assets editor's viewport size
	OnViewportParameterChanged();
}
void ULGUICanvasScaler::OnViewportParameterChanged()
{
	if (CheckCanvas())
	{
		if (Canvas->IsRootCanvas())
		{
			if (Canvas->GetRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay
				|| Canvas->GetRenderMode() == ELGUIRenderMode::RenderTarget
				)
			{
				auto canvasUIItem = Canvas->CheckAndGetUIItem();
				if (IsValid(canvasUIItem))
				{
					//adjust size
					switch (UIScaleMode)
					{
					case LGUIScaleMode::ConstantPixelSize:
					{
						canvasUIItem->SetWidth(ViewportSize.X);
						canvasUIItem->SetHeight(ViewportSize.Y);
#if WITH_EDITOR
						if (!GetWorld()->IsGameWorld()) {}//editor don't change size
						else
#endif
						{
							canvasUIItem->SetRelativeScale3D(FVector::OneVector);
						}
					}
					break;
					case LGUIScaleMode::ScaleWithScreenSize:
					{
						switch (ScreenMatchMode)
						{
						case LGUIScreenMatchMode::MatchWidthOrHeight:
						{
							float matchWidth_PreferredWidth = ReferenceResolution.X;
							float matchWidth_PreferredHeight = ReferenceResolution.X * ViewportSize.Y / ViewportSize.X;
							float matchWidth_ScaleRatio = ViewportSize.X / ReferenceResolution.X;

							float matchHeight_PreferredHeight = ReferenceResolution.Y;
							float matchHeight_PreferredWidth = ReferenceResolution.Y * ViewportSize.X / ViewportSize.Y;
							float matchHeight_ScaleRatio = ViewportSize.Y / ReferenceResolution.Y;

							canvasUIItem->SetWidth(FMath::Lerp(matchWidth_PreferredWidth, matchHeight_PreferredWidth, MatchFromWidthToHeight));
							canvasUIItem->SetHeight(FMath::Lerp(matchWidth_PreferredHeight, matchHeight_PreferredHeight, MatchFromWidthToHeight));
#if WITH_EDITOR
							if (!GetWorld()->IsGameWorld()) {}//editor don't change size
							else
#endif
							{
								canvasUIItem->SetRelativeScale3D(FVector::OneVector * FMath::Lerp(matchWidth_ScaleRatio, matchHeight_ScaleRatio, MatchFromWidthToHeight));
							}
						}
						break;
						case LGUIScreenMatchMode::Expand:
						case LGUIScreenMatchMode::Shrink:
						{
							float resultWidth = ViewportSize.X, resultHeight = ViewportSize.Y, resultScale = 1.0f;

							float screenAspect = (float)ViewportSize.X / ViewportSize.Y;
							float referenceAspect = ReferenceResolution.X / ReferenceResolution.Y;
							if (screenAspect > referenceAspect)//screen width > reference width
							{
								if (ScreenMatchMode == LGUIScreenMatchMode::Expand)
								{
									resultHeight = ReferenceResolution.Y;
									resultWidth = resultHeight * screenAspect;
									resultScale = (float)ViewportSize.Y / resultHeight;
								}
								else if (ScreenMatchMode == LGUIScreenMatchMode::Shrink)
								{
									resultWidth = ReferenceResolution.X;
									resultHeight = resultWidth / screenAspect;
									resultScale = (float)ViewportSize.X / resultWidth;
								}
							}
							else//screen height > reference height
							{
								if (ScreenMatchMode == LGUIScreenMatchMode::Expand)
								{
									resultWidth = ReferenceResolution.X;
									resultHeight = resultWidth / screenAspect;
									resultScale = (float)ViewportSize.X / resultWidth;
								}
								else if (ScreenMatchMode == LGUIScreenMatchMode::Shrink)
								{
									resultHeight = ReferenceResolution.Y;
									resultWidth = resultHeight * screenAspect;
									resultScale = (float)ViewportSize.Y / resultHeight;
								}
							}
							canvasUIItem->SetWidth(resultWidth);
							canvasUIItem->SetHeight(resultHeight);
#if WITH_EDITOR
							if (!GetWorld()->IsGameWorld()) {}//editor don't change size
							else
#endif
							{
								canvasUIItem->SetRelativeScale3D(FVector::OneVector * resultScale);
							}
						}
						break;
						}
					}
					break;
					}
#if WITH_EDITOR
					if (!GetWorld()->IsGameWorld()) {}//editor don't change size
					else
#endif
					{
						canvasUIItem->SetRelativeLocationAndRotation(FVector(ViewportSize.X * 0.5f, ViewportSize.Y * 0.5f, 0), FQuat::Identity);//set rotate to zero, and move left bottom corner to zero position
					}

					Canvas->MarkRebuildAllDrawcall();
					canvasUIItem->MarkAllDirtyRecursive();
					Canvas->MarkCanvasUpdate();
				}
			}
		}
	}
}


#if WITH_EDITOR
void ULGUICanvasScaler::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto property = PropertyChangedEvent.Property)
	{
		SetCanvasProperties();
		OnViewportParameterChanged();
	}
}
void ULGUICanvasScaler::OnRegister()
{
	Super::OnRegister();
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
	{
		if (ULGUIEditorManagerObject::Instance != nullptr)
		{
			EditorTickDelegateHandle = ULGUIEditorManagerObject::Instance->EditorTick.AddUObject(this, &ULGUICanvasScaler::OnEditorTick);
		}
	}
}
void ULGUICanvasScaler::OnUnregister()
{
	Super::OnUnregister();
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
	{
		if (ULGUIEditorManagerObject::Instance != nullptr)
		{
			ULGUIEditorManagerObject::Instance->EditorTick.Remove(EditorTickDelegateHandle);
		}
	}
}
void ULGUICanvasScaler::OnEditorTick(float DeltaTime)
{
	if (CheckCanvas())
	{
		if (Canvas->IsRootCanvas())
		{
			if (Canvas->GetRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay)
			{
				DrawVirtualCamera();

#if WITH_EDITORONLY_DATA
				if (TestWithEditorViewportSize && !GetWorld()->IsGameWorld())
				{
					if (auto viewport = GEditor->GetActiveViewport())
					{
						auto prevSize = ViewportSize;
						ViewportSize = viewport->GetSizeXY();
						if (prevSize != ViewportSize)
						{
							OnViewportParameterChanged();
						}
					}
				}
				else
#endif
				{
					auto newViewportSize = Canvas->GetViewportSize();
					if (newViewportSize != ViewportSize)
					{
						ViewportSize = newViewportSize;
						OnViewportParameterChanged();
					}
				}
			}
		}
	}
}
void DeprojectViewPointToWorld(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewPoint01, FVector& OutWorldStart, FVector& OutWorldEnd)
{
	FMatrix InvViewProjMatrix = InViewProjectionMatrix.InverseFast();

	const float ScreenSpaceX = (InViewPoint01.X - 0.5f) * 2.0f;
	const float ScreenSpaceY = (InViewPoint01.Y - 0.5f) * 2.0f;

	// The start of the raytrace is defined to be at mousex,mousey,1 in projection space (z=1 is near, z=0 is far - this gives us better precision)
	// To get the direction of the raytrace we need to use any z between the near and the far plane, so let's use (mousex, mousey, 0.5)
	const FVector4 RayStartProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 1.0f, 1.0f);
	const FVector4 RayEndProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 0, 1.0f);

	// Projection (changing the W coordinate) is not handled by the FMatrix transforms that work with vectors, so multiplications
	// by the projection matrix should use homogeneous coordinates (i.e. FPlane).
	const FVector4 HGRayStartWorldSpace = InvViewProjMatrix.TransformFVector4(RayStartProjectionSpace);
	const FVector4 HGRayEndWorldSpace = InvViewProjMatrix.TransformFVector4(RayEndProjectionSpace);
	FVector RayStartWorldSpace(HGRayStartWorldSpace.X, HGRayStartWorldSpace.Y, HGRayStartWorldSpace.Z);
	FVector RayEndWorldSpace(HGRayEndWorldSpace.X, HGRayEndWorldSpace.Y, HGRayEndWorldSpace.Z);
	// divide vectors by W to undo any projection and get the 3-space coordinate 
	if (HGRayStartWorldSpace.W != 0.0f)
	{
		RayStartWorldSpace /= HGRayStartWorldSpace.W;
	}
	if (HGRayEndWorldSpace.W != 0.0f)
	{
		RayEndWorldSpace /= HGRayEndWorldSpace.W;
	}
	// Finally, store the results in the outputs
	OutWorldStart = RayStartWorldSpace;
	OutWorldEnd = RayEndWorldSpace;
}
void ULGUICanvasScaler::DrawVirtualCamera()
{
	if (CheckCanvas())
	{
		if (!LGUIManager::IsSelected_Editor(this->GetOwner()))return;
		auto ViewProjectionMatrix = Canvas->GetViewProjectionMatrix();
		FVector leftBottom, rightBottom, leftTop, rightTop;
		FVector leftBottomEnd, rightBottomEnd, leftTopEnd, rightTopEnd;
		auto lineColor = FColor::Green;
		//draw view frustum
		DeprojectViewPointToWorld(ViewProjectionMatrix, FVector2D(0, 0), leftBottom, leftBottomEnd);
		DrawDebugLine(this->GetWorld(), leftBottom, leftBottomEnd, lineColor);
		DeprojectViewPointToWorld(ViewProjectionMatrix, FVector2D(1, 0), rightBottom, rightBottomEnd);
		DrawDebugLine(this->GetWorld(), rightBottom, rightBottomEnd, lineColor);
		DeprojectViewPointToWorld(ViewProjectionMatrix, FVector2D(0, 1), leftTop, leftTopEnd);
		DrawDebugLine(this->GetWorld(), leftTop, leftTopEnd, lineColor);
		DeprojectViewPointToWorld(ViewProjectionMatrix, FVector2D(1, 1), rightTop, rightTopEnd);
		DrawDebugLine(this->GetWorld(), rightTop, rightTopEnd, lineColor);
		//draw near clip plane
		DrawDebugLine(this->GetWorld(), leftBottom, rightBottom, lineColor);
		DrawDebugLine(this->GetWorld(), leftBottom, leftTop, lineColor);
		DrawDebugLine(this->GetWorld(), rightTop, rightBottom, lineColor);
		DrawDebugLine(this->GetWorld(), rightTop, leftTop, lineColor);
		//draw far clip plane
		DrawDebugLine(this->GetWorld(), leftBottomEnd, rightBottomEnd, lineColor);
		DrawDebugLine(this->GetWorld(), leftBottomEnd, leftTopEnd, lineColor);
		DrawDebugLine(this->GetWorld(), rightTopEnd, rightBottomEnd, lineColor);
		DrawDebugLine(this->GetWorld(), rightTopEnd, leftTopEnd, lineColor);

		auto viewRotationMatrix = Canvas->GetViewRotationMatrix();
		viewRotationMatrix = FMatrix(
			FPlane(0, 0, 1, 0),
			FPlane(1, 0, 0, 0),
			FPlane(0, 1, 0, 0),
			FPlane(0, 0, 0, 1)) * viewRotationMatrix;
		DrawDebugCamera(this->GetWorld(), Canvas->GetViewLocation(), viewRotationMatrix.Rotator(), FOVAngle, 1.0f, FColor::Green);
	}
}
#endif

bool ULGUICanvasScaler::CheckCanvas()
{
	if (Canvas != nullptr)return true;
	Canvas = GetOwner()->FindComponentByClass<ULGUICanvas>();
	if (Canvas == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("UICanvasScalar component should only attach to a actor which have LGUICanvas component!"));
		return false;
	}
	else
	{
		SetCanvasProperties();
		return true;
	}
}
void ULGUICanvasScaler::SetCanvasProperties()
{
	Canvas->SetProjectionParameters(ProjectionType, FOVAngle, NearClipPlane, FarClipPlane);
}

void ULGUICanvasScaler::SetProjectionType(TEnumAsByte<ECameraProjectionMode::Type> value)
{
	if (ProjectionType != value)
	{
		ProjectionType = ProjectionType = value;
		OnViewportParameterChanged();
		SetCanvasProperties();
	}
}
void ULGUICanvasScaler::SetFovAngle(float value)
{
	if (FOVAngle != value)
	{
		FOVAngle = FOVAngle = value;
		OnViewportParameterChanged();
		SetCanvasProperties();
	}
}
void ULGUICanvasScaler::SetNearClipPlane(float value)
{
	if (NearClipPlane != value)
	{
		NearClipPlane = value;
		OnViewportParameterChanged();
		SetCanvasProperties();
	}
}
void ULGUICanvasScaler::SetFarClipPlane(float value)
{
	if (FarClipPlane != value)
	{
		FarClipPlane = value;
		OnViewportParameterChanged();
		SetCanvasProperties();
	}
}

void ULGUICanvasScaler::SetUIScaleMode(LGUIScaleMode value)
{
	if (UIScaleMode != value)
	{
		UIScaleMode = value;
		OnViewportParameterChanged();
	}
}
void ULGUICanvasScaler::SetReferenceResolution(FVector2D value)
{
	if (ReferenceResolution != value)
	{
		ReferenceResolution = value;
		OnViewportParameterChanged();
	}
}
void ULGUICanvasScaler::SetMatchFromWidthToHeight(float value)
{
	if (MatchFromWidthToHeight != value)
	{
		MatchFromWidthToHeight = value;
		OnViewportParameterChanged();
	}
}
void ULGUICanvasScaler::SetScreenMatchMode(LGUIScreenMatchMode value)
{
	if (ScreenMatchMode != value)
	{
		ScreenMatchMode = value;
		OnViewportParameterChanged();
	}
}