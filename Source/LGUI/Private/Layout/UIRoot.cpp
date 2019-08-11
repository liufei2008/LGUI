// Copyright 2019 LexLiu. All Rights Reserved.

#include "Layout/UIRoot.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Utils/LGUIUtils.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#if WITH_EDITOR
#include "Editor.h"
#include "EditorViewportClient.h"
#include "Core/Actor/LGUIManagerActor.h"
#endif


UUIRoot::UUIRoot()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUIRoot::BeginPlay()
{
	Super::BeginPlay();
	PrevViewportSize = FIntPoint(0, 0);//force apply
	NeedUpdate = true;
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

#if WITH_EDITOR
	EditorSwitchSimulationDelegateHandle = FEditorDelegates::OnSwitchBeginPIEAndSIE.AddUObject(this, &UUIRoot::EditorSwitchSimulation);
#endif
}

void UUIRoot::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RuntimeCheckAndApplyValue();
}

void UUIRoot::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
#if WITH_EDITOR
	FEditorDelegates::OnSwitchBeginPIEAndSIE.Remove(EditorSwitchSimulationDelegateHandle);
#endif
}

void UUIRoot::RuntimeCheckAndApplyValue()
{
	if (UISnapMode == LGUISnapMode::SnapToViewTargetCamera || (UISnapMode == LGUISnapMode::SnapToSceneCapture && SceneCapture != nullptr))
	{
		FIntPoint viewportSize = PrevViewportSize; float fov = PrevFov; bool isOrthographic = PrevIsOrthographic; float orthographicWidth = PrevOrthographicWidth;
		if (auto world = this->GetWorld())
		{
			if (auto pc = world->GetFirstPlayerController())
			{
				pc->GetViewportSize(viewportSize.X, viewportSize.Y);
				if (UISnapMode == LGUISnapMode::SnapToViewTargetCamera)
				{
					if (CameraManager == nullptr)
					{
						CameraManager = pc->PlayerCameraManager;
					}
					if (CameraManager != nullptr)
					{
						fov = CameraManager->GetFOVAngle();
						isOrthographic = CameraManager->IsOrthographic();
						orthographicWidth = CameraManager->GetOrthoWidth();
					}
				}
				else if (UISnapMode == LGUISnapMode::SnapToSceneCapture)
				{
					fov = SceneCapture->GetCaptureComponent2D()->FOVAngle;
					isOrthographic = SceneCapture->GetCaptureComponent2D()->ProjectionType == ECameraProjectionMode::Orthographic;
					orthographicWidth = SceneCapture->GetCaptureComponent2D()->OrthoWidth;
				}
				if (NeedUpdate || viewportSize != PrevViewportSize || fov != PrevFov || isOrthographic != PrevIsOrthographic || orthographicWidth != PrevOrthographicWidth)
				{
					OnViewportParameterChanged(viewportSize, fov, isOrthographic, orthographicWidth);
					PrevViewportSize = viewportSize;
					PrevFov = fov;
					PrevIsOrthographic = isOrthographic;
					PrevOrthographicWidth = orthographicWidth;
					NeedUpdate = false;
				}
			}
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
}
void UUIRoot::EditorApplyValue()
{
	if (UISnapMode == LGUISnapMode::SnapToSceneCapture && SceneCapture != nullptr)
	{
		FIntPoint viewportSize = FIntPoint(2, 2); float fov = 90; bool isOrthographic = false; float orthographicWidth = 100;
		if (FViewport* viewport = GEditor->GetActiveViewport())
		{
			viewportSize = viewport->GetSizeXY();
			fov = SceneCapture->GetCaptureComponent2D()->FOVAngle;
			isOrthographic = SceneCapture->GetCaptureComponent2D()->ProjectionType == ECameraProjectionMode::Orthographic;
			orthographicWidth = SceneCapture->GetCaptureComponent2D()->OrthoWidth;

			if (NeedUpdate || viewportSize != PrevViewportSize || fov != PrevFov || isOrthographic != PrevIsOrthographic || orthographicWidth != PrevOrthographicWidth)
			{
				OnViewportParameterChanged(viewportSize, fov, isOrthographic, orthographicWidth);
				PrevViewportSize = viewportSize;
				PrevFov = fov;
				PrevIsOrthographic = isOrthographic;
				PrevOrthographicWidth = orthographicWidth;
				NeedUpdate = false;
			}
		}
	}
	else if (UISnapMode == LGUISnapMode::SnapToViewTargetCamera)
	{
		FIntPoint viewportSize = FIntPoint(2, 2); float fov = 90; bool isOrthographic = false; float orthographicWidth = 100;
		if (FViewport* viewport = GEditor->GetActiveViewport())
		{
			viewportSize = viewport->GetSizeXY();

			if (NeedUpdate || viewportSize != PrevViewportSize)
			{
				OnViewportParameterChanged(viewportSize, fov, isOrthographic, orthographicWidth);
				PrevViewportSize = viewportSize;
				PrevFov = fov;
				PrevIsOrthographic = isOrthographic;
				PrevOrthographicWidth = orthographicWidth;
				NeedUpdate = false;
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

void UUIRoot::OnViewportParameterChanged(FIntPoint viewportSize, float fov, bool isOrthographic, float orthographicWidth)
{
	USceneComponent* attachRootComp = nullptr;
	bool canSetScale = false;
	
	if (UISnapMode == LGUISnapMode::SnapToViewTargetCamera)
	{
		if (CameraManager != nullptr)
		{
			canSetScale = true;
			attachRootComp = CameraManager->GetRootComponent();
		}
	}
	else if (UISnapMode == LGUISnapMode::SnapToSceneCapture)
	{
		if (SceneCapture != nullptr)
		{
			canSetScale = true;
			attachRootComp = SceneCapture->GetCaptureComponent2D();
#if WITH_EDITOR
			if (this->GetWorld()->IsGameWorld())
#endif
			{
				if (SnapRenderTargetToViewportSize)
				{
					if (viewportSize != PrevViewportSize)//viewport size change
					{
						if (auto renderTarget = SceneCapture->GetCaptureComponent2D()->TextureTarget)
						{
							renderTarget->SizeX = viewportSize.X * RenderTargetSizeMultiply;
							renderTarget->SizeY = viewportSize.Y * RenderTargetSizeMultiply;
							renderTarget->UpdateResource();
						}
					}
				}
			}
		}
	}

	if (CheckUIPanel())
	{
		float worldUnitWidth = 0, worldUnitHeight = 0;
		if (attachRootComp != nullptr)
		{
			//snap to camera
			RootUIPanel->AttachToComponent(attachRootComp, FAttachmentTransformRules::KeepRelativeTransform);
			RootUIPanel->SetRelativeLocationAndRotation(FVector(DistanceToCamera, 0, 0), FQuat::MakeFromEuler(FVector(-90, 0, 90)));
			//adjust size and scale
			if (isOrthographic)
			{
				worldUnitWidth = orthographicWidth;
				worldUnitHeight = worldUnitWidth * viewportSize.Y / viewportSize.X;
			}
			else
			{
				auto halfFov = fov * 0.5f;//ue4 us horizontal fov
				worldUnitWidth = DistanceToCamera * FMath::Tan(FMath::DegreesToRadians(halfFov)) * 2;
				worldUnitHeight = worldUnitWidth * viewportSize.Y / viewportSize.X;
			}
		}
		switch (UIScaleMode)
		{
		case LGUIScaleMode::ScaleWithScreenWidth:
		{
			RootUIPanel->SetWidth(PreferredWidth);
			RootUIPanel->SetHeight(PreferredWidth * viewportSize.Y / viewportSize.X);
			if (canSetScale)
			{
				auto scale = worldUnitWidth / PreferredWidth;
				RootUIPanel->SetRelativeScale3D(FVector(scale, scale, scale));
			}
		}
		break;
		case LGUIScaleMode::ScaleWithScreenHeight:
		{
			RootUIPanel->SetHeight(PreferredHeight);
			RootUIPanel->SetWidth(PreferredHeight * viewportSize.X / viewportSize.Y);
			if (canSetScale)
			{
				auto scale = worldUnitHeight / PreferredHeight;
				RootUIPanel->SetRelativeScale3D(FVector(scale, scale, scale));
			}
		}
		break;
		case LGUIScaleMode::ConstantPixelSize:
		{
			RootUIPanel->SetWidth(viewportSize.X);
			RootUIPanel->SetHeight(viewportSize.Y);
			if (canSetScale)
			{
				auto scale = worldUnitHeight / viewportSize.Y;
				RootUIPanel->SetRelativeScale3D(FVector(scale, scale, scale));
			}
		}
		break;
		default:
			break;
		}
		RootUIPanel->MarkRebuildAllDrawcall();
		RootUIPanel->MarkNeedUpdate();
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
	}

#if WITH_EDITOR
	if (GetWorld() && GetWorld()->IsGameWorld())
#endif
	{
		//add to scenecapture's showonlylist
		if (UISnapMode == LGUISnapMode::SnapToSceneCapture && SceneCapture != nullptr && SceneCapture->GetCaptureComponent2D()->PrimitiveRenderMode == ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList)
		{
			auto& showOnlyActors = SceneCapture->GetCaptureComponent2D()->ShowOnlyActors;
			if (!showOnlyActors.Contains(InPanel->GetOwner()))
			{
				showOnlyActors.Add(InPanel->GetOwner());
			}
		}
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
void UUIRoot::RemovePanel(UUIPanel* InPanel)
{
	int existIndex;
	if (PanelsBelongToThisUIRoot.Find(InPanel, existIndex))
	{
		PanelsBelongToThisUIRoot.RemoveAt(existIndex);
	}
	//remove from scenecapture's showonlylist
	if (UISnapMode == LGUISnapMode::SnapToSceneCapture && SceneCapture != nullptr && SceneCapture->GetCaptureComponent2D()->PrimitiveRenderMode == ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList)
	{
		auto& showOnlyActors = SceneCapture->GetCaptureComponent2D()->ShowOnlyActors;
		if (showOnlyActors.Find(InPanel->GetOwner(), existIndex))
		{
			showOnlyActors.RemoveAt(existIndex);
		}
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
		NeedUpdate = true;
	}
}
void UUIRoot::SetPreferredHeight(float InValue)
{
	if (PreferredHeight != InValue)
	{
		PreferredHeight = InValue;
		NeedUpdate = true;
	}
}