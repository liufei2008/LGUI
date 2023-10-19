// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUIPrefabPreviewScene.h"
#include "PrefabEditor/LGUIPrefabEditor.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "Core/Actor/UIContainerActor.h"
#include "LGUIEditorModule.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SphereReflectionCaptureComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Layout/LGUICanvasScaler.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabPreviewScene"

const FString FLGUIPrefabPreviewScene::RootAgentActorName = TEXT("[temporary_RootAgent]");

FLGUIPrefabPreviewScene::FLGUIPrefabPreviewScene(ConstructionValues CVS) :FPreviewScene(CVS)
{
	//GetWorld()->GetWorldSettings()->NotifyBeginPlay();
	//GetWorld()->GetWorldSettings()->NotifyMatchStarted();
	//GetWorld()->GetWorldSettings()->SetActorHiddenInGame(false);
	//GetWorld()->bBegunPlay = true;

	// This is all from the AnimationEditorPreviewScene.cpp
	// set light options
	DirectionalLight->SetRelativeLocation(FVector(-1024.f, 1024.f, 2048.f));
	DirectionalLight->SetRelativeScale3D(FVector(15.f));
	DirectionalLight->Mobility = EComponentMobility::Movable;
	DirectionalLight->DynamicShadowDistanceStationaryLight = 3000.f;

	SetLightBrightness(4.f);

	DirectionalLight->InvalidateLightingCache();
	DirectionalLight->RecreateRenderState_Concurrent();

	// A background sky sphere
	//m_EditorSkyComp = NewObject<UStaticMeshComponent>();
	//UStaticMesh* StaticMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/MapTemplates/Sky/SM_SkySphere.SM_SkySphere"), NULL, LOAD_None, NULL);
 //   if (StaticMesh)
 //       m_EditorSkyComp->SetStaticMesh(StaticMesh);

	//// TODO: Clone this material in case it is ever removed?
	//UMaterial* SkyMaterial = LoadObject<UMaterial>(NULL, TEXT("/Engine/EditorMaterials/PersonaSky.PersonaSky"), NULL, LOAD_None, NULL);
 //   if(SkyMaterial)
	//    m_EditorSkyComp->SetMaterial(0, SkyMaterial);

	//const float SkySphereScale = 1000.f;
	//const FTransform SkyTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(SkySphereScale));

	//AddComponent(m_EditorSkyComp, SkyTransform);

	// now add height fog component

	//m_EditorHeightFogComponent = NewObject<UExponentialHeightFogComponent>();

	//m_EditorHeightFogComponent->FogDensity = 0.00075f;
	//m_EditorHeightFogComponent->FogInscatteringColor = FLinearColor(3.f, 4.f, 6.f, 0.f) * 0.3f;
	//m_EditorHeightFogComponent->DirectionalInscatteringExponent = 16.f;
	//m_EditorHeightFogComponent->DirectionalInscatteringColor = FLinearColor(1.1f, 0.9f, 0.538427f, 0.f);
	//m_EditorHeightFogComponent->FogHeightFalloff = 0.01f;
	//m_EditorHeightFogComponent->StartDistance = 30000.f;

	//const FTransform FogTransform(FRotator(0, 0, 0), FVector(3824.f, 34248.f, 50000.f), FVector(80.f));
	//AddComponent(m_EditorHeightFogComponent, FogTransform);

	// add capture component for reflection
	USphereReflectionCaptureComponent* CaptureComponent = NewObject<USphereReflectionCaptureComponent>();

	const FTransform CaptureTransform(FRotator(0, 0, 0), FVector(0.f, 0.f, 100.f), FVector(1.f));
	AddComponent(CaptureComponent, CaptureTransform);
	CaptureComponent->UpdateReflectionCaptureContents(GetWorld());

	// add floor
#if 0
	UStaticMesh* FloorMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/EditorMeshes/PhAT_FloorBox.PhAT_FloorBox"), NULL, LOAD_None, NULL);
	check(FloorMesh);

	m_EditorFloorComp = NewObject<UStaticMeshComponent>();
	m_EditorFloorComp->SetStaticMesh(FloorMesh);

	AddComponent(m_EditorFloorComp, FTransform::Identity);

	m_EditorFloorComp->SetRelativeScale3D(FVector(3.f, 3.f, 1.f));
	m_EditorFloorComp->SetRelativeLocation(FVector(0, 0, -300));

	// TODO: Again, clone since this is marked as Persona specific?
	UMaterial* Material = LoadObject<UMaterial>(NULL, TEXT("/Engine/EditorMaterials/PersonaFloorMat.PersonaFloorMat"), NULL, LOAD_None, NULL);
	check(Material);

	m_EditorFloorComp->SetMaterial(0, Material);
	m_EditorFloorComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	m_EditorFloorComp->SetCollisionObjectType(ECC_WorldStatic);
#endif

	for (TActorIterator<AActor> ActorItr(this->GetWorld()); ActorItr; ++ActorItr)
	{
		if (AActor* ItemActor = *ActorItr)
		{
			WorldDefaultActors.Add(ItemActor);
		}
	}
}

