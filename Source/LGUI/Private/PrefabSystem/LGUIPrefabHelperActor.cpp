// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "PrefabSystem/LGUIPrefabHelperComponent.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "Core/ActorComponent/UIItem.h"
#if WITH_EDITOR
#include "Editor.h"
#include "EditorActorFolders.h"
#include "Core/Actor/LGUIManagerActor.h"
#endif


// Sets default values
ALGUIPrefabHelperActor::ALGUIPrefabHelperActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIsEditorOnlyActor = true;
}

void ALGUIPrefabHelperActor::OnConstruction(const FTransform& Transform)
{
#if WITH_EDITORONLY_DATA
	if (AllColors.Num() == 0)
	{
		int ColorCount = 10;
		float Interval = 1.0f / ColorCount;
		float StartHue01 = 0;
		for (int i = 0; i < ColorCount; i++)
		{
			auto Hue = (uint8)(StartHue01 * 255);
			auto Color1 = FLinearColor::MakeFromHSV8(Hue, 255, 255).ToFColor(false);
			AllColors.Add(Color1);
			auto Color2 = FLinearColor::MakeFromHSV8(Hue, 255, 128).ToFColor(false);
			AllColors.Add(Color2);
			StartHue01 += Interval;
		}
	}

	{
		if (AllColors.Num() == 0)
		{
			IdentityColor = FColor::MakeRandomColor();
			IsRandomColor = true;
		}
		else
		{
			int RandomIndex = FMath::RandRange(0, AllColors.Num() - 1);
			IdentityColor = AllColors[RandomIndex];
			AllColors.RemoveAt(RandomIndex);
			IsRandomColor = false;
		}
	}
#endif
}

void ALGUIPrefabHelperActor::Destroyed()
{
	Super::Destroyed();
#if WITH_EDITORONLY_DATA
	{
		if (!IsRandomColor)
		{
			if (!AllColors.Contains(IdentityColor))
			{
				AllColors.Add(IdentityColor);
			}
		}
	}

	if (IsValid(LoadedRootActor))
	{
		LGUIUtils::DestroyActorWithHierarchy(LoadedRootActor);
	}
#endif
}

#if WITH_EDITORONLY_DATA
FName ALGUIPrefabHelperActor::PrefabFolderName(TEXT("--LGUIPrefabActor--"));
TArray<FColor> ALGUIPrefabHelperActor::AllColors;
#endif

#if WITH_EDITOR
void ALGUIPrefabHelperActor::MoveActorToPrefabFolder()
{
	FActorFolders::Get().CreateFolder(*this->GetWorld(), PrefabFolderName);
	this->SetFolderPath(PrefabFolderName);
}


void ALGUIPrefabHelperActor::LoadPrefab(USceneComponent* InParent)
{
	if (!IsValid(LoadedRootActor))
	{
		ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;

		LoadedRootActor = PrefabAsset->LoadPrefabForEdit(this->GetWorld()
			, InParent
			, MapGuidToObject, SubPrefabMap);
		AllLoadedActorArray.Empty();
		for (auto KeyValue : MapGuidToObject)
		{
			if (auto Actor = Cast<AActor>(KeyValue.Value))
			{
				AllLoadedActorArray.Add(Actor);
			}
		}

		FActorFolders::Get().CreateFolder(*this->GetWorld(), PrefabFolderName);
		this->SetFolderPath(PrefabFolderName);

		ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;
	}
}

void ALGUIPrefabHelperActor::SavePrefab()
{
	if (PrefabAsset)
	{
		TMap<UObject*, FGuid> MapObjectToGuid;
		for (auto KeyValue : MapGuidToObject)
		{
			if (IsValid(KeyValue.Value))
			{
				MapObjectToGuid.Add(KeyValue.Value, KeyValue.Key);
			}
		}
		PrefabAsset->SavePrefab(LoadedRootActor, MapObjectToGuid, SubPrefabMap);
		MapGuidToObject.Empty();
		AllLoadedActorArray.Empty();
		for (auto KeyValue : MapObjectToGuid)
		{
			MapGuidToObject.Add(KeyValue.Value, KeyValue.Key);
			if (auto Actor = Cast<AActor>(KeyValue.Key))
			{
				AllLoadedActorArray.Add(Actor);
			}
		}
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("PrefabAsset is null, please create a LGUIPrefab asset and assign to PrefabAsset"));
	}
}

void ALGUIPrefabHelperActor::RevertPrefab()
{
	if (IsValid(PrefabAsset))
	{
		AActor* OldParentActor = nullptr;
		bool haveRootTransform = true;
		FTransform OldTransform;
		FUIWidget OldWidget;
		//store root transform
		if (IsValid(LoadedRootActor))
		{
			OldParentActor = LoadedRootActor->GetAttachParentActor();
			if (auto RootComp = LoadedRootActor->GetRootComponent())
			{
				haveRootTransform = true;
				OldTransform = LoadedRootActor->GetRootComponent()->GetRelativeTransform();
				if (auto RootUIComp = Cast<UUIItem>(RootComp))
				{
					OldWidget = RootUIComp->GetWidget();
				}
			}
		}
		//Revert exsiting objects with parameters inside prefab
		{
			LoadedRootActor = nullptr;
			LoadPrefab(nullptr);
		}

		if (IsValid(LoadedRootActor))
		{
			LoadedRootActor->AttachToActor(OldParentActor, FAttachmentTransformRules::KeepRelativeTransform);
			if (haveRootTransform)
			{
				if (auto RootComp = LoadedRootActor->GetRootComponent())
				{
					RootComp->SetRelativeTransform(OldTransform);
					if (auto RootUIItem = Cast<UUIItem>(RootComp))
					{
						auto newWidget = RootUIItem->GetWidget();
						RootUIItem->SetWidget(OldWidget);
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("PrefabAsset is null, please create a LGUIPrefab asset and assign to PrefabAsset"));
	}
}

void ALGUIPrefabHelperActor::DeleteThisInstance()
{
	if (LoadedRootActor)
	{
		LGUIUtils::DestroyActorWithHierarchy(LoadedRootActor, true);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("This actor is not a prefab, ignore this action"));
	}
	LGUIUtils::DestroyActorWithHierarchy(this, false);
}

void ALGUIPrefabHelperActor::CleanupPrefabAndActor()
{
	//Cleanup AllLoadedActorArray, remove it if not under root actor
	for (int i = AllLoadedActorArray.Num() - 1; i >= 0; i--)
	{
		auto ActorItem = AllLoadedActorArray[i];
		if (!IsValid(ActorItem)//not valid
			|| (!ActorItem->IsAttachedTo(LoadedRootActor) && ActorItem != LoadedRootActor)//not attached to this prefab
			)
		{
			AllLoadedActorArray.RemoveAt(i);
		}
	}
}
#endif
