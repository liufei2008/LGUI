// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/UIRoot.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Utils/LGUIUtils.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SceneViewExtension.h"
#if WITH_EDITOR
#include "EditorViewportClient.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "DrawDebugHelpers.h"
#include "Event/Rayemitter/LGUI_ScreenSpaceUIMouseRayemitter.h"
#include "Core/Actor/LGUIManagerActor.h"
#endif
#include "Core/Render/LGUIRenderer.h"
#include "Event/Rayemitter/LGUI_SceneCapture2DMouseRayemitter.h"


UUIRoot::UUIRoot()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUIRoot::BeginPlay()
{
	Super::BeginPlay();
	PrevViewportSize = FIntPoint(0, 0);//force apply
	RuntimeCheckAndApplyValue();

	if (UseFirstPawnAsUIOwner)
	{
		if (auto world = this->GetWorld())
		{
			if (auto pc = world->GetFirstPlayerController())
			{
				auto pawn = pc->GetPawnOrSpectator();
				if (pawn != nullptr)
				{
					SetUIOwner(pawn);
				}
			}
		}
	}
	else
	{
		if(UIOwner)
			SetUIOwner(UIOwner);
	}
}

void UUIRoot::BuildProjectionMatrix(FIntPoint ViewportSize, ECameraProjectionMode::Type ProjectionType, float FOV, float InOrthoWidth, FMatrix& ProjectionMatrix)
{
	float XAxisMultiplier;
	float YAxisMultiplier;

	if (ViewportSize.X > ViewportSize.Y)
	{
		// if the viewport is wider than it is tall
		XAxisMultiplier = 1.0f;
		YAxisMultiplier = ViewportSize.X / (float)ViewportSize.Y;
	}
	else
	{
		// if the viewport is taller than it is wide
		XAxisMultiplier = ViewportSize.Y / (float)ViewportSize.X;
		YAxisMultiplier = 1.0f;
	}

	if (ProjectionType == ECameraProjectionMode::Orthographic)
	{
		check((int32)ERHIZBuffer::IsInverted);
		const float OrthoWidth = InOrthoWidth / 2.0f;
		const float OrthoHeight = InOrthoWidth / 2.0f * YAxisMultiplier;

		const float NearPlane = 0;
		const float FarPlane = WORLD_MAX / 8.0f;

		const float ZScale = 1.0f / (FarPlane - NearPlane);
		const float ZOffset = -NearPlane;

		ProjectionMatrix = FReversedZOrthoMatrix(
			OrthoWidth,
			OrthoHeight,
			ZScale,
			ZOffset
		);
	}
	else
	{
		if ((int32)ERHIZBuffer::IsInverted)
		{
			ProjectionMatrix = FReversedZPerspectiveMatrix(
				FOV,
				FOV,
				XAxisMultiplier,
				YAxisMultiplier,
				GNearClippingPlane,
				GNearClippingPlane
			);
		}
		else
		{
			ProjectionMatrix = FPerspectiveMatrix(
				FOV,
				FOV,
				XAxisMultiplier,
				YAxisMultiplier,
				GNearClippingPlane,
				GNearClippingPlane
			);
		}
	}
}
FMatrix UUIRoot::GetViewProjectionMatrix()
{
	FMatrix ViewProjectionMatrix = FMatrix::Identity;
	if (!CheckUIPanel())
	{
		UE_LOG(LGUI, Error, TEXT("[UUIRoot::GetViewProjectionMatrix]UIPanel not valid!"));
		return ViewProjectionMatrix;
	}
	FVector ViewLocation = RootUIPanel->GetComponentLocation() - RootUIPanel->GetUpVector() * DistanceToCamera;
	auto Transform = RootUIPanel->GetComponentToWorld();
	Transform.SetTranslation(FVector::ZeroVector);
	Transform.SetScale3D(FVector::OneVector);
	FMatrix ViewRotationMatrix = Transform.ToInverseMatrixWithScale();

	const float FOV = FOVAngle * (float)PI / 360.0f;

	FMatrix ProjectionMatrix;
	BuildProjectionMatrix(PrevViewportSize, ProjectionType, FOV, OrthoWidth, ProjectionMatrix);
	ViewProjectionMatrix = FTranslationMatrix(-ViewLocation) * ViewRotationMatrix * ProjectionMatrix;
	return ViewProjectionMatrix;
}
FMatrix UUIRoot::GetProjectionMatrix()
{
	FMatrix ProjectionMatrix = FMatrix::Identity;
	if (!CheckUIPanel())
	{
		UE_LOG(LGUI, Error, TEXT("[UUIRoot::GetViewProjectionMatrix]UIPanel not valid!"));
		return ProjectionMatrix;
	}
	const float FOV = FOVAngle * (float)PI / 360.0f;

	BuildProjectionMatrix(PrevViewportSize, ProjectionType, FOV, OrthoWidth, ProjectionMatrix);
	return ProjectionMatrix;
}
FVector UUIRoot::GetViewLocation()
{
	return RootUIPanel->GetComponentLocation() - RootUIPanel->GetUpVector() * DistanceToCamera;
}
FMatrix UUIRoot::GetViewRotationMatrix()
{
	auto Transform = RootUIPanel->GetComponentToWorld();
	Transform.SetTranslation(FVector::ZeroVector);
	Transform.SetScale3D(FVector::OneVector);
	return Transform.ToInverseMatrixWithScale();
}
FRotator UUIRoot::GetViewRotator()
{
	return RootUIPanel->GetComponentToWorld().Rotator().Add(90, 90, 0);
}

