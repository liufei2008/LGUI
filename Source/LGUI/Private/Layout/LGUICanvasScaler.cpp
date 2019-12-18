// Copyright 2019 LexLiu. All Rights Reserved.

#include "Layout/LGUICanvasScaler.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#if WITH_EDITOR
#include "Core/Actor/LGUIManagerActor.h"
#include "DrawDebugHelpers.h"
#endif


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
			if (Canvas->GetRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay)
			{
				CheckAndApplyViewportParameter();

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
	CurrentViewportSize = Canvas->GetViewportSize();
	OnViewportParameterChanged();
}
void ULGUICanvasScaler::OnViewportResized(FViewport* viewport, uint32)
{
	CurrentViewportSize = Canvas->GetViewportSize();//why not just get the viewport size from "viewport" parameter? because assets editor's viewport(ie. material, texture editor viewport) can fire the same event, and size is assets editor's viewport size
	OnViewportParameterChanged();
}
void ULGUICanvasScaler::OnViewportParameterChanged()
{
	if (CheckCanvas())
	{
		if (Canvas->IsRootCanvas())
		{
			if (Canvas->GetRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay)
			{
				auto halfFov = FOVAngle * 0.5f;//ue4 us horizontal fov
				Canvas->CheckAndGetUIItem()->SetRelativeScale3D(FVector::OneVector);

				//adjust size
#if WITH_EDITOR
				if (!GetWorld()->IsGameWorld())
				{
					Canvas->SetViewportParameterChange();
					Canvas->MarkRebuildAllDrawcall();
					Canvas->MarkCanvasUpdate();
				}
				else
#endif
				{
					switch (UIScaleMode)
					{
					case LGUIScaleMode::ScaleWithScreenWidth:
					{
						Canvas->CheckAndGetUIItem()->SetWidth(PreferredWidth);
						Canvas->CheckAndGetUIItem()->SetHeight(PreferredWidth * CurrentViewportSize.Y / CurrentViewportSize.X);
					}
					break;
					case LGUIScaleMode::ScaleWithScreenHeight:
					{
						Canvas->CheckAndGetUIItem()->SetHeight(PreferredHeight);
						auto tempPreferredWidth = PreferredHeight * CurrentViewportSize.X / CurrentViewportSize.Y;
						Canvas->CheckAndGetUIItem()->SetWidth(tempPreferredWidth);
					}
					break;
					case LGUIScaleMode::ConstantPixelSize:
					{
						Canvas->CheckAndGetUIItem()->SetWidth(CurrentViewportSize.X);
						Canvas->CheckAndGetUIItem()->SetHeight(CurrentViewportSize.Y);
					}
					break;
					default:
						break;
					}
					Canvas->SetViewportParameterChange();
					Canvas->MarkCanvasUpdate();
				}
			}
		}
	}
}

void ULGUICanvasScaler::SetProjectionType(TEnumAsByte<ECameraProjectionMode::Type> value)
{
	if (ProjectionType != value)
	{
		ProjectionType = ProjectionType = value;
		OnViewportParameterChanged();
	}
}
void ULGUICanvasScaler::SetFovAngle(float value)
{
	if (FOVAngle != value)
	{
		FOVAngle = FOVAngle = value;
		OnViewportParameterChanged();
	}
}
void ULGUICanvasScaler::SetNearClipPlane(float value)
{
	if (NearClipPlane != value)
	{
		NearClipPlane = value;
		OnViewportParameterChanged();
	}
}
void ULGUICanvasScaler::SetFarClipPlane(float value)
{
	if (FarClipPlane != value)
	{
		FarClipPlane = value;
		OnViewportParameterChanged();
	}
}


#if WITH_EDITOR
void ULGUICanvasScaler::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto property = PropertyChangedEvent.Property)
	{
		auto propertyName = property->GetName();
		if (propertyName == TEXT("PreferredHeight") || propertyName == TEXT("PreferredWidth"))
		{
			switch (UIScaleMode)
			{
			case LGUIScaleMode::ScaleWithScreenWidth:
			{
				Canvas->CheckAndGetUIItem()->SetWidth(PreferredWidth);
				Canvas->CheckAndGetUIItem()->SetHeight(PreferredWidth * CurrentViewportSize.Y / CurrentViewportSize.X);
			}
			break;
			case LGUIScaleMode::ScaleWithScreenHeight:
			{
				Canvas->CheckAndGetUIItem()->SetHeight(PreferredHeight);
				auto tempPreferredWidth = PreferredHeight * CurrentViewportSize.X / CurrentViewportSize.Y;
				Canvas->CheckAndGetUIItem()->SetWidth(tempPreferredWidth);
			}
			break;
			case LGUIScaleMode::ConstantPixelSize:
			{
				Canvas->CheckAndGetUIItem()->SetWidth(CurrentViewportSize.X);
				Canvas->CheckAndGetUIItem()->SetHeight(CurrentViewportSize.Y);
			}
			break;
			default:
				break;
			}
		}
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
				auto newViewportSize = Canvas->GetViewportSize();
				if (newViewportSize != CurrentViewportSize)
				{
					CurrentViewportSize = newViewportSize;
					OnViewportParameterChanged();
				}
				DrawVirtualCamera();
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
		viewRotationMatrix = viewRotationMatrix * FMatrix(
			FPlane(0, 0, 1, 0),
			FPlane(1, 0, 0, 0),
			FPlane(0, 1, 0, 0),
			FPlane(0, 0, 0, 1)).Inverse();
		DrawDebugCamera(this->GetWorld(), Canvas->GetViewLocation(), viewRotationMatrix.Rotator(), FOVAngle, 1.0f, FColor::Green);
	}
}
#endif

void ULGUICanvasScaler::SetPreferredWidth(float InValue)
{
	if (PreferredWidth != InValue)
	{
		PreferredWidth = InValue;
		OnViewportParameterChanged();
	}
}
void ULGUICanvasScaler::SetPreferredHeight(float InValue)
{
	if (PreferredHeight != InValue)
	{
		PreferredHeight = InValue;
		OnViewportParameterChanged();
	}
}

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