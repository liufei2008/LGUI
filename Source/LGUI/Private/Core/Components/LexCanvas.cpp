// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexCanvas.h"
#include "LGUI.h"
#include "Core/LexUIGeometry.h"
#include "Utils/LexUIUtils.h"
#include "Core/LexUISettings.h"
#include "Core/LexUIManager.h"
#include "Core/LexUIRender/LexUIRenderer.h"
#include "Core/LexUIMesh/LexUIMeshComponent.h"
#include "Core/LexUIDrawCall.h"
#include "Core/Components/LexVisual.h"
#include "Core/Components/LexVisualPostProcess.h"
#include "Core/Components/LexVisualDirectMesh.h"
#include "Core/Components/LexWidget.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "SceneViewExtension.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Math/TransformCalculus2D.h"
#include "TextureResource.h"
#include "Camera/CameraComponent.h"
#include "Core/LexCanvasDrawCallProcessingRunnable.h"
#include "Core/LexUIClipData.h"
#include "Core/LexUIDataAsTexture.h"
#include "Core/Components/LexLayout.h"


#define LOCTEXT_NAMESPACE "LexCanvas"

ULexCanvas::ULexCanvas()
{
	DefaultMeshType = ULexUIMeshComponent::StaticClass();
	DefaultMaterial = LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/LexUI_ImageAndFont"));
	bStartWithTickEnabled = false;
}

void ULexCanvas::Awake()
{
	Super::Awake();
	this->SetCanExecuteTick(false);

	CheckRootCanvas();
	CurrentRenderMode = this->GetActualRenderMode();
	if (auto LexWidget = GetWidget())
	{
		bPrevIsVisible = LexWidget->GetWidgetActiveInHierarchy();
	}
	else
	{
		bPrevIsVisible = false;
	}
	MarkCanvasUpdate(true);

	bNeedToSortRenderPriority = true;

	if (this->IsRootCanvas())
	{
		if (this->GetRenderMode() == ELexRenderMode::ScreenSpaceOverlay
				|| this->GetRenderMode() == ELexRenderMode::RenderTarget
				)
		{
			CheckAndApplyViewportParameter();
		}
	}
	
	if (IsValid(CustomScale))
	{
		CustomScale->Init(this);
	}
}

TSharedPtr<class FLexUIRenderer, ESPMode::ThreadSafe> ULexCanvas::GetRenderTargetViewExtension()
{
	if (!RenderTargetViewExtension.IsValid())
	{
		RenderTargetViewExtension = FSceneViewExtensions::NewExtension<FLexUIRenderer>(GetWorld(), ELexUIRendererType::RenderTarget);
	}
	return RenderTargetViewExtension;
}

void ULexCanvas::UpdateRootCanvas()
{
	if (!GetWorld())
		return;
	CheckRootCanvas();
	if (this == RootCanvas)
	{
		if (RenderModeIsLexRendererOrUERenderer(CurrentRenderMode))
		{
			auto ActualRenderMode = GetActualRenderMode();
#if WITH_EDITOR
			if (!GetWorld()->IsGameWorld())//edit mode
			{
				if (ActualRenderMode == ELexRenderMode::ScreenSpaceOverlay)
					ActualRenderMode = ELexRenderMode::WorldSpace_LexUI;
			}
#endif
			switch (ActualRenderMode)
			{
			case ELexRenderMode::ScreenSpaceOverlay:
			{
				if (!bHasAddToLexScreenSpaceRenderer)
				{
					auto ViewExtension = ULexUIManagerWorldSubsystem::GetViewExtension(GetWorld(), true);

					if (ViewExtension.IsValid())//only root canvas can add screen space UI to LGUIRenderer
					{
						ViewExtension->SetScreenSpaceRootCanvas(this);
						bHasAddToLexScreenSpaceRenderer = true;
					}
				}
			}
			break;
			case ELexRenderMode::RenderTarget:
			{
				if (!bHasAddToLexScreenSpaceRenderer)
				{
					GetRenderTargetViewExtension()->SetScreenSpaceRootCanvas(this);
					bHasAddToLexScreenSpaceRenderer = true;
				}
			}
			break;
			case ELexRenderMode::WorldSpace_LexUI:
			{
				if (!bHasSetInitialStateForLexWorldSpaceRenderer)
				{
					auto ViewExtension = ULexUIManagerWorldSubsystem::GetViewExtension(GetWorld(), true);

					if (ViewExtension.IsValid())//only root canvas can add screen space UI to LGUIRenderer
					{
						//put initial code here
						bHasSetInitialStateForLexWorldSpaceRenderer = true;
					}
				}
			}
			break;
			}
		}
		
		UpdateCanvasDrawCall();
	}
}

void ULexCanvas::UpdateRenderTarget(bool CallEvent)
{
	auto LexWidget = GetWidget();
	FIntPoint DesiredRenderTargetSize(LexWidget->GetWidth() * RenderTargetResolutionScale, LexWidget->GetHeight() * RenderTargetResolutionScale);
	static const int32 MaxAllowedDrawSize = GetMax2DTextureDimension();
	if (DesiredRenderTargetSize.X <= 0 || DesiredRenderTargetSize.Y <= 0)
	{
		return;
	}
	DesiredRenderTargetSize.X = FMath::Min(DesiredRenderTargetSize.X, MaxAllowedDrawSize);
	DesiredRenderTargetSize.Y = FMath::Min(DesiredRenderTargetSize.Y, MaxAllowedDrawSize);

	if (RenderTarget == nullptr)
	{
		RenderTarget = NewObject<UTextureRenderTarget2D>(this, NAME_None, EObjectFlags::RF_Transient);
		RenderTarget->AddressX = TextureAddress::TA_Clamp;
		RenderTarget->AddressY = TextureAddress::TA_Clamp;
		RenderTarget->ClearColor = FLinearColor::Transparent;
		RenderTarget->InitCustomFormat(DesiredRenderTargetSize.X, DesiredRenderTargetSize.Y, EPixelFormat::PF_B8G8R8A8, false);
		if (CallEvent)
		{
			OnRenderTargetChanged.Broadcast(RenderTarget);
		}
	}
	else
	{
		switch (RenderTargetSizeMode)
		{
		case ELexCanvasRenderTargetSizeMode::None:
		case ELexCanvasRenderTargetSizeMode::CanvasFitToRenderTarget:
			if (RenderTarget != nullptr)
			{
				DesiredRenderTargetSize.X = RenderTarget->SizeX;
				DesiredRenderTargetSize.Y = RenderTarget->SizeY;
			}
			break;
		case ELexCanvasRenderTargetSizeMode::RenderTargetFitToCanvas:
			break;
		}
		if (RenderTarget->SizeX != DesiredRenderTargetSize.X || RenderTarget->SizeY != DesiredRenderTargetSize.Y)
		{
			RenderTarget->ClearColor = FLinearColor::Transparent;
			RenderTarget->InitCustomFormat(DesiredRenderTargetSize.X, DesiredRenderTargetSize.Y, EPixelFormat::PF_B8G8R8A8, false);
			RenderTarget->UpdateResourceImmediate();
#if WITH_EDITOR
			RenderTarget->Modify();
#endif
			if (CallEvent)
			{
				OnRenderTargetChanged.Broadcast(RenderTarget);
			}
		}
	}
}

void ULexCanvas::CheckRenderTargetUpdate()
{
	bool bIsRenderTargetRenderer = false;
	if (RenderModeIsLexRendererOrUERenderer(CurrentRenderMode))
	{
		auto ActualRenderMode = GetActualRenderMode();
#if WITH_EDITOR
		if (!GetWorld()->IsGameWorld())//edit mode
		{
			if (ActualRenderMode == ELexRenderMode::ScreenSpaceOverlay)
				ActualRenderMode = ELexRenderMode::WorldSpace_LexUI;
		}
#endif
		if (ActualRenderMode == ELexRenderMode::RenderTarget)
		{
			bIsRenderTargetRenderer = true;
		}
	}
	if (bIsRenderTargetRenderer)
	{
		bool bCanUpdateRenderTarget = false;
		switch (RenderTargetUpdateMode)
		{
		default:
		case ELexCanvasRenderTargetUpdateMode::Automatic:
			{
				if (bAnythingChangedForRenderTarget || bPrevAnythingChangedForRenderTarget || bRequestUpdateForRenderTarget)
				{
					bPrevAnythingChangedForRenderTarget = bAnythingChangedForRenderTarget;
					bAnythingChangedForRenderTarget = false;
					bRequestUpdateForRenderTarget = false;
					bCanUpdateRenderTarget = true;
				}
			}
			break;
		case ELexCanvasRenderTargetUpdateMode::Always:
			bCanUpdateRenderTarget = true;
			break;
		case ELexCanvasRenderTargetUpdateMode::WhenRequest:
			{
				if (bRequestUpdateForRenderTarget)
				{
					bRequestUpdateForRenderTarget = false;
					bCanUpdateRenderTarget = true;
				}
			}
			break;
		}
		if (bCanUpdateRenderTarget)
		{
			UpdateRenderTarget(true);
			if (IsValid(RenderTarget))
			{
#if WITH_EDITOR
				if (!this->GetWorld()->IsGameWorld())
				{
					if (!RenderTarget->GameThread_GetRenderTargetResource())
					{
						RenderTarget->InitCustomFormat(RenderTarget->SizeX, RenderTarget->SizeY, EPixelFormat::PF_B8G8R8A8, false);
					}
				}
#endif
				if (RenderTargetViewExtension.IsValid())
				{
					RenderTargetViewExtension->UpdateRenderTargetRenderer(RenderTarget, RenderTargetClearColor);
				}
			}
		}
	}
}

void ULexCanvas::OnRegister()
{
	Super::OnRegister();
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(GetWorld()))
	{
		LexUIManager->AddCanvas(this);
	}
	if (DrawCallProcessingRunnable == nullptr)
	{
		DrawCallProcessingRunnable = MakeUnique<FLexCanvasDrawCallProcessingRunnable>();
		DrawCallProcessingRunnable->Start();
	}
	if (TransformVerticesAsyncFunctionRunnable == nullptr)
	{
		TransformVerticesAsyncFunctionRunnable = MakeUnique<FLexCanvasAsyncFunctionRunnable>();
		TransformVerticesAsyncFunctionRunnable->Start();
	}
	if (auto LexWidget = GetWidget())
	{
		LexWidget->RegisterRenderCanvas(this);
		LexWidget->GetAttachmentChangedEvent().AddUObject(this, &ULexCanvas::OnUIHierarchyAttachmentChanged);
		LexWidget->GetWidgetActiveChangedEvent().AddUObject(this, &ULexCanvas::OnWidgetActiveChanged);

		OnUIHierarchyAttachmentChanged();
	}

	if (!IsValid(ClipDataAsTexture))
	{
		ClipDataAsTexture = NewObject<ULexUIDataAsTexture>(this, ULexUIDataAsTexture::StaticClass(), NAME_None, RF_Transient);
		ClipDataAsTexture->Init(FLexUIClipData::BlockSizeInBytes, ELexUIDataAsTexturePixelFormat::R32G32B32A32, 128);
		ClipDataAsTexture->OnDataTextureChange.AddUObject(this, &ULexCanvas::OnClipDataTextureChanged);
		ClipDataAsTexture->RegisterBuffer();//register a zero position as a placeholder for not clipping type.
	}

	RegisterCanvasScaler();
}
void ULexCanvas::OnUnregister()
{
	Super::OnUnregister();
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(GetWorld()))
	{
		LexUIManager->RemoveCanvas(this);
	}
	ClearDrawCall();
	if (IsValid(UIMesh))
	{
		UIMesh->DestroyComponent();
		UIMesh = nullptr;
	}
	if (DrawCallProcessingRunnable.IsValid())
	{
		DrawCallProcessingRunnable->Stop();
		DrawCallProcessingRunnable.Reset();
	}
	if (TransformVerticesAsyncFunctionRunnable.IsValid())
	{
		TransformVerticesAsyncFunctionRunnable->Stop();
		TransformVerticesAsyncFunctionRunnable.Reset();
	}

	ClipDataList.Empty();
	
	{
		//these three functions is from OnUIHierarchyChanged()
		RemoveFromViewExtension(true);
		CheckRenderMode(true);
	}

	//tell Widget
	if (auto LexWidget = GetWidget())
	{
		LexWidget->UnregisterRenderCanvas();
		LexWidget->GetAttachmentChangedEvent().RemoveAll(this);
		LexWidget->GetWidgetActiveChangedEvent().RemoveAll(this);
	}

	UnregisterCanvasScaler();
}

void ULexCanvas::PostInitProperties()
{
	Super::PostInitProperties();
}

void ULexCanvas::ClearDrawCall()
{
	if (IsValid(UIMesh))
	{
		UIMesh->ClearRenderData();
		bUIMeshNeedToSetInitialParameters = true;
	}
	PooledDefaultMaterialList.Empty();
	MapSrcMatToDynamicMat.Empty();
	CurrentDrawCallData.DrawCallArray.Empty();
	bNeedToSetClipDataTextureMaterialParameter = true;
}

void ULexCanvas::RemoveFromViewExtension(bool PropogateToChildrenCanvas)
{
	if (bHasAddToLexScreenSpaceRenderer)
	{
		bHasAddToLexScreenSpaceRenderer = false;
		if (RenderTargetViewExtension.IsValid())//could be RenderTarget mode
		{
			RenderTargetViewExtension->ClearScreenSpaceRootCanvas();
		}
		else//if not RenderTarget mode, then should be ScreenSpaceOverlay
		{
			auto ViewExtension = ULexUIManagerWorldSubsystem::GetViewExtension(GetWorld(), false);
			if (ViewExtension.IsValid())
			{
				ViewExtension->ClearScreenSpaceRootCanvas();
			}
		}
	}
	if (bHasSetInitialStateForLexWorldSpaceRenderer)
	{
		bHasSetInitialStateForLexWorldSpaceRenderer = false;
	}

	if (PropogateToChildrenCanvas)
	{
		for (const auto& ChildCanvas : ChildrenCanvasArray)
		{
			if (!ChildCanvas.IsValid())continue;
			if (ChildCanvas->bForceRenderToTarget)continue;
			ChildCanvas->RemoveFromViewExtension(PropogateToChildrenCanvas);
		}
	}
}

