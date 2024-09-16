// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/LGUICanvasScaler.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#if WITH_EDITOR
#include "Core/LGUIManager.h"
#include "PrefabSystem/LGUIPrefabManager.h"
#include "DrawDebugHelpers.h"
#include "Editor.h"
#include "Core/LGUISettings.h"
#include "Core/LGUIRender/LGUIRenderer.h"
#include "EditorViewportClient.h"
#include "LevelEditorViewport.h"
#endif
#include "Engine/TextureRenderTarget2D.h"
#include "UnrealClient.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"

void ULGUICanvasScalerCustomScale::Init(class ULGUICanvasScaler* InCanvasScaler)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveInit(InCanvasScaler);
	}
}
void ULGUICanvasScalerCustomScale::CalculateSizeAndScale(class ULGUICanvasScaler* InCanvasScaler, const FIntPoint& InViewportSize, FIntPoint& OutLGUICanvasSize, float& OutScale)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveCalculateSizeAndScale(InCanvasScaler, InViewportSize, OutLGUICanvasSize, OutScale);
	}
}

ULGUICanvasScaler::ULGUICanvasScaler()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULGUICanvasScaler::Awake()
{
	Super::Awake();
	if (IsValid(CustomScale))
	{
		CustomScale->Init(this);
	}
}
void ULGUICanvasScaler::OnEnable()
{
	Super::OnEnable();
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

void ULGUICanvasScaler::OnDisable()
{
	Super::OnDisable();
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

void ULGUICanvasScaler::ForceUpdate()
{
	CheckAndApplyViewportParameter();
}


void ULGUICanvasScaler::CheckAndApplyViewportParameter()
{
	if (CheckCanvas())
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
			if (auto renderTarget = Canvas->GetRenderTarget())
			{
				ViewportSize.X = renderTarget->SizeX;
				ViewportSize.Y = renderTarget->SizeY;
				OnViewportParameterChanged();
			}
		}
		break;
		}
	}
}
void ULGUICanvasScaler::OnViewportResized(FViewport* viewport, uint32)
{
	ViewportSize = Canvas->GetViewportSize();//why not just get the viewport size from "viewport" parameter? because assets editor's viewport(ie. material, texture editor viewport) can fire the same event, and size is assets editor's viewport size
	OnViewportParameterChanged();
}
void ULGUICanvasScaler::OnViewportParameterChanged()
{
	if (ViewportSize.X <= 0 || ViewportSize.Y <= 0)return;
	if (CheckCanvas())
	{
		if (Canvas->IsRootCanvas())
		{
			if (Canvas->GetRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay
				|| Canvas->GetRenderMode() == ELGUIRenderMode::RenderTarget
				)
			{
				auto canvasUIItem = Canvas->GetUIItem();
				if (canvasUIItem != nullptr)
				{
					float canvasScale = 1.0f;
					//adjust size
					switch (UIScaleMode)
					{
					case ELGUICanvasScaleMode::ConstantPixelSize:
					{
						canvasUIItem->SetWidth(ViewportSize.X);
						canvasUIItem->SetHeight(ViewportSize.Y);
						canvasScale = 1.0f;
					}
					break;
					case ELGUICanvasScaleMode::ScaleWithScreenSize:
					{
						switch (ScreenMatchMode)
						{
						case ELGUICanvasScreenMatchMode::MatchWidthOrHeight:
						{
							float matchWidth_PreferredWidth = ReferenceResolution.X;
							float matchWidth_PreferredHeight = ReferenceResolution.X * ViewportSize.Y / ViewportSize.X;
							float matchWidth_ScaleRatio = ViewportSize.X / ReferenceResolution.X;

							float matchHeight_PreferredHeight = ReferenceResolution.Y;
							float matchHeight_PreferredWidth = ReferenceResolution.Y * ViewportSize.X / ViewportSize.Y;
							float matchHeight_ScaleRatio = ViewportSize.Y / ReferenceResolution.Y;

							canvasUIItem->SetWidth(FMath::Lerp(matchWidth_PreferredWidth, matchHeight_PreferredWidth, MatchFromWidthToHeight));
							canvasUIItem->SetHeight(FMath::Lerp(matchWidth_PreferredHeight, matchHeight_PreferredHeight, MatchFromWidthToHeight));

							canvasScale = FMath::Lerp(matchWidth_ScaleRatio, matchHeight_ScaleRatio, MatchFromWidthToHeight);
						}
						break;
						case ELGUICanvasScreenMatchMode::Expand:
						case ELGUICanvasScreenMatchMode::Shrink:
						{
							float resultWidth = ViewportSize.X, resultHeight = ViewportSize.Y;

							float screenAspect = (float)ViewportSize.X / ViewportSize.Y;
							float referenceAspect = ReferenceResolution.X / ReferenceResolution.Y;
							if (screenAspect > referenceAspect)//screen width > reference width
							{
								if (ScreenMatchMode == ELGUICanvasScreenMatchMode::Shrink)
								{
									resultHeight = ReferenceResolution.Y;
									resultWidth = resultHeight * screenAspect;
									canvasScale = (float)ViewportSize.Y / resultHeight;
								}
								else if (ScreenMatchMode == ELGUICanvasScreenMatchMode::Expand)
								{
									resultWidth = ReferenceResolution.X;
									resultHeight = resultWidth / screenAspect;
									canvasScale = (float)ViewportSize.X / resultWidth;
								}
							}
							else//screen height > reference height
							{
								if (ScreenMatchMode == ELGUICanvasScreenMatchMode::Shrink)
								{
									resultWidth = ReferenceResolution.X;
									resultHeight = resultWidth / screenAspect;
									canvasScale = (float)ViewportSize.X / resultWidth;
								}
								else if (ScreenMatchMode == ELGUICanvasScreenMatchMode::Expand)
								{
									resultHeight = ReferenceResolution.Y;
									resultWidth = resultHeight * screenAspect;
									canvasScale = (float)ViewportSize.Y / resultHeight;
								}
							}
							canvasUIItem->SetWidth(resultWidth);
							canvasUIItem->SetHeight(resultHeight);
						}
						break;
						}
					}
					break;
					case ELGUICanvasScaleMode::Custom:
					{
						if (IsValid(CustomScale))
						{
							canvasScale = 1.0f;
							auto ScaledViewportSize = ViewportSize;
							CustomScale->CalculateSizeAndScale(this, ViewportSize, ScaledViewportSize, canvasScale);
							canvasUIItem->SetWidth(ScaledViewportSize.X);
							canvasUIItem->SetHeight(ScaledViewportSize.Y);
						}
						else
						{
							//default is constant pixel
							canvasUIItem->SetWidth(ViewportSize.X);
							canvasUIItem->SetHeight(ViewportSize.Y);
							canvasScale = 1.0f;
						}
					}
					break;
					}
					Canvas->canvasScale = canvasScale;

					canvasUIItem->MarkAllDirtyRecursive();
					Canvas->MarkCanvasUpdate(false, true, false, true);
				}
			}
		}
	}
}