FIntPoint UUIRoot::GetViewportSize()
{
	FIntPoint viewportSize = PrevViewportSize;
	if (auto world = this->GetWorld())
	{
		if (auto pc = world->GetFirstPlayerController())
		{
			pc->GetViewportSize(viewportSize.X, viewportSize.Y);
		}
	}
	return viewportSize;
}

void UUIRoot::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RuntimeCheckAndApplyValue();
}
TSharedPtr<class FLGUIViewExtension, ESPMode::ThreadSafe> UUIRoot::GetViewExtension()
{
	if (!ViewExtension.IsValid())
	{
		if (RenderMode == ELGUIRenderMode::ScreenSpaceOverlay)
		{
			if (GEngine)
			{
				ViewExtension = FSceneViewExtensions::NewExtension<FLGUIViewExtension>(this);
			}
		}
	}
	return ViewExtension;
}

void UUIRoot::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UUIRoot::BeginDestroy()
{
	Super::BeginDestroy();
	if (ViewExtension.IsValid())
	{
		ViewExtension.Reset();
	}
}

void UUIRoot::RuntimeCheckAndApplyValue()
{
	if (RenderMode == ELGUIRenderMode::ScreenSpaceOverlay)
	{
		FIntPoint viewportSize = GetViewportSize();
		if (viewportSize != PrevViewportSize)
		{
			PrevViewportSize = viewportSize;
			OnViewportParameterChanged();
		}
	}
}