bool ULexCanvas::CheckRootCanvas(bool forceRecheck)const
{
	if (forceRecheck)
	{
		if (RootCanvas.IsValid())
		{
			RootCanvas = nullptr;
		}
	}
	if (RootCanvas.IsValid())return true;
	if (this->GetWorld() == nullptr)return false;
	auto FindRootCanvas = [](ULexWidget* Widget)
	{
		ULexCanvas* ResultCanvas = nullptr;
		auto ParentWidget = Widget;
		while (ParentWidget != nullptr)
		{
			if (auto FoundCanvas = ParentWidget->GetComponent<ULexCanvas>())
			{
				ResultCanvas = FoundCanvas;
				if (FoundCanvas->bForceRenderToTarget)
				{
					return ResultCanvas;
				}
			}
			ParentWidget = ParentWidget->GetParent();
		}
		return ResultCanvas;
	};
	auto NewRootCanvas = FindRootCanvas(this->GetWidget());
	if (NewRootCanvas != RootCanvas)
	{
		RootCanvas = NewRootCanvas;
		bNeedToSetClipDataTextureMaterialParameter = true;
	}
	if (RootCanvas.IsValid())
	{
		return true;
	}
	return false;
}

void ULexCanvas::SetParentCanvas(ULexCanvas* InParentCanvas)
{
	if (ParentCanvas != InParentCanvas)
	{
		this->ClearDrawCall();
		this->MarkCanvasUpdate(true);
		if (ParentCanvas.IsValid())
		{
			this->DrawCallAsChildCanvas = nullptr;

			ParentCanvas->ChildrenCanvasArray.Remove(this);
			ParentCanvas->bNeedToGenerateWidgetList = true;
			ParentCanvas->MarkCanvasUpdate(true);
		}
		ParentCanvas = InParentCanvas;
		if (ParentCanvas.IsValid())
		{
			ParentCanvas->bNeedToGenerateWidgetList = true;
			ParentCanvas->ChildrenCanvasArray.AddUnique(this);
			ParentCanvas->MarkCanvasUpdate(true);
		}
	}
}

void ULexCanvas::CollectChildrenCanvas(ULexCanvas* Target, TArray<ULexCanvas*>& OutAllChildrenCanvas, bool IncludeTarget)
{
	if (IncludeTarget)
	{
		OutAllChildrenCanvas.Add(Target);
	}
	for (auto& Child : Target->GetChildrenCanvasArray())
	{
		CollectChildrenCanvas(Child.Get(), OutAllChildrenCanvas, true);
	}
}

void ULexCanvas::CheckRenderMode(bool PropagateToChildrenCanvas)
{
	const auto OldRenderMode = CurrentRenderMode;
	if (CheckRootCanvas(true))
	{
		CurrentRenderMode = RootCanvas->GetRenderMode();
	}
	else
	{
		CurrentRenderMode = ELexRenderMode::None;
	}
	//if render space changed, we need to change recreate all render data
	if (CurrentRenderMode != OldRenderMode)
	{
		if (auto LexWidget = GetWidget())
		{
			LexWidget->MarkRenderModeChangeRecursive(this, OldRenderMode, CurrentRenderMode);
		}
		//clear drawcall, delete mesh, because UE/LGUI render's mesh data not compatible
		this->ClearDrawCall();
		OnRenderModeChanged.Broadcast(this, OldRenderMode, CurrentRenderMode);
	}

	if (PropagateToChildrenCanvas)
	{
		for (const auto& ChildCanvas : ChildrenCanvasArray)
		{
			if (!ChildCanvas.IsValid())continue;
			if (ChildCanvas->bForceRenderToTarget)continue;
			ChildCanvas->CheckRenderMode(PropagateToChildrenCanvas);
		}
	}
}
void ULexCanvas::OnUIHierarchyAttachmentChanged()
{
 	this->bCanTickUpdate = true;
	RemoveFromViewExtension(true);
	CheckRenderMode(true);

	auto NewParentCanvas = GetWidget()->GetComponentInParent<ULexCanvas>(false);
	SetParentCanvas(NewParentCanvas);
}

void ULexCanvas::OnWidgetActiveChanged(bool WidgetActive)
{
	if (GetWidget()->GetWidgetActiveInHierarchy())
	{
		if (ParentCanvas.IsValid())
		{
			ParentCanvas->bNeedToGenerateWidgetList = true;
			ParentCanvas->MarkCanvasUpdate(true);

		}
	}
	else
	{
		if (ParentCanvas.IsValid())
		{
			ParentCanvas->bNeedToGenerateWidgetList = true;
			ParentCanvas->MarkCanvasUpdate(true);
		}
	}
}

bool ULexCanvas::IsRenderToScreenSpace()const
{
	if (CheckRootCanvas())
	{
		return RootCanvas->RenderMode == ELexRenderMode::ScreenSpaceOverlay;
	}
	return false;
}
bool ULexCanvas::IsRenderToRenderTarget()const
{
	if (CheckRootCanvas())
	{
		return RootCanvas->RenderMode == ELexRenderMode::RenderTarget;
	}
	return false;
}
bool ULexCanvas::IsRenderToWorldSpace()const
{
	if (CheckRootCanvas())
	{
		return RootCanvas->RenderMode == ELexRenderMode::WorldSpace
			|| RootCanvas->RenderMode == ELexRenderMode::WorldSpace_LexUI
			;
	}
	return false;
}

bool ULexCanvas::IsRenderByLexUIRendererOrUERenderer()const
{
	if (CheckRootCanvas())
	{
		return RootCanvas->RenderMode == ELexRenderMode::ScreenSpaceOverlay
			|| RootCanvas->RenderMode == ELexRenderMode::RenderTarget
			|| RootCanvas->RenderMode == ELexRenderMode::WorldSpace_LexUI
			;
	}
	return false;
}

void ULexCanvas::MarkCanvasUpdate(bool bRebuildDrawCall)
{
	this->bCanTickUpdate = true;
	if (bRebuildDrawCall)
	{
		this->bShouldRebuildDrawCall = true;
	}
}

#if WITH_EDITOR
bool ULexCanvas::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty)
	{
		auto MemberName = InProperty->GetFName();
		bool bIsRootCanvas = this->IsRootCanvas()
		|| this->GetWorld() == nullptr;//world is null maybe it is blueprint editor
		if (MemberName == GET_MEMBER_NAME_CHECKED(ULexCanvas, ProjectionType))
		{
			return bIsRootCanvas;
		}
		if (MemberName == GET_MEMBER_NAME_CHECKED(ULexCanvas, FieldOfView))
		{
			return bIsRootCanvas;
		}
		if (MemberName == GET_MEMBER_NAME_CHECKED(ULexCanvas, NearClipPlane))
		{
			return bIsRootCanvas;
		}
		if (MemberName == GET_MEMBER_NAME_CHECKED(ULexCanvas, FarClipPlane))
		{
			return bIsRootCanvas;
		}
		if (MemberName == GET_MEMBER_NAME_CHECKED(ULexCanvas, ScaleMode))
		{
			return bIsRootCanvas;
		}
		if (MemberName == GET_MEMBER_NAME_CHECKED(ULexCanvas, ReferenceResolution))
		{
			return bIsRootCanvas;
		}
		if (MemberName == GET_MEMBER_NAME_CHECKED(ULexCanvas, MatchFromWidthToHeight))
		{
			return bIsRootCanvas;
		}
		if (MemberName == GET_MEMBER_NAME_CHECKED(ULexCanvas, ScreenMatchMode))
		{
			return bIsRootCanvas;
		}
		if (MemberName == GET_MEMBER_NAME_CHECKED(ULexCanvas, bFixedSizeInEditMode))
		{
			return bIsRootCanvas;
		}
		if (MemberName == GET_MEMBER_NAME_CHECKED(ULexCanvas, SizeInEditMode))
		{
			return bIsRootCanvas;
		}
		if (MemberName == GET_MEMBER_NAME_CHECKED(ULexCanvas, RenderMode))
		{
			if (bIsRootCanvas)
			{
				if (bForceRenderToTarget)
				{
					return false;
				}
				return true;
			}
		}
	}

	return Super::CanEditChange(InProperty);
}
void ULexCanvas::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (auto LexWidget = GetWidget())
	{
		LexWidget->MarkAllDirtyRecursive();
	}
	if (CheckRootCanvas())
	{
		RootCanvas->MarkCanvasUpdate(true);
		RootCanvas->bRequestUpdateForRenderTarget = true;
	}

	auto PropertyName = PropertyChangedEvent.GetMemberPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexCanvas, bForceRenderToTarget))
	{
		if (bForceRenderToTarget)
		{
			RenderMode = ELexRenderMode::RenderTarget;
			OnRenderTargetChanged.Broadcast(RenderTarget);
		}
		else
		{
			OnRenderTargetChanged.Broadcast(nullptr);
		}
	}

	OnViewportParameterChanged();
}
void ULexCanvas::PostLoad()
{
	Super::PostLoad();
}
void ULexCanvas::PostEditUndo()
{
	Super::PostEditUndo();

	ULexUIManagerWorldSubsystem::RefreshAllUI(this->GetWorld());
}
void ULexCanvas::EnsureDataForRebuild()
{
	struct LOCAL
	{
		static void RecheckRootCanvasRecursive(ULexCanvas* Target)
		{
			Target->MarkCanvasUpdate(true);
			Target->CheckRenderMode(false);
			for (int i = Target->ChildrenCanvasArray.Num() - 1; i >= 0; i--)
			{
				auto ChildCanvas = Target->ChildrenCanvasArray[i];
				if (ChildCanvas.IsValid())
				{
					RecheckRootCanvasRecursive(ChildCanvas.Get());
				}
				else
				{
					Target->ChildrenCanvasArray.RemoveAt(i);
				}
			}
		}
	};
	ULexUIManagerObject::AddOneShotTickFunction([WeakThis = MakeWeakObjectPtr(this)]() {
		if (WeakThis.IsValid())
		{
			LOCAL::RecheckRootCanvasRecursive(WeakThis.Get());
		}
		}, 0);
}
#endif

ULexCanvas* ULexCanvas::GetRootCanvas() const
{ 
	CheckRootCanvas(); 
	return RootCanvas.Get(); 
}
bool ULexCanvas::IsRootCanvas()const
{
	return GetRootCanvas() == this;
}

USceneComponent* ULexCanvas::GetAttachedRootSceneComponent() const
{
	return AttachedRootSceneComponent.Get();
}

void ULexCanvas::AttachToSceneComponent(USceneComponent* InSceneComp) const
{
	AttachedRootSceneComponent = InSceneComp;
}

void ULexCanvas::MarkVisualWillChange(ULexVisual* InOldVisual)
{
	MarkCanvasUpdate(false);
}

void ULexCanvas::RegisterVisual(ULexVisual* InVisual)
{
	auto WidgetPropertyDataStartPosition = InVisual->GetWidgetPropertyDataStartPosition();
	if(WidgetPropertyDataStartPosition == INDEX_NONE)
	{
		check(!VisualList.Contains(InVisual));
		VisualList.Add(InVisual);
		CheckWidgetPropertyData();
		InVisual->SetWidgetPropertyDataStartPosition(WidgetPropertyDataAsTexture->RegisterBuffer());
	}
}

void ULexCanvas::UnregisterVisual(ULexVisual* InVisual)
{
	auto WidgetPropertyDataStartPosition = InVisual->GetWidgetPropertyDataStartPosition();
	if (WidgetPropertyDataStartPosition > INDEX_NONE)
	{
		auto Count = VisualList.Remove(InVisual);
		check(Count == 1);
		if (IsValid(WidgetPropertyDataAsTexture))
		{
			WidgetPropertyDataAsTexture->UnregisterBuffer(WidgetPropertyDataStartPosition);
		}
		InVisual->SetWidgetPropertyDataStartPosition(INDEX_NONE);
	}
}

void ULexCanvas::AddLexWidget(ULexWidget* InWidget)
{
	bNeedToGenerateWidgetList = true;
	MarkCanvasUpdate(true);
}
void ULexCanvas::RemoveLexWidget(ULexWidget* InWidget)
{
	bNeedToGenerateWidgetList = true;
	MarkCanvasUpdate(true);
}

bool ULexCanvas::Is2DUITransform(const FTransform& Transform)
{
#if WITH_EDITOR
	float threshold = ULexUISettings::GetAutoBatchThreshold();
#else
	static float threshold = ULexUISettings::GetAutoBatchThreshold();
#endif
	if (FMath::Abs(Transform.GetLocation().X) > threshold)//location X moved
	{
		return false;
	}
	const auto rotation = Transform.GetRotation().Rotator();
	if (FMath::Abs(rotation.Yaw) > threshold || FMath::Abs(rotation.Pitch) > threshold)//rotate
	{
		return false;
	}
	return true;
}

void ULexCanvas::SetOverrideViewLocation(bool Override, FVector Value)
{
	bOverrideViewLocation = Override;
	OverrideViewLocation = Value;
}
void ULexCanvas::SetOverrideViewRotation(bool Override, FRotator Value)
{
	bOverrideViewRotation = Override;
	OverrideViewRotation = Value;
}
void ULexCanvas::SetOverrideFovAngle(bool Override, float Value)
{
	bOverrideFovAngle = Override;
	OverrideFovAngle = Value;
}
void ULexCanvas::SetOverrideProjectionMatrix(bool Override, FMatrix Value)
{
	bOverrideProjectionMatrix = Override;
	OverrideProjectionMatrix = Value;
}

void ULexCanvas::MarkTransformOrDimensionChanged()
{
	bIsViewProjectionMatrixDirty = true;
}

void ULexCanvas::SetDefaultMeshType(TSubclassOf<ULexUIMeshComponent> InValue)
{
	if (DefaultMeshType != InValue)
	{
		DefaultMeshType = InValue;
		//clear mesh
		if (IsValid(UIMesh))
		{
			UIMesh->DestroyComponent();
			UIMesh = nullptr;
		}
		MarkCanvasUpdate(true);
	}
}

