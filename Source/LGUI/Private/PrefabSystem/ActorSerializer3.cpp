// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializer3.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"

namespace LGUIPrefabSystem3
{
	FLGUISubPrefabDefaultOverrideParameter::FLGUISubPrefabDefaultOverrideParameter(USceneComponent* RootComp)
	{
		auto UIItem = Cast<UUIItem>(RootComp);
		this->bIsUI = UIItem != nullptr;
#if WITH_EDITOR
		this->ActorLabel = RootComp->GetOwner()->GetActorLabel();
#endif
		this->Location = UIItem->GetRelativeLocation();
		this->Rotation = UIItem->GetRelativeRotation();
		this->Scale = UIItem->GetRelativeScale3D();
		if (bIsUI)
		{
			this->displayName = UIItem->GetDisplayName();
			auto widget = UIItem->GetWidget();
			this->pivot = widget.pivot;
			this->anchorHAlign = (uint8)widget.anchorHAlign;
			this->anchorVAlign = (uint8)widget.anchorVAlign;
			this->anchorOffsetX = widget.anchorOffsetX;
			this->anchorOffsetY = widget.anchorOffsetY;
			this->width = widget.width;
			this->height = widget.height;
			this->stretchLeft = widget.stretchLeft;
			this->stretchRight = widget.stretchRight;
			this->stretchTop = widget.stretchTop;
			this->stretchBottom = widget.stretchBottom;
		}
	}
	void FLGUISubPrefabDefaultOverrideParameter::ApplyToTarget(USceneComponent* RootComp)
	{
#if WITH_EDITOR
		RootComp->GetOwner()->SetActorLabel(this->ActorLabel);
#endif
		RootComp->SetRelativeLocationAndRotation(this->Location, this->Rotation);
		RootComp->SetRelativeScale3D(this->Scale);
		auto UIItem = Cast<UUIItem>(RootComp);
		if (this->bIsUI && UIItem != nullptr)
		{
			UIItem->SetDisplayName(this->displayName);
			FUIWidget widget;
			widget.pivot = this->pivot;
			widget.anchorHAlign = (UIAnchorHorizontalAlign)this->anchorHAlign;
			widget.anchorVAlign = (UIAnchorVerticalAlign)this->anchorVAlign;
			widget.anchorOffsetX = this->anchorOffsetX;
			widget.anchorOffsetY = this->anchorOffsetY;
			widget.width = this->width;
			widget.height = this->height;
			widget.stretchLeft = this->stretchLeft;
			widget.stretchRight = this->stretchRight;
			widget.stretchTop = this->stretchTop;
			widget.stretchBottom = this->stretchBottom;
			UIItem->SetWidget(widget);
		}
	}



	ActorSerializer3::ActorSerializer3(UWorld* InTargetWorld)
	{
		TargetWorld = TWeakObjectPtr<UWorld>(InTargetWorld);
	}


	int32 ActorSerializer3::FindOrAddAssetIdFromList(UObject* AssetObject)
	{
		if (!AssetObject)return -1;
		int32 resultIndex;
		if (ReferenceAssetList.Find(AssetObject, resultIndex))
		{
			return resultIndex;//return index if found
		}
		else//add to list if not found
		{
			ReferenceAssetList.Add(AssetObject);
			return ReferenceAssetList.Num() - 1;
		}
	}

	int32 ActorSerializer3::FindOrAddClassFromList(UClass* Class)
	{
		if (!Class)return -1;
		int32 resultIndex;
		if (ReferenceClassList.Find(Class, resultIndex))
		{
			return resultIndex;
		}
		else
		{
			ReferenceClassList.Add(Class);
			return ReferenceClassList.Num() - 1;
		}
	}
	int32 ActorSerializer3::FindOrAddNameFromList(const FName& Name)
	{
		if (!Name.IsValid())return -1;
		int32 resultIndex;
		if (ReferenceNameList.Find(Name, resultIndex))
		{
			return resultIndex;
		}
		else
		{
			ReferenceNameList.Add(Name);
			return ReferenceNameList.Num() - 1;
		}
	}
	FName ActorSerializer3::FindNameFromListByIndex(int32 Id)
	{
		int32 count = ReferenceNameList.Num();
		if (Id >= count || Id < 0)
		{
			return NAME_None;
		}
		return ReferenceNameList[Id];
	}

	UObject* ActorSerializer3::FindAssetFromListByIndex(int32 Id)
	{
		int32 count = ReferenceAssetList.Num();
		if (Id >= count || Id < 0)
		{
			return nullptr;
		}
		return ReferenceAssetList[Id];
	}

	UClass* ActorSerializer3::FindClassFromListByIndex(int32 Id)
	{
		int32 count = ReferenceClassList.Num();
		if (Id >= count || Id < 0)
		{
			return nullptr;
		}
		return ReferenceClassList[Id];
	}

	const TSet<FName>& ActorSerializer3::GetSceneComponentExcludeProperties()
	{
		static TSet<FName> result = { FName("AttachParent") };
		return result;
	}

	static bool CanUseUnversionedPropertySerialization()
	{
		bool bTemp;
		static bool bAllow = GConfig->GetBool(TEXT("Core.System"), TEXT("CanUseUnversionedPropertySerialization"), bTemp, GEngineIni) && bTemp;
		return bAllow;
	}

	void ActorSerializer3::SetupArchive(FArchive& InArchive)
	{
		if (!bIsEditorOrRuntime && CanUseUnversionedPropertySerialization())
		{
			InArchive.SetUseUnversionedPropertySerialization(true);
		}
		InArchive.SetFilterEditorOnly(!bIsEditorOrRuntime);
		InArchive.SetWantBinaryPropertySerialization(!bIsEditorOrRuntime);
	}
}