#if WITH_EDITOR
void UUIRoot::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	PrevViewportSize = FIntPoint(0, 0);
	EditorApplyValue();
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropName = Property->GetFName();
		if (PropName == TEXT("UIOnlyOwnerSee"))
		{
			SetUIOnlyOwnerSee(UIOnlyOwnerSee);
		}
		if (PropName == TEXT("UIOnlyOwnerSee"))
		{
			SetUIOwnerNoSee(UIOwnerNoSee);
		}
		if (PropName == TEXT("UIOwner"))
		{
			SetUIOwner(UIOwner);
		}
	}
	if (CheckUIPanel())
	{
		RootUIPanel->EditorForceUpdateImmediately();
	}
}
void UUIRoot::OnEditorTick(float DeltaTime)
{
	if (this->GetOwner() == nullptr)return;
	if (this->IsPendingKillOrUnreachable())return;
	EditorApplyValue();
	CheckUIPanel();
	if (RenderMode == ELGUIRenderMode::ScreenSpaceOverlay)
	{
		DrawVirtualCamera();
	}
}
void UUIRoot::EditorApplyValue()
{
	if (RenderMode == ELGUIRenderMode::ScreenSpaceOverlay)
	{
		FIntPoint viewportSize = FIntPoint(2, 2);
		if (FViewport* viewport = GEditor->GetActiveViewport())
		{
			viewportSize = viewport->GetSizeXY();
			if (PrevViewportSize != viewportSize)
			{
				PrevViewportSize = viewportSize;
				OnViewportParameterChanged();
			}
		}
	}
}
void UUIRoot::OnRegister()
{
	Super::OnRegister();
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
	{
		if (ULGUIEditorManagerObject::Instance != nullptr)
		{
			EditorTickDelegateHandle = ULGUIEditorManagerObject::Instance->EditorTick.AddUObject(this, &UUIRoot::OnEditorTick);
		}
	}
}
void UUIRoot::OnUnregister()
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
UTextureRenderTarget2D* UUIRoot::GetPreviewRenderTarget()
{
	if (RenderMode == ELGUIRenderMode::ScreenSpaceOverlay)
	{

	}
	return nullptr;
}
void DeprojectViewPointToWorld(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewPoint01, FVector& OutWorldStart, FVector& OutWorldEnd)
{
	FMatrix InvViewProjMatrix = InViewProjectionMatrix.InverseFast();

	const float ScreenSpaceX = (InViewPoint01.X - 0.5f) * 2.0f;
	const float ScreenSpaceY = (InViewPoint01.Y - 0.5f) * 2.0f;

	// The start of the raytrace is defined to be at mousex,mousey,1 in projection space (z=1 is near, z=0 is far - this gives us better precision)
	// To get the direction of the raytrace we need to use any z between the near and the far plane, so let's use (mousex, mousey, 0.5)
	const FVector4 RayStartProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 1.0f, 1.0f);
	const FVector4 RayEndProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 0.00000001f, 1.0f);

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
void UUIRoot::DrawVirtualCamera()
{
	if (CheckUIPanel())
	{
		if (!LGUIManager::IsSelected_Editor(this))return;
		auto ViewProjectionMatrix = GetViewProjectionMatrix();
		FVector leftBottom, rightBottom, leftTop, rightTop, rayEnd;
		auto lineColor = FColor::White;
		//draw view frustum
		DeprojectViewPointToWorld(ViewProjectionMatrix, FVector2D(0, 0), leftBottom, rayEnd);
		DrawDebugLine(this->GetWorld(), leftBottom, rayEnd, lineColor);
		DeprojectViewPointToWorld(ViewProjectionMatrix, FVector2D(1, 0), rightBottom, rayEnd);
		DrawDebugLine(this->GetWorld(), rightBottom, rayEnd, lineColor);
		DeprojectViewPointToWorld(ViewProjectionMatrix, FVector2D(0, 1), leftTop, rayEnd);
		DrawDebugLine(this->GetWorld(), leftTop, rayEnd, lineColor);
		DeprojectViewPointToWorld(ViewProjectionMatrix, FVector2D(1, 1), rightTop, rayEnd);
		DrawDebugLine(this->GetWorld(), rightTop, rayEnd, lineColor);

		DrawDebugLine(this->GetWorld(), leftBottom, rightBottom, lineColor);
		DrawDebugLine(this->GetWorld(), leftBottom, leftTop, lineColor);
		DrawDebugLine(this->GetWorld(), rightTop, rightBottom, lineColor);
		DrawDebugLine(this->GetWorld(), rightTop, leftTop, lineColor);
	}
}
#endif

bool UUIRoot::CheckUIPanel()
{
	if (IsValid(RootUIPanel))return true;
	if (!GetOwner())return false;
	RootUIPanel = Cast<UUIPanel>(GetOwner()->GetRootComponent());
	if (IsValid(RootUIPanel))
	{
		AddPanel(RootUIPanel);
		return true;
	}
	UE_LOG(LGUI, Error, TEXT("[UUIRoot::CheckUIPanel]This component should only on a actor that use UIPanel as root component!"));
	return false;
}

