// Copyright 2019 LexLiu. All Rights Reserved.

#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/UIDrawcall.h"
#include "Sound/SoundBase.h"
#include "Core/ActorComponent/UIRenderable.h"

void LGUIUtils::DeleteActor(AActor* Target, bool WithHierarchy)
{
	if (!Target->IsValidLowLevelFast())
	{
		UE_LOG(LGUI, Error, TEXT("[LGUIUtils::DeleteActor]Try to delete not valid actor"));
		return;
	}
	if (WithHierarchy)
	{
		TArray<AActor*> AllChildrenActors;
		CollectChildrenActors(Target, AllChildrenActors);//collect all actor
		for (auto item : AllChildrenActors)
		{
#if WITH_EDITOR
			if (auto world = item->GetWorld())
			{
				if (world->WorldType == EWorldType::Editor || world->WorldType == EWorldType::EditorPreview)
				{
					world->EditorDestroyActor(item, false);
				}
				else
				{
					item->Destroy();
				}
			}
#else
			item->Destroy();
#endif
		}
	}
	else
	{
#if WITH_EDITOR
		if (auto world = Target->GetWorld())
		{
			if (world->WorldType == EWorldType::Editor || world->WorldType == EWorldType::EditorPreview)
			{
				world->EditorDestroyActor(Target, false);
			}
			else
			{
				Target->Destroy();
			}
		}
#else
		Target->Destroy();
#endif
	}
}
void LGUIUtils::CollectChildrenActors(AActor* Target, TArray<AActor*>& AllChildrenActors)
{
	AllChildrenActors.Add(Target);
	TArray<AActor*> actorList;
	Target->GetAttachedActors(actorList);
	for (auto item : actorList)
	{
		CollectChildrenActors(item, AllChildrenActors);
	}
}
UTexture2D* LGUIUtils::CreateTransientBlackTransparentTexture(int32 InSize)
{
	auto texture = NewObject<UTexture2D>(
		GetTransientPackage(),
		NAME_None,
		RF_Transient
		);
	texture->PlatformData = new FTexturePlatformData();
	texture->PlatformData->SizeX = InSize;
	texture->PlatformData->SizeY = InSize;
	texture->PlatformData->PixelFormat = PF_B8G8R8A8;
	// Allocate first mipmap.
	int32 NumBlocksX = InSize / GPixelFormats[PF_B8G8R8A8].BlockSizeX;
	int32 NumBlocksY = InSize / GPixelFormats[PF_B8G8R8A8].BlockSizeY;
	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	texture->PlatformData->Mips.Add(Mip);
	Mip->SizeX = InSize;
	Mip->SizeY = InSize;
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	void* dataPtr = Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[PF_B8G8R8A8].BlockBytes);
	FMemory::Memset(dataPtr, 0, Mip->BulkData.GetBulkDataSize());
	Mip->BulkData.Unlock();
	return texture;
}


void LGUIUtils::SortUIItemDepth(TArray<UUIRenderable*>& shapeList)
{
	shapeList.Sort([](const UUIRenderable& A, const UUIRenderable& B)
	{
		return A.GetDepth() < B.GetDepth();
	});
}
void LGUIUtils::SortUIItemDepth(TArray<TSharedPtr<UIGeometry>>& shapeList)
{
	shapeList.Sort([](const TSharedPtr<UIGeometry>& A, const TSharedPtr<UIGeometry>& B)
	{
		return A->depth < B->depth;
	});
}