void ULexCanvas::MarkFinishUpdateCanvasDrawCall()
{
	//All children canvas clip data is stored in root canvas, so update from root canvas
	if (this == RootCanvas)
	{
		for (const auto& ClipData : ClipDataList)
		{
			ClipData->UpdateData();
		}
	}
	//sort render priority
	if (bNeedToSortRenderPriority)
	{
		bNeedToSortRenderPriority = false;
		if (this->IsRootCanvas() || this->GetOverrideSorting())
		{
			this->SortDrawCall();
		}
	}

	//update children canvas
	for (auto& item : ChildrenCanvasArray)
	{
		if (!item.IsValid())continue;
		if (item->bForceRenderToTarget)continue;
		item->MarkFinishUpdateCanvasDrawCall();
	}
}

DECLARE_CYCLE_STAT(TEXT("Canvas PrepareDrawCallBatchingData"), STAT_PrepareDrawCallBatching, STATGROUP_LGUI);
void ULexCanvas::PrepareDrawCallBatchingData(TArray<FLexUIRenderData>& OutRenderDataArray)
{
	SCOPE_CYCLE_COUNTER(STAT_PrepareDrawCallBatching);
	OutRenderDataArray.Reset();
	for (int i = 0; i < WidgetList.Num(); i++)
	{
		auto& Widget = WidgetList[i];
		if (Widget->IsCanvasWidget() && Widget->GetRenderCanvas() != this)//is child canvas
		{
			auto ChildCanvas = Widget->GetRenderCanvas();
			if (ChildCanvas == nullptr)continue;//normally this won't be nullptr, but when redo in editor this breaks
			if (ChildCanvas->bForceRenderToTarget)continue;//skip this type
			if (ChildCanvas->GetOverrideSorting())continue;//override sorting means render by itself, then no need to use it as child-canvas
			auto RenderData = FLexUIRenderData(ELexUIDrawCallType::ChildCanvas);
			RenderData.ChildCanvas = ChildCanvas;
			OutRenderDataArray.Add(MoveTemp(RenderData));
		}
		else
		{
			auto Visual = Widget->GetVisual();
			if (!Visual)continue;
			if (!Widget->GetWidgetActiveInHierarchy())//if not visible, need to remove the draw-call from draw-call list
			{
				continue;
			}
			switch (Visual->GetVisualType())
			{
			default:
			case ELexVisualType::BatchMesh:
				{
					auto LexVisualBatchMesh = static_cast<ULexVisualBatchMesh*>(Visual);
					auto ItemGeo = LexVisualBatchMesh->GetGeometry();
					if (ItemGeo == nullptr)continue;
					while (ItemGeo->bIsCalculating)
					{
						FPlatformProcess::Sleep(0.001f);//we must wait until geometry calculation finish, or CopyDataForPrepare will get wrong data
					}
					if (ItemGeo->Vertices.Num() == 0)continue;
					if (ItemGeo->Vertices.Num() > LEXUI_MAX_VERTEX_COUNT)continue;
					auto RenderData = FLexUIRenderData(ELexUIDrawCallType::BatchMesh);
					RenderData.BatchMeshGeometry.CopyDataForPrepare(*ItemGeo);
					RenderData.BatchMeshVisualObject = LexVisualBatchMesh;
					OutRenderDataArray.Add(MoveTemp(RenderData));
				}
				break;
			case ELexVisualType::PostProcess:
				{
					auto LexVisualPostProcess = static_cast<ULexVisualPostProcess*>(Visual);
					if (!LexVisualPostProcess->HaveValidData())continue;
					auto RenderData = FLexUIRenderData(ELexUIDrawCallType::PostProcess);
					RenderData.PostProcessVisualObject = LexVisualPostProcess;
					OutRenderDataArray.Add(MoveTemp(RenderData));
				}
				break;
			case ELexVisualType::DirectMesh:
				{
					auto LexVisualDirectMesh = static_cast<ULexVisualDirectMesh*>(Visual);
					if (!LexVisualDirectMesh->HaveValidData())continue;
					auto RenderData = FLexUIRenderData(ELexUIDrawCallType::DirectMesh);
					RenderData.DirectMeshVisualObject = LexVisualDirectMesh;
					OutRenderDataArray.Add(MoveTemp(RenderData));
				}
				break;
			}
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("Canvas BatchDrawCallAsync"), STAT_BatchDrawCall, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("Canvas BatchDrawCall/OverlapTest"), STAT_OverlapTest, STATGROUP_LGUI);

void ULexCanvas::BatchDrawCallAsync(const FVector2D& InCanvasLeftBottom, const FVector2D& InCanvasRightTop,
	const TArray<FLexUIRenderData>& InRenderDataArray, TArray<FLexUIDrawCall>& InOutUIDrawCallList)
{
	SCOPE_CYCLE_COUNTER(STAT_BatchDrawCall);

	InOutUIDrawCallList.Reset();
	
	auto CanvasRect = LexUIQuadTree::Rectangle(InCanvasLeftBottom, InCanvasRightTop);

	auto IntersectBounds = [](FVector2D aMin, FVector2D aMax, FVector2D bMin, FVector2D bMax) {
		return !(bMin.X >= aMax.X
			|| bMax.X <= aMin.X
			|| bMax.Y <= aMin.Y
			|| bMin.Y >= aMax.Y
			);
	};
	auto OverlapWithOtherDrawCall = [&](const FLexUIGeometry& InGeo, const FLexUIDrawCall& OtherDrawCallItem) {
		// SCOPE_CYCLE_COUNTER(STAT_OverlapTest);
		switch (OtherDrawCallItem.Type)
		{
		case ELexUIDrawCallType::BatchMesh:
			{
				//compare draw-call item's bounds
				if (OtherDrawCallItem.BatchMeshTreeNode->Overlap(LexUIQuadTree::Rectangle(InGeo.BoundsMin2DInCanvasSpace, InGeo.BoundsMax2DInCanvasSpace)))
				{
					return true;
				}
			}
			break;
		case ELexUIDrawCallType::PostProcess:
			{
				auto OtherUIGeo = OtherDrawCallItem.PostProcessVisualObject->GetGeometry();
				//check bounds overlap
				if (IntersectBounds(InGeo.BoundsMin2DInCanvasSpace, InGeo.BoundsMax2DInCanvasSpace, OtherUIGeo->BoundsMin2DInCanvasSpace, OtherUIGeo->BoundsMax2DInCanvasSpace))
				{
					return true;
				}
			}
			break;
		case ELexUIDrawCallType::DirectMesh://mostly direct mesh are difficult to calculate 2d bounds (particles or static-mesh), so just return true-overlap
				return true;
		}

		return false;
	};

	int FitInDrawCallMinIndex = 0;
	auto CanFitInDrawCall = [&](const FLexUIGeometry& InGeo, bool InIs2DUI, int32& OutDrawCallIndexToFitin){
		const auto LastDrawCallIndex = InOutUIDrawCallList.Num() - 1;
		if (LastDrawCallIndex < 0)
		{
			return false;
		}

		if (!InIs2DUI)
		{
			//3d UI can only batch into last draw-call
			const auto& LastDrawCall = InOutUIDrawCallList[LastDrawCallIndex];
			if (LastDrawCall.CanConsumeUIGeometryForBatchMesh(InGeo))
			{
				OutDrawCallIndexToFitin = LastDrawCallIndex;
				return true;
			}
			return false;
		}
		TArray<int> CanFitinDrawCallIndexArray;
		//get all draw-call that can fit-in this UI item, then use the first one (because we iterate from tail to head)
		for (int i = LastDrawCallIndex; i >= FitInDrawCallMinIndex; i--)//from tail to head
		{
			const auto& OtherDrawCall = InOutUIDrawCallList[i];
			if (!OtherDrawCall.bIs2DSpace)//draw-call is 3d, can't batch
			{
				return false;
			}

			if (!OtherDrawCall.CanConsumeUIGeometryForBatchMesh(InGeo))//can't fit in this draw-call, should check overlap
			{
				if (OverlapWithOtherDrawCall(InGeo, OtherDrawCall))//overlap with other draw-call, can't batch
				{
					if (CanFitinDrawCallIndexArray.Num() > 0)
					{
						OutDrawCallIndexToFitin = CanFitinDrawCallIndexArray[CanFitinDrawCallIndexArray.Num() - 1];
						return true;
					}
					return false;
				}
				continue;//not overlap with other draw-call, keep searching
			}
			//can fit-in this drawcall but also overlap with it, then no need to go deeper because it must not batch in other deeper drawcall
			if (OtherDrawCall.BatchMeshTreeNode->Overlap(LexUIQuadTree::Rectangle(InGeo.BoundsMin2DInCanvasSpace, InGeo.BoundsMax2DInCanvasSpace)))
			{
				OutDrawCallIndexToFitin = i;
				return true;
			}
			CanFitinDrawCallIndexArray.Add(i);
		}
		if (CanFitinDrawCallIndexArray.Num() > 0)
		{
			OutDrawCallIndexToFitin = CanFitinDrawCallIndexArray[CanFitinDrawCallIndexArray.Num() - 1];
			return true;
		}
		return false;
	};

	auto PushSingleDrawCall = [&](const FLexUIRenderData& InRenderData, ELexUIDrawCallType InDrawCallType, bool InIs2DSpace = true) {
		switch (InDrawCallType)
		{
		default:
		case ELexUIDrawCallType::BatchMesh:
			{
				auto& InItemGeo = InRenderData.BatchMeshGeometry;
				auto DrawCallItem = FLexUIDrawCall(CanvasRect);
				if (InItemGeo.bIsFont)
				{
					DrawCallItem.FontTexture = InItemGeo.Texture;
				}
				else
				{
					DrawCallItem.Texture = InItemGeo.Texture;
				}
				DrawCallItem.Material = InItemGeo.Material.Get();
				DrawCallItem.BatchMeshGeometryArray.Add(InItemGeo);
				DrawCallItem.BatchMeshVisualArray.Add(InRenderData.BatchMeshVisualObject);
				DrawCallItem.VerticesCount = InItemGeo.Vertices.Num();
				DrawCallItem.IndicesCount = InItemGeo.Triangles.Num();
				DrawCallItem.BatchMeshTreeNode->Insert(LexUIQuadTree::Rectangle(InItemGeo.BoundsMin2DInCanvasSpace, InItemGeo.BoundsMax2DInCanvasSpace));
				DrawCallItem.bIs2DSpace = InIs2DSpace;
				InOutUIDrawCallList.Add(MoveTemp(DrawCallItem));
			}
			break;
		case ELexUIDrawCallType::PostProcess:
			{
				auto DrawCallItem = FLexUIDrawCall(InDrawCallType);
				DrawCallItem.PostProcessVisualObject = InRenderData.PostProcessVisualObject;
				DrawCallItem.bIs2DSpace = InIs2DSpace;
				InOutUIDrawCallList.Add(MoveTemp(DrawCallItem));
			}
			break;
		case ELexUIDrawCallType::DirectMesh:
			{
				auto DrawCallItem = FLexUIDrawCall(InDrawCallType);
				DrawCallItem.DirectMeshVisualObject = InRenderData.DirectMeshVisualObject;
				DrawCallItem.bIs2DSpace = InIs2DSpace;
				InOutUIDrawCallList.Add(MoveTemp(DrawCallItem));
			}
			break;
		}
	};

	//for sorted ui items, iterate from head to tail, compare draw-call from tail to head
	for (int i = 0; i < InRenderDataArray.Num(); i++)
	{
		auto& RenderData = InRenderDataArray[i];
		switch (RenderData.Type)
		{
		case ELexUIDrawCallType::ChildCanvas:
			{
				auto ChildCanvasDrawCall = FLexUIDrawCall(ELexUIDrawCallType::ChildCanvas);
				ChildCanvasDrawCall.ChildCanvas = RenderData.ChildCanvas;
				InOutUIDrawCallList.Add(MoveTemp(ChildCanvasDrawCall));

				FitInDrawCallMinIndex = InOutUIDrawCallList.Num();
			}
			break;
		case ELexUIDrawCallType::BatchMesh:
			{
				auto& ItemGeo = RenderData.BatchMeshGeometry;

				bool is2DUIItem = Is2DUITransform(ItemGeo.TransformRelativeToCanvas);
				int DrawCallIndexToFitin;
				if (ItemGeo.bSupportDrawcallBatching && CanFitInDrawCall(ItemGeo, is2DUIItem, DrawCallIndexToFitin))
				{
					auto& DrawCallItem = InOutUIDrawCallList[DrawCallIndexToFitin];
					DrawCallItem.bIs2DSpace = DrawCallItem.bIs2DSpace && is2DUIItem;
					if (ItemGeo.bIsFont)
					{
						if (DrawCallItem.FontTexture != ItemGeo.Texture)
						{
							DrawCallItem.FontTexture = ItemGeo.Texture;
						}
					}
					else
					{
						if (DrawCallItem.Texture != ItemGeo.Texture)
						{
							DrawCallItem.Texture = ItemGeo.Texture;
						}
					}
					//add to this draw-call
					DrawCallItem.BatchMeshGeometryArray.Add(ItemGeo);
					DrawCallItem.BatchMeshVisualArray.Add(RenderData.BatchMeshVisualObject);
					DrawCallItem.BatchMeshTreeNode->Insert(LexUIQuadTree::Rectangle(ItemGeo.BoundsMin2DInCanvasSpace, ItemGeo.BoundsMax2DInCanvasSpace));
					DrawCallItem.VerticesCount += ItemGeo.Vertices.Num();
					DrawCallItem.IndicesCount += ItemGeo.Triangles.Num();
					check(DrawCallItem.VerticesCount < LEXUI_MAX_VERTEX_COUNT);
				}
				else//cannot fit in any other draw-call
				{
					//make a new draw-call
					PushSingleDrawCall(RenderData, ELexUIDrawCallType::BatchMesh, is2DUIItem);
				}
			}
			break;
		case ELexUIDrawCallType::DirectMesh:
			{
				//every direct mesh is a draw-call
				bool is2DUIItem = true;//post process just use true because it not matter
				PushSingleDrawCall(RenderData, ELexUIDrawCallType::DirectMesh, is2DUIItem);
				FitInDrawCallMinIndex = InOutUIDrawCallList.Num();
			}
			break;
		case ELexUIDrawCallType::PostProcess:
			{
				//every postprocess is a draw-call
				bool is2DUIItem = true;//post process just use true because it not matter
				PushSingleDrawCall(RenderData, ELexUIDrawCallType::PostProcess, is2DUIItem);
				FitInDrawCallMinIndex = InOutUIDrawCallList.Num();
			}
			break;
		}
	}

	for (auto& DrawCallItem : InOutUIDrawCallList)
	{
		if (DrawCallItem.Type == ELexUIDrawCallType::BatchMesh)
		{
			DrawCallItem.ApplyBatchMeshGeometryToCombined();
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("Canvas UpdateDrawCall"), STAT_UpdateDrawCall, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("Canvas CopyBatchMeshGeometry&UpdateMeshSection"), STAT_CopyBatchMeshGeometry, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("Canvas UpdateClipAndGeometry"), STAT_UpdateClipAndGeometry, STATGROUP_LGUI);
void ULexCanvas::UpdateCanvasDrawCall()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateDrawCall)

	//update children canvas
	for (auto& item : ChildrenCanvasArray)
	{
		if (!item.IsValid())continue;
		if (item->bForceRenderToTarget)continue;
		item->UpdateCanvasDrawCall();
	}

	auto LexWidget = GetWidget();
	if (!LexWidget)return;
	/**
	 * Why use bPrevIsVisible?:
	 * If Canvas is rendering in frame 1, and in frame 2 the Canvas is disabled(set WidgetActive to false), then the Canvas will not do draw-call calculation, and the prev existing draw-call mesh is still there and render,
	 * so we check bPrevIsVisible, then we can still do draw-call calculation at this frame, and the prev existing draw-call will be removed.
	 */
	const bool bNowIsVisible = LexWidget->GetWidgetActiveInHierarchy();
	if (bNowIsVisible || bPrevIsVisible)
	{
		if (bNowIsVisible != bPrevIsVisible)
		{
			bCanTickUpdate = true;
		}
		bPrevIsVisible = bNowIsVisible;
	}

	//update draw-call
	bHasPendingUpdateData = false;
	if (bCanTickUpdate)
	{
		bCanTickUpdate = false;
		RootCanvas->bAnythingChangedForRenderTarget = true;
		CheckUIMesh();
		struct LOCAL
		{
			static void CollectRenderWidget(ULexWidget* Widget
				, ULexCanvas* ThisCanvas
				, TArray<TObjectPtr<ULexWidget>>& WidgetCollection)
			{
				WidgetCollection.Add(Widget);//maybe sub-canvas, so collect it before tell canvas
				if (Widget->GetRenderCanvas() == ThisCanvas)
				{
					for (auto Child : Widget->GetChildren())
					{
						CollectRenderWidget(Child, ThisCanvas, WidgetCollection);
					}
				}
			}
		};
		if (bNeedToGenerateWidgetList)
		{
			bNeedToGenerateWidgetList = false;
			WidgetList.Reset();
			LOCAL::CollectRenderWidget(GetWidget(), this, WidgetList);
		}

		CheckWidgetPropertyData();
		WidgetPropertyDataAsTexture->PrepareForBatchUpdate();
		//update clip and geometry from head to tail
		{
			SCOPE_CYCLE_COUNTER(STAT_UpdateClipAndGeometry)
			for (const auto& Widget : WidgetList)
			{
				Widget->UpdateClip(RootCanvas->ClipDataAsTexture, RootCanvas->ClipDataList);
				if (Widget->GetWidgetActiveInHierarchy() && Widget->GetRenderCanvas() == this)
				{
					Widget->UpdateVisual();
				}
			}
		}
		WidgetPropertyDataAsTexture->Flush();
		
		if (bShouldRebuildDrawCall)
		{
			bShouldRebuildDrawCall = false;
			NewestDrawCallFrameNumber = GFrameCounter;

			//rect size minimal at 100, so UIQuadTree can work properly (prevent too small rect)
			//@todo: use a better size, maybe screen size (only for screen space UI)
			const auto Width = FMath::Max(LexWidget->GetWidth(), 100.0f);
			const auto Height = FMath::Max(LexWidget->GetHeight(), 100.0f);
			FVector2D LeftBottomPoint;
			LeftBottomPoint.X = Width * -LexWidget->GetPivot().X;
			LeftBottomPoint.Y = Height * -LexWidget->GetPivot().Y;
			FVector2D RightTopPoint;
			RightTopPoint.X = Width * (1.0f - LexWidget->GetPivot().X);
			RightTopPoint.Y = Height * (1.0f - LexWidget->GetPivot().Y);
			//prepare
			{
				FLexCanvasPreparedDrawCallData PreparedDrawCallData;
				PreparedDrawCallData.LeftBottomPoint = LeftBottomPoint;
				PreparedDrawCallData.RightTopPoint = RightTopPoint;
				PreparedDrawCallData.FrameNumber = GFrameCounter;
				PrepareDrawCallBatchingData(PreparedDrawCallData.DataArray);
				//push to async thread
				DrawCallProcessingRunnable->PushPreparedDrawCallData(MoveTemp(PreparedDrawCallData));
			}
		}
		else
		{
			bHasPendingUpdateData = true;
		}
	}

	if (this == RootCanvas)
	{
		CheckRenderTargetUpdate();
	}
}

void ULexCanvas::UpdateDrawCallBatchData()
{
	if(!GetWidget()->HasRegistered())return;
	//update children canvas
	for (auto& item : ChildrenCanvasArray)
	{
		if (!item.IsValid())continue;
		if (item->bForceRenderToTarget)continue;
		item->UpdateDrawCallBatchData();
	}

	if (!IsValid(UIMesh))return;

	if (!bAllowDropFrame)
	{
		while (DrawCallProcessingRunnable->IsBatching())
		{
			FPlatformProcess::Sleep(0.001f);
		}
	}

	if (DrawCallProcessingRunnable->TryGetDrawCallData(CurrentDrawCallData))
	{
		//update draw-call mesh
		UpdateDrawCallMesh();
		//update draw-call material
		UpdateDrawCallMaterial();

		if (bNeedToVerifyMaterials)
		{
			bNeedToVerifyMaterials = false;
			UIMesh->VerifyMaterials();
		}

		MarkFinishUpdateCanvasDrawCall();
	}
	else
	{
		if (bHasPendingUpdateData)//make sure there is no pending data in async thread, if there is pending data we may update draw-call with wrong data
		{
			//current draw-call data only need to update, then we compare the frame-number,
			//if frame-number is greater than current rendering draw-call's frame-number, that means we can safely update it
			if (GFrameCounter > CurrentDrawCallData.FrameNumber && CurrentDrawCallData.FrameNumber == NewestDrawCallFrameNumber)
			{
				for (int i = 0; i < CurrentDrawCallData.DrawCallArray.Num(); i++)
				{
					auto& DrawCallItem = CurrentDrawCallData.DrawCallArray[i];
					if (DrawCallItem.Type == ELexUIDrawCallType::BatchMesh)
					{
						DrawCallItem.CopyBatchMeshGeometry();
						UIMesh->UpdateMeshSection(i, &DrawCallItem);
					}
				}
			}
		}
	}
	if (IsValid(UIMesh))
	{
		UIMesh->FlushRenderCommand();
	}
}

DECLARE_CYCLE_STAT(TEXT("Canvas UpdateDrawCallMesh"), STAT_UpdateDrawCallMesh, STATGROUP_LGUI);
void ULexCanvas::UpdateDrawCallMesh()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateDrawCallMesh);
	if (!IsValid(UIMesh))return;
	UIMesh->PoolAllRenderSection();
	bool bNeedToUpdateBounds = false;
	for (int i = 0; i < CurrentDrawCallData.DrawCallArray.Num(); i++)
	{
		auto& DrawCallItem = CurrentDrawCallData.DrawCallArray[i];
		switch (DrawCallItem.Type)
		{
		case ELexUIDrawCallType::DirectMesh:
			{
				if (!DrawCallItem.DirectMeshVisualObject.IsValid())
				{
					UE_LOG(LGUI, Warning, TEXT("[%s].%d Invalid DirectMesh draw-call, will ignore it"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
					continue;
				}
				UIMesh->SetupRenderSection(ELexUIRenderSectionType::DirectMesh, &DrawCallItem);
				bNeedToSortRenderPriority = true;
				bNeedToUpdateBounds = true;
			}
			break;
		case ELexUIDrawCallType::BatchMesh:
			{
				UIMesh->SetupRenderSection(ELexUIRenderSectionType::Mesh, &DrawCallItem);
				bNeedToSortRenderPriority = true;
				bNeedToUpdateBounds = true;
			}
			break;
		case ELexUIDrawCallType::PostProcess:
			{
				//only LexUI renderer can render post process
				if (this->GetActualRenderMode() == ELexRenderMode::WorldSpace)
				{
					continue;
				}
				if (!DrawCallItem.PostProcessVisualObject.IsValid())
				{
					UE_LOG(LGUI, Warning, TEXT("[%s].%d Invalid PostProcess draw-call, will ignore it"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
					continue;
				}

				UIMesh->SetupRenderSection(ELexUIRenderSectionType::PostProcess, &DrawCallItem);
				//create new section, need to sort it
				bNeedToSortRenderPriority = true;
				bNeedToUpdateBounds = true;
			}
			break;
		case ELexUIDrawCallType::ChildCanvas:
			{
				if (!DrawCallItem.ChildCanvas.IsValid())
				{
					UE_LOG(LGUI, Warning, TEXT("[%s].%d Invalid ChildCanvas draw-call, will ignore it"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
					continue;
				}
				
				UIMesh->SetupRenderSection(ELexUIRenderSectionType::ChildCanvas, &DrawCallItem);
				//create new section, need to sort it
				bNeedToSortRenderPriority = true;
				bNeedToUpdateBounds = true;
			}
			break;
		}
	}
	
	if (this->IsRootCanvas())
	{
		UIMesh->UpdateChildCanvasSectionBox();
	}
	if (bNeedToUpdateBounds)
	{
		UIMesh->UpdateLocalBounds();//update bounds for UE-Renderer
	}
}

void ULexCanvas::CheckUIMesh()const
{
	if (!IsValid(UIMesh))
	{
		auto MeshType = DefaultMeshType.Get();
		if (MeshType == nullptr)MeshType = ULexUIMeshComponent::StaticClass();
		auto LexWidget = GetWidget();
		auto ObjectName = MakeUniqueObjectName(LexWidget, MeshType, FName(*this->GetWidget()->GetDisplayName()));
		UIMesh = NewObject<ULexUIMeshComponent>(LexWidget, MeshType, ObjectName, RF_Transient);
		UIMesh->RegisterComponentWithWorld(this->GetWorld());
		UIMesh->AttachToComponent(this->GetAttachedRootSceneComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		UIMesh->SetRelativeTransform(FTransform::Identity);
		UIMesh->Init(const_cast<ULexCanvas*>(this));
		bUIMeshNeedToSetInitialParameters = true;
	}
	if (IsValid(UIMesh))
	{
		UIMesh->SetComponentToWorld(GetWidget()->GetWorldTransform());
	}

	if (bUIMeshNeedToSetInitialParameters)
	{
		bUIMeshNeedToSetInitialParameters = false;
		if (RenderModeIsLexRendererOrUERenderer(CurrentRenderMode))
		{
			auto ActualRenderMode = GetActualRenderMode();
#if WITH_EDITOR
			if (!GetWorld()->IsGameWorld())//edit mode
			{
				if (ActualRenderMode == ELexRenderMode::ScreenSpaceOverlay)
					ActualRenderMode = ELexRenderMode::WorldSpace_LexUI;
			}
#endif
			switch (ActualRenderMode)
			{
			case ELexRenderMode::RenderTarget:
			{
				UIMesh->SetSupportLexUIRenderer(true, this->GetRootCanvas()->GetRenderTargetViewExtension(), false);
				UIMesh->SetSupportUERenderer(false);
			}
			break;
			case ELexRenderMode::ScreenSpaceOverlay:
			{
#if WITH_EDITOR
				if (!GetWorld()->IsGameWorld())
				{
					UIMesh->SetSupportLexUIRenderer(true, ULexUIManagerWorldSubsystem::GetViewExtension(GetWorld(), true), false);
					UIMesh->SetSupportUERenderer(true);
				}
				else
#endif
				{
					UIMesh->SetSupportLexUIRenderer(true, ULexUIManagerWorldSubsystem::GetViewExtension(GetWorld(), true), false);
					UIMesh->SetSupportUERenderer(false);
				}
			}
			break;
			case ELexRenderMode::WorldSpace_LexUI:
			{
				UIMesh->SetSupportLexUIRenderer(true, ULexUIManagerWorldSubsystem::GetViewExtension(GetWorld(), true), true);
				UIMesh->SetSupportUERenderer(false);
			}
			break;
			}
		}
		else
		{
			UIMesh->SetSupportLexUIRenderer(false, nullptr, false);
			UIMesh->SetSupportUERenderer(true);
		}
	}
}

void ULexCanvas::SortDrawCall()
{
	UIMesh->SetUITranslucentSortPriority(this->GetActualSortOrder());
	int MeshSectionIndex = 0;
	for (int i = 0; i < CurrentDrawCallData.DrawCallArray.Num(); i++)
	{
		auto& DrawCallItem = CurrentDrawCallData.DrawCallArray[i];
		UIMesh->SetRenderSectionRenderPriority(i, MeshSectionIndex++);
		switch (DrawCallItem.Type)
		{
		case ELexUIDrawCallType::BatchMesh:
		case ELexUIDrawCallType::DirectMesh:
		case ELexUIDrawCallType::PostProcess:
		{
		}
		break;
		case ELexUIDrawCallType::ChildCanvas:
		{
			DrawCallItem.ChildCanvas->SortDrawCall();
		}
		break;
		}
	}

	if (this->IsRootCanvas())
	{
		switch (this->GetActualRenderMode())
		{
		case ELexRenderMode::ScreenSpaceOverlay:
			ULexUIManagerWorldSubsystem::GetViewExtension(GetWorld(), true)->MarkNeedToSortScreenSpacePrimitiveRenderPriority();
			break;
		case ELexRenderMode::RenderTarget:
			GetRenderTargetViewExtension()->MarkNeedToSortScreenSpacePrimitiveRenderPriority();
			break;
		case ELexRenderMode::WorldSpace_LexUI:
			ULexUIManagerWorldSubsystem::GetViewExtension(GetWorld(), true)->MarkNeedToSortWorldSpacePrimitiveRenderPriority();
			break;
		}
	}
}

FName ULexCanvas::LexUI_MainTextureMaterialParameterName = FName(TEXT("LexUI_MainTexture"));
FName ULexCanvas::LexUI_FontTextureMaterialParameterName = FName(TEXT("LexUI_FontTexture"));
FName ULexCanvas::LexUI_ClipDataTexture_MaterialParameterName = FName(TEXT("LexUI_ClipDataTexture"));
FName ULexCanvas::LexUI_WidgetPropertyDataTexture_MaterialParameterName = FName(TEXT("LexUI_WidgetPropertyDataTexture"));
FName ULexCanvas::LexUI_IsRenderByLexUIRenderer_MaterialParameterName = FName(TEXT("LexUI_IsRenderByLexUIRenderer"));

bool ULexCanvas::IsMaterialContainsLexUIParameter(const UMaterialInterface* InMaterial)
{
	static TArray<FMaterialParameterInfo> ParameterInfos;
	static TArray<FGuid> ParameterIds;
	InMaterial->GetAllTextureParameterInfo(ParameterInfos, ParameterIds);
	auto FoundIndex = ParameterInfos.IndexOfByPredicate([](const FMaterialParameterInfo& Item)
		{
			return
				Item.Name == LexUI_MainTextureMaterialParameterName
				|| Item.Name == LexUI_FontTextureMaterialParameterName
				|| Item.Name == LexUI_ClipDataTexture_MaterialParameterName
				|| Item.Name == LexUI_WidgetPropertyDataTexture_MaterialParameterName
				|| Item.Name == LexUI_IsRenderByLexUIRenderer_MaterialParameterName
				;
		});
	return FoundIndex != INDEX_NONE;
}

DECLARE_CYCLE_STAT(TEXT("Canvas UpdateDrawCallMaterial"), STAT_UpdateDrawCallMaterial, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("Canvas SetMaterialParameter"), STAT_SetMaterialParameter, STATGROUP_LGUI);
void ULexCanvas::UpdateDrawCallMaterial()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateDrawCallMaterial);

	//pool and reuse material
	{
		UsingMaterialStartIndex = PooledDefaultMaterialList.Num() - 1;
	}
	//reset index for dynamic material
	{
		for (auto& KeyValue : MapSrcMatToDynamicMat)
		{
			KeyValue.Value.CurrentIndex = 0;
		}
	}

	auto SetParameterForNewlyCreatedMaterial = [&](UMaterialInstanceDynamic* InMaterialInstanceDynamic)
	{
		InMaterialInstanceDynamic->SetScalarParameterValue(LexUI_IsRenderByLexUIRenderer_MaterialParameterName, this->IsRenderByLexUIRendererOrUERenderer());
		InMaterialInstanceDynamic->SetTextureParameterValue(LexUI_WidgetPropertyDataTexture_MaterialParameterName, this->WidgetPropertyDataAsTexture->GetDataTexture());
		InMaterialInstanceDynamic->SetTextureParameterValue(LexUI_ClipDataTexture_MaterialParameterName, RootCanvas->ClipDataAsTexture->GetDataTexture());
	};

	for (int i = 0; i < CurrentDrawCallData.DrawCallArray.Num(); i++)
	{
		auto& DrawCallItem = CurrentDrawCallData.DrawCallArray[i];
		switch (DrawCallItem.Type)
		{
		case ELexUIDrawCallType::BatchMesh:
			{
				UMaterialInterface* RenderMat = nullptr;
				bool bShouldSetMaterialParameter = false;
				if (DrawCallItem.Material.IsValid())
				{
					if (DrawCallItem.Material->IsA<UMaterialInstanceDynamic>())
					{
						auto RenderMatDynamic = static_cast<UMaterialInstanceDynamic*>(RenderMat);
						RenderMat = RenderMatDynamic;
						bShouldSetMaterialParameter = true;
						SetParameterForNewlyCreatedMaterial(RenderMatDynamic);
					}
					else
					{
						auto DynamicMaterialContainerPtr = MapSrcMatToDynamicMat.Find(DrawCallItem.Material.Get());
						if (!DynamicMaterialContainerPtr)
						{
							if (IsMaterialContainsLexUIParameter(DrawCallItem.Material.Get()))
							{
								bShouldSetMaterialParameter = true;
								auto RenderMatDynamic = UMaterialInstanceDynamic::Create(DrawCallItem.Material.Get(), this);
								SetParameterForNewlyCreatedMaterial(RenderMatDynamic);
								auto MaterialContainer = FLexCanvasDynamicMaterialArrayContainer();
								MaterialContainer.MaterialArray.Add(RenderMatDynamic);
								MaterialContainer.CurrentIndex = 1;
								MapSrcMatToDynamicMat.Add(DrawCallItem.Material.Get(), MaterialContainer);
								RenderMat = RenderMatDynamic;
								for (auto& BatchMeshVisual : DrawCallItem.BatchMeshVisualArray)
								{
									if (!BatchMeshVisual.IsValid())continue;
									BatchMeshVisual->OnMaterialInstanceDynamicCreated(RenderMatDynamic);
								}
								bNeedToVerifyMaterials = true;//verify material when new material will be used
							}
							else
							{
								RenderMat = DrawCallItem.Material.Get();
								bNeedToVerifyMaterials = true;//verify material when new material will be used
							}
						}
						else
						{
							bShouldSetMaterialParameter = true;
							auto& MaterialArray = DynamicMaterialContainerPtr->MaterialArray;
							if (!MaterialArray.IsValidIndex(DynamicMaterialContainerPtr->CurrentIndex))//materials used up, need more
							{
								auto RenderMatDynamic = UMaterialInstanceDynamic::Create(DrawCallItem.Material.Get(), this);
								MaterialArray.Add(RenderMatDynamic);
								SetParameterForNewlyCreatedMaterial(RenderMatDynamic);
								RenderMat = RenderMatDynamic;
								DynamicMaterialContainerPtr->CurrentIndex++;
								bNeedToVerifyMaterials = true;//verify material when new material will be used
								for (auto& BatchMeshVisual : DrawCallItem.BatchMeshVisualArray)
								{
									if (!BatchMeshVisual.IsValid())continue;
									BatchMeshVisual->OnMaterialInstanceDynamicCreated(RenderMatDynamic);
								}
							}
							else//enough material, use index one
							{
								auto RenderMatDynamic = MaterialArray[DynamicMaterialContainerPtr->CurrentIndex];
								RenderMat = RenderMatDynamic;
								if (bWidgetPropertyDataAsTextureChanged || RootCanvas->bClipDataAsTextureChanged)
								{
									SetParameterForNewlyCreatedMaterial(RenderMatDynamic);//update texture to material
								}
								DynamicMaterialContainerPtr->CurrentIndex++;
								for (auto& BatchMeshVisual : DrawCallItem.BatchMeshVisualArray)
								{
									if (!BatchMeshVisual.IsValid())continue;
									BatchMeshVisual->OnMaterialInstanceDynamicCreated(RenderMatDynamic);
								}
							}
						}
					}
				}
				else
				{
					auto GetUIMaterialFromPool = [&]()
					{
						if (UsingMaterialStartIndex < 0)
						{
							auto SrcMaterial = GetDefaultMaterial();
							auto RenderMatDynamic = UMaterialInstanceDynamic::Create(SrcMaterial, this);
							RenderMatDynamic->SetFlags(RF_Transient);
							PooledDefaultMaterialList.Add(RenderMatDynamic);
							SetParameterForNewlyCreatedMaterial(RenderMatDynamic);
							bNeedToVerifyMaterials = true;//verify material when new material will be used
							return RenderMatDynamic;
						}
						auto RenderMatDynamic = PooledDefaultMaterialList[UsingMaterialStartIndex];
						if (bWidgetPropertyDataAsTextureChanged || RootCanvas->bClipDataAsTextureChanged)
						{
							SetParameterForNewlyCreatedMaterial(RenderMatDynamic);//update texture to material
						}
						UsingMaterialStartIndex--;
						return RenderMatDynamic.Get();
					};
					RenderMat = GetUIMaterialFromPool();
					bShouldSetMaterialParameter = true;//pooled material definitely contains LexUIParam
				}
				if (bShouldSetMaterialParameter)
				{
					SCOPE_CYCLE_COUNTER(STAT_SetMaterialParameter)
					auto RenderMat_MID = static_cast<UMaterialInstanceDynamic*>(RenderMat);
					auto& ParamCache = MapMatToParamCache.FindOrAdd(RenderMat_MID);
					if (ParamCache.Texture != DrawCallItem.Texture || ParamCache.FontTexture != DrawCallItem.FontTexture)
					{
						RenderMat_MID->SetTextureParameterValue(LexUI_MainTextureMaterialParameterName, DrawCallItem.Texture.Get());
						RenderMat_MID->SetTextureParameterValue(LexUI_FontTextureMaterialParameterName, DrawCallItem.FontTexture.Get());
						ParamCache.Texture = DrawCallItem.Texture;
						ParamCache.FontTexture = DrawCallItem.FontTexture;
					}
					if (bNeedToSetClipDataTextureMaterialParameter)
					{
						RenderMat_MID->SetTextureParameterValue(LexUI_ClipDataTexture_MaterialParameterName, RootCanvas->ClipDataAsTexture->GetDataTexture());
					}
				}
				UIMesh->SetMeshSectionMaterial(i, RenderMat);
			}
			break;
		case ELexUIDrawCallType::PostProcess:
		case ELexUIDrawCallType::ChildCanvas:
		case ELexUIDrawCallType::DirectMesh:
			{

			}
			break;
		}
	}

	if (bNeedToVerifyMaterials
		|| CurrentDrawCallData.DrawCallArray.Num() == 0
		)
	{
		MarkNeedVerifyMaterials();//tell parent canvas to verify material
	}

	bNeedToSetClipDataTextureMaterialParameter = false;
	bWidgetPropertyDataAsTextureChanged = false;
	if (RootCanvas == this)
	{
		RootCanvas->bClipDataAsTextureChanged = false;
	}
}

void ULexCanvas::MarkNeedVerifyMaterials()
{
	bNeedToVerifyMaterials = true;
	if (ParentCanvas.IsValid()
		&& !this->GetOverrideSorting()//if override sorting, then render by self(not parent)
		)
	{
		ParentCanvas->MarkNeedVerifyMaterials();
	}
}

void ULexCanvas::SetRenderTargetResolutionScale(float Value)
{
	if (RenderTargetResolutionScale != Value)
	{
		RenderTargetResolutionScale = Value;
		bAnythingChangedForRenderTarget = true;
	}
}

void ULexCanvas::SetRenderTargetSizeMode(ELexCanvasRenderTargetSizeMode Value)
{
	if (RenderTargetSizeMode != Value)
	{
		RenderTargetSizeMode = Value;
		bAnythingChangedForRenderTarget = true;
	}
}

void ULexCanvas::SetRenderTargetUpdateMode(ELexCanvasRenderTargetUpdateMode Value)
{
	if (RenderTargetUpdateMode != Value)
	{
		RenderTargetUpdateMode = Value;
		bAnythingChangedForRenderTarget = true;
	}
}

void ULexCanvas::RequestUpdateForRenderTarget()
{
	if (RootCanvas == this)
	{
		bRequestUpdateForRenderTarget = true;
	}
}

void ULexCanvas::SetSortOrderAdditionalValueRecursive(int32 InAdditionalValue)
{
	if (FMath::Abs(this->SortOrder + InAdditionalValue) > MAX_int16)
	{
		auto errorMsg = FText::Format(LOCTEXT("SortOrderOutOfRange", "{0} sortOrder out of range!\nNOTE! sortOrder value is stored with int16 type, so valid range is -32768 to 32767")
			, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)));
		UE_LOG(LGUI, Error, TEXT("%s"), *errorMsg.ToString());
#if WITH_EDITOR
		FLexUIUtils::EditorNotification(errorMsg, false);
#endif
		return;
	}

	this->SortOrder += InAdditionalValue;
	for (auto ChildCanvas : ChildrenCanvasArray)
	{
		if (!ChildCanvas.IsValid())continue;
		if (ChildCanvas->bForceRenderToTarget)continue;
		ChildCanvas->SetSortOrderAdditionalValueRecursive(InAdditionalValue);
	}
}

void ULexCanvas::SetSortOrder(int32 InSortOrder, bool InPropagateToChildrenCanvas)
{
	if (SortOrder != InSortOrder)
	{
		if (CheckRootCanvas())
		{
			RootCanvas->bNeedToSortRenderPriority = true;
		}
		MarkCanvasUpdate(false);
		if (InPropagateToChildrenCanvas)
		{
			int32 Diff = InSortOrder - SortOrder;
			SetSortOrderAdditionalValueRecursive(Diff);
		}
		else
		{
			if (FMath::Abs(InSortOrder) > MAX_int16)
			{
				auto errorMsg = FText::Format(LOCTEXT("SortOrderOutOfRange", "{0} sortOrder out of range!\nNOTE! sortOrder value is stored with int16 type, so valid range is -32768 to 32767")
					, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)));
				UE_LOG(LGUI, Error, TEXT("%s"), *errorMsg.ToString());
#if WITH_EDITOR
				FLexUIUtils::EditorNotification(errorMsg, false);
#endif
				InSortOrder = FMath::Clamp(InSortOrder, (int32)MIN_int16, (int32)MAX_int16);
			}
			this->SortOrder = InSortOrder;
		}

		if (CheckRootCanvas())
		{
			RootCanvas->bNeedToSortRenderPriority = true;
		}
	}
}
void ULexCanvas::SetSortOrderToHighestOfHierarchy(bool InPropagateToChildrenCanvas)
{
	int32 Min = INT_MAX, Max = INT_MIN;
	GetMinMaxSortOrderOfHierarchy(Min, Max);
	SetSortOrder(Max + 1, InPropagateToChildrenCanvas);
}
void ULexCanvas::SetSortOrderToLowestOfHierarchy(bool InPropagateToChildrenCanvas)
{
	int32 Min = INT_MAX, Max = INT_MIN;
	GetMinMaxSortOrderOfHierarchy(Min, Max);
	SetSortOrder(Min - 1, InPropagateToChildrenCanvas);
}

void ULexCanvas::GetMinMaxSortOrderOfHierarchy(int32& OutMin, int32& OutMax)
{
	auto ThisCanvasSortOrder = this->GetActualSortOrder();
	if (ThisCanvasSortOrder < OutMin)
	{
		OutMin = ThisCanvasSortOrder;
	}
	if (ThisCanvasSortOrder > OutMax)
	{
		OutMax = ThisCanvasSortOrder;
	}
	for (auto ChildCanvas : ChildrenCanvasArray)
	{
		if (!ChildCanvas.IsValid())continue;
		if (ChildCanvas->bForceRenderToTarget)continue;
		ChildCanvas->GetMinMaxSortOrderOfHierarchy(OutMin, OutMax);
	}
}


UMaterialInterface* ULexCanvas::GetDefaultMaterial()const
{
	if (!DefaultMaterial)
	{
		DefaultMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/LGUI/Materials/LexUI_ImageAndFont"));
		if (!DefaultMaterial)
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d Load DefaultMaterial error! Missing some content of LexUI plugin, reinstall this plugin may fix the issue."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		}
	}
	return DefaultMaterial;
}

void ULexCanvas::SetDefaultMaterial(UMaterialInterface* InMaterial)
{
	if (DefaultMaterial != InMaterial)
	{
		ClearDrawCall();
		MarkCanvasUpdate(true);
	}
}

void ULexCanvas::SetTraceChannel(TEnumAsByte<ETraceTypeQuery> InTraceChannel)
{
	if (TraceChannel != InTraceChannel)
	{
		TraceChannel = InTraceChannel;
	}
}

float ULexCanvas::GetActualBlendDepth()const
{
	if (IsRootCanvas())
	{
		return BlendDepth;
	}
	else
	{
		if (GetOverrideBlendDepth())
		{
			return BlendDepth;
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->GetActualBlendDepth();
			}
		}
	}
	return BlendDepth;
}

