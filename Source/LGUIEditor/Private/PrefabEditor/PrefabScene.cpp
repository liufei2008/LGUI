// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	PrefabScene.cpp: Preview scene implementation.
=============================================================================*/

#include "PrefabScene.h"
#include "Misc/ConfigCacheIni.h"
#include "UObject/Package.h"
#include "SceneInterface.h"
#include "Components/MeshComponent.h"
#include "AudioDevice.h"
#include "Engine/TextureCube.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/LineBatchComponent.h"
#include "Components/ReflectionCaptureComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameMode.h"

FPrefabScene::FPrefabScene(FPrefabScene::ConstructionValues CVS)
	: PreviewWorld(nullptr)
	, bForceAllUsedMipsResident(CVS.bForceMipsResident)
{
	EObjectFlags NewObjectFlags = RF_NoFlags;
	if (CVS.bTransactional)
	{
		NewObjectFlags = RF_Transactional;
	}

	PreviewWorld = NewObject<UWorld>(GetTransientPackage(), CVS.Name, NewObjectFlags);
	PreviewWorld->WorldType = CVS.bEditor ? EWorldType::Editor : EWorldType::GamePreview;

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(PreviewWorld->WorldType);
	WorldContext.SetCurrentWorld(PreviewWorld);

	PreviewWorld->InitializeNewWorld(UWorld::InitializationValues()
										.AllowAudioPlayback(CVS.bAllowAudioPlayback)
										.CreatePhysicsScene(CVS.bCreatePhysicsScene)
										.RequiresHitProxies(CVS.bEditor) // Only Need hit proxies in an editor scene
										.CreateNavigation(false)
										.CreateAISystem(false)
										.ShouldSimulatePhysics(CVS.bShouldSimulatePhysics)
										.SetTransactional(CVS.bTransactional)
										.SetDefaultGameMode(CVS.DefaultGameMode)
										.ForceUseMovementComponentInNonGameWorld(CVS.bForceUseMovementComponentInNonGameWorld));

	FURL URL = FURL();
	//URL += TEXT("?SpectatorOnly=1");
	//URL = FURL(NULL, *EditorEngine->BuildPlayWorldURL(*PIEMapName, Params.bStartInSpectatorMode, ExtraURLOptions), TRAVEL_Absolute);

	if (CVS.OwningGameInstance && PreviewWorld->WorldType == EWorldType::GamePreview)
	{
		PreviewWorld->SetGameInstance(CVS.OwningGameInstance);

		FWorldContext& PreviewWorldContext = GEngine->GetWorldContextFromWorldChecked(PreviewWorld);
		PreviewWorldContext.OwningGameInstance = CVS.OwningGameInstance;
		PreviewWorldContext.GameViewport = CVS.OwningGameInstance->GetGameViewportClient();
		PreviewWorldContext.AddRef(PreviewWorld);
		
		//PreviewWorldContext.PIEInstance =

		if (CVS.DefaultGameMode)
		{
			PreviewWorld->SetGameMode(URL);

			AGameModeBase* Mode = PreviewWorld->GetAuthGameMode<AGameModeBase>();
			ensure(Mode);
		}
	}

	PreviewWorld->InitializeActorsForPlay(URL);

	if (CVS.bDefaultLighting)
	{
		DirectionalLight = NewObject<UDirectionalLightComponent>(GetTransientPackage(), NAME_None, RF_Transient);
		DirectionalLight->Intensity = CVS.LightBrightness;
		DirectionalLight->LightColor = FColor::White;
		AddComponent(DirectionalLight, FTransform(CVS.LightRotation));

		SkyLight = NewObject<USkyLightComponent>(GetTransientPackage(), NAME_None, RF_Transient);
		SkyLight->bLowerHemisphereIsBlack = false;
		SkyLight->SourceType = ESkyLightSourceType::SLS_SpecifiedCubemap;
		SkyLight->Intensity = CVS.SkyBrightness;
		SkyLight->Mobility = EComponentMobility::Movable;
		AddComponent(SkyLight, FTransform::Identity);

		LineBatcher = NewObject<ULineBatchComponent>(GetTransientPackage());
		LineBatcher->bCalculateAccurateBounds = false;
		AddComponent(LineBatcher, FTransform::Identity);
	}
}

FPrefabScene::~FPrefabScene()
{
	// Stop any audio components playing in this scene
	if (GEngine)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			if (FAudioDeviceHandle AudioDevice = World->GetAudioDevice())
			{
				AudioDevice->Flush(GetWorld(), false);
			}
		}
	}

	// Remove all the attached components
	for( int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++ )
	{
		UActorComponent* Component = Components[ ComponentIndex ];

		if (bForceAllUsedMipsResident)
		{
			// Remove the mip streaming override on the mesh to be removed
			UMeshComponent* pMesh = Cast<UMeshComponent>(Component);
			if (pMesh != NULL)
			{
				pMesh->SetTextureForceResidentFlag(false);
			}
		}

		Component->UnregisterComponent();
	}
	
	// The world may be released by now.
	if (PreviewWorld && GEngine)
	{
		PreviewWorld->CleanupWorld();
		GEngine->DestroyWorldContext(GetWorld());
		// Release PhysicsScene for fixing big fbx importing bug
		PreviewWorld->ReleasePhysicsScene();
	}
}

