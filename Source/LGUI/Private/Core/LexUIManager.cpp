// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LexUIManager.h"

#include "LGUI.h"
#include "Utils/LexUIUtils.h"
#include "Core/Components/LexWidget.h"
#include "Core/Components/LexCanvas.h"
#include "Event/LexBaseRaycaster.h"
#include "Engine/World.h"
#include "Interaction/UISelectable.h"
#include "Core/LexUISettings.h"
#include "Core/Components/LexVisual.h"
#include "Engine/Engine.h"
#include "Core/LexUIRender/LexUIRenderer.h"
#include "Core/ILexUICultureChangedInterface.h"
#include "Core/LexUIBehaviour.h"
#include "Core/Components/LexLayout.h"
#include "Core/LexUIMesh/LexUIGizmoMesh.h"
#include "Event/LexEventSystem.h"
#include "PrefabSystem/LexUIPrefabHelperObject.h"
#include "PrefabSystem/LexUIPrefabPresenterComponent.h"
#include "HAL/PlatformTime.h"
#if WITH_EDITOR
#include "Editor.h"
#include "EditorViewportClient.h"
#include "Core/LexUISpriteData.h"
#endif

#define LOCTEXT_NAMESPACE "LexUIManager"



ULexUIManagerObject* ULexUIManagerObject::Instance = nullptr;
#if WITH_EDITOR
bool ULexUIManagerObject::bIsBlueprintCompiling = false;
#endif
ULexUIManagerObject::ULexUIManagerObject()
{

}
void ULexUIManagerObject::BeginDestroy()
{
#if WITH_EDITORONLY_DATA
	if (OnAssetReimportDelegateHandle.IsValid())
	{
		if (GEditor)
		{
			if (auto ImportSubsystem = GEditor->GetEditorSubsystem<UImportSubsystem>())
			{
				ImportSubsystem->OnAssetReimport.Remove(OnAssetReimportDelegateHandle);
			}
		}
	}
	if (OnMapOpenedDelegateHandle.IsValid())
	{
		FEditorDelegates::OnMapOpened.Remove(OnMapOpenedDelegateHandle);
	}
	if (OnPackageReloadedDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::OnPackageReloaded.Remove(OnPackageReloadedDelegateHandle);
	}
	if (OnBlueprintPreCompileDelegateHandle.IsValid())
	{
		if (GEditor)
		{
			GEditor->OnBlueprintPreCompile().Remove(OnBlueprintPreCompileDelegateHandle);
		}
	}
	if (OnBlueprintCompiledDelegateHandle.IsValid())
	{
		if (GEditor)
		{
			GEditor->OnBlueprintCompiled().Remove(OnBlueprintCompiledDelegateHandle);
		}
	}
#endif
	Instance = nullptr;
	Super::BeginDestroy();
}

void ULexUIManagerObject::Tick(float DeltaTime)
{
#if WITH_EDITOR
	if (EditorTick.IsBound())
	{
		EditorTick.Broadcast(DeltaTime);
	}
	if (OneShotFunctionsToExecuteInTick.Num() > 0)
	{
		for (int i = 0; i < OneShotFunctionsToExecuteInTick.Num(); i++)
		{
			auto& Item = OneShotFunctionsToExecuteInTick[i];
			if (Item.Key <= 0)
			{
				Item.Value();
				OneShotFunctionsToExecuteInTick.RemoveAt(i);
				i--;
			}
			else
			{
				Item.Key--;
			}
		}
	}
#endif
}
TStatId ULexUIManagerObject::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULGUIEditorManagerObject, STATGROUP_Tickables);
}

#if WITH_EDITOR

void ULexUIManagerObject::AddOneShotTickFunction(const TFunction<void()>& InFunction, int InDelayFrameCount)
{
	InitCheck();
	InDelayFrameCount = FMath::Max(0, InDelayFrameCount);
	TTuple<int, TFunction<void()>> Item;
	Item.Key = InDelayFrameCount;
	Item.Value = InFunction;
	Instance->OneShotFunctionsToExecuteInTick.Add(Item);
}

FLexUIEditorTickMulticastDelegate& ULexUIManagerObject::GetEditorTickDelegate()
{
	return EditorTick;
}

ULexUIManagerObject* ULexUIManagerObject::GetInstance(bool CreateIfNotValid)
{
	if (CreateIfNotValid)
	{
		InitCheck();
	}
	return Instance;
}
bool ULexUIManagerObject::InitCheck()
{
	if (Instance == nullptr)
	{
		UE_LOG(LGUI, Log, TEXT("[%s].%d No Instance of class %s, create it"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *ULexUIManagerObject::StaticClass()->GetName());
		Instance = NewObject<ULexUIManagerObject>();
		Instance->AddToRoot();
		//open map
		Instance->OnMapOpenedDelegateHandle = FEditorDelegates::OnMapOpened.AddUObject(Instance, &ULexUIManagerObject::OnMapOpened);
		Instance->OnPackageReloadedDelegateHandle = FCoreUObjectDelegates::OnPackageReloaded.AddUObject(Instance, &ULexUIManagerObject::OnPackageReloaded);
		if (GEditor)
		{
			//reimport asset
			Instance->OnAssetReimportDelegateHandle = GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetReimport.AddUObject(Instance, &ULexUIManagerObject::OnAssetReimport);
			//blueprint recompile
			Instance->OnBlueprintPreCompileDelegateHandle = GEditor->OnBlueprintPreCompile().AddUObject(Instance, &ULexUIManagerObject::OnBlueprintPreCompile);
			Instance->OnBlueprintCompiledDelegateHandle = GEditor->OnBlueprintCompiled().AddUObject(Instance, &ULexUIManagerObject::OnBlueprintCompiled);
		}
	}
	return true;
}

void ULexUIManagerObject::OnBlueprintPreCompile(UBlueprint* InBlueprint)
{
	bIsBlueprintCompiling = true;
}
void ULexUIManagerObject::OnBlueprintCompiled()
{
	ULexUIManagerObject::AddOneShotTickFunction([] {
		bIsBlueprintCompiling = false;
		ULexUIManagerWorldSubsystem::RefreshAllUI();
		});
}

void ULexUIManagerObject::OnAssetReimport(UObject* Asset)
{
	if (IsValid(Asset))
	{
		if (auto TextureAsset = Cast<UTexture2D>(Asset))
		{
			bool bNeedToRebuildUI = false;
			//find sprite data that reference this texture
			for (TObjectIterator<ULexUISpriteData> Itr; Itr; ++Itr)
			{
				ULexUISpriteData* SpriteData = *Itr;
				if (IsValid(SpriteData))
				{
					if (SpriteData->GetSpriteTexture() == TextureAsset)
					{
						SpriteData->ReloadTexture();
						SpriteData->MarkPackageDirty();
						bNeedToRebuildUI = true;
					}
				}
			}
			//Refresh ui
			if (bNeedToRebuildUI)
			{
				ULexUIManagerWorldSubsystem::RefreshAllUI();
			}
		}
		else if (Asset->IsA<ULexUIPrefab>())
		{
			for (TObjectIterator<ULexUIPrefabPresenterComponent> Itr; Itr; ++Itr)
			{
				if (Itr->GetWorld())
				{
					Itr->CheckPrefabVersion();
				}
			}
		}
	}
}

void ULexUIManagerObject::OnMapOpened(const FString& FileName, bool AsTemplate)
{

}

void ULexUIManagerObject::OnPackageReloaded(EPackageReloadPhase Phase, FPackageReloadedEvent* Event)
{
	if (Phase == EPackageReloadPhase::PostBatchPostGC && Event != nullptr && Event->GetNewPackage() != nullptr)
	{
		auto Asset = Event->GetNewPackage()->FindAssetInPackage();
	}
}


ULexUISelection* ULexUISelection::GetInstance(UWorld* InWorld)
{
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(InWorld))
	{
		return LexUIManager->GetSelection();
	}
	return nullptr;
}

void ULexUISelection::SelectWidget(ULexWidget* Widget)
{
	SelectedWidgetArray.Add(Widget);
	OnSelectionChanged.Broadcast();
}
void ULexUISelection::SelectComponent(ULexUIBehaviour* Component)
{
	SelectedComponentArray.Add(Component);
	OnSelectionChanged.Broadcast();
}

void ULexUISelection::ClearComponentSelection()
{
	SelectedComponentArray.Empty();
	OnSelectionChanged.Broadcast();
}

void ULexUISelection::SelectNone()
{
	SelectedWidgetArray.Empty();
	SelectedComponentArray.Empty();
	OnSelectionChanged.Broadcast();
}