void LGUIUtils::CreateDrawcallFast(TArray<UUIRenderable*>& sortedList, TArray<TSharedPtr<UUIDrawcall>>& drawcallList)
{
	UTexture* prevTex = nullptr;
	TSharedPtr<UUIDrawcall> prevUIDrawcall = nullptr;
	auto shapeCount = sortedList.Num();
	int drawcallCount = 0;
	int prevDrawcallListCount = drawcallList.Num();
	for (int i = 0; i < shapeCount; i++)
	{
		auto itemGeo = sortedList[i]->GetGeometry();
		if (itemGeo.Get() == nullptr)continue;
		if (itemGeo->material != nullptr)//consider every custom material as a drawcall
		{
			prevUIDrawcall = GetAvalibleDrawcall(drawcallList, prevDrawcallListCount, drawcallCount);
			prevUIDrawcall->texture = itemGeo->texture;
			prevUIDrawcall->material = itemGeo->material;
			prevUIDrawcall->isFontTexture = itemGeo->isFontTexture;
			prevUIDrawcall->geometryList.Add(itemGeo);
			prevTex = nullptr;
		}
		else//batch elements into drawcall by comparing their texture
		{
			auto itemTex = itemGeo->texture;
			if (itemTex != prevTex)//this ui element's texture is different from previous one
			{
				prevUIDrawcall = GetAvalibleDrawcall(drawcallList, prevDrawcallListCount, drawcallCount);
				prevUIDrawcall->texture = itemTex;
				prevUIDrawcall->isFontTexture = itemGeo->isFontTexture;
				prevUIDrawcall->geometryList.Add(itemGeo);
			}
			else//same texture means same drawcall
			{
				if (!prevUIDrawcall.IsValid())//if first is null
				{
					prevUIDrawcall = GetAvalibleDrawcall(drawcallList, prevDrawcallListCount, drawcallCount);
					prevUIDrawcall->texture = itemTex;
					prevUIDrawcall->isFontTexture = itemGeo->isFontTexture;
					prevUIDrawcall->geometryList.Add(itemGeo);
				}
				else
				{
					prevUIDrawcall->geometryList.Add(itemGeo);
				}
			}
			prevTex = itemTex;
		}
		itemGeo->drawcallIndex = drawcallCount - 1;
	}
	while (prevDrawcallListCount > drawcallCount)//delete needless drawcall
	{
		drawcallList.RemoveAt(prevDrawcallListCount - 1);
		prevDrawcallListCount--;
	}
}

//find first Canvas in hierarchy
void LGUIUtils::FindTopMostCanvas(AActor* actor, ULGUICanvas*& resultCanvas)
{
	auto tempComp = GetComponentInParent<ULGUICanvas>(actor);
	if (tempComp != nullptr)
	{
		resultCanvas = tempComp;
	}

	auto parentActor = actor->GetAttachParentActor();
	if (parentActor != nullptr)
	{
		FindTopMostCanvas(parentActor, resultCanvas);
	}
}

void LGUIUtils::FindParentCanvas(AActor* actor, ULGUICanvas*& resultCanvas)
{
	auto parentActor = actor->GetAttachParentActor();
	if (parentActor != nullptr)
	{
		auto tempComp = GetComponentInParent<ULGUICanvas>(parentActor);
		if (tempComp != nullptr)
		{
			resultCanvas = tempComp;
		}
		return;
	}
}

float LGUIUtils::INV_255 = 1.0f / 255.0f;
FColor LGUIUtils::MultiplyColor(FColor A, FColor B)
{
	FColor result;
	result.R = (uint8)(A.R * B.R * INV_255);
	result.G = (uint8)(A.G * B.G * INV_255);
	result.B = (uint8)(A.B * B.B * INV_255);
	result.A = (uint8)(A.A * B.A * INV_255);
	return result;
}

#if WITH_EDITOR
//nodify some informations in editor
void LGUIUtils::EditorNotification(FText NofityText)
{
	FNotificationInfo Info(NofityText);
	Info.Image = FEditorStyle::GetBrush(TEXT("LevelEditor.RecompileGameCode"));
	Info.FadeInDuration = 0.1f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = 5.0f;
	Info.bUseThrobber = false;
	Info.bUseSuccessFailIcons = true;
	Info.bUseLargeFont = true;
	Info.bFireAndForget = false;
	Info.bAllowThrottleWhenFrameRateIsLow = false;
	auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
	NotificationItem->ExpireAndFadeout();

	auto CompileFailSound = LoadObject<USoundBase>(NULL, TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
	GEditor->PlayEditorSound(CompileFailSound);
}
#endif
TSharedPtr<UUIDrawcall> LGUIUtils::GetAvalibleDrawcall(TArray<TSharedPtr<UUIDrawcall>>& drawcallList, int& prevDrawcallListCount, int& drawcallCount)
{
	TSharedPtr<UUIDrawcall> result;
	drawcallCount++;
	if (drawcallCount > prevDrawcallListCount)
	{
		result = TSharedPtr<UUIDrawcall>(new UUIDrawcall());
		drawcallList.Add(result);
		prevDrawcallListCount++;
	}
	else
	{
		result = drawcallList[drawcallCount - 1];
		if (result.IsValid())
			result->Clear();
	}
	return result;
}