USceneComponent* FLGUIPrefabPreviewScene::GetParentComponentForPrefab(ULGUIPrefab* InPrefab)
{
	if (RootAgentActor != nullptr)
	{
		return RootAgentActor->GetRootComponent();
	}
	if (!IsValid(InPrefab))return nullptr;
	auto Prefab = InPrefab;
	if (InPrefab->GetIsPrefabVariant())
	{
		auto RootSubPrefab = InPrefab;
		while (RootSubPrefab->GetIsPrefabVariant())
		{
			if (RootSubPrefab->ReferenceAssetList.Num() <= 0)
			{
				return nullptr;
			}
			RootSubPrefab = Cast<ULGUIPrefab>(RootSubPrefab->ReferenceAssetList[0]);
			if (!RootSubPrefab)
			{
				return nullptr;
			}
		}
		Prefab = RootSubPrefab;
	}
	if (Prefab->ReferenceClassList.Num() > 0)
	{
		if (Prefab->ReferenceClassList[0]->IsChildOf(AUIBaseActor::StaticClass()))//ui
		{
			auto CanvasSize = Prefab->PrefabDataForPrefabEditor.CanvasSize;
			//create Canvas for UI
			auto RootUICanvasActor = (AUIContainerActor*)(this->GetWorld()->SpawnActor<AActor>(AUIContainerActor::StaticClass(), FTransform::Identity));
			RootUICanvasActor->SetActorLabel(*RootAgentActorName);
			RootUICanvasActor->GetRootComponent()->SetWorldLocationAndRotationNoPhysics(FVector::ZeroVector, FRotator(0, 0, 0));

			if (Prefab->PrefabDataForPrefabEditor.bNeedCanvas)
			{
				auto RenderMode = (ELGUIRenderMode)Prefab->PrefabDataForPrefabEditor.CanvasRenderMode;
				auto CanvasComp = NewObject<ULGUICanvas>(RootUICanvasActor);
				CanvasComp->RegisterComponent();
				RootUICanvasActor->AddInstanceComponent(CanvasComp);
				CanvasComp->SetRenderMode(RenderMode);
			}

			RootUICanvasActor->GetUIItem()->SetWidth(CanvasSize.X);
			RootUICanvasActor->GetUIItem()->SetHeight(CanvasSize.Y);
			RootUICanvasActor->GetUIItem()->SetHierarchyIndex(0);

			RootAgentActor = RootUICanvasActor;
		}
		else//not ui
		{
			auto CreatedActor = this->GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, FActorSpawnParameters());
			//create SceneComponent
			{
				USceneComponent* RootComponent = NewObject<USceneComponent>(CreatedActor, USceneComponent::GetDefaultSceneRootVariableName(), RF_Transactional);
				RootComponent->Mobility = EComponentMobility::Static;
				RootComponent->bVisualizeComponent = false;

				CreatedActor->SetRootComponent(RootComponent);
				RootComponent->RegisterComponent();
				CreatedActor->AddInstanceComponent(RootComponent);
			}
			RootAgentActor = CreatedActor;
		}

		//set properties
		{
			//auto bEditable_Property = FindFProperty<FBoolProperty>(AUIContainerActor::StaticClass(), TEXT("bEditable"));
			//bEditable_Property->SetPropertyValue_InContainer(RootAgentActor, false);

			//RootAgentActor->bHiddenEd = true;
			//RootAgentActor->bHiddenEdLayer = true;
			//RootAgentActor->bHiddenEdLevel = true;
			RootAgentActor->SetLockLocation(true);

			auto bActorLabelEditable_Property = FindFProperty<FBoolProperty>(AUIContainerActor::StaticClass(), TEXT("bActorLabelEditable"));
			bActorLabelEditable_Property->SetPropertyValue_InContainer(RootAgentActor, false);
		}

		RootAgentActor->SetActorLabel(*RootAgentActorName);
		RootAgentActor->GetRootComponent()->SetWorldLocationAndRotationNoPhysics(FVector::ZeroVector, FRotator(0, 0, 0));

		return RootAgentActor->GetRootComponent();
	}
	return nullptr;
}

bool FLGUIPrefabPreviewScene::IsWorldDefaultActor(AActor* InActor)const
{
	return WorldDefaultActors.Contains(InActor);
}

#undef LOCTEXT_NAMESPACE