bool ULexUISelection::IsSelected(ULexWidget* Widget)const
{
	return SelectedWidgetArray.Contains(Widget);
}

void ULexUIManagerWorldSubsystem::DrawFrameOnWidget(ULexWidget* Widget, bool ScreenOrWorld)
{
	if (GetSelection()->IsSelected(Widget))//select self
	{
		auto RectDrawColor = FColor(160, 160, 160, 255);//gray means normal object
		auto DrawWidget = [=](ULexWidget* InWidget, const FColor& Color)
		{
			auto WorldTransform = InWidget->GetWorldTransform();
			FVector RelativeOffset(0, 0, 0);
			RelativeOffset.Y = (0.5f - InWidget->GetPivot().X) * InWidget->GetWidth();
			RelativeOffset.Z = (0.5f - InWidget->GetPivot().Y) * InWidget->GetHeight();
			auto Extends = FVector2D(InWidget->GetWidth(), InWidget->GetHeight()) * 0.5f;
			ULexUIManagerWorldSubsystem::DrawDebugRect(InWidget->GetWorld()
				, RelativeOffset, WorldTransform.ToMatrixWithScale()
				, Extends, Color
				, InWidget, InWidget->GetDisplayName(), ScreenOrWorld);
		};
		//parent
		if (auto Parent = Widget->GetParent())
		{
			DrawWidget(Parent, RectDrawColor);
		}
		//child
		for (auto& Child : Widget->GetChildren())
		{
			if (IsValid(Child)
				&& Child->GetWidgetActiveInHierarchy())
			{
				DrawWidget(Child, RectDrawColor);
			}
		}
		//other object of same hierarchy is selected
		if (auto Parent = Widget->GetParent())
		{
			for (auto& SiblingWidget : Parent->GetChildren())
			{
				if (IsValid(SiblingWidget)
					&& SiblingWidget->GetWidgetActiveInHierarchy()
					&& SiblingWidget != Widget)
				{
					DrawWidget(SiblingWidget, RectDrawColor);
				}
			}
		}

		//self
		{
			RectDrawColor = FColor(0, 255, 0, 255);//green means selected object
			auto WorldTransform = Widget->GetWorldTransform();
			FVector RelativeOffset(0, 0, 0);
			RelativeOffset.Y = (0.5f - Widget->GetPivot().X) * Widget->GetWidth();
			RelativeOffset.Z = (0.5f - Widget->GetPivot().Y) * Widget->GetHeight();
			auto Extends = FVector2D(Widget->GetWidth(), Widget->GetHeight()) * 0.5f;
			ULexUIManagerWorldSubsystem::DrawDebugRect(Widget->GetWorld()
				, RelativeOffset, WorldTransform.ToMatrixWithScale()
				, Extends, RectDrawColor
				, Widget, Widget->GetDisplayName(), ScreenOrWorld);

			if (auto Visual = Cast<ULexVisual>(Widget->GetVisual()))
			{
				FVector Min, Max;
				Visual->GetGeometryBounds3DInLocalSpace(Min, Max);
				auto GeometryBoundsDrawColor = FColor(255, 255, 0, 255);//yellow for geometry bounds
				auto GeometryBoundsExtends = (Max - Min) * 0.5f;
				auto GeometryRelativeOffset = (Min + Max) * 0.5f;
				auto WidgetExtends3D = FVector(0, Extends.X, Extends.Y); 
				if (WidgetExtends3D != GeometryBoundsExtends || RelativeOffset != GeometryRelativeOffset)
				{
					ULexUIManagerWorldSubsystem::DrawDebugBox(Widget->GetWorld()
						, GeometryRelativeOffset, WorldTransform.ToMatrixWithScale()
						, GeometryBoundsExtends, GeometryBoundsDrawColor
						, Widget->GetVisual() , FString::Printf(TEXT("%s.Visual"), *Widget->GetDisplayName()), ScreenOrWorld);
				}
			}
		}
	}
}

void ULexUIManagerWorldSubsystem::DrawNavigationArrow(UWorld* InWorld, const TArray<FVector>& InControlPoints, const FVector& InArrowPointA, const FVector& InArrowPointB, FColor const& InColor, void* Object, const FString& DebugName, bool ScreenOrWorld)
{
	if (InControlPoints.Num() != 4)return;
	TArray<FVector3f> ResultPoints;
	TArray<FLexUIMeshVertex> VertexArray;
	TArray<FLexUIMeshIndex> IndexArray;
	const int Segment = FMath::Min(40, FMath::CeilToInt(FVector::Distance(InControlPoints[0], InControlPoints[3]) * 0.5f));

	auto CalculateCubicBezierPoint = [](float t, FVector p0, FVector p1, FVector p2, FVector p3)
	{
		float u = 1 - t;
		float tt = t * t;
		float uu = u * u;
		float uuu = uu * u;
		float ttt = tt * t;

		FVector p = uuu * p0;
		p += 3 * uu * t * p1;
		p += 3 * u * tt * p2;
		p += ttt * p3;

		return p;
	};

	IndexArray.Add(InControlPoints.Num());
	new(VertexArray) FLexUIMeshVertex(FVector3f(InControlPoints[0]), InColor);
	for (int i = 1; i <= Segment; i++)
	{
		float t = i / (float)Segment;
		auto InterPoint = CalculateCubicBezierPoint(t, InControlPoints[0], InControlPoints[1], InControlPoints[2], InControlPoints[3]);
		IndexArray.Add(InControlPoints.Num());
		new(VertexArray) FLexUIMeshVertex(FVector3f(InterPoint), InColor);
	}
	
	auto ViewExtension = ULexUIManagerWorldSubsystem::GetViewExtension(InWorld, true);
	if (ViewExtension.IsValid())
	{
		//arrow
		IndexArray.Add(InControlPoints.Num());
		new(VertexArray) FLexUIMeshVertex(FVector3f(InControlPoints[3]), InColor);
		IndexArray.Add(InControlPoints.Num());
		new(VertexArray) FLexUIMeshVertex(FVector3f(InArrowPointA), InColor);
		IndexArray.Add(InControlPoints.Num());
		new(VertexArray) FLexUIMeshVertex(FVector3f(InControlPoints[3]), InColor);
		IndexArray.Add(InControlPoints.Num());
		new(VertexArray) FLexUIMeshVertex(FVector3f(InArrowPointB), InColor);

		auto LineMesh = MakeShared<FLexUIGizmoMesh>(VertexArray, IndexArray, ELexUIGizmoMeshPrimitiveType::Line);
		LineMesh->Material = TStrongObjectPtr(GetDefaultGizmoMaterial());
		LineMesh->LocalToWorldMatrix = FMatrix::Identity;
		LineMesh->UpdateLocalBounds();
		LineMesh->Render(ViewExtension, ScreenOrWorld);
	}
}

