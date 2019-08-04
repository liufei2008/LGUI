// Copyright 2019 LexLiu. All Rights Reserved.

#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/UIPanel.h"
#include "Sound/SoundBase.h"

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

void LGUIUtils::CreateDrawcallFast(TArray<UUIRenderable*>& sortedList, TArray<UUIDrawcall*>& drawcallList)
{
	UTexture* prevTex = nullptr;
	UUIDrawcall* prevUIDrawcall = nullptr;
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
				if (prevUIDrawcall == nullptr)//if first is null
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
		auto last = drawcallList[prevDrawcallListCount - 1];
		delete last;
		drawcallList.RemoveAt(prevDrawcallListCount - 1);
		prevDrawcallListCount--;
	}
}

//find first UIPanel in hierarchy
void LGUIUtils::FindFirstUIPanel(UUIItem* uiItem, UUIPanel*& resultUIPanel)
{
	auto itemType = uiItem->GetUIItemType();
	if (itemType == UIItemType::UIPanel)
	{
		resultUIPanel = (UUIPanel*)uiItem;
	}

	auto parent = uiItem->GetAttachParent();
	if (parent != nullptr)
	{
		UUIItem* parentItem = Cast<UUIItem>(parent);
		if (parentItem != nullptr)
		{
			FindFirstUIPanel(parentItem, resultUIPanel);
		}
	}
}
//find UIPanel which response for render the UIItem
void LGUIUtils::FindRenderUIPanel(UUIItem* uiItem, UUIPanel*& resultUIPanel)
{
	auto itemType = uiItem->GetUIItemType();
	if (itemType == UIItemType::UIPanel)
	{
		resultUIPanel = (UUIPanel*)uiItem;
		return;
	}

	auto parent = uiItem->GetAttachParent();
	if (parent != nullptr)
	{
		UUIItem* parentItem = Cast<UUIItem>(parent);
		if (parentItem != nullptr)
		{
			FindRenderUIPanel(parentItem, resultUIPanel);
		}
	}
}
void LGUIUtils::FindParentUIPanel(UUIItem* uiItem, UUIPanel*& resultUIPanel)
{
	auto parent = uiItem->GetAttachParent();
	if (parent != nullptr)
	{
		if (auto parentUIItem = Cast<UUIItem>(parent))
		{
			if (parentUIItem->GetUIItemType() == UIItemType::UIPanel)
			{
				resultUIPanel = (UUIPanel*)parentUIItem;
			}
			else
			{
				FindParentUIPanel(parentUIItem, resultUIPanel);
			}
		}
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
UUIDrawcall* LGUIUtils::GetAvalibleDrawcall(TArray<UUIDrawcall*>& drawcallList, int& prevDrawcallListCount, int& drawcallCount)
{
	UUIDrawcall* result;
	drawcallCount++;
	if (drawcallCount > prevDrawcallListCount)
	{
		result = new UUIDrawcall();
		drawcallList.Add(result);
		prevDrawcallListCount++;
	}
	else
	{
		result = drawcallList[drawcallCount - 1];
		if (result != nullptr)
			result->Clear();
	}
	return result;
}