// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DataFactory/LexUIMLActorFactory.h"
#include "AssetRegistry/AssetData.h"
#include "Core/LexUIManager.h"
#include "Event/LexScreenSpaceRaycaster.h"
#include "XMLSupport/LexUIMLBehaviour.h"
#include "XMLSupport/LexUIMLPresenterComponent.h"


#define LOCTEXT_NAMESPACE "LexUIMLActorFactory"


ULexUIMLActorFactory::ULexUIMLActorFactory()
{
	DisplayName = LOCTEXT("LexUIMLDisplayName", "LexUIML");
	bShowInEditorQuickMenu = false;
	bUseSurfaceOrientation = false;
}

bool ULexUIMLActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (!AssetData.IsValid())return false;
	auto Asset = AssetData.GetAsset();
	auto Blueprint = Cast<UBlueprint>(Asset);
	if (Blueprint && Blueprint->GeneratedClass && Blueprint->GeneratedClass->IsChildOf(ULexUIMLBehaviour::StaticClass()))
	{
		return true;
	}

	return false;
}

bool ULexUIMLActorFactory::PreSpawnActor(UObject* Asset, FTransform& InOutLocation)
{
	auto Blueprint = Cast<UBlueprint>(Asset);
	if (Blueprint && Blueprint->GeneratedClass && Blueprint->GeneratedClass->IsChildOf(ULexUIMLBehaviour::StaticClass()))
	{
		return true;
	}
	return false;
}

AActor* ULexUIMLActorFactory::SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform,
	const FActorSpawnParameters& InSpawnParams)
{
	auto Actor = Super::SpawnActor(InAsset, InLevel, InTransform, InSpawnParams);
	auto UIMLPresenterComponent = Actor->FindComponentByClass<ULexUIMLPresenterComponent>();
	if (!UIMLPresenterComponent)
	{
		UIMLPresenterComponent = NewObject<ULexUIMLPresenterComponent>(Actor, ULexUIMLPresenterComponent::StaticClass());
		Actor->SetRootComponent(UIMLPresenterComponent);
		UIMLPresenterComponent->RegisterComponent();
		Actor->AddInstanceComponent(UIMLPresenterComponent);
	}
	UIMLPresenterComponent->bIsSpawnFromFactory = true;
	return Actor;
}

void ULexUIMLActorFactory::PostSpawnActor(UObject* Asset, AActor* InNewActor)
{
	Super::PostSpawnActor(Asset, InNewActor);

	auto Blueprint = Cast<UBlueprint>(Asset);
	auto LexUIMLClass = Cast<UClass>(Blueprint->GeneratedClass);
	
	auto WidgetPresenterComponent = InNewActor->FindComponentByClass<ULexUIMLPresenterComponent>();
	WidgetPresenterComponent->SetScriptClass(LexUIMLClass);
	
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

void ULexUIMLActorFactory::PostPlaceAsset(TArrayView<const FTypedElementHandle> InHandle,
	const FAssetPlacementInfo& InPlacementInfo, const FPlacementOptions& InPlacementOptions)
{
	Super::PostPlaceAsset(InHandle, InPlacementInfo, InPlacementOptions);
}

UObject* ULexUIMLActorFactory::GetAssetFromActorInstance(AActor* ActorInstance)
{
	auto WidgetPresenterComponent = ActorInstance->FindComponentByClass<ULexUIMLPresenterComponent>();
	check(WidgetPresenterComponent);
	return WidgetPresenterComponent->GetScriptClass();
}

UClass* ULexUIMLActorFactory::GetDefaultActorClass(const FAssetData& AssetData)
{
	auto Asset = AssetData.GetAsset();
	if (!Asset)return nullptr;
	auto Blueprint = Cast<UBlueprint>(Asset);
	if (!Blueprint)return nullptr;
	auto LexUIMLClass = Cast<UClass>(Blueprint->GeneratedClass);
	if (!LexUIMLClass)return nullptr;
	if (!LexUIMLClass->IsChildOf(ULexUIMLBehaviour::StaticClass()))return nullptr;
	auto LexUIML = GetDefault<ULexUIMLBehaviour>(LexUIMLClass);
	if (LexUIML)
	{
		FString ClassName;
		auto RenderMode = LexUIML->DefaultRenderMode;
		switch (RenderMode)
		{
		case ELexRenderMode::WorldSpace:
			ClassName = TEXT("WorldSpaceRoot_UERenderer");
			break;
		case ELexRenderMode::WorldSpace_LexUI:
			ClassName = TEXT("WorldSpaceRoot_LexRenderer");
			break;
		case ELexRenderMode::ScreenSpaceOverlay:
			ClassName = TEXT("ScreenSpaceRoot");
		}
		
		NewActorClass = LoadClass<AActor>(NULL, *FString::Printf(TEXT("/LGUI/Blueprints/XMLSupport/%s.%s_C"), *ClassName, *ClassName));
		if (!NewActorClass)
		{
			NewActorClass = AActor::StaticClass();
		}
		return NewActorClass;
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