void ULexUIManagerWorldSubsystem::DrawNavigationVisualizerOnUISelectable(UWorld* InWorld, UUISelectable* InSelectable, bool IsScreenSpace)
{
	auto SourceWidget = InSelectable->GetWidget();
	if (!IsValid(SourceWidget))return;
	const FColor Color = GetSelection()->IsSelected(SourceWidget) ? FColor(255, 255, 0, 255) : FColor(140, 140, 0, 255);
	constexpr float Offset = 2;
	constexpr float ArrowSize = 5;
	
	auto GetArrowSizeScaledByDistanceToCamera = [=, this](FVector WorldPoint)
	{
		if (this->GetWorld()->IsGameWorld())
		{
			if (auto PC = this->GetWorld()->GetFirstPlayerController())
			{
				if (auto CameraManager = PC->PlayerCameraManager)
				{
					auto ViewLocation = CameraManager->GetCameraLocation();
					float Distance = FVector::Distance(WorldPoint, ViewLocation);
					return Distance * 0.01f;
				}
			}
		}
		else
		{
			if (auto ViewportClient = GetEditorViewportClient())
			{
				if (ViewportClient->IsOrtho())
				{
					return ViewportClient->GetOrthoZoom() * 0.001f; 
				}
				else
				{
					auto ViewLocation = ViewportClient->GetViewLocation();
					float Distance = FVector::Distance(WorldPoint, ViewLocation);
					return Distance * 0.01f;
				}
			}
		}
		return ArrowSize;
	};

	if (auto ToLeftComp = InSelectable->FindSelectableOnLeft())
	{
		if (ToLeftComp != InSelectable)
		{
			auto SourceLeftPoint = FVector(0, SourceWidget->GetLocalSpaceLeft(), 0.5f * (SourceWidget->GetLocalSpaceTop() + SourceWidget->GetLocalSpaceBottom()) + Offset);
			SourceLeftPoint = SourceWidget->GetWorldTransform().TransformPosition(SourceLeftPoint);
			auto DestWidget = ToLeftComp->GetWidget();
			auto LocalDestRightPoint = FVector(0, DestWidget->GetLocalSpaceRight(), 0.5f * (DestWidget->GetLocalSpaceTop() + DestWidget->GetLocalSpaceBottom()) + Offset);
			auto DestRightPoint = DestWidget->GetWorldTransform().TransformPosition(LocalDestRightPoint);
			float Distance = FVector::Distance(SourceLeftPoint, DestRightPoint);
			Distance *= 0.2f;
			auto ScaledArrowSize = ArrowSize;
			if (!IsScreenSpace)
			{
				ScaledArrowSize = GetArrowSizeScaledByDistanceToCamera(DestRightPoint);
			}
			auto ArrowPointA = DestWidget->GetWorldTransform().TransformPosition(LocalDestRightPoint + FVector(0, ScaledArrowSize, ScaledArrowSize));
			auto ArrowPointB = DestWidget->GetWorldTransform().TransformPosition(LocalDestRightPoint + FVector(0, ScaledArrowSize, -ScaledArrowSize));
			DrawNavigationArrow(InWorld
				, {
					SourceLeftPoint,
					SourceLeftPoint - SourceWidget->GetRightVector() * Distance,
					DestRightPoint + DestWidget->GetRightVector() * Distance,
					DestRightPoint,
				}
				, ArrowPointA, ArrowPointB
				, Color, InSelectable, FString::Printf(TEXT("%s.NavigationLeft"), *InSelectable->GetWidget()->GetDisplayName()), IsScreenSpace);
		}
	}
	if (auto ToRightComp = InSelectable->FindSelectableOnRight())
	{
		if (ToRightComp != InSelectable)
		{
			auto SourceRightPoint = FVector(0, SourceWidget->GetLocalSpaceRight(), 0.5f * (SourceWidget->GetLocalSpaceTop() + SourceWidget->GetLocalSpaceBottom()) - Offset);
			SourceRightPoint = SourceWidget->GetWorldTransform().TransformPosition(SourceRightPoint);
			auto DestWidget = ToRightComp->GetWidget();
			auto LocalDestLeftPoint = FVector(0, DestWidget->GetLocalSpaceLeft(), 0.5f * (DestWidget->GetLocalSpaceTop() + DestWidget->GetLocalSpaceBottom()) - Offset);
			auto DestLeftPoint = DestWidget->GetWorldTransform().TransformPosition(LocalDestLeftPoint);
			float Distance = FVector::Distance(SourceRightPoint, DestLeftPoint);
			Distance *= 0.2f;
			auto ScaledArrowSize = ArrowSize;
			if (!IsScreenSpace)
			{
				ScaledArrowSize = GetArrowSizeScaledByDistanceToCamera(DestLeftPoint);
			}
			auto ArrowPointA = DestWidget->GetWorldTransform().TransformPosition(LocalDestLeftPoint + FVector(0, -ScaledArrowSize, ScaledArrowSize));
			auto ArrowPointB = DestWidget->GetWorldTransform().TransformPosition(LocalDestLeftPoint + FVector(0, -ScaledArrowSize, -ScaledArrowSize));
			DrawNavigationArrow(InWorld
				, {
					SourceRightPoint,
					SourceRightPoint + SourceWidget->GetRightVector() * Distance,
					DestLeftPoint - DestWidget->GetRightVector() * Distance,
					DestLeftPoint,
				}
				, ArrowPointA, ArrowPointB
				, Color, InSelectable, FString::Printf(TEXT("%s.NavigationRight"), *InSelectable->GetWidget()->GetDisplayName()), IsScreenSpace);
		}
	}
	if (auto ToDownComp = InSelectable->FindSelectableOnDown())
	{
		if (ToDownComp != InSelectable)
		{
			auto SourceDownPoint = FVector(0, 0.5f * (SourceWidget->GetLocalSpaceLeft() + SourceWidget->GetLocalSpaceRight()) - Offset, SourceWidget->GetLocalSpaceBottom());
			SourceDownPoint = SourceWidget->GetWorldTransform().TransformPosition(SourceDownPoint);
			auto DestWidget = ToDownComp->GetWidget();
			auto LocalDestUpPoint = FVector(0, 0.5f * (DestWidget->GetLocalSpaceLeft() + DestWidget->GetLocalSpaceRight()) - Offset, DestWidget->GetLocalSpaceTop());
			auto DestUpPoint = DestWidget->GetWorldTransform().TransformPosition(LocalDestUpPoint);
			float Distance = FVector::Distance(SourceDownPoint, DestUpPoint);
			Distance *= 0.2f;
			auto ScaledArrowSize = ArrowSize;
			if (!IsScreenSpace)
			{
				ScaledArrowSize = GetArrowSizeScaledByDistanceToCamera(DestUpPoint);
			}
			auto ArrowPointA = DestWidget->GetWorldTransform().TransformPosition(LocalDestUpPoint + FVector(0, ScaledArrowSize, ScaledArrowSize));
			auto ArrowPointB = DestWidget->GetWorldTransform().TransformPosition(LocalDestUpPoint + FVector(0, -ScaledArrowSize, ScaledArrowSize));
			DrawNavigationArrow(InWorld
				, {
					SourceDownPoint,
					SourceDownPoint - SourceWidget->GetUpVector() * Distance,
					DestUpPoint + DestWidget->GetUpVector() * Distance,
					DestUpPoint,
				}
				, ArrowPointA, ArrowPointB
				, Color, InSelectable, FString::Printf(TEXT("%s.NavigationDown"), *InSelectable->GetWidget()->GetDisplayName()), IsScreenSpace);
		}
	}
	if (auto ToUpComp = InSelectable->FindSelectableOnUp())
	{
		if (ToUpComp != InSelectable)
		{
			auto SourceUpPoint = FVector(0, 0.5f * (SourceWidget->GetLocalSpaceLeft() + SourceWidget->GetLocalSpaceRight()) + Offset, SourceWidget->GetLocalSpaceTop());
			SourceUpPoint = SourceWidget->GetWorldTransform().TransformPosition(SourceUpPoint);
			auto DestWidget = ToUpComp->GetWidget();
			auto LocalDestDownPoint = FVector(0, 0.5f * (DestWidget->GetLocalSpaceLeft() + DestWidget->GetLocalSpaceRight()) + Offset, DestWidget->GetLocalSpaceBottom());
			auto DestDownPoint = DestWidget->GetWorldTransform().TransformPosition(LocalDestDownPoint);
			float Distance = FVector::Distance(SourceUpPoint, DestDownPoint);
			Distance *= 0.2f;
			auto ScaledArrowSize = ArrowSize;
			if (!IsScreenSpace)
			{
				ScaledArrowSize = GetArrowSizeScaledByDistanceToCamera(DestDownPoint);
			}
			auto ArrowPointA = DestWidget->GetWorldTransform().TransformPosition(LocalDestDownPoint + FVector(0, ScaledArrowSize, -ScaledArrowSize));
			auto ArrowPointB = DestWidget->GetWorldTransform().TransformPosition(LocalDestDownPoint + FVector(0, -ScaledArrowSize, -ScaledArrowSize));
			DrawNavigationArrow(InWorld
				, {
					SourceUpPoint,
					SourceUpPoint + SourceWidget->GetUpVector() * Distance,
					DestDownPoint - DestWidget->GetUpVector() * Distance,
					DestDownPoint,
				}
				, ArrowPointA, ArrowPointB
				, Color, InSelectable, FString::Printf(TEXT("%s.NavigationUp"), *InSelectable->GetWidget()->GetDisplayName()), IsScreenSpace);
		}
	}
}

FEditorViewportClient* ULexUIManagerWorldSubsystem::GetEditorViewportClient()
{
	if (CacheViewportClient == nullptr)
	{
		for (auto& ViewportClient : GEditor->GetAllViewportClients())
		{
			if (ViewportClient->GetWorld() == this->GetWorld())
			{
				if (ViewportClient->IsVisible())
				{
					CacheViewportClient = ViewportClient;
				}
			}
		}
	}
	return CacheViewportClient;
}