void FPrefabScene::AddComponent(UActorComponent* Component,const FTransform& LocalToWorld, bool bAttachToRoot /*= false*/)
{
	Components.AddUnique(Component);

	USceneComponent* SceneComp = Cast<USceneComponent>(Component);
	if(SceneComp && SceneComp->GetAttachParent() == NULL)
	{
		SceneComp->SetRelativeTransform(LocalToWorld);
	}

	Component->RegisterComponentWithWorld(GetWorld());

	if (bForceAllUsedMipsResident)
	{
		// Add a mip streaming override to the new mesh
		UMeshComponent* pMesh = Cast<UMeshComponent>(Component);
		if (pMesh != NULL)
		{
			pMesh->SetTextureForceResidentFlag(true);
		}
	}

	{
		UStaticMeshComponent* pStaticMesh = Cast<UStaticMeshComponent>(Component);
		if(pStaticMesh != nullptr)
		{
			pStaticMesh->bEvaluateWorldPositionOffset = true;
			pStaticMesh->bEvaluateWorldPositionOffsetInRayTracing = true;
		}
	}

	GetScene()->UpdateSpeedTreeWind(0.0);
}

void FPrefabScene::RemoveComponent(UActorComponent* Component)
{
	Component->UnregisterComponent();
	Components.Remove(Component);

	if (bForceAllUsedMipsResident)
	{
		// Remove the mip streaming override on the old mesh
		UMeshComponent* pMesh = Cast<UMeshComponent>(Component);
		if (pMesh != NULL)
		{
			pMesh->SetTextureForceResidentFlag(false);
		}
	}
}

void FPrefabScene::AddReferencedObjects( FReferenceCollector& Collector )
{
	Collector.AddReferencedObjects( Components );
	Collector.AddReferencedObject( PreviewWorld );
}

FString FPrefabScene::GetReferencerName() const
{
	return TEXT("FPrefabScene");
}

void FPrefabScene::UpdateCaptureContents()
{
	// This function is called from FAdvancedPrefabScene::Tick, FBlueprintEditor::Tick, and FThumbnailPrefabScene::Tick,
	// so assume we are inside a Tick function.
	const bool bInsideTick = true;

	USkyLightComponent::UpdateSkyCaptureContents(PreviewWorld);
	UReflectionCaptureComponent::UpdateReflectionCaptureContents(PreviewWorld, nullptr, false, false, bInsideTick);
}

void FPrefabScene::ClearLineBatcher()
{
	if (LineBatcher != NULL)
	{
		LineBatcher->Flush();
	}
}

/** Accessor for finding the current direction of the preview scene's DirectionalLight. */
FRotator FPrefabScene::GetLightDirection()
{
	return DirectionalLight->GetComponentTransform().GetUnitAxis( EAxis::X ).Rotation();
}

/** Function for modifying the current direction of the preview scene's DirectionalLight. */
void FPrefabScene::SetLightDirection(const FRotator& InLightDir)
{
#if WITH_EDITOR
	DirectionalLight->PreEditChange(NULL);
#endif // WITH_EDITOR
	DirectionalLight->SetAbsolute(true, true, true);
	DirectionalLight->SetRelativeRotation(InLightDir);
#if WITH_EDITOR
	DirectionalLight->PostEditChange();
#endif // WITH_EDITOR
}

void FPrefabScene::SetLightBrightness(float LightBrightness)
{
#if WITH_EDITOR
	DirectionalLight->PreEditChange(NULL);
#endif // WITH_EDITOR
	DirectionalLight->Intensity = LightBrightness;
#if WITH_EDITOR
	DirectionalLight->PostEditChange();
#endif // WITH_EDITOR
}

void FPrefabScene::SetLightColor(const FColor& LightColor)
{
#if WITH_EDITOR
	DirectionalLight->PreEditChange(NULL);
#endif // WITH_EDITOR
	DirectionalLight->LightColor = LightColor;
#if WITH_EDITOR
	DirectionalLight->PostEditChange();
#endif // WITH_EDITOR
}

void FPrefabScene::SetSkyBrightness(float SkyBrightness)
{
	SkyLight->SetIntensity(SkyBrightness);
}

void FPrefabScene::SetSkyCubemap(UTextureCube* Cubemap)
{
	SkyLight->SetCubemap(Cubemap);
}

void FPrefabScene::LoadSettings(const TCHAR* Section)
{
	FRotator LightDir;
	if ( GConfig->GetRotator( Section, TEXT("LightDir"), LightDir, GEditorPerProjectIni ) )
	{
		SetLightDirection( LightDir );
	}
}

void FPrefabScene::SaveSettings(const TCHAR* Section)
{
	GConfig->SetRotator( Section, TEXT("LightDir"), GetLightDirection(), GEditorPerProjectIni );
}

FLinearColor FPrefabScene::GetBackgroundColor() const
{
	FLinearColor BackgroundColor = FColor(55, 55, 55);
	return BackgroundColor;
}