int ULexCanvas::GetActualDepthFade()const
{
	if (IsRootCanvas())
	{
		return DepthFade;
	}
	else
	{
		if (GetOverrideDepthFade())
		{
			return DepthFade;
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->GetActualDepthFade();
			}
		}
	}
	return DepthFade;
}

int32 ULexCanvas::GetActualSortOrder()const
{
	if (IsRootCanvas())
	{
		if (bOverrideSorting)
		{
			return SortOrder;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if (bOverrideSorting)
		{
			return SortOrder;
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->GetActualSortOrder();
			}
		}
	}
	return SortOrder;
}

void ULexCanvas::SetOverrideSorting(bool Value)
{
	if (bOverrideSorting != Value)
	{
		bOverrideSorting = Value;
		if (CheckRootCanvas())
		{
			RootCanvas->bNeedToSortRenderPriority = true;
		}
		MarkCanvasUpdate(false);
	}
}

bool ULexCanvas::GetActualRequireNormalAndTangent()const
{
	if (IsRootCanvas())
	{
		return bRequireNormalAndTangent;
	}
	else
	{
		if (GetOverrideRequireNormalAndTangent())
		{
			return bRequireNormalAndTangent;
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->GetActualRequireNormalAndTangent();
			}
		}
	}
	return bRequireNormalAndTangent;
}
void ULexCanvas::SetRequireNormalAndTangent(bool Value)
{
	if (bRequireNormalAndTangent != Value)
	{
		bRequireNormalAndTangent = Value;
		MarkCanvasUpdate(false);
		if (auto LexWidget = GetWidget())
		{
			LexWidget->MarkAllDirtyRecursive();
		}
	}
}

