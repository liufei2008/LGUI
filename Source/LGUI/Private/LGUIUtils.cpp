// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/UIDrawcall.h"
#include "Sound/SoundBase.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#if WITH_EDITOR
#include "Editor.h"
#include "EditorStyleSet.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#endif

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
					world->EditorDestroyActor(item, true);
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
				world->EditorDestroyActor(Target, true);
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
void LGUIUtils::CollectChildrenActors(AActor* Target, TArray<AActor*>& AllChildrenActors, bool IncludeTarget)
{
	if (IncludeTarget)
	{
		AllChildrenActors.Add(Target);
	}
	TArray<AActor*> actorList;
	Target->GetAttachedActors(actorList);
	for (auto item : actorList)
	{
		CollectChildrenActors(item, AllChildrenActors, true);
	}
}
UTexture2D* LGUIUtils::CreateTexture(int32 InSize, FColor InDefaultColor, UObject* InOuter, FName InDefaultName)
{
	auto texture = NewObject<UTexture2D>(
		InOuter,
		InDefaultName,
		RF_Transient
		);
	auto PlatformData = new FTexturePlatformData();
	PlatformData->SizeX = InSize;
	PlatformData->SizeY = InSize;
	PlatformData->PixelFormat = PF_B8G8R8A8;
	texture->SetPlatformData(PlatformData);
	// Allocate first mipmap.
	int32 NumBlocksX = InSize / GPixelFormats[PF_B8G8R8A8].BlockSizeX;
	int32 NumBlocksY = InSize / GPixelFormats[PF_B8G8R8A8].BlockSizeY;
	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	texture->GetPlatformData()->Mips.Add(Mip);
	Mip->SizeX = InSize;
	Mip->SizeY = InSize;
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	void* dataPtr = Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[PF_B8G8R8A8].BlockBytes);
	FColor* pixelPtr = static_cast<FColor*>(dataPtr);
	for (int i = 0, count = InSize * InSize, pixelSize = sizeof(FColor); i < count; i++)
	{
		pixelPtr[i] = InDefaultColor;
	}
	Mip->BulkData.Unlock();
	return texture;
}


//find first Canvas in hierarchy
void LGUIUtils::FindRootCanvas(AActor* actor, ULGUICanvas*& resultCanvas)
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
		FindRootCanvas(parentActor, resultCanvas);
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

#if WITH_EDITOR
void LGUIUtils::NotifyPropertyChanged(UObject* Object, FProperty* Property)
{
	TArray<UObject*> ModifiedObjects;
	ModifiedObjects.Add(Object);
	FPropertyChangedEvent PropertyChangedEvent(Property, EPropertyChangeType::ValueSet, MakeArrayView(ModifiedObjects));
	Object->PostEditChangeProperty(PropertyChangedEvent);
	FEditPropertyChain PropertyChain;
	PropertyChain.AddHead(Property);
	FPropertyChangedChainEvent PropertyChangedChainEvent(PropertyChain, PropertyChangedEvent);
	Object->PostEditChangeChainProperty(PropertyChangedChainEvent);
}
void LGUIUtils::NotifyPropertyChanged(UObject* Object, FName PropertyName)
{
	auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
	NotifyPropertyChanged(Object, Property);
}
#endif

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
	if (!IsValid(GEditor))return;
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


void LGUIUtils::LogObjectFlags(UObject* obj)
{
	EObjectFlags of = obj->GetFlags();
	UE_LOG(LGUI, Log, TEXT("obj:%s\
\n	flagValue:%d\
\n	RF_Public:%d\
\n	RF_Standalone:%d\
\n	RF_MarkAsNative:%d\
\n	RF_Transactional:%d\
\n	RF_ClassDefaultObject:%d\
\n	RF_ArchetypeObject:%d\
\n	RF_Transient:%d\
\n	RF_MarkAsRootSet:%d\
\n	RF_TagGarbageTemp:%d\
\n	RF_NeedInitialization:%d\
\n	RF_NeedLoad:%d\
\n	RF_KeepForCooker:%d\
\n	RF_NeedPostLoad:%d\
\n	RF_NeedPostLoadSubobjects:%d\
\n	RF_NewerVersionExists:%d\
\n	RF_BeginDestroyed:%d\
\n	RF_FinishDestroyed:%d\
\n	RF_BeingRegenerated:%d\
\n	RF_DefaultSubObject:%d\
\n	RF_WasLoaded:%d\
\n	RF_TextExportTransient:%d\
\n	RF_LoadCompleted:%d\
\n	RF_InheritableComponentTemplate:%d\
\n	RF_DuplicateTransient:%d\
\n	RF_StrongRefOnFrame:%d\
\n	RF_NonPIEDuplicateTransient:%d\
\n	RF_WillBeLoaded:%d\
")
, *obj->GetPathName()
, obj->GetFlags()
, obj->HasAnyFlags(EObjectFlags::RF_Public)
, obj->HasAnyFlags(EObjectFlags::RF_Standalone)
, obj->HasAnyFlags(EObjectFlags::RF_MarkAsNative)
, obj->HasAnyFlags(EObjectFlags::RF_Transactional)
, obj->HasAnyFlags(EObjectFlags::RF_ClassDefaultObject)
, obj->HasAnyFlags(EObjectFlags::RF_ArchetypeObject)
, obj->HasAnyFlags(EObjectFlags::RF_Transient)
, obj->HasAnyFlags(EObjectFlags::RF_MarkAsRootSet)
, obj->HasAnyFlags(EObjectFlags::RF_TagGarbageTemp)
, obj->HasAnyFlags(EObjectFlags::RF_NeedInitialization)
, obj->HasAnyFlags(EObjectFlags::RF_NeedLoad)
, obj->HasAnyFlags(EObjectFlags::RF_KeepForCooker)
, obj->HasAnyFlags(EObjectFlags::RF_NeedPostLoad)
, obj->HasAnyFlags(EObjectFlags::RF_NeedPostLoadSubobjects)
, obj->HasAnyFlags(EObjectFlags::RF_NewerVersionExists)
, obj->HasAnyFlags(EObjectFlags::RF_BeginDestroyed)
, obj->HasAnyFlags(EObjectFlags::RF_FinishDestroyed)
, obj->HasAnyFlags(EObjectFlags::RF_BeingRegenerated)
, obj->HasAnyFlags(EObjectFlags::RF_DefaultSubObject)
, obj->HasAnyFlags(EObjectFlags::RF_WasLoaded)
, obj->HasAnyFlags(EObjectFlags::RF_TextExportTransient)
, obj->HasAnyFlags(EObjectFlags::RF_LoadCompleted)
, obj->HasAnyFlags(EObjectFlags::RF_InheritableComponentTemplate)
, obj->HasAnyFlags(EObjectFlags::RF_DuplicateTransient)
, obj->HasAnyFlags(EObjectFlags::RF_StrongRefOnFrame)
, obj->HasAnyFlags(EObjectFlags::RF_NonPIEDuplicateTransient)
, obj->HasAnyFlags(EObjectFlags::RF_WillBeLoaded)
);
}