void UUIRoot::OnViewportParameterChanged()
{
	if (RenderMode == ELGUIRenderMode::ScreenSpaceOverlay)
	{
		if (CheckUIPanel())
		{
			auto halfFov = FOVAngle * 0.5f;//ue4 us horizontal fov
			RootUIPanel->SetRelativeRotation(FQuat::MakeFromEuler(FVector(-90, 0, 90)));
			RootUIPanel->SetRelativeScale3D(FVector::OneVector);
			//adjust size and scale
			if (ProjectionType == ECameraProjectionMode::Orthographic)
			{
				switch (UIScaleMode)
				{
				case LGUIScaleMode::ScaleWithScreenWidth:
				{
					RootUIPanel->SetWidth(PreferredWidth);
					RootUIPanel->SetHeight(PreferredWidth * PrevViewportSize.Y / PrevViewportSize.X);
				}
				break;
				case LGUIScaleMode::ScaleWithScreenHeight:
				{
					RootUIPanel->SetHeight(PreferredHeight);
					auto tempPreferredWidth = PreferredHeight * PrevViewportSize.X / PrevViewportSize.Y;
					RootUIPanel->SetWidth(tempPreferredWidth);
				}
				break;
				case LGUIScaleMode::ConstantPixelSize:
				{
					RootUIPanel->SetWidth(PrevViewportSize.X);
					RootUIPanel->SetHeight(PrevViewportSize.Y);
				}
				break;
				default:
					break;
				}
			}
			else
			{
				switch (UIScaleMode)
				{
				case LGUIScaleMode::ScaleWithScreenWidth:
				{
					RootUIPanel->SetWidth(PreferredWidth);
					RootUIPanel->SetHeight(PreferredWidth * PrevViewportSize.Y / PrevViewportSize.X);
					DistanceToCamera = PreferredWidth * 0.5f / FMath::Tan(FMath::DegreesToRadians(halfFov));
				}
				break;
				case LGUIScaleMode::ScaleWithScreenHeight:
				{
					RootUIPanel->SetHeight(PreferredHeight);
					auto tempPreferredWidth = PreferredHeight * PrevViewportSize.X / PrevViewportSize.Y;
					RootUIPanel->SetWidth(tempPreferredWidth);
					DistanceToCamera = tempPreferredWidth * 0.5f / FMath::Tan(FMath::DegreesToRadians(halfFov));
				}
				break;
				case LGUIScaleMode::ConstantPixelSize:
				{
					RootUIPanel->SetWidth(PrevViewportSize.X);
					RootUIPanel->SetHeight(PrevViewportSize.Y);
					DistanceToCamera = PrevViewportSize.X * 0.5f / FMath::Tan(FMath::DegreesToRadians(halfFov));
				}
				break;
				default:
					break;
				}
			}
			RootUIPanel->MarkRebuildAllDrawcall();
			RootUIPanel->MarkPanelUpdate();
		}
	}
}

void UUIRoot::ApplyToPanelDefaultMaterials()
{
	if (CheckUIPanel())
	{
		for (int i = 0; i < PanelsBelongToThisUIRoot.Num(); i++)
		{
			auto itemPanel = PanelsBelongToThisUIRoot[i];
			if (IsValid(itemPanel))
			{
				itemPanel->SetDefaultMaterials(OverrideMaterials);
			}
		}
	}
}

