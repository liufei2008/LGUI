// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "UIItemComponentVisualizer.h"
#include "Core/ActorComponent/UIItem.h"
#include "LGUIComponentVisualizerModule.h"
#include "LGUI.h"
#include "LGUIEditorUtils.h"
#include "Utils/LGUIUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Core/LGUISettings.h"
#include "Layout/UIPanelLayoutBase.h"
#include "PrefabSystem/LGUIPrefabManager.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_DISABLE_OPTIMIZATION
#endif

#define LOCTEXT_NAMESPACE "UIItemComponentVisualizer"

FUIItemComponentVisualizer::FUIItemComponentVisualizer()
	: FComponentVisualizer()
{

}
void FUIItemComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	auto UIItem = Cast<UUIItem>(Component);
	if (!UIItem)return;
	TargetComp = (UUIItem*)UIItem;
	if (TargetComp->GetWorld() != View->Family->Scene->GetWorld())return;

	auto Center = TargetComp->GetLocalSpaceCenter();
	auto Left = TargetComp->GetLocalSpaceLeft();
	auto Right = TargetComp->GetLocalSpaceRight();
	auto Bottom = TargetComp->GetLocalSpaceBottom();
	auto Top = TargetComp->GetLocalSpaceTop();
	auto LeftPoint = TargetComp->GetComponentTransform().TransformPosition(FVector(0, Left, Center.Y));
	auto RightPoint = TargetComp->GetComponentTransform().TransformPosition(FVector(0, Right, Center.Y));
	auto BottomPoint = TargetComp->GetComponentTransform().TransformPosition(FVector(0, Center.X, Bottom));
	auto TopPoint = TargetComp->GetComponentTransform().TransformPosition(FVector(0, Center.X, Top));

	auto LeftBottomPoint = TargetComp->GetComponentTransform().TransformPosition(FVector(0, Left, Bottom));
	auto LeftTopPoint = TargetComp->GetComponentTransform().TransformPosition(FVector(0, Left, Top));
	auto RightBottomPoint = TargetComp->GetComponentTransform().TransformPosition(FVector(0, Right, Bottom));
	auto RightTopPoint = TargetComp->GetComponentTransform().TransformPosition(FVector(0, Right, Top));

	auto Area = TargetComp->GetWidth() * TargetComp->GetHeight();
	Area = FMath::Sqrt(Area);
	auto DrawHitProxy = [=, this](FVector Position, EUIItemVisualizerSelectorType Type, UTexture2D* IconTexture, float AreaMultiply = 1.0f) {
		float DistScale = View->WorldToScreen(Position).W * (4.0f / View->UnscaledViewRect.Width() / View->ViewMatrices.GetProjectionMatrix().M[0][0]);
		float Scale = DistScale * 0.25f;
		float AreaScale = 100 - DistScale / (Area * 0.001f) * AreaMultiply;
		AreaScale = FMath::Clamp(AreaScale, 0.0f, 1.0f);
		if (AreaScale > 0.01f)
		{
			auto Color = FLinearColor(1, 1, 1, AreaScale);
			PDI->SetHitProxy(new HUIItemAnchorVisProxy(TargetComp.Get(), Type));
			PDI->DrawSprite(Position, IconTexture->GetSizeX() * Scale, IconTexture->GetSizeY() * Scale, IconTexture->GetResource(), Color, SDPG_Foreground, 0.0f, 0.0f, 0.0f, 0.0f, SE_BLEND_AlphaBlend);
			PDI->SetHitProxy(NULL);
		}
	};

	static FString LGUIBasePath = IPluginManager::Get().FindPlugin(TEXT("LGUI"))->GetBaseDir();

	//draw panel layout button
	bool bHaveDrawPanelLayout = false;
	if (auto Parent = TargetComp->GetParentUIItem())
	{
		if (auto PanelLayout = Parent->GetOwner()->FindComponentByClass<UUIPanelLayoutBase>())
		{
			if (auto Slot = PanelLayout->GetChildSlot(TargetComp.Get()))
			{
				if (!Slot->GetIgnoreLayout())
				{
					const float AreaScale = 3.0f;
					static auto PanelLayoutReorderButton_Left_Texture = LGUIEditorUtils::LoadTexture(LGUIBasePath + TEXT("/Resources/Icons/PanelLayoutReorderButton_Left.png"));
					static auto PanelLayoutReorderButton_Right_Texture = LGUIEditorUtils::LoadTexture(LGUIBasePath + TEXT("/Resources/Icons/PanelLayoutReorderButton_Right.png"));
					static auto PanelLayoutReorderButton_Top_Texture = LGUIEditorUtils::LoadTexture(LGUIBasePath + TEXT("/Resources/Icons/PanelLayoutReorderButton_Top.png"));
					static auto PanelLayoutReorderButton_Bottom_Texture = LGUIEditorUtils::LoadTexture(LGUIBasePath + TEXT("/Resources/Icons/PanelLayoutReorderButton_Bottom.png"));
					if (PanelLayout->CanMoveChildToCell(TargetComp.Get(), UUIPanelLayoutBase::EMoveChildDirectionType::Left))
					{
						DrawHitProxy(LeftPoint, EUIItemVisualizerSelectorType::PanelLayout_Left, PanelLayoutReorderButton_Left_Texture, AreaScale);
					}
					if (PanelLayout->CanMoveChildToCell(TargetComp.Get(), UUIPanelLayoutBase::EMoveChildDirectionType::Right))
					{
						DrawHitProxy(RightPoint, EUIItemVisualizerSelectorType::PanelLayout_Right, PanelLayoutReorderButton_Right_Texture, AreaScale);
					}
					if (PanelLayout->CanMoveChildToCell(TargetComp.Get(), UUIPanelLayoutBase::EMoveChildDirectionType::Top))
					{
						DrawHitProxy(TopPoint, EUIItemVisualizerSelectorType::PanelLayout_Top, PanelLayoutReorderButton_Top_Texture, AreaScale);
					}
					if (PanelLayout->CanMoveChildToCell(TargetComp.Get(), UUIPanelLayoutBase::EMoveChildDirectionType::Bottom))
					{
						DrawHitProxy(BottomPoint, EUIItemVisualizerSelectorType::PanelLayout_Bottom, PanelLayoutReorderButton_Bottom_Texture, AreaScale);
					}
					bHaveDrawPanelLayout = true;
				}
			}
		}
	}

	//draw anchor tool
	if (GetDefault<ULGUIEditorSettings>()->bShowAnchorTool)
	{
		if (!bHaveDrawPanelLayout)//anchor is controlled by panel layout, so no need to draw it
		{
			static auto AnchorVisTexture = LGUIEditorUtils::LoadTexture(LGUIBasePath + TEXT("/Resources/Icons/AnchorVisSelector.png"));
			DrawHitProxy(LeftPoint, EUIItemVisualizerSelectorType::Left, AnchorVisTexture);
			DrawHitProxy(RightPoint, EUIItemVisualizerSelectorType::Right, AnchorVisTexture);
			DrawHitProxy(TopPoint, EUIItemVisualizerSelectorType::Top, AnchorVisTexture);
			DrawHitProxy(BottomPoint, EUIItemVisualizerSelectorType::Bottom, AnchorVisTexture);

			DrawHitProxy(LeftBottomPoint, EUIItemVisualizerSelectorType::LeftBottom, AnchorVisTexture);
			DrawHitProxy(RightBottomPoint, EUIItemVisualizerSelectorType::RightBottom, AnchorVisTexture);
			DrawHitProxy(LeftTopPoint, EUIItemVisualizerSelectorType::LeftTop, AnchorVisTexture);
			DrawHitProxy(RightTopPoint, EUIItemVisualizerSelectorType::RightTop, AnchorVisTexture);
		}

		static auto PivotVisTexture = LGUIEditorUtils::LoadTexture(LGUIBasePath + TEXT("/Resources/Icons/PivotVisSelector.png"));
		DrawHitProxy(TargetComp->GetComponentLocation(), EUIItemVisualizerSelectorType::Pivot, PivotVisTexture);
	}
}
bool FUIItemComponentVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	if (!TargetComp.IsValid())return false;

	if (VisProxy->IsA(HUIItemAnchorVisProxy::StaticGetType()))
	{
		const HUIItemAnchorVisProxy* Proxy = (HUIItemAnchorVisProxy*)VisProxy;
		switch (Proxy->Type)
		{
		case EUIItemVisualizerSelectorType::Left:
		case EUIItemVisualizerSelectorType::Right:
		case EUIItemVisualizerSelectorType::Top:
		case EUIItemVisualizerSelectorType::Bottom:
		case EUIItemVisualizerSelectorType::LeftTop:
		case EUIItemVisualizerSelectorType::RightTop:
		case EUIItemVisualizerSelectorType::LeftBottom:
		case EUIItemVisualizerSelectorType::RightBottom:
		case EUIItemVisualizerSelectorType::Pivot:
		{
			SelectorType = Proxy->Type;
		}
		break;
		case EUIItemVisualizerSelectorType::PanelLayout_Left:
		case EUIItemVisualizerSelectorType::PanelLayout_Right:
		case EUIItemVisualizerSelectorType::PanelLayout_Top:
		case EUIItemVisualizerSelectorType::PanelLayout_Bottom:
		{
			if (auto Parent = TargetComp->GetParentUIItem())
			{
				if (auto PanelLayout = Parent->GetOwner()->FindComponentByClass<UUIPanelLayoutBase>())
				{
					GEditor->BeginTransaction(LOCTEXT("ChangeLayoutOrder", "Change Layout Order"));
					TargetComp->Modify();
					PanelLayout->GetChildSlot(TargetComp.Get())->Modify();
					auto Direction = (int)Proxy->Type - (int)EUIItemVisualizerSelectorType::PanelLayout_Left;
					PanelLayout->MoveChildToCell(TargetComp.Get(), (UUIPanelLayoutBase::EMoveChildDirectionType)Direction);
					GEditor->EndTransaction();
					ULGUIPrefabManagerObject::AddOneShotTickFunction([] {
						GEditor->RedrawAllViewports();
						}, 1);
				}
			}
		}
		break;
		}
		return true;
	}
	return false;
}
bool FUIItemComponentVisualizer::HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	return false;
}
bool FUIItemComponentVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltalRotate, FVector& DeltaScale)
{
	if (!GetDefault<ULGUIEditorSettings>()->bShowAnchorTool)return false;
	if (!TargetComp.IsValid())return false;
	if (DeltaTranslate.IsZero())return false;

	TargetComp->Modify();
	bool bAnchorChanged = false;
	auto LocalSpaceDeltaTranslate = TargetComp->GetComponentTransform().InverseTransformVector(DeltaTranslate);
	bool bLeft = false, bRight = false, bBottom = false, bTop = false;
	switch (SelectorType)
	{
	case EUIItemVisualizerSelectorType::Left:
		bLeft = true;
		break;
	case EUIItemVisualizerSelectorType::Right:
		bRight = true;
		break;
	case EUIItemVisualizerSelectorType::Bottom:
		bBottom = true;
		break;
	case EUIItemVisualizerSelectorType::Top:
		bTop = true;
		break;
	case EUIItemVisualizerSelectorType::LeftTop:
		bLeft = true; bTop = true;
		break;
	case EUIItemVisualizerSelectorType::LeftBottom:
		bLeft = true; bBottom = true;
		break;
	case EUIItemVisualizerSelectorType::RightTop:
		bRight = true; bTop = true;
		break;
	case EUIItemVisualizerSelectorType::RightBottom:
		bRight = true; bBottom = true;
		break;
	case EUIItemVisualizerSelectorType::Pivot:
	{
		if (LocalSpaceDeltaTranslate.Y != 0 || LocalSpaceDeltaTranslate.Z != 0)
		{
			auto DeltaTranslatePivot = FVector2D(LocalSpaceDeltaTranslate.Y / TargetComp->GetWidth(), LocalSpaceDeltaTranslate.Z / TargetComp->GetHeight());
			FMargin PrevAnchorAsMargin(TargetComp->GetAnchorLeft(), TargetComp->GetAnchorTop(), TargetComp->GetAnchorRight(), TargetComp->GetAnchorBottom());
			TargetComp->SetPivot(TargetComp->GetPivot() + DeltaTranslatePivot);
			TargetComp->SetAnchorLeft(PrevAnchorAsMargin.Left);
			TargetComp->SetAnchorRight(PrevAnchorAsMargin.Right);
			TargetComp->SetAnchorBottom(PrevAnchorAsMargin.Bottom);
			TargetComp->SetAnchorTop(PrevAnchorAsMargin.Top);
			bAnchorChanged = true;
		}
	}
	break;
	}
	if (bLeft)
	{
		if (LocalSpaceDeltaTranslate.Y != 0)
		{
			TargetComp->SetAnchorLeft(TargetComp->GetAnchorLeft() + LocalSpaceDeltaTranslate.Y);
			if (IsAltDown(Viewport))
			{
				TargetComp->SetAnchorRight(TargetComp->GetAnchorRight() + LocalSpaceDeltaTranslate.Y);
			}
			bAnchorChanged = true;
		}
	}
	if (bRight)
	{
		if (LocalSpaceDeltaTranslate.Y != 0)
		{
			TargetComp->SetAnchorRight(TargetComp->GetAnchorRight() - LocalSpaceDeltaTranslate.Y);
			if (IsAltDown(Viewport))
			{
				TargetComp->SetAnchorLeft(TargetComp->GetAnchorLeft() - LocalSpaceDeltaTranslate.Y);
			}
			bAnchorChanged = true;
		}
	}
	if (bBottom)
	{
		if (LocalSpaceDeltaTranslate.Z != 0)
		{
			TargetComp->SetAnchorBottom(TargetComp->GetAnchorBottom() + LocalSpaceDeltaTranslate.Z);
			if (IsAltDown(Viewport))
			{
				TargetComp->SetAnchorTop(TargetComp->GetAnchorTop() + LocalSpaceDeltaTranslate.Z);
			}
			bAnchorChanged = true;
		}
	}
	if (bTop)
	{
		if (LocalSpaceDeltaTranslate.Z != 0)
		{
			TargetComp->SetAnchorTop(TargetComp->GetAnchorTop() - LocalSpaceDeltaTranslate.Z);
			if (IsAltDown(Viewport))
			{
				TargetComp->SetAnchorBottom(TargetComp->GetAnchorBottom() - LocalSpaceDeltaTranslate.Z);
			}
			bAnchorChanged = true;
		}
	}
	if (bAnchorChanged)
	{
		LGUIUtils::NotifyPropertyChanged(TargetComp.Get(), UUIItem::GetAnchorDataPropertyName());
	}
	return true;
}
bool FUIItemComponentVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	if (!TargetComp.IsValid())return false;

	auto Center = TargetComp->GetLocalSpaceCenter();
	auto Left = TargetComp->GetLocalSpaceLeft();
	auto Right = TargetComp->GetLocalSpaceRight();
	auto Bottom = TargetComp->GetLocalSpaceBottom();
	auto Top = TargetComp->GetLocalSpaceTop();
	FVector LocalPosition;
	switch (SelectorType)
	{
	case EUIItemVisualizerSelectorType::Left:
	{
		LocalPosition = FVector(0, TargetComp->GetLocalSpaceLeft(), Center.Y);
	}
	break;
	case EUIItemVisualizerSelectorType::Right:
	{
		LocalPosition = FVector(0, TargetComp->GetLocalSpaceRight(), Center.Y);
	}
	break;
	case EUIItemVisualizerSelectorType::Top:
	{
		LocalPosition = FVector(0, Center.X, TargetComp->GetLocalSpaceTop());
	}
	break;
	case EUIItemVisualizerSelectorType::Bottom:
	{
		LocalPosition = FVector(0, Center.X, TargetComp->GetLocalSpaceBottom());
	}
	break;
	case EUIItemVisualizerSelectorType::LeftBottom:
	{
		LocalPosition = FVector(0, Left, Bottom);
	}
	break;
	case EUIItemVisualizerSelectorType::RightBottom:
	{
		LocalPosition = FVector(0, Right, Bottom);
	}
	break;
	case EUIItemVisualizerSelectorType::LeftTop:
	{
		LocalPosition = FVector(0, Left, Top);
	}
	break;
	case EUIItemVisualizerSelectorType::RightTop:
	{
		LocalPosition = FVector(0, Right, Top);
	}
	break;
	case EUIItemVisualizerSelectorType::Pivot:
	{
		LocalPosition = FVector::ZeroVector;
	}
	break;
	}
	OutLocation = TargetComp->GetComponentTransform().TransformPosition(LocalPosition);

	return true;
}
bool FUIItemComponentVisualizer::GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const
{
	if (!TargetComp.IsValid())return false;
	OutMatrix = FRotationMatrix(TargetComp->GetComponentTransform().Rotator());
	return true;
}

IMPLEMENT_HIT_PROXY(HUIItemAnchorVisProxy, HComponentVisProxy);
HUIItemAnchorVisProxy::HUIItemAnchorVisProxy(const UUIItem* InComponent, EUIItemVisualizerSelectorType InType)
	: HComponentVisProxy(InComponent, HPP_Foreground)
{
	Type = InType;
}

#undef LOCTEXT_NAMESPACE

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_ENABLE_OPTIMIZATION
#endif
