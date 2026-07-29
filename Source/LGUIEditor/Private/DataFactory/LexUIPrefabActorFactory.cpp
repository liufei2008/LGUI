// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DataFactory/LexUIPrefabActorFactory.h"
#include "PrefabSystem/LexUIPrefab.h"
#include "AssetRegistry/AssetData.h"
#include "Core/LexUIManager.h"
#include "Event/LexScreenSpaceRaycaster.h"
#include "PrefabSystem/LexUIPrefabPresenterComponent.h"


#define LOCTEXT_NAMESPACE "LexUIPrefabActorFactory"


ULexUIPrefabActorFactory::ULexUIPrefabActorFactory()
{
	DisplayName = LOCTEXT("PrefabDisplayName", "Prefab");
	bShowInEditorQuickMenu = false;
	bUseSurfaceOrientation = false;
}

bool ULexUIPrefabActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (AssetData.IsValid() && AssetData.GetClass()->IsChildOf(ULexUIPrefab::StaticClass()))
	{
		return true;
	}

	return false;
}

bool ULexUIPrefabActorFactory::PreSpawnActor(UObject* Asset, FTransform& InOutLocation)
{
	ULexUIPrefabPresenterComponent::MarkNeedCheckNecessaryObjects();
	auto Prefab = CastChecked<ULexUIPrefab>(Asset);

	if (Prefab == NULL)
	{
		return false;
	}
	return true;
}

AActor* ULexUIPrefabActorFactory::SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform,
	const FActorSpawnParameters& InSpawnParams)
{
	auto Actor = Super::SpawnActor(InAsset, InLevel, InTransform, InSpawnParams);
	auto WidgetPresenterComponent = Actor->FindComponentByClass<ULexUIPrefabPresenterComponent>();
	if (!WidgetPresenterComponent)
	{
		WidgetPresenterComponent = NewObject<ULexUIPrefabPresenterComponent>(Actor, ULexUIPrefabPresenterComponent::StaticClass());
		Actor->SetRootComponent(WidgetPresenterComponent);
		WidgetPresenterComponent->RegisterComponent();
		Actor->AddInstanceComponent(WidgetPresenterComponent);
	}
	WidgetPresenterComponent->bIsSpawnFromFactory = true;
	return Actor;
}

void ULexUIPrefabActorFactory::PostSpawnActor(UObject* Asset, AActor* InNewActor)
{
	Super::PostSpawnActor(Asset, InNewActor);

	auto Prefab = CastChecked<ULexUIPrefab>(Asset);

	auto WidgetPresenterComponent = InNewActor->FindComponentByClass<ULexUIPrefabPresenterComponent>();
	// WidgetPresenterComponent->GetLoadedWidget()->SetSizeDelta(Prefab->PrefabDataForPrefabEditor.CanvasSize);
	WidgetPresenterComponent->SetPrefab(Prefab);

	auto World = InNewActor->GetWorld();
	if (World && World->WorldType != EWorldType::EditorPreview && !World->IsGameWorld())//Edit mode and not BlueprintEditorPreview
	{
		ULexUIManagerObject::AddOneShotTickFunction([WeakObject = MakeWeakObjectPtr(WidgetPresenterComponent)]()
		{
			if (WeakObject.IsValid())
			{
				WeakObject->CheckNecessaryObjects();
			}
		}, 1);
	}
}

void ULexUIPrefabActorFactory::PostPlaceAsset(TArrayView<const FTypedElementHandle> InHandle,
	const FAssetPlacementInfo& InPlacementInfo, const FPlacementOptions& InPlacementOptions)
{
	Super::PostPlaceAsset(InHandle, InPlacementInfo, InPlacementOptions);
}

UObject* ULexUIPrefabActorFactory::GetAssetFromActorInstance(AActor* ActorInstance)
{
	auto WidgetPresenterComponent = ActorInstance->FindComponentByClass<ULexUIPrefabPresenterComponent>();
	check(WidgetPresenterComponent);
	return WidgetPresenterComponent->GetPrefab();
}

UClass* ULexUIPrefabActorFactory::GetDefaultActorClass(const FAssetData& AssetData)
{
	if (auto Prefab = Cast<ULexUIPrefab>(AssetData.GetAsset()))
	{
		FString ClassName;
		auto RenderMode = (ELexRenderMode)Prefab->PrefabDataForPrefabEditor.CanvasRenderMode;
		switch (RenderMode)
		{
		case ELexRenderMode::WorldSpace:
			ClassName = TEXT("LexWorldSpaceRoot_UERenderer");
			break;
		case ELexRenderMode::WorldSpace_LexUI:
			ClassName = TEXT("LexWorldSpaceRoot_LexRenderer");
			break;
		case ELexRenderMode::ScreenSpaceOverlay:
			ClassName = TEXT("LexScreenSpaceRoot");
		}
		
		NewActorClass = LoadClass<AActor>(NULL, *FString::Printf(TEXT("/LGUI/Blueprints/%s.%s_C"), *ClassName, *ClassName));
		if (!NewActorClass)
		{
			NewActorClass = AActor::StaticClass();
		}
		return NewActorClass;
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