void ULexCanvas::BuildProjectionMatrix(FIntPoint InViewportSize, ECameraProjectionMode::Type InProjectionType, float InFOV, float FarClipPlane, float NearClipPlane, FMatrix& OutProjectionMatrix)
{
	if (InViewportSize.X == 0 || InViewportSize.Y == 0)//in DebugCamera mode(toggle in editor by press ';'), viewport size is 0
	{
		InViewportSize.X = InViewportSize.Y = 1;
	}
	if (InProjectionType == ECameraProjectionMode::Orthographic)
	{
		check((int32)ERHIZBuffer::IsInverted);
		const float tempOrthoWidth = InViewportSize.X * 0.5f;
		const float tempOrthoHeight = InViewportSize.Y * 0.5f;

		const float ZScale = 1.0f / (FarClipPlane - NearClipPlane);
		const float ZOffset = -NearClipPlane;

		if ((int32)ERHIZBuffer::IsInverted)
		{
			OutProjectionMatrix = FReversedZOrthoMatrix(
				tempOrthoWidth,
				tempOrthoHeight,
				ZScale,
				ZOffset
			);
		}
		else
		{
			OutProjectionMatrix = FOrthoMatrix(
				tempOrthoWidth,
				tempOrthoHeight,
				ZScale,
				ZOffset
			);
		}
	}
	else
	{
		float XAxisMultiplier = 1.0f;
		float YAxisMultiplier = InViewportSize.X / (float)InViewportSize.Y;

		if ((int32)ERHIZBuffer::IsInverted)
		{
			OutProjectionMatrix = FReversedZPerspectiveMatrix(
				InFOV,
				InFOV,
				XAxisMultiplier,
				YAxisMultiplier,
				NearClipPlane,
				FarClipPlane
			);
		}
		else
		{
			OutProjectionMatrix = FPerspectiveMatrix(
				InFOV,
				InFOV,
				XAxisMultiplier,
				YAxisMultiplier,
				NearClipPlane,
				FarClipPlane
			);
		}
	}
}
float ULexCanvas::CalculateDistanceToCamera()const
{
	if (ProjectionType == ECameraProjectionMode::Orthographic)
	{
		return 1000;
	}
	else
	{
		if (auto LexWidget = GetWidget())
		{
			return LexWidget->GetWidth() * 0.5f / FMath::Tan(FMath::DegreesToRadians(FieldOfView * 0.5f)) * LexWidget->GetWorldScale().X;
		}
		return 1;
	}
}
FMatrix ULexCanvas::GetViewProjectionMatrix()const
{
	if (bIsViewProjectionMatrixDirty)
	{
		bIsViewProjectionMatrixDirty = false;

		FVector ViewLocation = GetViewLocation();
		FMatrix ViewRotationMatrix = FInverseRotationMatrix(GetViewRotator())
			* FMatrix(
				FPlane(0, 0, 1, 0),
				FPlane(1, 0, 0, 0),
				FPlane(0, 1, 0, 0),
				FPlane(0, 0, 0, 1))
			;
		FMatrix ProjectionMatrix = GetProjectionMatrix();
		CacheViewProjectionMatrix = FTranslationMatrix(-ViewLocation) * ViewRotationMatrix * ProjectionMatrix;
	}
	return CacheViewProjectionMatrix;
}
FMatrix ULexCanvas::GetProjectionMatrix()const
{
	if (bOverrideProjectionMatrix)
		return OverrideProjectionMatrix;

	FMatrix ProjectionMatrix = FMatrix::Identity;
	const float FOV = (bOverrideFovAngle ? OverrideFovAngle : FieldOfView) * (float)PI / 360.0f;
	auto LexWidget = GetWidget();
	BuildProjectionMatrix(FIntPoint(LexWidget->GetWidth(), LexWidget->GetHeight()), ProjectionType, FOV, FarClipPlane, NearClipPlane, ProjectionMatrix);
	return ProjectionMatrix;
}
FVector ULexCanvas::GetViewLocation()const
{
	if (bOverrideViewLocation)
		return OverrideViewLocation;

	auto LexWidget = GetWidget();
	return LexWidget->GetWorldLocation() - LexWidget->GetForwardVector() * CalculateDistanceToCamera();
}
FRotator ULexCanvas::GetViewRotator()const
{
	if (bOverrideViewRotation)
		return OverrideViewRotation;

	return GetWidget()->GetWorldRotation().Rotator();
}
FIntPoint ULexCanvas::GetViewportSize()const
{
	auto TempViewportSize = FIntPoint(2, 2);
	if (auto world = this->GetWorld())
	{
#if WITH_EDITOR
		if (!world->IsGameWorld())
		{
			if (auto LexWidget = GetWidget())
			{
				TempViewportSize.X = LexWidget->GetWidth();
				TempViewportSize.Y = LexWidget->GetHeight();
			}
		}
		else
#endif
		{
			if (RenderMode == ELexRenderMode::ScreenSpaceOverlay)
			{
				if (auto pc = world->GetFirstPlayerController())
				{
					pc->GetViewportSize(TempViewportSize.X, TempViewportSize.Y);
				}
			}
			else if (RenderMode == ELexRenderMode::RenderTarget && IsValid(RenderTarget))
			{
				TempViewportSize.X = RenderTarget->SizeX / RenderTargetResolutionScale;
				TempViewportSize.Y = RenderTarget->SizeY / RenderTargetResolutionScale;
			}
		}
	}
	return TempViewportSize;
}