UMaterialInterface* ULexUIManagerWorldSubsystem::GetDefaultGizmoMaterial()
{
	static TWeakObjectPtr<UMaterialInterface> GizmoMaterial;
	if (!GizmoMaterial.IsValid())
	{
		GizmoMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/LGUI/EditorGizmo/GizmoMaterial"));
	}
	return GizmoMaterial.Get();
}

void ULexUIManagerWorldSubsystem::OnEndOfFrame()
{
	CacheViewportClient = nullptr;
}

void ULexUIManagerWorldSubsystem::OnEnginePreExit()
{
	// for (TObjectIterator<ULexUIPrefab> Itr; Itr; ++Itr)
	// {
	// 	auto Prefab = *Itr;
	// 	Prefab->ClearPrefabInstanceScene();
	// }
}

void ULexUIManagerWorldSubsystem::DrawDebugRect(UWorld* InWorld, const FVector& Center, const FMatrix& LocalToWorld, FVector2D const& Rect, FColor const& Color, void* Object, const FString& DebugName, bool ScreenOrWorld)
{
	auto ViewExtension = ULexUIManagerWorldSubsystem::GetViewExtension(InWorld, true);
	if (ViewExtension.IsValid())
	{
		TArray<FLexUIMeshVertex> VertexArray;
		TArray<FLexUIMeshIndex> IndexArray;
		auto PushNewLine = [&](FVector Start, FVector End)
		{
			IndexArray.Add(VertexArray.Num());
			new(VertexArray) FLexUIMeshVertex(FVector3f(Center + Start), Color);
			IndexArray.Add(VertexArray.Num());
			new(VertexArray) FLexUIMeshVertex(FVector3f(Center + End), Color);
		};

		auto Start = FVector(0, Rect.X, Rect.Y);
		auto End = FVector(0, -Rect.X, Rect.Y);
		PushNewLine(Start, End);

		Start = FVector(0, Rect.X, -Rect.Y);
		End = FVector(0, -Rect.X, -Rect.Y);
		PushNewLine(Start, End);

		Start = FVector(0, Rect.X, Rect.Y);
		End = FVector(0, Rect.X, -Rect.Y);
		PushNewLine(Start, End);

		Start = FVector(0, -Rect.X, Rect.Y);
		End = FVector(0, -Rect.X, -Rect.Y);
		PushNewLine(Start, End);

		auto LineMesh = MakeShared<FLexUIGizmoMesh>(VertexArray, IndexArray, ELexUIGizmoMeshPrimitiveType::Line);
		LineMesh->Material = TStrongObjectPtr(GetDefaultGizmoMaterial());
		LineMesh->LocalToWorldMatrix = LocalToWorld;
		LineMesh->UpdateLocalBounds();
		LineMesh->Render(ViewExtension, ScreenOrWorld);
	}
}

void ULexUIManagerWorldSubsystem::DrawDebugBox(UWorld* InWorld, const FVector& Center, const FMatrix& LocalToWorld,
	FVector const& Box, FColor const& Color, void* Object, const FString& DebugName, bool ScreenOrWorld)
{
	auto ViewExtension = ULexUIManagerWorldSubsystem::GetViewExtension(InWorld, true);
	if (ViewExtension.IsValid())
	{
		TArray<FLexUIMeshVertex> VertexArray;
		TArray<FLexUIMeshIndex> IndexArray;
		auto PushNewLine = [&](const FVector& Start, const FVector& End)
		{
			IndexArray.Add(VertexArray.Num());
			new(VertexArray) FLexUIMeshVertex(FVector3f(Center + Start), Color);
			IndexArray.Add(VertexArray.Num());
			new(VertexArray) FLexUIMeshVertex(FVector3f(Center + End), Color);
		};

		FVector Start, End;
		Start = FVector(Box.X, Box.Y, Box.Z);
		End = FVector(Box.X, -Box.Y, Box.Z);
		PushNewLine(Start, End);

		Start = FVector(Box.X, Box.Y, -Box.Z);
		End = FVector(Box.X, -Box.Y, -Box.Z);
		PushNewLine(Start, End);

		Start = FVector(Box.X, Box.Y, Box.Z);
		End = FVector(Box.X, Box.Y, -Box.Z);
		PushNewLine(Start, End);

		Start = FVector(Box.X, -Box.Y, Box.Z);
		End = FVector(Box.X, -Box.Y, -Box.Z);
		PushNewLine(Start, End);

		Start = FVector(-Box.X, Box.Y, Box.Z);
		End = FVector(-Box.X, -Box.Y, Box.Z);
		PushNewLine(Start, End);

		Start = FVector(-Box.X, Box.Y, -Box.Z);
		End = FVector(-Box.X, -Box.Y, -Box.Z);
		PushNewLine(Start, End);

		Start = FVector(-Box.X, Box.Y, Box.Z);
		End = FVector(-Box.X, Box.Y, -Box.Z);
		PushNewLine(Start, End);

		Start = FVector(-Box.X, -Box.Y, Box.Z);
		End = FVector(-Box.X, -Box.Y, -Box.Z);
		PushNewLine(Start, End);

		Start = FVector(Box.X, Box.Y, Box.Z);
		End = FVector(-Box.X, Box.Y, Box.Z);
		PushNewLine(Start, End);

		Start = FVector(Box.X, -Box.Y, Box.Z);
		End = FVector(-Box.X, -Box.Y, Box.Z);
		PushNewLine(Start, End);

		Start = FVector(Box.X, Box.Y, -Box.Z);
		End = FVector(-Box.X, Box.Y, -Box.Z);
		PushNewLine(Start, End);

		Start = FVector(Box.X, -Box.Y, -Box.Z);
		End = FVector(-Box.X, -Box.Y, -Box.Z);
		PushNewLine(Start, End);

		auto LineMesh = MakeShared<FLexUIGizmoMesh>(VertexArray, IndexArray, ELexUIGizmoMeshPrimitiveType::Line);
		LineMesh->Material = TStrongObjectPtr(GetDefaultGizmoMaterial());
		LineMesh->LocalToWorldMatrix = LocalToWorld;
		LineMesh->UpdateLocalBounds();
		LineMesh->Render(ViewExtension, ScreenOrWorld);
	}
}

void ULexUIManagerWorldSubsystem::DrawDebugLine(UWorld* InWorld, const FMatrix& LocalToWorld,
	const TArray<FVector3f>& LinePoints, FColor const& Color, void* Object, const FString& DebugName,
	bool ScreenOrWorld)
{
	auto ViewExtension = ULexUIManagerWorldSubsystem::GetViewExtension(InWorld, true);
	if (ViewExtension.IsValid())
	{
		TArray<FLexUIMeshVertex> VertexArray;
		TArray<FLexUIMeshIndex> IndexArray;
		//lines
		for (int i = 0; i < LinePoints.Num(); i+=2)
		{
			IndexArray.Add(VertexArray.Num());
			new(VertexArray) FLexUIMeshVertex(LinePoints[i], Color);
			IndexArray.Add(VertexArray.Num());
			new(VertexArray) FLexUIMeshVertex(LinePoints[i + 1], Color);
		}
		auto LineMesh = MakeShared<FLexUIGizmoMesh>(VertexArray, IndexArray, ELexUIGizmoMeshPrimitiveType::Line);
		LineMesh->Material = TStrongObjectPtr(GetDefaultGizmoMaterial());
		LineMesh->LocalToWorldMatrix = LocalToWorld;
		LineMesh->UpdateLocalBounds();
		LineMesh->Render(ViewExtension, ScreenOrWorld);
	}
}

