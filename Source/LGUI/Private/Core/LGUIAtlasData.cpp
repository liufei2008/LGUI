// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/LGUIAtlasData.h"
#include "LGUI.h"
#include "Core/LGUISettings.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/ActorComponent/UISpriteBase.h"

ULGUIAtlasManager* ULGUIAtlasManager::Instance = nullptr;
bool ULGUIAtlasManager::InitCheck()
{
	if (Instance == nullptr)
	{
		Instance = NewObject<ULGUIAtlasManager>();
		Instance->AddToRoot();
	}
	return true;
}
void ULGUIAtlasManager::BeginDestroy()
{
	ResetAtlasMap();
#if WITH_EDITOR
	ULGUISpriteData::MarkAllSpritesNeedToReinitialize();
#endif
	Instance = nullptr;
	Super::BeginDestroy();
}

const TMap<FName, FLGUIAtlasData>& ULGUIAtlasManager::GetAtlasMap()
{
	return atlasMap;
}
FLGUIAtlasData* ULGUIAtlasManager::FindOrAdd(const FName& packingTag)
{
	if (InitCheck())
	{
		return &(Instance->atlasMap.FindOrAdd(packingTag));
	}
	return nullptr;
}
FLGUIAtlasData* ULGUIAtlasManager::Find(const FName& packingTag)
{
	if (Instance != nullptr)
	{
		return Instance->atlasMap.Find(packingTag);
	}
	return nullptr;
}
void ULGUIAtlasManager::ResetAtlasMap()
{
	if (Instance != nullptr)
	{
		Instance->atlasMap.Reset();
		for (auto item : Instance->atlasMap)
		{
			item.Value.atlasTexture->RemoveFromRoot();
			item.Value.atlasTexture->ConditionalBeginDestroy();
		}
	}
}