void ULexCanvas::SetRenderMode(ELexRenderMode Value)
{
	if (RenderMode != Value)
	{
		RenderMode = Value;
		MarkCanvasUpdate(true);
		CheckRenderMode(true);

		UnregisterCanvasScaler();
		RegisterCanvasScaler();
	}
}

void ULexCanvas::SetForceRenderToTarget(bool Value)
{
	if (bForceRenderToTarget != Value)
	{
		bForceRenderToTarget = Value;
		if (bForceRenderToTarget)
		{
			MarkCanvasUpdate(true);
			GetWidget()->MarkAllDirtyRecursive();
		}
	}
}

void ULexCanvas::SetProjectionParameters(TEnumAsByte<ECameraProjectionMode::Type> InProjectionType, float InFovAngle, float InNearClipPlane, float InFarClipPlane)
{
	ProjectionType = InProjectionType;
	FieldOfView = InFovAngle;
	NearClipPlane = InNearClipPlane;
	FarClipPlane = InFarClipPlane;

	bIsViewProjectionMatrixDirty = true;
}

void ULexCanvas::SetRenderTarget(UTextureRenderTarget2D* Value)
{
	if (RenderTarget != Value)
	{
		RenderTarget = Value;
		if (CheckRootCanvas() && RootCanvas == this)
		{
			UpdateRenderTarget(false);
		}
		OnRenderTargetChanged.Broadcast(RenderTarget);
	}
}

void ULexCanvas::SetRenderTargetClearColor(FColor Value)
{
	if (RenderTargetClearColor != Value)
	{
		RenderTargetClearColor = Value;
		if (CheckRootCanvas() && RootCanvas == this)
		{
			this->bRequestUpdateForRenderTarget = true;
			this->MarkCanvasUpdate(false);
		}
	}
}