bool ULexUIManagerWorldSubsystem::RaycastHitUI(UWorld* InWorld, const TArray<ULexWidget*>& InWidgets, const FVector& LineStart, const FVector& LineEnd
                                               , ULexWidget*& ResultSelectTarget, int& InOutTargetIndexInHitArray
)
{
	TArray<FLexUIHitResult> HitResultArray;
	for (auto Widget : InWidgets)
	{
		if (!IsValid(Widget))continue;
		if (Widget->GetWorld() == InWorld)
		{
			if (auto Visual = Widget->GetVisual())
			{
				if (Widget->GetWidgetActiveInHierarchy() && Widget->GetRenderCanvas() != nullptr)
				{
					FLexUIHitResult HitInfo;
					auto OriginRaycastType = Visual->GetRaycastType();
					Visual->SetRaycastType(ELexVisualRaycastType::Mesh);//in editor selection, make the ray hit actural triangle
					if (Visual->LineTraceUI(HitInfo, LineStart, LineEnd))
					{
						if (Widget->IsPointVisibleOnClip(HitInfo.Location))
						{
							HitResultArray.Add(HitInfo);
						}
					}
					Visual->SetRaycastType(OriginRaycastType);
				}
			}
		}
	}
	if (HitResultArray.Num() > 0)//hit something
	{
		HitResultArray.Sort([](const FLexUIHitResult& A, const FLexUIHitResult& B)
			{
				auto AWidget = (ULexWidget*)(A.Widget.Get());
				auto BWidget = (ULexWidget*)(B.Widget.Get());
				if (AWidget->GetRenderCanvas()->GetActualSortOrder() == BWidget->GetRenderCanvas()->GetActualSortOrder())//if Canvas's sort order is equal then sort on item's depth
				{
					return AWidget->GetFlattenHierarchyIndex() > BWidget->GetFlattenHierarchyIndex();
				}
				else//if Canvas's depth not equal then sort on Canvas's SortOrder
				{
					return AWidget->GetRenderCanvas()->GetActualSortOrder() > BWidget->GetRenderCanvas()->GetActualSortOrder();
				}
			});
		InOutTargetIndexInHitArray++;
		if (InOutTargetIndexInHitArray >= HitResultArray.Num() || InOutTargetIndexInHitArray < 0)
		{
			InOutTargetIndexInHitArray = 0;
		}
		auto HitWidget = (ULexWidget*)(HitResultArray[InOutTargetIndexInHitArray].Widget.Get());//target need to select
		ResultSelectTarget = HitWidget;
		return true;
	}
	return false;
}
#endif

bool ULexUIManagerWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer);
}

bool ULexUIManagerWorldSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::Editor || WorldType == EWorldType::PIE || WorldType == EWorldType::EditorPreview;
}

void ULexUIManagerWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
#if WITH_EDITOR
	InstanceArray.Add(this);
	if (this->GetWorld()->WorldType == EWorldType::EditorPreview//EditorPreview world don't tick, so manually tick it
		|| this->GetWorld()->WorldType == EWorldType::Editor)
	{
		EditorTickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(TEXT("LexUIManagerWorldSubsystemEditorTick"), 0, [WeakThis = MakeWeakObjectPtr(this)](float DeltaTime) {
			if (WeakThis.IsValid())
			{
				WeakThis->Tick(DeltaTime);
				return true;
			}
			return false;
			});
	}
	if (this->GetWorld()->IsGameWorld() || this->GetWorld()->WorldType == EWorldType::Editor)//game world or editor world, skip editor preview world
	{
		bShouldTickInEditor = true;
	}
	else
	{
		bShouldTickInEditor = true;
		//wait for editor preview world to be ready
		GetWorld()->GetTimerManager().SetTimer(EditorPreviewTickTimerHandle, [this]()
		{
			GetWorld()->GetTimerManager().ClearTimer(EditorPreviewTickTimerHandle);
			bShouldTickInEditor = false;
		}, 0, false, 1);
	}
	FCoreDelegates::OnEndFrame.AddUObject(this, &ULexUIManagerWorldSubsystem::OnEndOfFrame);
	FCoreDelegates::OnEnginePreExit.AddUObject(this, &ULexUIManagerWorldSubsystem::OnEnginePreExit);
	ULexUIManagerObject::GetInstance(true);//make sure it is created
#endif
	//localization
	OnCultureChangedDelegateHandle = FInternationalization::Get().OnCultureChanged().AddUObject(this, &ULexUIManagerWorldSubsystem::OnCultureChanged);
}
void ULexUIManagerWorldSubsystem::PostInitialize()
{
	Super::PostInitialize();
	FWorldDelegates::OnWorldPreSendAllEndOfFrameUpdates.AddUObject(this, &ULexUIManagerWorldSubsystem::OnWorldPreSendAllEndOfFrameUpdates);
}
void ULexUIManagerWorldSubsystem::Deinitialize()
{
#if WITH_EDITOR
	InstanceArray.Remove(this);
	if (EditorTickDelegateHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(EditorTickDelegateHandle);
		EditorTickDelegateHandle.Reset();
	}
	OnDeinitialize.Broadcast();
#endif
	if (MainViewportViewExtension.IsValid())
	{
		MainViewportViewExtension.Reset();
	}
	if (OnCultureChangedDelegateHandle.IsValid())
	{
		FInternationalization::Get().OnCultureChanged().Remove(OnCultureChangedDelegateHandle);
	}
	FWorldDelegates::OnWorldPreSendAllEndOfFrameUpdates.RemoveAll(this);
	Super::Deinitialize();
}

void ULexUIManagerWorldSubsystem::BeginDestroy()
{
	check(!IsInitialized());
#if WITH_EDITOR
	auto CopiedWidgetArray = AllWidgetArray;//use a copied array, because when Widget.OnUnregister the AllWidgetArray will change
	for (int i = 0; i < CopiedWidgetArray.Num(); i++)
	{
		auto& Widget = CopiedWidgetArray[i];
		if (Widget->HasRegistered())
		{
			Widget->OnUnregister();
		}
		check(!Widget->HasBegunPlay());//edit mode should never begin play
	}
#endif
	Super::BeginDestroy();
}

TStatId ULexUIManagerWorldSubsystem::GetStatId() const
{
	//return GetStatID();
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULGUIManagerWorldSubsystem, STATGROUP_Tickables);
}
bool ULexUIManagerWorldSubsystem::IsTickableWhenPaused() const
{
	return true;
}

void ULexUIManagerWorldSubsystem::OnCultureChanged()
{
	bShouldUpdateOnCultureChanged = true;
}

ULexUIManagerWorldSubsystem* ULexUIManagerWorldSubsystem::GetInstance(UWorld* InWorld)
{
	return IsValid(InWorld) ? InWorld->GetSubsystem<ULexUIManagerWorldSubsystem>() : nullptr;
}

#if WITH_EDITOR
ULexUISelection* ULexUIManagerWorldSubsystem::GetSelection() const
{
	if (!IsValid(Selection))
	{
		Selection = NewObject<ULexUISelection>();
		Selection->SetFlags(RF_Transactional);
	}
	return Selection;
}

void ULexUIManagerWorldSubsystem::MarkLexUIWidgetOutlinerChanged()
{
	bLexUIWidgetOutlinerChanged = true;
}
#endif

void ULexUIManagerWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	//normally BeginPlay is already called when LoadPrefab, but if World has not BeginPlay then this will work
	for (int i = 0; i < AllWidgetArray.Num(); i++)
	{
		auto& Widget = AllWidgetArray[i];
		if (!Widget->HasBegunPlay())
		{
			Widget->BeginPlay();
		}
	}
}

void ULexUIManagerWorldSubsystem::OnWorldEndPlay(UWorld& InWorld)
{
#if WITH_EDITOR
	OnEndPlay.Broadcast();
	if (this->GetWorld()->IsGameWorld())//game mode should deinit when EndPlay
#endif
	{
		auto CopiedWidgetArray = AllWidgetArray;//use a copied array, because when Widget.OnUnregister the AllWidgetArray will change
		for (int i = 0; i < CopiedWidgetArray.Num(); i++)
		{
			auto& Widget = CopiedWidgetArray[i];
			if (Widget->HasRegistered())
			{
				Widget->OnUnregister();
			}
			if (Widget->HasBegunPlay())
			{
				Widget->EndPlay();
			}
		}
	}
	Super::OnWorldEndPlay(InWorld);
}

#if WITH_EDITOR
TArray<ULexUIManagerWorldSubsystem*> ULexUIManagerWorldSubsystem::InstanceArray;
#endif

DECLARE_CYCLE_STAT(TEXT("LexUIBehaviour Tick"), STAT_LexUIBehaviourTick, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("LexUIBehaviour Start"), STAT_LexUIBehaviourStart, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("UpdateLayout"), STAT_UpdateLayout, STATGROUP_LGUI);

void ULexUIManagerWorldSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
#if WITH_EDITOR
	if (bShouldTickInEditor)
#endif
	{
		this->TickLexUI(DeltaTime);
	}
}