void UUIRoot::AddPanel(UUIPanel* InPanel)
{
	if (!PanelsBelongToThisUIRoot.Contains(InPanel))
	{
		PanelsBelongToThisUIRoot.Add(InPanel);
		SortUIPanelOnDepth();
	}

#if WITH_EDITOR
	if (GetWorld() && GetWorld()->IsGameWorld())
#endif
	{
		//replace material
		if (UseOverrideMaterials)
		{
			InPanel->SetDefaultMaterials(OverrideMaterials);
		}

		InPanel->SetUIOnlyOwnerSee(UIOnlyOwnerSee);
		InPanel->SetUIOwnerNoSee(UIOwnerNoSee);
		InPanel->GetOwner()->SetOwner(UIOwner);

		OnUIPanelAttached(InPanel);
	}
}
void UUIRoot::SortUIPanelOnDepth()
{
	PanelsBelongToThisUIRoot.Sort([](const UUIPanel& A, const UUIPanel& B)
	{
		return A.GetDepth() < B.GetDepth();
	});
}
void UUIRoot::RemovePanel(UUIPanel* InPanel)
{
	int existIndex;
	if (PanelsBelongToThisUIRoot.Find(InPanel, existIndex))
	{
		PanelsBelongToThisUIRoot.RemoveAt(existIndex);
		GetViewExtension();
	}

	InPanel->SetUIOnlyOwnerSee(false);
	InPanel->SetUIOwnerNoSee(false);
	InPanel->GetOwner()->SetOwner(nullptr);

	OnUIPanelDetached(InPanel);
}

void UUIRoot::SetUIOnlyOwnerSee(bool InValue)
{
	if (CheckUIPanel())
	{
		UIOnlyOwnerSee = InValue;
		for (int i = 0; i < PanelsBelongToThisUIRoot.Num(); i++)
		{
			auto& itemPanel = PanelsBelongToThisUIRoot[i];
			if (IsValid(itemPanel))
			{
				itemPanel->SetUIOnlyOwnerSee(UIOnlyOwnerSee);
			}
		}
	}
}
void UUIRoot::SetUIOwnerNoSee(bool InValue)
{
	if (CheckUIPanel())
	{
		UIOwnerNoSee = InValue;
		for (int i = 0; i < PanelsBelongToThisUIRoot.Num(); i++)
		{
			auto& itemPanel = PanelsBelongToThisUIRoot[i];
			if (IsValid(itemPanel))
			{
				itemPanel->SetUIOwnerNoSee(UIOwnerNoSee);
			}
		}
	}
}
void UUIRoot::SetUIOwner(AActor* InValue)
{
	if (CheckUIPanel())
	{
		UIOwner = InValue;
		for (int i = 0; i < PanelsBelongToThisUIRoot.Num(); i++)
		{
			auto& itemPanel = PanelsBelongToThisUIRoot[i];
			if (IsValid(itemPanel))
			{
				if (itemPanel->GetOwner())
				{
					if (itemPanel->GetOwner()->GetOwner() != UIOwner)
					{
						itemPanel->GetOwner()->SetOwner(UIOwner);
					}
				}
			}
		}
		//Update related parameters
		SetUIOnlyOwnerSee(UIOnlyOwnerSee);
		SetUIOwnerNoSee(UIOwnerNoSee);
	}
}
void UUIRoot::SetUseOverrideMaterials(bool InValue)
{
	if (UseOverrideMaterials != InValue)
	{
		UseOverrideMaterials = InValue;
		if (UseOverrideMaterials)
		{
			ApplyToPanelDefaultMaterials();
		}
	}
}
void UUIRoot::SetPreferredWidth(float InValue)
{
	if (PreferredWidth != InValue)
	{
		PreferredWidth = InValue;
		OnViewportParameterChanged();
	}
}
void UUIRoot::SetPreferredHeight(float InValue)
{
	if (PreferredHeight != InValue)
	{
		PreferredHeight = InValue;
		OnViewportParameterChanged();
	}
}
void UUIRoot::SetProjectionType(TEnumAsByte<ECameraProjectionMode::Type> InValue)
{
	if (ProjectionType != InValue)
	{
		ProjectionType = InValue;
		OnViewportParameterChanged();
	}
}
void UUIRoot::SetFieldOfView(float InValue)
{
	if (FOVAngle != InValue)
	{
		FOVAngle = InValue;
		OnViewportParameterChanged();
	}
}
void UUIRoot::SetOrthoWidth(float InValue)
{
	if (OrthoWidth != InValue)
	{
		OrthoWidth = InValue;
		OnViewportParameterChanged();
	}
}