ELexRenderMode ULexCanvas::GetActualRenderMode()const
{
	if (IsRootCanvas())
	{
		return this->RenderMode;
	}
	else
	{
		if (bForceRenderToTarget)
		{
			checkf(this->RenderMode == ELexRenderMode::RenderTarget, TEXT("[%s].%d This error should not happen!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return this->RenderMode;
		}
		if (CheckRootCanvas())
		{
			return RootCanvas->RenderMode;
		}
	}
	return ELexRenderMode::WorldSpace;
}

void ULexCanvas::SetBlendDepth(float Value)
{
	if (BlendDepth != Value)
	{
		BlendDepth = Value;

		if (CheckRootCanvas())
		{
			if (RootCanvas->RenderModeIsLexRendererOrUERenderer(CurrentRenderMode))
			{
				if (RootCanvas->IsRenderToWorldSpace())
				{
					auto ViewExtension = ULexUIManagerWorldSubsystem::GetViewExtension(GetWorld(), false);
					if (ViewExtension.IsValid())
					{
						ViewExtension->SetRenderCanvasDepthParameter(this, this->GetActualBlendDepth(), this->GetActualDepthFade());
					}
				}
			}
		}
	}
}

void ULexCanvas::SetDepthFade(int Value)
{
	if (DepthFade != Value)
	{
		DepthFade = Value;

		if (CheckRootCanvas())
		{
			if (RootCanvas->RenderModeIsLexRendererOrUERenderer(CurrentRenderMode))
			{
				if (RootCanvas->IsRenderToWorldSpace())
				{
					auto ViewExtension = ULexUIManagerWorldSubsystem::GetViewExtension(GetWorld(), false);
					if (ViewExtension.IsValid())
					{
						ViewExtension->SetRenderCanvasDepthParameter(this, this->GetActualBlendDepth(), this->GetActualDepthFade());
					}
				}
			}
		}
	}
}

void ULexCanvas::SetEnableDepthTest(bool Value)
{
	if (bEnableDepthTest != Value)
	{
		bEnableDepthTest = Value;
	}
}

UTextureRenderTarget2D* ULexCanvas::GetActualRenderTarget()const
{
	if (IsRootCanvas())
	{
		return this->RenderTarget;
	}
	else
	{
		if (CheckRootCanvas())
		{
			return RootCanvas->RenderTarget;
		}
	}
	return nullptr;
}

int32 ULexCanvas::GetDrawCallCount()const
{
	int32 Result = 0;
	for (auto& Item : CurrentDrawCallData.DrawCallArray)
	{
		if (Item.Type != ELexUIDrawCallType::ChildCanvas)
		{
			Result++;
		}
	}
	return Result;
}

void ULexCanvas::OnClipDataTextureChanged(UTexture* NewTexture)
{
	check(this == RootCanvas);//only root canvas use ClipDataTexture
	MarkCanvasUpdate(true);
	bClipDataAsTextureChanged = true;
}

void ULexCanvas::OnWidgetPropertyDataTextureChanged(UTexture* NewTexture)
{
	MarkCanvasUpdate(true);
	bWidgetPropertyDataAsTextureChanged = true;
}

void ULexCanvas::CheckWidgetPropertyData()
{
	if (!IsValid(WidgetPropertyDataAsTexture))
	{
		WidgetPropertyDataAsTexture = NewObject<ULexUIDataAsTexture>(this, ULexUIDataAsTexture::StaticClass(), NAME_None, RF_Transient);
		WidgetPropertyDataAsTexture->Init(ULexVisual::WidgetPropertyDataLength, ELexUIDataAsTexturePixelFormat::R32, 128, 2048);
		WidgetPropertyDataAsTexture->OnDataTextureChange.AddUObject(this, &ULexCanvas::OnWidgetPropertyDataTextureChanged);
	}
}

void ULexCanvas::PushAsyncFunction_TransformVertices(TFunction<void()> InFunction)
{
	TransformVerticesAsyncFunctionRunnable->PushFunction(MoveTemp(InFunction));
}

void ULexCanvas::RemoveClipData(const TSharedPtr<FLexUIClipData>& InClipData)
{
	RootCanvas->ClipDataList.Remove(InClipData);
}
UTexture* ULexCanvas::GetClipDataTexture()const
{
	return IsValid(RootCanvas->ClipDataAsTexture) ? RootCanvas->ClipDataAsTexture->GetDataTexture() : nullptr;
}

FTransform2D ULexCanvas::ConvertTo2DTransform(const FTransform& Transform)
{
	auto itemToCanvasMatrix = Transform.ToMatrixWithScale();
	auto itemLocation = Transform.GetLocation();
	auto itemToCanvasTf2D = FTransform2D(FMatrix2x2(itemToCanvasMatrix.M[1][1], itemToCanvasMatrix.M[1][2], itemToCanvasMatrix.M[2][1], itemToCanvasMatrix.M[2][2]), FVector2D(itemLocation.Y, itemLocation.Z));
	return itemToCanvasTf2D;
}

template<class T>
FORCEINLINE void GetMinMax(T a, T b, T c, T d, T& min, T& max)
{
	float abMin = FMath::Min(a, b);
	float abMax = FMath::Max(a, b);
	float cdMin = FMath::Min(c, d);
	float cdMax = FMath::Max(c, d);
	min = FMath::Min(abMin, cdMin);
	max = FMath::Max(abMax, cdMax);
}
void ULexCanvas::CalculateVisual2DBounds(ULexVisual* Visual, const FTransform2D& OutTransform2D, FVector2D& OutMin, FVector2D& OutMax)
{
	FVector2D LocalPoint1, LocalPoint2;
	Visual->GetGeometryBoundsInLocalSpace(LocalPoint1, LocalPoint2);
	const auto Point1 = OutTransform2D.TransformPoint(LocalPoint1);
	const auto Point2 = OutTransform2D.TransformPoint(LocalPoint2);
	const auto Point3 = OutTransform2D.TransformPoint(FVector2D(LocalPoint2.X, LocalPoint1.Y));
	const auto Point4 = OutTransform2D.TransformPoint(FVector2D(LocalPoint1.X, LocalPoint2.Y));

	GetMinMax(Point1.X, Point2.X, Point3.X, Point4.X, OutMin.X, OutMax.X);
	GetMinMax(Point1.Y, Point2.Y, Point3.Y, Point4.Y, OutMin.Y, OutMax.Y);
}

#undef LOCTEXT_NAMESPACE


#pragma region CanvasScaler
void ULexCanvasCustomScale::Init(ULexCanvas* InCanvas)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveInit(InCanvas);
	}
}
void ULexCanvasCustomScale::CalculateSizeAndScale(ULexCanvas* InCanvas, const FIntPoint& InViewportSize, FIntPoint& OutLGUICanvasSize, float& OutScale)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveCalculateSizeAndScale(InCanvas, InViewportSize, OutLGUICanvasSize, OutScale);
	}
}

bool ULexCanvasCustomScale::ConvertPositionFromViewportToCanvas(const FVector2D& InPosition, FVector2D& Result) const
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		return ReceiveConvertPositionFromViewportToCanvas(InPosition, Result);
	}
	return false;
}

bool ULexCanvasCustomScale::ConvertPositionFromCanvasToViewport(const FVector2D& InPosition, FVector2D& Result) const
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		return ReceiveConvertPositionFromCanvasToViewport(InPosition, Result);
	}
	return false;
}

void ULexCanvas::CheckAndApplyViewportParameter()
{
	switch (this->GetRenderMode())
	{
	case ELexRenderMode::ScreenSpaceOverlay:
	{
		ViewportSize = this->GetViewportSize();
		OnViewportParameterChanged();
	}
	break;
	case ELexRenderMode::RenderTarget:
	{
		if (IsValid(RenderTarget))
		{
			ViewportSize.X = RenderTarget->SizeX / RenderTargetResolutionScale;
			ViewportSize.Y = RenderTarget->SizeY / RenderTargetResolutionScale;
			OnViewportParameterChanged();
		}
	}
	break;
	}
}

void ULexCanvas::RegisterCanvasScaler()
{
#if WITH_EDITOR
	if (GetWorld() && !GetWorld()->IsGameWorld() && this->IsRootCanvas())
	{
		if (auto LexUIManagerObject = ULexUIManagerObject::GetInstance(true))
		{
			EditorTickDelegateHandle = LexUIManagerObject->GetEditorTickDelegate().AddWeakLambda(this, [this](float deltaTime) {
				this->OnEditorTick(deltaTime);
				});
		}
	}
#endif

	bIsViewProjectionMatrixDirty = true;

	if (this->IsRootCanvas())
	{
		if (this->GetRenderMode() == ELexRenderMode::ScreenSpaceOverlay
			|| this->GetRenderMode() == ELexRenderMode::RenderTarget
			)
		{
			CheckAndApplyViewportParameter();

			if (this->GetRenderMode() == ELexRenderMode::ScreenSpaceOverlay)
			{
				if (auto world = GetWorld())
				{
					if (auto gameViewport = world->GetGameViewport())
					{
						if (auto viewport = gameViewport->Viewport)
						{
							ViewportResizeDelegateHandle = viewport->ViewportResizedEvent.AddWeakLambda(this, [this](FViewport*, uint32)
							{
								CheckAndApplyViewportParameter();
							});
						}
					}
				}
			}
		}
	}
}

void ULexCanvas::UnregisterCanvasScaler()
{
#if WITH_EDITOR
	if (EditorTickDelegateHandle.IsValid())
	{
		if (auto LexUIManagerObject = ULexUIManagerObject::GetInstance(false))
		{
			LexUIManagerObject->GetEditorTickDelegate().Remove(EditorTickDelegateHandle);
		}
	}
#endif
	//reset the canvasScale to default
	CanvasScale = 1.0f;

	if (ViewportResizeDelegateHandle.IsValid())
	{
		if (auto world = GetWorld())
		{
			if (auto gameViewport = world->GetGameViewport())
			{
				if (auto viewport = gameViewport->Viewport)
				{
					viewport->ViewportResizedEvent.Remove(ViewportResizeDelegateHandle);
				}
			}
		}
	}
}

