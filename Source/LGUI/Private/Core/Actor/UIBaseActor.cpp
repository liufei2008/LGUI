// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Actor/UIBaseActor.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"
#if WITH_EDITOR
#include "PrefabSystem/LGUIPrefabManager.h"
#include "Utils/LGUIUtils.h"
#endif


AUIBaseActor::AUIBaseActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}

#if WITH_EDITOR
AActor* AUIBaseActor::FirstTemporarilyHiddenActor = nullptr;
void AUIBaseActor::SetIsTemporarilyHiddenInEditor(bool bIsHidden)
{
	if (ULGUIPrefabWorldSubsystem::IsLGUIPrefabSystemProcessingActor(this))//when deserialize from prefab, no need to set it because everything is done when serialize it
	{

	}
	else
	{
		if (FirstTemporarilyHiddenActor == nullptr)
		{
			//when click on editor outliner's eye button, only need to set first UI Actor (the editing one)
			FirstTemporarilyHiddenActor = this;
			if (IsTemporarilyHiddenInEditor() != bIsHidden)
			{
				bool bShouldNotify = false;
				if (bIsHidden)
				{
					if (GetUIItem()->GetIsUIActiveSelf() != false)
					{
						bShouldNotify = true;
					}
					GetUIItem()->SetIsUIActive(false);
				}
				else
				{
					if (GetUIItem()->GetIsUIActiveSelf() != true)
					{
						bShouldNotify = true;
					}
					GetUIItem()->SetIsUIActive(true);
				}
				if (bShouldNotify)
				{
					LGUIUtils::NotifyPropertyChanged(GetUIItem(), FName(TEXT("bIsUIActive")));
				}
			}
			ULGUIPrefabManagerObject::AddOneShotTickFunction([WeakThis = MakeWeakObjectPtr(this)] {
				FirstTemporarilyHiddenActor = nullptr;
				if (WeakThis.IsValid())
				{
					WeakThis->GetUIItem()->SetIsTemporarilyHiddenInEditor_Recursive_By_IsUIActiveState();//restore Temporary hidden state by UI item's IsUIActive state.
				}
				});
		}
	}

	Super::SetIsTemporarilyHiddenInEditor(bIsHidden);
}
#endif

AUIBaseRenderableActor::AUIBaseRenderableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}
UUIItem* AUIBaseRenderableActor::GetUIItem()const 
{
	return GetUIRenderable(); 
}

AUIBasePostProcessActor::AUIBasePostProcessActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}
UUIBaseRenderable* AUIBasePostProcessActor::GetUIRenderable()const 
{
	return GetUIPostProcessRenderable(); 
}

