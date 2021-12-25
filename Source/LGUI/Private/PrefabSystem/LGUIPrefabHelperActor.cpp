// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "PrefabSystem/LGUIPrefabOverrideParameter.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
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

#if WITH_EDITORONLY_DATA
	PrefabHelperObject = CreateDefaultSubobject<ULGUIPrefabHelperObject>(TEXT("PrefabHelper"));
	PrefabHelperObject->bIsInsidePrefabEditor = false;
#endif
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

	if (PrefabHelperObject->LoadedRootActor.IsValid())
	{
		LGUIUtils::DestroyActorWithHierarchy(PrefabHelperObject->LoadedRootActor.Get());
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
	PrefabHelperObject->LoadPrefab(this->GetWorld(), InParent);
}

void ALGUIPrefabHelperActor::SavePrefab()
{
	PrefabHelperObject->SavePrefab();
}

void ALGUIPrefabHelperActor::RevertPrefab()
{
	PrefabHelperObject->RevertPrefab();
}

void ALGUIPrefabHelperActor::DeleteThisInstance()
{
	if (PrefabHelperObject->LoadedRootActor.IsValid())
	{
		LGUIUtils::DestroyActorWithHierarchy(PrefabHelperObject->LoadedRootActor.Get(), true);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("This actor is not a prefab, ignore this action"));
	}
	LGUIUtils::DestroyActorWithHierarchy(this, false);
}
#endif