void ULexCanvas::OnViewportParameterChanged()
{
	if (ViewportSize.X <= 0 || ViewportSize.Y <= 0)return;
	if (this->IsRootCanvas())
	{
		if (this->GetRenderMode() == ELexRenderMode::ScreenSpaceOverlay
			|| this->GetRenderMode() == ELexRenderMode::RenderTarget
			)
		{
			if (auto LexWidget = GetWidget())
			{
				float TempCanvasScale = 1.0f;
				//adjust size
				switch (ScaleMode)
				{
				case ELexCanvasScaleMode::ConstantPixelSize:
					{
						LexWidget->SetWidth(ViewportSize.X);
						LexWidget->SetHeight(ViewportSize.Y);
						TempCanvasScale = 1.0f;
					}
					break;
				case ELexCanvasScaleMode::ScaleWithScreenSize:
					{
						switch (ScreenMatchMode)
						{
						case ELexCanvasScreenMatchMode::MatchWidthOrHeight:
							{
								float matchWidth_PreferredWidth = ReferenceResolution.X;
								float matchWidth_PreferredHeight = ReferenceResolution.X * ViewportSize.Y / ViewportSize.X;
								float matchWidth_ScaleRatio = ViewportSize.X / ReferenceResolution.X;

								float matchHeight_PreferredHeight = ReferenceResolution.Y;
								float matchHeight_PreferredWidth = ReferenceResolution.Y * ViewportSize.X / ViewportSize.Y;
								float matchHeight_ScaleRatio = ViewportSize.Y / ReferenceResolution.Y;

								LexWidget->SetWidth(FMath::Lerp(matchWidth_PreferredWidth, matchHeight_PreferredWidth, MatchFromWidthToHeight));
								LexWidget->SetHeight(FMath::Lerp(matchWidth_PreferredHeight, matchHeight_PreferredHeight, MatchFromWidthToHeight));

								TempCanvasScale = FMath::Lerp(matchWidth_ScaleRatio, matchHeight_ScaleRatio, MatchFromWidthToHeight);
							}
							break;
						case ELexCanvasScreenMatchMode::Expand:
						case ELexCanvasScreenMatchMode::Shrink:
							{
								float resultWidth = ViewportSize.X, resultHeight = ViewportSize.Y;

								float screenAspect = (float)ViewportSize.X / ViewportSize.Y;
								float referenceAspect = ReferenceResolution.X / ReferenceResolution.Y;
								if (screenAspect > referenceAspect)//screen width > reference width
								{
									if (ScreenMatchMode == ELexCanvasScreenMatchMode::Shrink)
									{
										resultHeight = ReferenceResolution.Y;
										resultWidth = resultHeight * screenAspect;
										TempCanvasScale = (float)ViewportSize.Y / resultHeight;
									}
									else if (ScreenMatchMode == ELexCanvasScreenMatchMode::Expand)
									{
										resultWidth = ReferenceResolution.X;
										resultHeight = resultWidth / screenAspect;
										TempCanvasScale = (float)ViewportSize.X / resultWidth;
									}
								}
								else//screen height > reference height
								{
									if (ScreenMatchMode == ELexCanvasScreenMatchMode::Shrink)
									{
										resultWidth = ReferenceResolution.X;
										resultHeight = resultWidth / screenAspect;
										TempCanvasScale = (float)ViewportSize.X / resultWidth;
									}
									else if (ScreenMatchMode == ELexCanvasScreenMatchMode::Expand)
									{
										resultHeight = ReferenceResolution.Y;
										resultWidth = resultHeight * screenAspect;
										TempCanvasScale = (float)ViewportSize.Y / resultHeight;
									}
								}
								LexWidget->SetWidth(resultWidth);
								LexWidget->SetHeight(resultHeight);
							}
							break;
						}
					}
					break;
				case ELexCanvasScaleMode::Custom:
					{
						if (IsValid(CustomScale))
						{
							TempCanvasScale = 1.0f;
							auto ScaledViewportSize = ViewportSize;
							CustomScale->CalculateSizeAndScale(this, ViewportSize, ScaledViewportSize, TempCanvasScale);
							LexWidget->SetWidth(ScaledViewportSize.X);
							LexWidget->SetHeight(ScaledViewportSize.Y);
						}
						else
						{
							//default is constant pixel
							LexWidget->SetWidth(ViewportSize.X);
							LexWidget->SetHeight(ViewportSize.Y);
							TempCanvasScale = 1.0f;
						}
					}
					break;
				}
				this->CanvasScale = TempCanvasScale;

				LexWidget->MarkAllDirtyRecursive();
				this->MarkCanvasUpdate(true);
			}
		}
	}
}
#if WITH_EDITOR
void ULexCanvas::OnEditorTick(float DeltaTime)
{
	if (!GetWorld())
		return;
	if (GetWorld()->IsGameWorld())//When hit play there is still an editor world and DrawViewportArea is called, which could cause frame dropdown, so skip it when playing
		return;
	if (this->IsUnreachable())
		return;
	if (auto WidgetPresenter = this->GetAttachedRootSceneComponent())
	{
		if (WidgetPresenter->GetName().Contains(TEXT("SKEL_")) || WidgetPresenter->GetName().Contains(TEXT("TRASH_")))
			return;
	}

	if (this->IsRootCanvas() && !this->bForceRenderToTarget)
	{
		if (this->GetRenderMode() == ELexRenderMode::ScreenSpaceOverlay
			|| this->GetRenderMode() == ELexRenderMode::RenderTarget
			)
		{
			DrawViewportArea();
			if (ULexUISelection::GetInstance(this->GetWorld())->IsSelected(this->GetWidget()))
			{
				if (auto ViewportClient = ULexUIManagerWorldSubsystem::GetInstance(this->GetWorld())->GetEditorViewportClient())
				{
					if (!ViewportClient->IsOrtho())
					{
						DrawVirtualCamera();
					}
				}
			}
				
			if (!GetWorld()->IsGameWorld())
			{
				if (this->GetRenderMode() == ELexRenderMode::ScreenSpaceOverlay)
				{
					TOptional<FIntPoint> NewViewportSize;
#if WITH_EDITOR
					if (bFixedSizeInEditMode)//Edit mode
					{
						NewViewportSize = SizeInEditMode;
					}
					else
#endif
					{
						if (auto ViewportClient = ULexUIManagerWorldSubsystem::GetInstance(this->GetWorld())->GetEditorViewportClient())
						{
							auto Viewport = ViewportClient->Viewport;
							if (Viewport == nullptr)
							{
								Viewport = GEditor->GetActiveViewport();
							}
							if (Viewport != nullptr)
							{
								NewViewportSize = Viewport->GetSizeXY();
							}
						}
					}

					if (NewViewportSize.IsSet())
					{
						if (NewViewportSize.GetValue() != ViewportSize)
						{
							ViewportSize = NewViewportSize.GetValue();
							OnViewportParameterChanged();
						}
					}
				}
				if (this->GetRenderMode() == ELexRenderMode::RenderTarget && IsValid(this->RenderTarget))
				{
					auto prevSize = ViewportSize;
					ViewportSize.X = this->RenderTarget->SizeX;
					ViewportSize.Y = this->RenderTarget->SizeY;
					if (prevSize != ViewportSize)
					{
						OnViewportParameterChanged();
					}
				}
			}
			else
			{
				auto newViewportSize = this->GetViewportSize();
				if (newViewportSize != ViewportSize)
				{
					ViewportSize = newViewportSize;
					OnViewportParameterChanged();
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

void ULexCanvas::DrawViewportArea()
{
	auto LexWidget = GetWidget();
	auto RectExtends = FVector(0.1f, LexWidget->GetWidth(), LexWidget->GetHeight()) * 0.5f;
	auto RectDrawColor = FColor(128, 128, 128, 128);//gray means normal object
	auto WorldTransform = LexWidget->GetWorldTransform();

	ULexUIManagerWorldSubsystem::DrawDebugBox(GetWorld()
		, FVector::Zero(), WorldTransform.ToMatrixWithScale()
		, RectExtends, RectDrawColor, this, FString::Printf(TEXT("%s.LexCanvas.ViewportArea"), *this->GetWidget()->GetDisplayName())
		, false);
}

void ULexCanvas::DrawVirtualCamera()
{
	auto ViewLocation = this->GetViewLocation();
	auto ViewRotationMatrix = FInverseRotationMatrix(this->GetViewRotator()) * FMatrix(
		FPlane(0, 0, 1, 0),
		FPlane(1, 0, 0, 0),
		FPlane(0, 1, 0, 0),
		FPlane(0, 0, 0, 1));
	auto ProjectionMatrix = this->GetProjectionMatrix();
	auto ViewProjectionMatrix = FTranslationMatrix(-ViewLocation) * ViewRotationMatrix * ProjectionMatrix;

	FVector leftBottom, rightBottom, leftTop, rightTop;
	FVector leftBottomEnd, rightBottomEnd, leftTopEnd, rightTopEnd;
	auto lineColor = FColor::Green;
	TArray<FVector3f> LinePoints;
	//draw view frustum
	DeprojectViewPointToWorld(ViewProjectionMatrix, FVector2D(0, 0), leftBottom, leftBottomEnd);
	new(LinePoints)FVector3f(leftBottom);
	new(LinePoints)FVector3f(leftBottomEnd);
	DeprojectViewPointToWorld(ViewProjectionMatrix, FVector2D(1, 0), rightBottom, rightBottomEnd);
	new(LinePoints)FVector3f(rightBottom);
	new(LinePoints)FVector3f(rightBottomEnd);
	DeprojectViewPointToWorld(ViewProjectionMatrix, FVector2D(0, 1), leftTop, leftTopEnd);
	new(LinePoints)FVector3f(leftTop);
	new(LinePoints)FVector3f(leftTopEnd);
	DeprojectViewPointToWorld(ViewProjectionMatrix, FVector2D(1, 1), rightTop, rightTopEnd);
	new(LinePoints)FVector3f(rightTop);
	new(LinePoints)FVector3f(rightTopEnd);
	//draw near clip plane
	new(LinePoints)FVector3f(leftBottom);
	new(LinePoints)FVector3f(rightBottom);
	
	new(LinePoints)FVector3f(leftBottom);
	new(LinePoints)FVector3f(leftTop);
	
	new(LinePoints)FVector3f(rightTop);
	new(LinePoints)FVector3f(rightBottom);
	
	new(LinePoints)FVector3f(rightTop);
	new(LinePoints)FVector3f(leftTop);
	//draw far clip plane
	new(LinePoints)FVector3f(leftBottomEnd);
	new(LinePoints)FVector3f(rightBottomEnd);

	new(LinePoints)FVector3f(leftBottomEnd);
	new(LinePoints)FVector3f(leftTopEnd);

	new(LinePoints)FVector3f(rightTopEnd);
	new(LinePoints)FVector3f(rightBottomEnd);

	new(LinePoints)FVector3f(rightTopEnd);
	new(LinePoints)FVector3f(leftTopEnd);

	ULexUIManagerWorldSubsystem::DrawDebugLine(GetWorld(), FMatrix::Identity
		, LinePoints, lineColor, this, FString::Printf(TEXT("%s.LexCanvas.VirtualCamera"), *this->GetWidget()->GetDisplayName())
		, false);

	// if (LexWidget.IsValid())
	// {
	// 	DrawDebugCamera(this->GetWorld(), this->GetViewLocation(), this->GetViewRotator(), FieldOfView, this->GetLexWidget()->GetComponentScale().X * 3.0f, FColor::Green);
	// }
}
#endif

void ULexCanvas::SetProjectionType(TEnumAsByte<ECameraProjectionMode::Type> Value)
{
	if (ProjectionType != Value)
	{
		ProjectionType = ProjectionType = Value;
		OnViewportParameterChanged();
	}
}
void ULexCanvas::SetFieldOfView(float Value)
{
	if (FieldOfView != Value)
	{
		FieldOfView = FieldOfView = Value;
		OnViewportParameterChanged();
	}
}
void ULexCanvas::SetNearClipPlane(float Value)
{
	if (NearClipPlane != Value)
	{
		NearClipPlane = Value;
		OnViewportParameterChanged();
	}
}
void ULexCanvas::SetFarClipPlane(float Value)
{
	if (FarClipPlane != Value)
	{
		FarClipPlane = Value;
		OnViewportParameterChanged();
	}
}

void ULexCanvas::SetScaleMode(ELexCanvasScaleMode Value)
{
	if (ScaleMode != Value)
	{
		ScaleMode = Value;
		OnViewportParameterChanged();
	}
}
void ULexCanvas::SetReferenceResolution(FVector2D Value)
{
	if (ReferenceResolution != Value)
	{
		ReferenceResolution = Value;
		OnViewportParameterChanged();
	}
}
void ULexCanvas::SetMatchFromWidthToHeight(float Value)
{
	if (MatchFromWidthToHeight != Value)
	{
		MatchFromWidthToHeight = Value;
		OnViewportParameterChanged();
	}
}
void ULexCanvas::SetScreenMatchMode(ELexCanvasScreenMatchMode Value)
{
	if (ScreenMatchMode != Value)
	{
		ScreenMatchMode = Value;
		OnViewportParameterChanged();
	}
}
void ULexCanvas::SetCustomScale(ULexCanvasCustomScale* Value)
{
	if (CustomScale != Value)
	{
		CustomScale = Value;
		CustomScale->Init(this);//need to initialize when first set
		if (ScaleMode == ELexCanvasScaleMode::Custom)
		{
			OnViewportParameterChanged();
		}
	}
}

bool ULexCanvas::ConvertPositionFromViewportToCanvas(const FVector2D& InPosition, FVector2D& Result)const
{
	if (RootCanvas != this)return false;
	switch (ScaleMode)
	{
	case ELexCanvasScaleMode::ConstantPixelSize:
		Result = FVector2D(InPosition.X, ViewportSize.Y - InPosition.Y);
		return true;
	case ELexCanvasScaleMode::ScaleWithScreenSize:
		Result = FVector2D(InPosition.X, ViewportSize.Y - InPosition.Y) / this->CanvasScale;
		return true;
	case ELexCanvasScaleMode::Custom:
		if (IsValid(CustomScale))
		{
			return CustomScale->ConvertPositionFromViewportToCanvas(InPosition, Result);
		}
	}
	return false;
}
bool ULexCanvas::ConvertPositionFromCanvasToViewport(const FVector2D& InPosition, FVector2D& Result)const
{
	if (RootCanvas != this)return false;
	switch (ScaleMode)
	{
	case ELexCanvasScaleMode::ConstantPixelSize:
		Result = FVector2D(InPosition.X, ViewportSize.Y - InPosition.Y);
		return true;
	case ELexCanvasScaleMode::ScaleWithScreenSize:
		Result = FVector2D(InPosition.X * this->CanvasScale, ViewportSize.Y - InPosition.Y * this->CanvasScale);
		return true;
	case ELexCanvasScaleMode::Custom:
		if (IsValid(CustomScale))
		{
			return CustomScale->ConvertPositionFromCanvasToViewport(InPosition, Result);
		}
	}
	return false;
}
bool ULexCanvas::Project3DToScreen(const FVector& Position3D, FVector2D& OutPosition2D)const
{
	if (RootCanvas != this)return false;
	auto viewProjectionMatrix = this->GetViewProjectionMatrix();
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
		OutPosition2D *= this->GetViewportSize();
		OutPosition2D /= this->CanvasScale;

		return true;
	}
	return false;
}

bool ULexCanvas::ProjectWorldToScreenWithPlayerCamera(APlayerController* Player, UCameraComponent* PlayerCamera, const FVector& InPosition, FVector2D& OutPosition2D)
{
	if (Player != nullptr && PlayerCamera != nullptr)
	{
		ULocalPlayer* const LP = Player ? Player->GetLocalPlayer() : nullptr;
		if (LP && LP->ViewportClient)
		{
			FSceneViewProjectionData ProjectionData;
			LP->GetProjectionData(LP->ViewportClient->Viewport, /*out*/ ProjectionData);

			auto ViewLocation = PlayerCamera->GetComponentLocation();
			auto ViewRotationMatrix = FInverseRotationMatrix(PlayerCamera->GetComponentRotation()) * FMatrix(
				FPlane(0, 0, 1, 0),
				FPlane(1, 0, 0, 0),
				FPlane(0, 1, 0, 0),
				FPlane(0, 0, 0, 1));

			auto ViewRect = ProjectionData.GetConstrainedViewRect();
			auto ViewportSize = ViewRect.Size();
#if 0//not sure what is wrong but this calculation can't get correct result
			auto FovInRadians = PlayerCamera->FieldOfView * UE_PI / 360.0f;//we need half fov so 360 instead of 180
			FMatrix ProjectionMatrix;
			ULexCanvas::BuildProjectionMatrix(ViewportSize, PlayerCamera->ProjectionMode
				, FovInRadians, 1000000, 0.01f, ProjectionMatrix);
			auto ViewProjectionMatrix = FTranslationMatrix(-ViewLocation) * ViewRotationMatrix * ProjectionMatrix;
#else
			ProjectionData.ViewOrigin = ViewLocation;
			ProjectionData.ViewRotationMatrix = ViewRotationMatrix;
			auto ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
#endif

			auto ScreenPos = ViewProjectionMatrix.TransformFVector4(FVector4(InPosition, 1.0f));
			if (ScreenPos.W > 0.0f)
			{
				// the result of this will be x and y coords in -1..1 projection space
				const float RHW = 1.0f / ScreenPos.W;
				FPlane PosInScreenSpace = FPlane(ScreenPos.X * RHW, ScreenPos.Y * RHW, ScreenPos.Z * RHW, ScreenPos.W);

				// Move from projection space to normalized 0..1 UI space
				const float NormalizedX = (PosInScreenSpace.X * 0.5f) + 0.5f;
				const float NormalizedY = 1 - (PosInScreenSpace.Y * 0.5f) - 0.5f;

				FVector2D RayStartViewRectSpace(
					NormalizedX * (float)ViewportSize.X,
					NormalizedY * (float)ViewportSize.Y
				);
				
				OutPosition2D = FVector2D(RayStartViewRectSpace.X, RayStartViewRectSpace.Y) + FVector2D(static_cast<float>(ViewRect.Min.X), static_cast<float>(ViewRect.Min.Y));
				return true;
			}
		}
	}
	return false;
}

bool ULexCanvas::BuildViewProjectionMatrixForPlayerCamera(APlayerController* Player, UCameraComponent* PlayerCamera, FMatrix& OutViewProjectionMatrix)
{
	if (Player != nullptr && PlayerCamera != nullptr)
	{
		ULocalPlayer* const LP = Player ? Player->GetLocalPlayer() : nullptr;
		if (LP && LP->ViewportClient)
		{
			FSceneViewProjectionData ProjectionData;
			LP->GetProjectionData(LP->ViewportClient->Viewport, /*out*/ ProjectionData);

			auto ViewLocation = PlayerCamera->GetComponentLocation();
			auto ViewRotationMatrix = FInverseRotationMatrix(PlayerCamera->GetComponentRotation()) * FMatrix(
				FPlane(0, 0, 1, 0),
				FPlane(1, 0, 0, 0),
				FPlane(0, 1, 0, 0),
				FPlane(0, 0, 0, 1));

			auto ViewRect = ProjectionData.GetConstrainedViewRect();
			auto ViewportSize = ViewRect.Size();
#if 0//not sure what is wrong but this calculation can't get correct result
			auto FovInRadians = PlayerCamera->FieldOfView * UE_PI / 360.0f;//we need half fov so 360 instead of 180
			FMatrix ProjectionMatrix;
			ULexCanvas::BuildProjectionMatrix(ViewportSize, PlayerCamera->ProjectionMode
				, FovInRadians, 1000000, 0.01f, ProjectionMatrix);
			OutViewProjectionMatrix = FTranslationMatrix(-ViewLocation) * ViewRotationMatrix * ProjectionMatrix;
#else
			ProjectionData.ViewOrigin = ViewLocation;
			ProjectionData.ViewRotationMatrix = ViewRotationMatrix;
			OutViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
#endif
		}
	}
	return false;
}

bool ULexCanvas::ProjectWorldToScreenWithViewProjectionMatrix(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewportSize, const FVector& InPosition, FVector2D& OutPosition2D)
{
	auto ScreenPos = InViewProjectionMatrix.TransformFVector4(FVector4(InPosition, 1.0f));
	if (ScreenPos.W > 0.0f)
	{
		// the result of this will be x and y coords in -1..1 projection space
		const float RHW = 1.0f / ScreenPos.W;
		FPlane PosInScreenSpace = FPlane(ScreenPos.X * RHW, ScreenPos.Y * RHW, ScreenPos.Z * RHW, ScreenPos.W);

		// Move from projection space to normalized 0..1 UI space
		const float NormalizedX = (PosInScreenSpace.X / 2.f) + 0.5f;
		const float NormalizedY = 1.f - (PosInScreenSpace.Y / 2.f) - 0.5f;

		OutPosition2D.X = (NormalizedX * (float)InViewportSize.X);
		OutPosition2D.Y = (NormalizedY * (float)InViewportSize.Y);

		OutPosition2D = FVector2D(OutPosition2D.X, InViewportSize.Y - OutPosition2D.Y);
		return true;
	}
	return false;
}

#pragma endregion


