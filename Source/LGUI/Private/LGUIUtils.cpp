// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Utils/LGUIUtils.h"
#include "Utils/BitConverter.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/UIDrawcall.h"
#include "Sound/SoundBase.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Core/ActorComponent/UIPostProcess.h"
#include "Core/ActorComponent/UIRenderable.h"

void LGUIUtils::DeleteActor(AActor* Target, bool WithHierarchy)
{
	DestroyActorWithHierarchy(Target, WithHierarchy);
}
void LGUIUtils::DestroyActorWithHierarchy(AActor* Target, bool WithHierarchy)
{
	if (!Target->IsValidLowLevelFast())
	{
		UE_LOG(LGUI, Error, TEXT("[LGUIUtils::DestroyActorWithHierarchy]Try to delete not valid actor"));
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
UTexture2D* LGUIUtils::CreateTransientBlackTransparentTexture(int32 InSize, FName InDefaultName)
{
	auto texture = NewObject<UTexture2D>(
		GetTransientPackage(),
		InDefaultName,
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


void LGUIUtils::CreateDrawcall(TArray<TWeakObjectPtr<UUIBaseRenderable>>& sortedList, TArray<TSharedPtr<UUIDrawcall>>& drawcallList)
{
	UTexture* prevTex = nullptr;
	TSharedPtr<UUIDrawcall> prevUIDrawcall = nullptr;
	int drawcallCount = 0;
	int prevDrawcallListCount = drawcallList.Num();
	for (int i = 0; i < sortedList.Num(); i++)
	{
		switch (sortedList[i]->GetUIRenderableType())
		{
		default:
		case EUIRenderableType::UIGeometryRenderable:
		{
			auto sortedItem = (UUIRenderable*)sortedList[i].Get();
			auto itemGeo = sortedItem->GetGeometry();
			if (itemGeo.IsValid() == false)continue;
			if (itemGeo->vertices.Num() == 0)continue;

			if (itemGeo->material.IsValid())//consider every custom material as a drawcall
			{
				prevUIDrawcall = GetAvalibleDrawcall(drawcallList, prevDrawcallListCount, drawcallCount);
				prevUIDrawcall->texture = itemGeo->texture;
				prevUIDrawcall->material = itemGeo->material;
				prevUIDrawcall->geometryList.Add(itemGeo);
				prevUIDrawcall->type = EUIDrawcallType::Geometry;
				prevTex = nullptr;
			}
			else//batch elements into drawcall by comparing their texture
			{
				auto itemTex = itemGeo->texture;
				if (itemTex.Get() != prevTex)//this ui element's texture is different from previous one
				{
					prevUIDrawcall = GetAvalibleDrawcall(drawcallList, prevDrawcallListCount, drawcallCount);
					prevUIDrawcall->texture = itemTex;
					prevUIDrawcall->geometryList.Add(itemGeo);
					prevUIDrawcall->type = EUIDrawcallType::Geometry;
				}
				else//same texture means same drawcall
				{
					if (!prevUIDrawcall.IsValid())//if first is null
					{
						prevUIDrawcall = GetAvalibleDrawcall(drawcallList, prevDrawcallListCount, drawcallCount);
						prevUIDrawcall->texture = itemTex;
						prevUIDrawcall->geometryList.Add(itemGeo);
					}
					else
					{
						prevUIDrawcall->geometryList.Add(itemGeo);
					}
					prevUIDrawcall->type = EUIDrawcallType::Geometry;
				}
				prevTex = itemTex.Get();
			}
			itemGeo->drawcallIndex = drawcallCount - 1;
		}
		break;
		case EUIRenderableType::UIPostProcessRenderable:
		{
			auto sortedItem = (UUIPostProcess*)sortedList[i].Get();
			//every postprocess is a drawcall
			prevUIDrawcall = GetAvalibleDrawcall(drawcallList, prevDrawcallListCount, drawcallCount);
			prevUIDrawcall->postProcessObject = sortedItem;
			prevUIDrawcall->type = EUIDrawcallType::PostProcess;
			prevUIDrawcall = nullptr;
			prevTex = nullptr;
		}
		break;
		}
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
	if (!IsValid(actor))return;
	auto tempComp = GetComponentInParent<ULGUICanvas>(actor, false);
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
	if (!IsValid(actor))return;
	auto parentActor = actor->GetAttachParentActor();
	if (parentActor != nullptr)
	{
		auto tempComp = GetComponentInParent<ULGUICanvas>(parentActor, false);
		if (tempComp != nullptr)
		{
			resultCanvas = tempComp;
		}
		return;
	}
}

float LGUIUtils::INV_255 = 1.0f / 255.0f;

FColor LGUIUtils::ColorHSVDataToColorRGB(FVector InHSVColor)
{
	FLinearColor colorHSV(InHSVColor);
	return colorHSV.HSVToLinearRGB().ToFColor(false);
}
FVector LGUIUtils::ColorRGBToColorHSVData(FColor InRGBColor)
{
	auto linearColorRGB = FLinearColor(InRGBColor.R * INV_255, InRGBColor.G * INV_255, InRGBColor.B * INV_255, 1.0f);
	auto linearColorHSV = linearColorRGB.LinearRGBToHSV();
	return FVector(linearColorHSV);
}

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
void LGUIUtils::EditorNotification(FText NofityText, float ExpireDuration)
{
	FNotificationInfo Info(NofityText);
	Info.Image = FEditorStyle::GetBrush(TEXT("LevelEditor.RecompileGameCode"));
	Info.FadeInDuration = 0.1f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = ExpireDuration;
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

const TCHAR BitConverter::ErrorMsgFormat[] = TEXT("[BitConvert/%s]bytes.Num %d is not enough! \
If this happens when loading a LGUIPrefab, then open that prefab and click \"RecreateThis\" can fix it. \
If this problem still exist, please contact the plugin author.");