void ULexUIManagerWorldSubsystem::TickLexUI(float DeltaTime)
{
	//Update culture
	{
		if (bShouldUpdateOnCultureChanged)
		{
			bShouldUpdateOnCultureChanged = false;
			for (auto& Culture : AllCultureChangedArray)
			{
				ILexUICultureChangedInterface::Execute_OnCultureChanged(Culture.Get());
			}
		}
	}

	//LexUIBehaviour start
	{
		if (LexUIBehavioursForStart.Num() > 0)
		{
			bIsExecutingStart = true;
			SCOPE_CYCLE_COUNTER(STAT_LexUIBehaviourStart);
			for (int i = 0; i < LexUIBehavioursForStart.Num(); i++)
			{
				auto item = LexUIBehavioursForStart[i];
				if (item.IsValid())
				{
					item->Call_Start();
					if (item->bCanExecuteTick)
					{
						LexUIBehavioursForTick.AddUnique(item);
					}
				}
			}
			LexUIBehavioursForStart.Reset();
			bIsExecutingStart = false;
		}
	}

	//LexUIBehaviour tick
	{
		bIsExecutingTick = true;
		auto bIsGamePaused = GetWorld()->IsPaused();
		auto Settings = GetDefault<ULexUISettings>();
		SCOPE_CYCLE_COUNTER(STAT_LexUIBehaviourTick);
		for (int i = 0; i < LexUIBehavioursForTick.Num(); i++)
		{
			CurrentExecutingTickIndex = i;
			auto Behaviour = LexUIBehavioursForTick[i];
			if (auto Widget = Behaviour->GetWidget())
			{
				bool bAffectByGamePause;
				if (Widget->IsScreenSpaceOverlayUI())
				{
					bAffectByGamePause = Settings->bScreenSpaceUIAffectByGamePause;
				}
				else
				{
					bAffectByGamePause = Settings->bWorldSpaceUIAffectByGamePause;
				}
				if (!bIsGamePaused || (bIsGamePaused && !bAffectByGamePause))
				{
					Behaviour->Tick(DeltaTime);
				}
			}
			else
			{
				if (!bIsGamePaused || (bIsGamePaused && Behaviour->bTickEvenWhenPaused))
				{
					Behaviour->Tick(DeltaTime);
				}
			}
		}
		bIsExecutingTick = false;
		CurrentExecutingTickIndex = -1;
		//remove these padding things
		if (LexUIBehavioursNeedToRemoveFromTick.Num() > 0)
		{
			for (auto& item : LexUIBehavioursNeedToRemoveFromTick)
			{
				LexUIBehavioursForTick.Remove(item);
			}
			LexUIBehavioursNeedToRemoveFromTick.Reset();
		}
	}

	//update layout
	CalculateLayout();

#if WITH_EDITOR
	int ScreenSpaceOverlayCanvasCount = 0;
	for (auto& Canvas : AllCanvasArray)
	{
		if (!Canvas.IsValid())continue;
		if (!Canvas->IsRootCanvas())continue;
		if (Canvas->GetRenderMode() == ELexRenderMode::ScreenSpaceOverlay)
		{
			ScreenSpaceOverlayCanvasCount++;
		}
	}
	if (ScreenSpaceOverlayCanvasCount > 1)
	{
		if (PrevScreenSpaceOverlayCanvasCount != ScreenSpaceOverlayCanvasCount)//only show message when change
		{
			PrevScreenSpaceOverlayCanvasCount = ScreenSpaceOverlayCanvasCount;
			auto errMsg = FText::Format(LOCTEXT("MultipleLexUICanvasRenderScreenSpaceOverlay", "[{0}].{1} Detect multiple LexCanvas rendered with ScreenSpaceOverlay mode, this is not allowed! There should be only one ScreenSpace UI in a world!\
\n	World: {2}, type: {3}")
			, FText::FromString(ANSI_TO_TCHAR(__FUNCTION__)), __LINE__, FText::FromString(this->GetWorld()->GetPathName()), (int)(this->GetWorld()->WorldType));
			UE_LOG(LGUI, Error, TEXT("%s"), *errMsg.ToString());
			FLexUIUtils::EditorNotification(errMsg, false, 10.0f);
		}
	}
	else
	{
		PrevScreenSpaceOverlayCanvasCount = 0;
	}
	if (bLexUIWidgetOutlinerChanged)
	{
		bLexUIWidgetOutlinerChanged = false;
		OnLexUIWidgetOutlinerChanged.Broadcast();
	}
#endif
	//update draw-call
	{
		auto UpdateCanvas = [this](ELexRenderMode RenderMode) {
			for (auto& Canvas : AllCanvasArray)
			{
				if (!Canvas.IsValid())continue;
				if (!Canvas->IsRootCanvas())continue;
				if (Canvas->GetActualRenderMode() != RenderMode)continue;
				Canvas->UpdateRootCanvas();
			}
		};
		UpdateCanvas(ELexRenderMode::ScreenSpaceOverlay);
		UpdateCanvas(ELexRenderMode::WorldSpace);
		UpdateCanvas(ELexRenderMode::WorldSpace_LexUI);
		UpdateCanvas(ELexRenderMode::RenderTarget);
	}
}

void ULexUIManagerWorldSubsystem::OnWorldPreSendAllEndOfFrameUpdates(UWorld* InWorld)
{
	if (InWorld == this->GetWorld())
	{
#if WITH_EDITOR
		this->DrawHelperGizmo();
#endif
		this->SubmitCanvasDrawCall();
	}
}

#if WITH_EDITOR
void ULexUIManagerWorldSubsystem::DrawHelperGizmo()
{
	//editor draw helper frame
	auto Settings = GetDefault<ULexUIEditorSettings>();
	if (Settings->bDrawHelperFrame)
	{
		if (this->GetWorld()->WorldType == EWorldType::Game
			|| this->GetWorld()->WorldType == EWorldType::PIE
			|| this->GetWorld()->WorldType == EWorldType::Editor
			// || this->GetWorld()->WorldType == EWorldType::EditorPreview
			)
		{
			struct LOCAL
			{
				static void ForEachWidget(ULexUIManagerWorldSubsystem* LexUIManager, ULexWidget* Widget, bool bIsGameWorld)
				{
					if (!IsValid(Widget))return;

					bool bIsScreenSpace = false;
					if (bIsGameWorld)
					{
						if (auto RenderCanvas = Widget->GetRenderCanvas())
						{
							bIsScreenSpace = RenderCanvas->IsRenderToScreenSpace() || RenderCanvas->IsRenderToRenderTarget();
						}
					}
					LexUIManager->DrawFrameOnWidget(Widget, bIsScreenSpace);

					for (auto& Child : Widget->GetChildren())
					{
						ForEachWidget(LexUIManager, Child, bIsGameWorld);
					}
				}
			};
			auto bIsGameWorld = this->GetWorld()->IsGameWorld();
			for (auto& Canvas : AllCanvasArray)
			{
				if (!Canvas.IsValid())continue;
				if (!Canvas->IsRootCanvas())continue;
				LOCAL::ForEachWidget(this, Canvas->GetWidget(), bIsGameWorld);
			}
		}
	}

	if (Settings->bDrawSelectableNavigationVisualizer)
	{
		for (auto& Selectable : AllSelectableArray)
		{
			if (!Selectable.IsValid())continue;
			if (!IsValid(Selectable->GetWorld()))continue;
			if (!IsValid(Selectable->GetWidget()))continue;
			if (!IsValid(Selectable->GetWidget()->GetRenderCanvas()))continue;
			if (!Selectable->GetWidget()->GetInteractableInHierarchy())continue;

			bool bIsScreenSpace = false;
			if (Selectable->GetWorld()->IsGameWorld())
			{
				auto RenderCanvas = Selectable->GetWidget()->GetRenderCanvas();
				bIsScreenSpace = RenderCanvas->IsRenderToScreenSpace() || RenderCanvas->IsRenderToRenderTarget();
			}
			DrawNavigationVisualizerOnUISelectable(Selectable->GetWorld(), Selectable.Get()
				, bIsScreenSpace);
		}
	}
}
#endif

void ULexUIManagerWorldSubsystem::SubmitCanvasDrawCall()
{
	//update draw-call
	{
		auto UpdateCanvas = [this](ELexRenderMode RenderMode) {
			for (auto& Canvas : AllCanvasArray)
			{
				if (!Canvas.IsValid())continue;
				if (!Canvas->IsRootCanvas())continue;
				if (Canvas->GetRenderMode() != RenderMode)continue;
				Canvas->UpdateDrawCallBatchData();
			}
		};
		UpdateCanvas(ELexRenderMode::ScreenSpaceOverlay);
		UpdateCanvas(ELexRenderMode::WorldSpace);
		UpdateCanvas(ELexRenderMode::WorldSpace_LexUI);
		UpdateCanvas(ELexRenderMode::RenderTarget);
	}
}