void ULGUICanvasScaler::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITOR
	if (GetWorld() && !GetWorld()->IsGameWorld())
	{
		EditorTickDelegateHandle = ULGUIPrefabManagerObject::RegisterEditorTickFunction([this](float deltaTime) {
			this->OnEditorTick(deltaTime);
			});
		EditorViewportIndexAndKeyChangeDelegateHandle = ULGUIEditorManagerObject::RegisterEditorViewportIndexAndKeyChange([this] {
			this->OnEditorViewportIndexAndKeyChange();
			});
		LGUIPreview_ViewportIndexChangeDelegateHandle = ULGUIEditorSettings::LGUIPreviewSetting_EditorPreviewViewportIndexChange.AddUObject(this, &ULGUICanvasScaler::OnPreviewSetting_EditorPreviewViewportIndexChange);
	}
#endif
}
void ULGUICanvasScaler::OnUnregister()
{
	Super::OnUnregister();
#if WITH_EDITOR
	if (EditorTickDelegateHandle.IsValid())
	{
		ULGUIPrefabManagerObject::UnregisterEditorTickFunction(EditorTickDelegateHandle);
	}
	if (EditorViewportIndexAndKeyChangeDelegateHandle.IsValid())
	{
		ULGUIEditorManagerObject::UnregisterEditorViewportIndexAndKeyChange(EditorViewportIndexAndKeyChangeDelegateHandle);
	}
	if (LGUIPreview_ViewportIndexChangeDelegateHandle.IsValid())
	{
		ULGUIEditorSettings::LGUIPreviewSetting_EditorPreviewViewportIndexChange.Remove(LGUIPreview_ViewportIndexChangeDelegateHandle);
	}
#endif
	//reset the canvasScale to default
	if (Canvas.IsValid())
	{
		Canvas->canvasScale = 1.0f;
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
void ULGUICanvasScaler::OnEditorTick(float DeltaTime)
{
	if (ULGUIManagerWorldSubsystem::GetIsPlaying())//When hit play there is still a editor world and DrawViewportArea is called, which could cause frame dropdown, so skip it when playing
		return;
	if (CheckCanvas())
	{
		if (Canvas->IsRootCanvas())
		{
			if (Canvas->GetRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay
				|| Canvas->GetRenderMode() == ELGUIRenderMode::RenderTarget
				)
			{
				DrawViewportArea();
				if (ULGUIPrefabManagerObject::IsSelected(this->GetOwner()))
				{
					DrawVirtualCamera();
				}
				
				if (!GetWorld()->IsGameWorld())
				{
					if (Canvas->GetRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay)
					{
						FViewport* viewport = nullptr;

						int32 editorViewIndex = ULGUIEditorSettings::GetLGUIPreview_EditorViewIndex();
						auto& LevelViewportClients = GEditor->GetLevelViewportClients();
						for (auto& ViewportClient : LevelViewportClients)
						{
							if (ViewportClient->ViewIndex == editorViewIndex)
							{
								viewport = ViewportClient->Viewport;
								break;
							}
						}
						if (viewport == nullptr)
						{
							viewport = GEditor->GetActiveViewport();
						}
						if (viewport != nullptr)
						{
							auto prevSize = ViewportSize;
							ViewportSize = viewport->GetSizeXY();
							if (prevSize != ViewportSize)
							{
								OnViewportParameterChanged();
							}
						}
					}
					if (Canvas->GetRenderMode() == ELGUIRenderMode::RenderTarget && IsValid(Canvas->renderTarget))
					{
						auto prevSize = ViewportSize;
						ViewportSize.X = Canvas->renderTarget->SizeX;
						ViewportSize.Y = Canvas->renderTarget->SizeY;
						if (prevSize != ViewportSize)
						{
							OnViewportParameterChanged();
						}
					}
				}
				else
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
void ULGUICanvasScaler::OnEditorViewportIndexAndKeyChange()
{
	
}
void ULGUICanvasScaler::OnPreviewSetting_EditorPreviewViewportIndexChange()
{
	int32 editorViewIndex = ULGUIEditorSettings::GetLGUIPreview_EditorViewIndex();
	FLGUIRenderer::EditorPreview_ViewKey = ULGUIEditorManagerObject::Instance->GetViewportKeyFromIndex(editorViewIndex);
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

void ULGUICanvasScaler::DrawViewportArea()
{
	if (CheckCanvas())
	{
		auto RectExtends = FVector(0.1f, Canvas->GetUIItem()->GetWidth(), Canvas->GetUIItem()->GetHeight()) * 0.5f;
		auto GeometryBoundsExtends = FVector(0, 0, 0);
		bool bCanDrawRect = false;
		auto RectDrawColor = FColor(128, 128, 128, 128);//gray means normal object

		auto WorldTransform = Canvas->GetUIItem()->GetComponentTransform();
		FVector RelativeOffset(0, 0, 0);
		RelativeOffset.Y = (0.5f - Canvas->GetUIItem()->GetPivot().X) * Canvas->GetUIItem()->GetWidth();
		RelativeOffset.Z = (0.5f - Canvas->GetUIItem()->GetPivot().Y) * Canvas->GetUIItem()->GetHeight();
		auto WorldLocation = WorldTransform.TransformPosition(RelativeOffset);

		DrawDebugBox(Canvas->GetUIItem()->GetWorld(), WorldLocation, RectExtends * WorldTransform.GetScale3D(), WorldTransform.GetRotation(), RectDrawColor);
	}
}

void ULGUICanvasScaler::DrawVirtualCamera()
{
	if (CheckCanvas())
	{
		auto ViewLocation = Canvas->GetViewLocation();
		auto ViewRotationMatrix = FInverseRotationMatrix(Canvas->GetViewRotator()) * FMatrix(
			FPlane(0, 0, 1, 0),
			FPlane(1, 0, 0, 0),
			FPlane(0, 1, 0, 0),
			FPlane(0, 0, 0, 1));
		auto ProjectionMatrix = Canvas->GetProjectionMatrix();
		auto ViewProjectionMatrix = FTranslationMatrix(-ViewLocation) * ViewRotationMatrix * ProjectionMatrix;

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

		if (Canvas->UIItem.IsValid())
		{
			DrawDebugCamera(this->GetWorld(), Canvas->GetViewLocation(), Canvas->GetViewRotator(), FOVAngle, Canvas->GetUIItem()->GetComponentScale().X * 3.0f, FColor::Green);
		}
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
	if (CheckCanvas())
	{
		Canvas->SetProjectionParameters(ProjectionType, FOVAngle, NearClipPlane, FarClipPlane);
	}
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

void ULGUICanvasScaler::SetUIScaleMode(ELGUICanvasScaleMode value)
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
void ULGUICanvasScaler::SetScreenMatchMode(ELGUICanvasScreenMatchMode value)
{
	if (ScreenMatchMode != value)
	{
		ScreenMatchMode = value;
		OnViewportParameterChanged();
	}
}
void ULGUICanvasScaler::SetCustomScale(ULGUICanvasScalerCustomScale* value)
{
	if (CustomScale != value)
	{
		CustomScale = value;
		CustomScale->Init(this);//need to initialize when first set
		if (UIScaleMode == ELGUICanvasScaleMode::Custom)
		{
			OnViewportParameterChanged();
		}
	}
}

FVector2D ULGUICanvasScaler::ConvertPositionFromViewportToLGUICanvas(const FVector2D& position)const
{
	switch (UIScaleMode)
	{
	default:
	case ELGUICanvasScaleMode::ConstantPixelSize:
	{
		return FVector2D(position.X, ViewportSize.Y - position.Y);
	}
	break;
	case ELGUICanvasScaleMode::ScaleWithScreenSize:
	{
		if (!Canvas.IsValid())return position;
		return FVector2D(position.X, ViewportSize.Y - position.Y) / Canvas->canvasScale;
	}
	break;
	}
}
FVector2D ULGUICanvasScaler::ConvertPositionFromLGUICanvasToViewport(const FVector2D& position)const
{
	switch (UIScaleMode)
	{
	default:
	case ELGUICanvasScaleMode::ConstantPixelSize:
	{
		return FVector2D(position.X, ViewportSize.Y - position.Y);
	}
	break;
	case ELGUICanvasScaleMode::ScaleWithScreenSize:
	{
		if (!Canvas.IsValid())return position;
		return FVector2D(position.X * Canvas->canvasScale, ViewportSize.Y - position.Y * Canvas->canvasScale);
	}
	break;
	}
}
bool ULGUICanvasScaler::Project3DToScreen(const FVector& Position3D, FVector2D& OutPosition2D)const
{
	if (!Canvas.IsValid())return false;
	auto viewProjectionMatrix = Canvas->GetViewProjectionMatrix();
	auto result = viewProjectionMatrix.TransformFVector4(FVector4(Position3D, 1.0f));
	if (result.W > 0.0f)
	{
		// the result of this will be x and y coords in -1..1 projection space
		const float RHW = 1.0f / result.W;
		FPlane PosInScreenSpace = FPlane(result.X * RHW, result.Y * RHW, result.Z * RHW, result.W);

		// Move from projection space to normalized 0..1 UI space
		OutPosition2D.X = (PosInScreenSpace.X / 2.f) + 0.5f;
		OutPosition2D.Y = (PosInScreenSpace.Y / 2.f) + 0.5f;
		//Convert to LGUI's viewport size
		OutPosition2D *= Canvas->GetViewportSize();
		OutPosition2D /= Canvas->canvasScale;

		return true;
	}
	return false;
}

#if 1
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
bool ULGUICanvasScaler::ProjectWorldToScreen(APlayerController* Player, const FVector& Position3D, FVector2D& OutPosition2D)const
{
	ULocalPlayer* const LP = Player ? Player->GetLocalPlayer() : nullptr;
	if (LP && LP->ViewportClient)
	{
		auto TempFovAngle = Player->PlayerCameraManager->GetFOVAngle() * (float)PI / 360.0f;
		auto TempViewportSize = LP->ViewportClient->Viewport->GetSizeXY();
		FMatrix ProjectionMatrix;
		ULGUICanvas::BuildProjectionMatrix(TempViewportSize, ECameraProjectionMode::Perspective
			, TempFovAngle, 1000, 0.01f, ProjectionMatrix);

		auto ViewLocation = Player->PlayerCameraManager->GetCameraLocation();
		auto ViewRotationMatrix = FInverseRotationMatrix(Player->GetRootComponent()->GetComponentRotation()) * FMatrix(
			FPlane(0, 0, 1, 0),
			FPlane(1, 0, 0, 0),
			FPlane(0, 1, 0, 0),
			FPlane(0, 0, 0, 1));
		auto ViewProjectionMatrix = FTranslationMatrix(-ViewLocation) * ViewRotationMatrix * ProjectionMatrix;

		auto ScreenPos = ViewProjectionMatrix.TransformFVector4(FVector4(Position3D, 1.0f));
		if (ScreenPos.W > 0.0f)
		{
			// the result of this will be x and y coords in -1..1 projection space
			const float RHW = 1.0f / ScreenPos.W;
			FPlane PosInScreenSpace = FPlane(ScreenPos.X * RHW, ScreenPos.Y * RHW, ScreenPos.Z * RHW, ScreenPos.W);

			// Move from projection space to normalized 0..1 UI space
			const float NormalizedX = (PosInScreenSpace.X / 2.f) + 0.5f;
			const float NormalizedY = 1.f - (PosInScreenSpace.Y / 2.f) - 0.5f;

			OutPosition2D.X = (NormalizedX * (float)ViewportSize.X);
			OutPosition2D.Y = (NormalizedY * (float)ViewportSize.Y);

			OutPosition2D = ConvertPositionFromViewportToLGUICanvas(OutPosition2D);
			return true;
		}
	}
	return false;
}
#endif
