// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializer3.h"
#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "Components/PrimitiveComponent.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "LGUI.h"
#if WITH_EDITOR
#include "Tools/UEdMode.h"
#include "Utils/LGUIUtils.h"
#endif

PRAGMA_DISABLE_OPTIMIZATION
namespace LGUIPrefabSystem3
{
	AActor* ActorSerializer3::DuplicateActor(AActor* RootActor, USceneComponent* Parent)
	{
		if (!RootActor)
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer3::DuplicateActor]RootActor Or InPrefab is null!"));
			return nullptr;
		}
		if (!RootActor->GetWorld())
		{
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer3::DuplicateActor]Cannot get World from RootActor!"));
			return nullptr;
		}
		ActorSerializer3 serializer(RootActor->GetWorld());
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		return serializer.SerializeActor_ForDuplicate(RootActor, Parent);
	}

	AActor* ActorSerializer3::SerializeActor_ForDuplicate(AActor* RootActor, USceneComponent* Parent)
	{
		auto StartTime = FDateTime::Now();

		//serialize
		WriterOrReaderFunction = [this](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIDuplicateObjectWriter Writer(InObject, InOutBuffer, *this, ExcludeProperties);
		};
		FLGUIPrefabSaveData SaveData;
		SerializeActorToData(RootActor, SaveData);

		//deserialize
		WriterOrReaderFunction = [this](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? GetSceneComponentExcludeProperties() : TSet<FName>();
			FLGUIDuplicateObjectReader Reader(InObject, InOutBuffer, *this, ExcludeProperties);
		};
		auto CreatedRootActor = DeserializeActorFromData(SaveData, Parent, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);

		auto TimeSpan = FDateTime::Now() - StartTime;
		auto Name =
#if WITH_EDITOR
			RootActor->GetActorLabel();
#else
			RootActor->GetPathName();
#endif
		UE_LOG(LGUI, Log, TEXT("Take %fs duplicating actor: %s"), TimeSpan.GetTotalSeconds(), *Name);
		return CreatedRootActor;
	}
}
PRAGMA_ENABLE_OPTIMIZATION