void ULexUIManagerWorldSubsystem::AddLexUIBehavioursForTick(ULexUIBehaviour* InComp)
{
	if (IsValid(InComp))
	{
		if (auto Instance = GetInstance(InComp->GetWorld()))
		{
			int32 index = INDEX_NONE;
			if (!Instance->LexUIBehavioursForTick.Find(InComp, index))
			{
				Instance->LexUIBehavioursForTick.Add(InComp);
				return;
			}
			UE_LOG(LGUI, Warning, TEXT("[%s].%d Already exist, comp:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(InComp->GetPathName()));
		}
	}
}
void ULexUIManagerWorldSubsystem::RemoveLexUIBehavioursFromTick(ULexUIBehaviour* InComp)
{
	if (IsValid(InComp))
	{
		if (auto Instance = GetInstance(InComp->GetWorld()))
		{
			auto& TickArray = Instance->LexUIBehavioursForTick;
			int32 Index = INDEX_NONE;
			if (TickArray.Find(InComp, Index))
			{
				if (Instance->bIsExecutingTick)
				{
					if (Index > Instance->CurrentExecutingTickIndex)//not execute it yet, safe to remove
					{
						TickArray.RemoveAt(Index);
					}
					else//already execute or current execute it, not safe to remove. should remove it after execute process complete
					{
						Instance->LexUIBehavioursNeedToRemoveFromTick.Add(InComp);
					}
				}
				else//not executing tick, safe to remove
				{
					TickArray.RemoveAt(Index);
				}
			}
			else
			{
				UE_LOG(LGUI, Warning, TEXT("[%s].%d Not exist, comp:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(InComp->GetPathName()));
			}

			//cleanup array
			int InvalidCount = 0;
			for (int i = TickArray.Num() - 1; i >= 0; i--)
			{
				if (!TickArray[i].IsValid())
				{
					TickArray.RemoveAt(i);
					InvalidCount++;
				}
			}
			if (InvalidCount > 0)
			{
				UE_LOG(LGUI, Warning, TEXT("[%s].%d Cleanup %d invalid LexUIBehaviour"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, InvalidCount);
			}
		}
	}
}
void ULexUIManagerWorldSubsystem::AddLexUIBehavioursForStart(ULexUIBehaviour* InComp)
{
	if (IsValid(InComp))
	{
		if (auto Instance = GetInstance(InComp->GetWorld()))
		{
			int32 index = INDEX_NONE;
			if (!Instance->LexUIBehavioursForStart.Find(InComp, index))
			{
				Instance->LexUIBehavioursForStart.Add(InComp);
				return;
			}
			UE_LOG(LGUI, Warning, TEXT("[%s].%d Already exist, comp:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(InComp->GetPathName()));
		}
	}
}
void ULexUIManagerWorldSubsystem::RemoveLexUIBehavioursFromStart(ULexUIBehaviour* InComp)
{
	if (IsValid(InComp))
	{
		if (auto Instance = GetInstance(InComp->GetWorld()))
		{
			auto& startArray = Instance->LexUIBehavioursForStart;
			int32 index = INDEX_NONE;
			if (startArray.Find(InComp, index))
			{
				if (Instance->bIsExecutingStart)
				{
					if (!InComp->bIsStartCalled)//if already called start then nothing to do, because start array will be cleared after execute start
					{
						startArray.RemoveAt(index);//not execute start yet, safe to remove
					}
				}
				else
				{
					startArray.RemoveAt(index);//not executing start, safe to remove
				}
			}
			else
			{
				UE_LOG(LGUI, Warning, TEXT("[%s].%d Not exist, comp:%s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(InComp->GetPathName()));
			}

			//cleanup array
			int inValidCount = 0;
			for (int i = startArray.Num() - 1; i >= 0; i--)
			{
				if (!startArray[i].IsValid())
				{
					startArray.RemoveAt(i);
					inValidCount++;
				}
			}
			if (inValidCount > 0)
			{
				UE_LOG(LGUI, Warning, TEXT("[%s].%d Cleanup %d invalid LexUIBehaviour"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, inValidCount);
			}
		}
	}
}

void ULexUIManagerWorldSubsystem::RegisterLexUICultureChangedEvent(TScriptInterface<ILexUICultureChangedInterface> InItem)
{
	if (auto Instance = GetInstance(InItem.GetObject()->GetWorld()))
	{
		Instance->AllCultureChangedArray.AddUnique(InItem.GetObject());
	}
}
void ULexUIManagerWorldSubsystem::UnregisterLexUICultureChangedEvent(TScriptInterface<ILexUICultureChangedInterface> InItem)
{
	if (auto Instance = GetInstance(InItem.GetObject()->GetWorld()))
	{
		Instance->AllCultureChangedArray.RemoveSingle(InItem.GetObject());
	}
}

TArray<ULexCanvas*> ULexUIManagerWorldSubsystem::GetCanvasArrayByRenderMode(ELexRenderMode RenderMode) const
{
	TArray<ULexCanvas*> CanvasArray;
	for (auto& Canvas : AllCanvasArray)
	{
		if (!Canvas.IsValid())continue;
		if (Canvas->GetActualRenderMode() == RenderMode)
		{
			CanvasArray.Add(Canvas.Get());
		}
	}
	return CanvasArray;
}

#if WITH_EDITOR
void ULexUIManagerWorldSubsystem::RefreshAllUI(UWorld* InWorld)
{
	for (auto InstanceItem : InstanceArray)
	{
		if (InstanceItem != nullptr)
		{
			if (InWorld != nullptr && InstanceItem->GetWorld() != InWorld)
			{
				continue;
			}
		}
		auto Instance = InstanceItem;
		for (auto& Canvas : Instance->AllCanvasArray)
		{
			if (!Canvas.IsValid())continue;
			if (!Canvas->IsRootCanvas())continue;
			if (auto Widget = Canvas->GetWidget())
			{
				Widget->EnsureDataForRebuild();
				Widget->MarkCanvasUpdate(true);
			}
		}
	}
}

#endif

void ULexUIManagerWorldSubsystem::AddCanvas(ULexCanvas* InCanvas)
{
#if !UE_BUILD_SHIPPING
	if (this->AllCanvasArray.Contains(InCanvas))
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d break here for debug"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
	}
#endif
	this->AllCanvasArray.AddUnique(InCanvas);
}

void ULexUIManagerWorldSubsystem::RemoveCanvas(ULexCanvas* InCanvas)
{
#if !UE_BUILD_SHIPPING
	if (!this->AllCanvasArray.Contains(InCanvas))
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d break here for debug"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
	}
#endif
	this->AllCanvasArray.RemoveSingle(InCanvas);
}

void ULexUIManagerWorldSubsystem::AddWidget(ULexWidget* InWidget)
{
#if !UE_BUILD_SHIPPING
	if (AllWidgetArray.Contains(InWidget))
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d break here for debug"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
	}
#endif
	AllWidgetArray.AddUnique(InWidget);
}

void ULexUIManagerWorldSubsystem::RemoveWidget(ULexWidget* InWidget)
{
#if !UE_BUILD_SHIPPING
	if (!AllWidgetArray.Contains(InWidget))
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d break here for debug"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
	}
#endif
	AllWidgetArray.RemoveSingle(InWidget);
}

void ULexUIManagerWorldSubsystem::AddLayoutDirtyWidget(ULexWidget* InWidget)
{
	LayoutDirtyWidgetArray.AddUnique(InWidget);
}

void ULexUIManagerWorldSubsystem::MarkRebuildLayoutTree(ULexWidget* InWidget)
{
	if (!bIsExecutingLayout)
	{
		MapWidgetToLayoutTree.Remove(InWidget);
	}
}

void ULexUIManagerWorldSubsystem::MarkRebuildAllLayoutTree()
{
	if (!bIsExecutingLayout)
	{
		MapWidgetToLayoutTree.Empty();
	}
}

void ULexUIManagerWorldSubsystem::CalculateLayoutTree(ULexWidget* RootLayoutWidget)
{
	struct LOCAL
	{
		static void CollectLayoutTree(ULexWidget* Widget, TArray<TObjectPtr<ULexWidget>>& LayoutTreeArray)
		{
			if (!IsValid(Widget))return;
			if (!Widget->GetWidgetActiveInHierarchy())return;
			if (!Widget->HasRegistered())return;//if not registered, means it could about to remove
			LayoutTreeArray.Add(Widget);
			if (Widget->GetLayoutContainer())
			{
				for (auto& Child : Widget->GetChildren())
				{
					CollectLayoutTree(Child, LayoutTreeArray);
				}
			}
		}
	};
	
	auto& LayoutTree = MapWidgetToLayoutTree.FindOrAdd(RootLayoutWidget);
	if (LayoutTree.WidgetArray.IsEmpty())
	{
		LOCAL::CollectLayoutTree(RootLayoutWidget, LayoutTree.WidgetArray);
	}
	auto& LayoutTreeArray = LayoutTree.WidgetArray;
	for (int i = 0; i < LayoutTreeArray.Num(); i++)
	{
		auto Widget = LayoutTreeArray[i];
		if (auto LayoutContainer = Widget->GetLayoutContainer())
		{
			if (!LayoutContainerArrayWhichHasSnapshot.Contains(LayoutContainer))
			{
				LayoutContainer->SnapshotLayout();
				LayoutContainerArrayWhichHasSnapshot.Add(LayoutContainer);
			}
		}
		Widget->UpdateLayout();
	}
}
void ULexUIManagerWorldSubsystem::CalculateLayout()
{
	if (LayoutDirtyWidgetArray.Num() > 0)
	{
		bIsExecutingLayout = true;
		int LayoutCalcCount = 0;
#if LEXUI_LAYOUT_DEBUG
		const double Time = FPlatformTime::Seconds();
		UE_LOG(LGUI, Log, TEXT("---Begin layout frame:%d, World:%s---"), GFrameNumber, *GetWorld()->GetPathName());
#endif
		LayoutContainerArrayWhichHasSnapshot.Reset();
		while (LayoutDirtyWidgetArray.Num() > 0)
		{
			SCOPE_CYCLE_COUNTER(STAT_UpdateLayout);
			LayoutCalcCount++;

			TArray<TWeakObjectPtr<ULexWidget>> CopiedLayoutDirtyWidgetArray;
			Swap(CopiedLayoutDirtyWidgetArray, LayoutDirtyWidgetArray);
			
			for (int i = CopiedLayoutDirtyWidgetArray.Num() - 1; i >= 0; i--)
			{
				auto& Widget = CopiedLayoutDirtyWidgetArray[i];
				CalculateLayoutTree(Widget.Get());
			}
		}
		for (auto& SnapshotLayout : LayoutContainerArrayWhichHasSnapshot)
		{
			SnapshotLayout->ApplyLayoutResult();
		}
#if LEXUI_LAYOUT_DEBUG
		for (auto& CalcCountKeyValue : LayoutCalculationCounterMap)
		{
			if (CalcCountKeyValue.Value >= 2)
			{
				UE_LOG(LGUI, Warning, TEXT("Widget %s has been calculated layout %d times in a frame"), *CalcCountKeyValue.Key, CalcCountKeyValue.Value);
			}
		}
		LayoutCalculationCounterMap.Reset();
		const double ElapsedMs = (FPlatformTime::Seconds() - Time) * 1000.0;
		UE_LOG(LGUI, Log, TEXT("---end layout frame:%d, count:%d, time:%f"), GFrameNumber, LayoutCalcCount, ElapsedMs);
#endif
		bIsExecutingLayout = false;
	}
}
void ULexUIManagerWorldSubsystem::RebuildLayoutImmediately(ULexWidget* InWidget)
{
#if 0//@todo: optimize layout calculation, only calculate specific layout tree, not all 
	auto RootLayoutWidget = InWidget;
	//move up, find if parent widget affect by layout then mark dirty
	while (RootLayoutWidget)
	{
		if (auto ParentWidget = RootLayoutWidget->GetParent())
		{
			if (ParentWidget->GetLayoutContainer())//parent contains LayoutContainer, need calculate layout
			{
				RootLayoutWidget = ParentWidget;
				continue;
			}
		}
		break;
	}
	
	bool bCanCalculateLayoutTree = true;
	if (RootLayoutWidget == InWidget)//no valid layout parent
	{
		if (InWidget->GetLayoutContainer())//self contains layout container
		{
			bCanCalculateLayoutTree = true;
		}
		else
		{
			bCanCalculateLayoutTree = false;
		}
	}
	if (bCanCalculateLayoutTree)
	{
		CalculateLayoutTree(RootLayoutWidget);
	}
#else
	CalculateLayout();
#endif
}

#if LEXUI_LAYOUT_DEBUG
int ULexUIManagerWorldSubsystem::IncreateLayoutCalculationCounter(const FString& InPathName)
{
	if (auto CounterPtr = LayoutCalculationCounterMap.Find(InPathName))
	{
		(*CounterPtr)++;
		if (*CounterPtr >= 2)
		{
			// UE_LOG(LGUI, Warning, TEXT("Widget %s has been calculated layout %d times in a frame"), *InPathName, *CounterPtr);
		}
		return *CounterPtr;
	}
	else
	{
		LayoutCalculationCounterMap.Add(InPathName, 1);
		return 1;
	}
}
#endif

TSharedPtr<class FLexUIRenderer, ESPMode::ThreadSafe> ULexUIManagerWorldSubsystem::GetViewExtension(UWorld* InWorld, bool InCreateIfNotExist)
{
	if (auto Instance = GetInstance(InWorld))
	{
		if (!Instance->MainViewportViewExtension.IsValid())
		{
			if (InCreateIfNotExist)
			{
				Instance->MainViewportViewExtension = FSceneViewExtensions::NewExtension<FLexUIRenderer>(InWorld, ELexUIRendererType::ScreenSpace_and_WorldSpace);
			}
		}
		return Instance->MainViewportViewExtension;
	}
	return nullptr;
}

void ULexUIManagerWorldSubsystem::AddRaycaster(ULexBaseRaycaster* InRaycaster)
{
	if (auto Instance = GetInstance(InRaycaster->GetWorld()))
	{
		auto& AllRaycasterArray = Instance->AllRaycasterArray;
		if (AllRaycasterArray.Contains(InRaycaster))return;
		AllRaycasterArray.Add(InRaycaster);
	}
}
void ULexUIManagerWorldSubsystem::RemoveRaycaster(ULexBaseRaycaster* InRaycaster)
{
	if (auto Instance = GetInstance(InRaycaster->GetWorld()))
	{
		int32 index;
		if (Instance->AllRaycasterArray.Find(InRaycaster, index))
		{
			Instance->AllRaycasterArray.RemoveAt(index);
		}
	}
}

void ULexUIManagerWorldSubsystem::AddSelectable(UUISelectable* InSelectable)
{
	if (auto Instance = GetInstance(InSelectable->GetWorld()))
	{
		auto& AllSelectableArray = Instance->AllSelectableArray;
#if !UE_BUILD_SHIPPING
		if (AllSelectableArray.Contains(InSelectable))
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d break here for debug"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		}
#endif
		AllSelectableArray.AddUnique(InSelectable);
	}
}
void ULexUIManagerWorldSubsystem::RemoveSelectable(UUISelectable* InSelectable)
{
	if (auto Instance = GetInstance(InSelectable->GetWorld()))
	{
		int32 index;
		if (Instance->AllSelectableArray.Find(InSelectable, index))
		{
			Instance->AllSelectableArray.RemoveAt(index);
		}
	}
}

ULexEventSystem* ULexUIManagerWorldSubsystem::GetEventSystemByUserIndex(int UserIndex)
{
	if (auto ResultPtr = MapUserIndexToEventSystem.Find(UserIndex))
	{
		return ResultPtr->Get();
	}
	return nullptr;
}

void ULexUIManagerWorldSubsystem::AddEventSystem(ULexEventSystem* InEventSystem)
{
	if (auto InstancePtr = MapUserIndexToEventSystem.Find(InEventSystem->GetUserIndex()))
	{
		auto Instance = *InstancePtr;
		FString ActorName =
#if WITH_EDITOR
			Instance->GetOwner()->GetActorLabel();
#else
			Instance->GetOwner()->GetName();
#endif
		FString ErrorMsg = FString::Printf(TEXT("[%s].%d LexEventSystem component is already exist in actor:%s, pathName:%s, world:%s, multiple LexEventSystem with same UserIndex in same world is not allowed!")
			, ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *ActorName, *Instance->GetPathName(), *GetWorld()->GetPathName());
		UE_LOG(LGUI, Error, TEXT("%s"), *ErrorMsg);
		GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Red, ErrorMsg);
#if WITH_EDITOR
		FLexUIUtils::EditorNotification(FText::FromString(ErrorMsg), false, 10);
#endif
	}
	else
	{
		MapUserIndexToEventSystem.Add(InEventSystem->GetUserIndex(), InEventSystem);
	}
}

void ULexUIManagerWorldSubsystem::RemoveEventSystem(ULexEventSystem* InEventSystem)
{
	MapUserIndexToEventSystem.Remove(InEventSystem->GetUserIndex());
}

#undef LOCTEXT_NAMESPACE