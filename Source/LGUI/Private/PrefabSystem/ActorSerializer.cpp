// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializer.h"
#include "PrefabSystem/LGUIPrefabHelperComponent.h"
#include "Utils/BitConverter.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "Components/PrimitiveComponent.h"
#include "UObject/TextProperty.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Core/Actor/LGUIManagerActor.h"


ActorSerializer::ActorSerializer(UWorld* InTargetWorld)
{
	TargetWorld = TWeakObjectPtr<UWorld>(InTargetWorld);
}
#if WITH_EDITOR
AActor* ActorSerializer::LoadPrefabForEdit(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, TArray<AActor*>& AllLoadedActorArray)
{
	ActorSerializer serializer(InWorld);
	serializer.IsLoadForEdit = true;
	auto rootActor = serializer.DeserializeActor(Parent, InPrefab, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector, true);
	AllLoadedActorArray = serializer.CreatedActors;
	return rootActor;
}
AActor* ActorSerializer::LoadPrefabForEdit(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity)
{
	ActorSerializer serializer(InWorld);
	serializer.IsLoadForEdit = true;
	if (SetRelativeTransformToIdentity)
	{
		return serializer.DeserializeActor(Parent, InPrefab, true, FVector::ZeroVector, FQuat::Identity, FVector::OneVector, true);
	}
	else
	{
		return serializer.DeserializeActor(Parent, InPrefab);
	}
}
#endif
AActor* ActorSerializer::LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity)
{
	ActorSerializer serializer(InWorld);
	AActor* result = nullptr;
	if (SetRelativeTransformToIdentity)
	{
		result = serializer.DeserializeActor(Parent, InPrefab, true);
	}
	else
	{
		result = serializer.DeserializeActor(Parent, InPrefab);
	}
	return result;
}
AActor* ActorSerializer::LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, FVector RelativeLocation, FQuat RelativeRotation, FVector RelativeScale)
{
	ActorSerializer serializer(InWorld);
	return serializer.DeserializeActor(Parent, InPrefab, true, RelativeLocation, RelativeRotation, RelativeScale);
}

AActor* ActorSerializer::DeserializeActor(USceneComponent* Parent, ULGUIPrefab* InPrefab, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale, bool ForceUseEditorData)
{
	if (!InPrefab)
	{
		UE_LOG(LGUI, Error, TEXT("Load Prefab, InPrefab is null!"));
		return nullptr;
	}
	if (!TargetWorld.IsValid())
	{
		UE_LOG(LGUI, Error, TEXT("Load Prefab, World is null!"));
		return nullptr;
	}
#if WITH_EDITORONLY_DATA
	if (!IsLoadForEdit)
#endif
	{
		if (InPrefab->EngineMajorVersion != ENGINE_MAJOR_VERSION || InPrefab->EngineMinorVersion != ENGINE_MINOR_VERSION)
		{
			UE_LOG(LGUI, Warning, TEXT("This prefab is made by a different engine version, this may cause crash, please rebuild the prefab.\n\
	You can double click on the prefab asset and click \"RecreateThis\" button.\n\
	Prefab:%s.\n\
	Prefab engine version:%d.%d, current engine version:%d.%d")
				, *InPrefab->GetPathName(), InPrefab->EngineMajorVersion, InPrefab->EngineMinorVersion, ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION);
#if WITH_EDITORONLY_DATA
			if (InPrefab->UseBuildData && !ForceUseEditorData)
			{
				return nullptr;
			}
#endif
		}
	}
	//auto StartTime = FDateTime::Now();
	FLGUIActorSaveDataForBuild SaveDataForBuild;
#if WITH_EDITORONLY_DATA
	FLGUIPrefabSaveData SaveData;
	if (!InPrefab->UseBuildData || ForceUseEditorData)
	{
		auto LoadedData = InPrefab->BinaryData;
		if (LoadedData.Num() <= 0)
		{
			UE_LOG(LGUI, Warning, TEXT("Loaded data is empty!"));
			return nullptr;
		}
		auto FromBinary = FMemoryReader(LoadedData, true);
		FromBinary.Seek(0);
		FromBinary << SaveData;
		FromBinary.FlushCache();
		FromBinary.Close();
		LoadedData.Empty();
	}
	else
	{
		auto& LoadedDataForBuild = InPrefab->BinaryDataForBuild;
		if (LoadedDataForBuild.Num() <= 0)
		{
			UE_LOG(LGUI, Warning, TEXT("Loaded data for build is empty!"));
			return nullptr;
		}
		auto FromBinaryForBuild = FMemoryReader(LoadedDataForBuild, true);
		FromBinaryForBuild.Seek(0);
		FromBinaryForBuild << SaveDataForBuild;
		FromBinaryForBuild.FlushCache();
		FromBinaryForBuild.Close();
	}
#else
	{
		auto& LoadedDataForBuild = InPrefab->BinaryDataForBuild;
		if (LoadedDataForBuild.Num() <= 0)
		{
			UE_LOG(LGUI, Warning, TEXT("Loaded data for build is empty!"));
			return nullptr;
		}
		auto FromBinaryForBuild = FMemoryReader(LoadedDataForBuild, true);
		FromBinaryForBuild.Seek(0);
		FromBinaryForBuild << SaveDataForBuild;
		FromBinaryForBuild.FlushCache();
		FromBinaryForBuild.Close();
	}
#endif
	Prefab = TWeakObjectPtr<ULGUIPrefab>(InPrefab);
#if WITH_EDITOR
	IsEditMode = !TargetWorld->IsGameWorld();
	if (IsEditMode)
	{
		ULGUIEditorManagerObject::BeginPrefabSystemProcessingActor(TargetWorld.Get());
	}
	else
#endif
	{
		ALGUIManagerActor::BeginPrefabSystemProcessingActor(TargetWorld.Get());
	}
	int32 id = 0;
	AActor* CreatedActor = nullptr;
#if WITH_EDITORONLY_DATA
	if (!InPrefab->UseBuildData || ForceUseEditorData)
	{
		CreatedActor = DeserializeActorRecursive(Parent, SaveData.SavedActor, id);
	}
	else
	{
		CreatedActor = DeserializeActorRecursive(Parent, SaveDataForBuild, id);
	}
#else
	{
		CreatedActor = DeserializeActorRecursive(Parent, SaveDataForBuild, id);
	}
#endif
	if (CreatedActor != nullptr && ReplaceTransform)
	{
		CreatedActor->GetRootComponent()->SetRelativeLocationAndRotation(InLocation, FQuat(InRotation));
		CreatedActor->GetRootComponent()->SetRelativeScale3D(InScale);
	}

	//reassign actor reference after all actor are created
	for (auto item : ObjectMapStructList)
	{
		if (auto obj = MapIDToActor.Find(item.id))
		{
			auto objProperty = (FObjectPropertyBase*)item.ObjProperty;
			objProperty->SetObjectPropertyValue_InContainer(item.Dest, *obj, item.cppArrayIndex);
		}
	}
	//finish Actor Spawn
	for (auto Actor : CreatedActors)
	{
		if (Actor->IsValidLowLevel() && !Actor->IsPendingKill())//check, incase some actor is destroyed by other actor when BeginPlay
		{
#if WITH_EDITORONLY_DATA
			if (!IsEditMode)
			{
				Actor->bIsEditorPreviewActor = false;//make this to false, or BeginPlay won't be called
			}
#endif
			Actor->FinishSpawning(FTransform::Identity, true);//BeginPlay is called at this point
		}
	}
	//assign parent for blueprint actor after actor spawn, otherwise blueprint actor will attach to world root
	for (auto item : BlueprintAndParentArray)
	{
		if (item.BlueprintActor->IsValidLowLevel() && !item.BlueprintActor->IsPendingKill() && item.ParentActor->IsValidLowLevel() && !item.ParentActor->IsPendingKill())
		{
			item.BlueprintActor->GetRootComponent()->AttachToComponent(item.ParentActor, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}

#if WITH_EDITORONLY_DATA
	if (IsEditMode)
	{
		for (auto item : DeserializingActorCollection)
		{
			ULGUIEditorManagerObject::RemoveActorForPrefabSystem(item);
		}
		ULGUIEditorManagerObject::EndPrefabSystemProcessingActor();
	}
	else
#endif
	{
		for (auto item : DeserializingActorCollection)
		{
			ALGUIManagerActor::RemoveActorForPrefabSystem(item);
		}
		ALGUIManagerActor::EndPrefabSystemProcessingActor(TargetWorld.Get());
	}
	//UE_LOG(LGUI, Display, TEXT("Dserialize Prefab Duration:%s"), *((FDateTime::Now() - StartTime).ToString()));
	return CreatedActor;
}

AActor* ActorSerializer::DeserializeActorRecursive(USceneComponent* Parent, FLGUIActorSaveData& SaveData, int32& id)
{
	if (auto ActorClass = FindClassFromListByIndex(SaveData.ActorClass))
	{
		if (!ActorClass->IsChildOf(AActor::StaticClass()))//if not the right class, use default
		{
			ActorClass = AActor::StaticClass();
			UE_LOG(LGUI, Error, TEXT("Class:%s is not a Actor, use default"), *(ActorClass->GetFName().ToString()));
		}

		auto NewActor = TargetWorld->SpawnActorDeferred<AActor>(ActorClass, FTransform::Identity);
		DeserializingActorCollection.Add(NewActor);
#if WITH_EDITORONLY_DATA
		if (IsEditMode)
		{
			ULGUIEditorManagerObject::AddActorForPrefabSystem(NewActor);
		}
		else
#endif
		{
			ALGUIManagerActor::AddActorForPrefabSystem(NewActor);
		}
		LoadProperty(NewActor, SaveData.ActorPropertyData, GetActorExcludeProperties());
		CreatedActors.Add(NewActor);

		auto RootCompSaveData = SaveData.ComponentPropertyData[0];
		auto RootComp = NewActor->GetRootComponent();
		if (RootComp)//if this actor have default root component
		{
			//actor's default root component dont need mannually register
			LoadProperty(RootComp, RootCompSaveData.PropertyData, GetComponentExcludeProperties());
		}
		else
		{
			if (RootCompSaveData.ComponentClass != -1)//have RootComponent data
			{
				if (auto CompClass = FindClassFromListByIndex(RootCompSaveData.ComponentClass))
				{
					if (!CompClass->IsChildOf(USceneComponent::StaticClass()))//if not the right class, use default
					{
						CompClass = USceneComponent::StaticClass();
						UE_LOG(LGUI, Error, TEXT("Class:%s is not a USceneComponent, use default"), *(CompClass->GetFName().ToString()));
					}
					RootComp = NewObject<USceneComponent>(NewActor, CompClass, RootCompSaveData.ComponentName, RF_Transactional);
					NewActor->SetRootComponent(RootComp);
					LoadProperty(RootComp, RootCompSaveData.PropertyData, GetComponentExcludeProperties());
					if (!RootComp->IsDefaultSubobject())
					{
						NewActor->FinishAndRegisterComponent(RootComp);//mannually register component
					}
				}
			}
		}
		TArray<USceneComponent*> ThisActorSceneComponents;//all SceneComponent of this actor, when a SceneComponent need to attach to a parent, search from this list
		if (RootComp)
		{
			if (auto PrimitiveComp = Cast<UPrimitiveComponent>(RootComp))
			{
				PrimitiveComp->BodyInstance.FixupData(PrimitiveComp);
			}
			RootComp->RecreatePhysicsState();
			ThisActorSceneComponents.Add(RootComp);
		}

		TArray<SceneComponentToParentIDStruct> NeedReparent_SceneComponents;//SceneComponent collection of this actor that need to reattach to parent
		int ComponentCount = SaveData.ComponentPropertyData.Num();
		for (int i = 1; i < ComponentCount; i++)//start from 1, skip RootComponent
		{
			auto CompData = SaveData.ComponentPropertyData[i];
			if (auto CompClass = FindClassFromListByIndex(CompData.ComponentClass))
			{
				if (!CompClass->IsChildOf(UActorComponent::StaticClass()))//if not the right class, use default
				{
					CompClass = UActorComponent::StaticClass();
					UE_LOG(LGUI, Error, TEXT("Class:%s is not a UActorComponent, use default"), *(CompClass->GetFName().ToString()));
				}
				auto Comp = NewObject<UActorComponent>(NewActor, CompClass, CompData.ComponentName, RF_Transactional);
				LoadProperty(Comp, CompData.PropertyData, GetComponentExcludeProperties());
				if (!Comp->IsDefaultSubobject())
				{
					NewActor->FinishAndRegisterComponent(Comp);
				}

				if (auto PrimitiveComp = Cast<UPrimitiveComponent>(Comp))
				{
					PrimitiveComp->BodyInstance.FixupData(PrimitiveComp);
				}
				if (auto SceneComp = Cast<USceneComponent>(Comp))
				{
					SceneComp->RecreatePhysicsState();

					SceneComponentToParentIDStruct ParentNameStruct;
					ParentNameStruct.Comp = SceneComp;
					ParentNameStruct.ParentCompID = CompData.SceneComponentParentID;
					NeedReparent_SceneComponents.Add(ParentNameStruct);
					ThisActorSceneComponents.Add(SceneComp);
				}
			}
			else
			{
				UE_LOG(LGUI, Warning, TEXT("[DeserializeActorRecursive]Component Class of index:%d not found!"), (CompData.ComponentClass));
			}
		}
		//SceneComponent reattach to parent
		for (int i = 0, count = NeedReparent_SceneComponents.Num(); i < count; i++)
		{
			auto& SceneCompStructData = NeedReparent_SceneComponents[i];
			if (SceneCompStructData.ParentCompID != -1)
			{
				SceneCompStructData.Comp->AttachToComponent(ThisActorSceneComponents[SceneCompStructData.ParentCompID], FAttachmentTransformRules::KeepRelativeTransform);
			}
		}
		//set parent
		if (Parent)
		{
			if (NewActor->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || Parent->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))//for blueprint actor, we need to attach parent after actor finish spawn
			{
				BlueprintActorToParentActorStruct itemStruct;
				itemStruct.BlueprintActor = NewActor;
				itemStruct.ParentActor = Parent;
				BlueprintAndParentArray.Add(itemStruct);
			}
			if (RootComp)
			{
				RootComp->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
			}
		}

		id++;
		MapIDToActor.Add(id, NewActor);

		for (auto ChildSaveData : SaveData.ChildActorData)
		{
			DeserializeActorRecursive(RootComp, ChildSaveData, id);
		}
		return NewActor;
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[DeserializeActorRecursive]Actor Class of index:%d not found!"), (SaveData.ActorClass));
		return nullptr;
	}
}
AActor* ActorSerializer::DeserializeActorRecursive(USceneComponent* Parent, FLGUIActorSaveDataForBuild& SaveData, int32& id)
{
	if (auto ActorClass = FindClassFromListByIndex(SaveData.ActorClass))
	{
		if (!ActorClass->IsChildOf(AActor::StaticClass()))//if not the right class, use default
		{
			ActorClass = AActor::StaticClass();
			UE_LOG(LGUI, Error, TEXT("Class:%s is not a Actor, use default"), *(ActorClass->GetFName().ToString()));
		}

		auto NewActor = TargetWorld->SpawnActorDeferred<AActor>(ActorClass, FTransform::Identity);
		DeserializingActorCollection.Add(NewActor);
#if WITH_EDITORONLY_DATA
		if (IsEditMode)
		{
			ULGUIEditorManagerObject::AddActorForPrefabSystem(NewActor);
		}
		else
#endif
		{
			ALGUIManagerActor::AddActorForPrefabSystem(NewActor);
		}
		LoadProperty(NewActor, SaveData.ActorPropertyData, GetActorExcludeProperties());
		CreatedActors.Add(NewActor);

		auto RootCompSaveData = SaveData.ComponentPropertyData[0];
		auto RootComp = NewActor->GetRootComponent();
		if (RootComp)//if this actor have default root component
		{
			//actor's default root component dont need mannually register
			LoadProperty(RootComp, RootCompSaveData.PropertyData, GetComponentExcludeProperties());
		}
		else
		{
			if (RootCompSaveData.ComponentClass != -1)//have RootComponent data
			{
				if (auto CompClass = FindClassFromListByIndex(RootCompSaveData.ComponentClass))
				{
					if (!CompClass->IsChildOf(USceneComponent::StaticClass()))//if not the right class, use default
					{
						CompClass = USceneComponent::StaticClass();
						UE_LOG(LGUI, Error, TEXT("Class:%s is not a USceneComponent, use default"), *(CompClass->GetFName().ToString()));
					}
					RootComp = NewObject<USceneComponent>(NewActor, CompClass, RootCompSaveData.ComponentName, RF_Transactional);
					NewActor->SetRootComponent(RootComp);
					LoadProperty(RootComp, RootCompSaveData.PropertyData, GetComponentExcludeProperties());
					if (!RootComp->IsDefaultSubobject())
					{
						NewActor->FinishAndRegisterComponent(RootComp);//mannually register component
					}
				}
			}
		}
		TArray<USceneComponent*> ThisActorSceneComponents;//all SceneComponent of this actor, when a SceneComponent need to attach to a parent, search from this list
		if (RootComp)
		{
			if (auto PrimitiveComp = Cast<UPrimitiveComponent>(RootComp))
			{
				PrimitiveComp->BodyInstance.FixupData(PrimitiveComp);
			}
			RootComp->RecreatePhysicsState();
			ThisActorSceneComponents.Add(RootComp);
		}

		TArray<SceneComponentToParentIDStruct> NeedReparent_SceneComponents;//SceneComponent collection of this actor that need to reattach to parent
		int ComponentCount = SaveData.ComponentPropertyData.Num();
		for (int i = 1; i < ComponentCount; i++)//start from 1, skip RootComponent
		{
			auto CompData = SaveData.ComponentPropertyData[i];
			if (auto CompClass = FindClassFromListByIndex(CompData.ComponentClass))
			{
				if (!CompClass->IsChildOf(UActorComponent::StaticClass()))//if not the right class, use default
				{
					CompClass = UActorComponent::StaticClass();
					UE_LOG(LGUI, Error, TEXT("Class:%s is not a UActorComponent, use default"), *(CompClass->GetFName().ToString()));
				}
				auto Comp = NewObject<UActorComponent>(NewActor, CompClass, CompData.ComponentName, RF_Transactional);
				LoadProperty(Comp, CompData.PropertyData, GetComponentExcludeProperties());
				if (!Comp->IsDefaultSubobject())
				{
					NewActor->FinishAndRegisterComponent(Comp);
				}

				if (auto PrimitiveComp = Cast<UPrimitiveComponent>(Comp))
				{
					PrimitiveComp->BodyInstance.FixupData(PrimitiveComp);
				}
				if (auto SceneComp = Cast<USceneComponent>(Comp))
				{
					SceneComp->RecreatePhysicsState();

					SceneComponentToParentIDStruct ParentNameStruct;
					ParentNameStruct.Comp = SceneComp;
					ParentNameStruct.ParentCompID = CompData.SceneComponentParentID;
					NeedReparent_SceneComponents.Add(ParentNameStruct);
					ThisActorSceneComponents.Add(SceneComp);
				}
			}
			else
			{
#if WITH_EDITOR
				auto ErrorMsg = FString::Printf(TEXT("[ActorSerializer/DeserializeActorRecursive]Error prefab:%s. \nComponent Class of index:%d not found!"), *(Prefab->GetPathName()), (CompData.ComponentClass));
				UE_LOG(LGUI, Error, TEXT("%s"), *ErrorMsg); 
				LGUIUtils::EditorNotification(FText::FromString(ErrorMsg)); 
#else
				checkf(false, TEXT("[ActorSerializer/DeserializeActorRecursive]Error prefab:%s. \nComponent Class of index:%d not found!"), *(Prefab->GetPathName()), (CompData.ComponentClass));
#endif
			}
		}
		//SceneComponent reattach to parent
		for (int i = 0, count = NeedReparent_SceneComponents.Num(); i < count; i++)
		{
			auto& SceneCompStructData = NeedReparent_SceneComponents[i];
			if (SceneCompStructData.ParentCompID != -1)
			{
				SceneCompStructData.Comp->AttachToComponent(ThisActorSceneComponents[SceneCompStructData.ParentCompID], FAttachmentTransformRules::KeepRelativeTransform);
			}
		}
		//set parent
		if (Parent)
		{
			if (NewActor->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || Parent->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))//for blueprint actor, we need to attach parent after actor finish spawn
			{
				BlueprintActorToParentActorStruct itemStruct;
				itemStruct.BlueprintActor = NewActor;
				itemStruct.ParentActor = Parent;
				BlueprintAndParentArray.Add(itemStruct);
			}
			if (RootComp)
			{
				RootComp->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
			}
		}

		id++;
		MapIDToActor.Add(id, NewActor);

		for (auto ChildSaveData : SaveData.ChildActorData)
		{
			DeserializeActorRecursive(RootComp, ChildSaveData, id);
		}
		return NewActor;
	}
	else
	{
#if WITH_EDITOR
		auto ErrorMsg = FString::Printf(TEXT("[ActorSerializer/DeserializeActorRecursive]Error prefab:%s. \nActor Class of index:%d not found!"), *(Prefab->GetPathName()), (SaveData.ActorClass));
		UE_LOG(LGUI, Error, TEXT("%s"), *ErrorMsg);
		LGUIUtils::EditorNotification(FText::FromString(ErrorMsg));
#else
		checkf(false, TEXT("[ActorSerializer/DeserializeActorRecursive]Error prefab:%s. \nActor Class of index:%d not found!"), *(Prefab->GetPathName()), (SaveData.ActorClass));
#endif
		return nullptr;
	}
}

void ActorSerializer::LoadProperty(UObject* Target, const TArray<FLGUIPropertyData>& PropertyData, TArray<FName> ExcludeProperties)
{
	OutterArray.Add(Target);
	Outter = Target;

	auto propertyField = TFieldRange<FProperty>(Target->GetClass());
	int excludePropertyCount = ExcludeProperties.Num();
	int propertyIndex = 0;
	for (const auto propertyItem : propertyField)
	{
		if (auto objProperty = CastField<FObjectPropertyBase>(propertyItem))
		{
			if (objProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))//ignore ActorComponent, just need to handle normal properties
			{
				continue;
			}
		}
		auto propertyName = propertyItem->GetFName();
		if (excludePropertyCount != 0)
		{
			int32 index;
			if (ExcludeProperties.Find(propertyName, index))
			{
				ExcludeProperties.RemoveAt(index);
				excludePropertyCount--;
				continue;
			}
		}
		if (LoadCommonProperty(propertyItem, ItemType_Normal, propertyIndex, (uint8*)Target, PropertyData))
		{
			propertyIndex++;
		}
	}

	OutterArray.RemoveAt(OutterArray.Num() - 1);
	if (OutterArray.Num() >= 1)
	{
		Outter = OutterArray[OutterArray.Num() - 1];
	}
	else
	{
		Outter = nullptr;
	}
}
void ActorSerializer::LoadProperty(UObject* Target, const TArray<FLGUIPropertyDataForBuild>& PropertyData, TArray<FName> ExcludeProperties)
{
	OutterArray.Add(Target);
	Outter = Target;

	auto propertyField = TFieldRange<FProperty>(Target->GetClass());
	int excludePropertyCount = ExcludeProperties.Num();
	int propertyIndex = 0;
	for (const auto propertyItem : propertyField)
	{
		if (auto objProperty = CastField<FObjectPropertyBase>(propertyItem))
		{
			if (objProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))//ignore ActorComponent, just need to handle normal properties
			{
				continue;
			}
		}
		auto propertyName = propertyItem->GetFName();
		if (excludePropertyCount != 0)
		{
			int32 index;
			if (ExcludeProperties.Find(propertyName, index))
			{
				ExcludeProperties.RemoveAt(index);
				excludePropertyCount--;
				continue;
			}
		}
		if (LoadCommonProperty(propertyItem, ItemType_Normal, propertyIndex, (uint8*)Target, PropertyData))
		{
			propertyIndex++;
		}
	}

	OutterArray.RemoveAt(OutterArray.Num() - 1);
	if (OutterArray.Num() >= 1)
	{
		Outter = OutterArray[OutterArray.Num() - 1];
	}
	else
	{
		Outter = nullptr;
	}
}
bool ActorSerializer::LoadCommonProperty(FProperty* Property, int itemType, int containerItemIndex, uint8* Dest, const TArray<FLGUIPropertyData>& PropertyData, int cppArrayIndex, bool isInsideCppArray)
{
	if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_DisableEditOnInstance))return false;//skip property with these flags
	FLGUIPropertyData ItemPropertyData;
	bool HaveData = false;

	switch (itemType)
	{
	case ItemType_Normal:
	{
		if (FLGUIPropertyData::GetPropertyDataFromArray(Property->GetFName(), PropertyData, ItemPropertyData))
			HaveData = true;
	}
	break;
	case ItemType_Array:
	case ItemType_Map:
	case ItemType_Set:
	{
		if (containerItemIndex < PropertyData.Num())
		{
			ItemPropertyData = PropertyData[containerItemIndex];
			HaveData = true;
		}
	}
	break;
	}

	if (HaveData)
	{
		if (Property->ArrayDim > 1 && isInsideCppArray == false)
		{
			for (int i = 0; i < Property->ArrayDim; i++)
			{
				LoadCommonProperty(Property, ItemType_Array, i, Dest, ItemPropertyData.ContainerData, i, true);
			}
			return true;
		}
		else
		{
			if (CastField<FClassProperty>(Property) != nullptr || CastField<FSoftClassProperty>(Property) != nullptr)
			{
				int index = -2;
				if (ItemPropertyData.Data.Num() == 4)
				{
					index = BitConverter::ToInt32(ItemPropertyData.Data);
				}
				if (index <= -1)return true;
				if (auto asset = FindClassFromListByIndex(index))
				{
					auto classProperty = (FObjectPropertyBase*)Property;
					classProperty->SetObjectPropertyValue_InContainer(Dest, asset, cppArrayIndex);
				}
				return true;
			}
			else if (auto objProperty = CastField<FObjectPropertyBase>(Property))
			{
				int index = -2;
				if (ItemPropertyData.Data.Num() == 4)
				{
					index = BitConverter::ToInt32(ItemPropertyData.Data);
				}
				if (index <= -1) 
				{
					if (!objProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))//if this property is ActorComponent, if clear the value then it will become ineditable
					{
						objProperty->ClearValue_InContainer(Dest, cppArrayIndex);
					}
					return true;
				}
				if (objProperty->PropertyClass->IsChildOf(AActor::StaticClass()))//if is Actor
				{
					UPropertyMapStruct mapStruct;//store these data, after all actor created, reassign actor reference
					mapStruct.id = index;
					mapStruct.Dest = Dest;
					mapStruct.ObjProperty = objProperty;
					mapStruct.cppArrayIndex = cppArrayIndex;
					ObjectMapStructList.Add(mapStruct);
					return true;
				}
				else
				{
					if (Property->HasAnyPropertyFlags(CPF_InstancedReference))//is instanced object
					{
						UObject* newObj = objProperty->GetObjectPropertyValue_InContainer(Dest, cppArrayIndex);
						if (newObj == nullptr)
						{
							if (auto newObjClass = FindClassFromListByIndex(index))
							{
								newObj = NewObject<UObject>(Outter, newObjClass);
							}
						}
						if (newObj != nullptr)
						{
							LoadProperty(newObj, ItemPropertyData.ContainerData, {});
							objProperty->SetObjectPropertyValue_InContainer(Dest, newObj, cppArrayIndex);
						}
					}
					else if (auto asset = FindAssetFromListByIndex(index))
					{
						objProperty->SetObjectPropertyValue_InContainer(Dest, asset, cppArrayIndex);
					}
					else
					{
						if (!objProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))//if this property is ActorComponent, if clear the value then it will become ineditable
						{
							objProperty->ClearValue_InContainer(Dest, cppArrayIndex);
						}
					}
					return true;
				}
				return true;
			}
			
			else if (auto interfaceProperty = CastField<FInterfaceProperty>(Property))
			{
				//UE_LOG(LGUI, Error, TEXT("[ActorSerializerNotHandled]FInterfaceProperty:%s"), *(Property->GetFName().ToString()));
			}

			else if (auto arrProperty = CastField<FArrayProperty>(Property))
			{
				FScriptArrayHelper ArrayHelper(arrProperty, arrProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
				int ArrayCount = BitConverter::ToInt32(ItemPropertyData.Data);//array count
				ArrayHelper.Resize(ArrayCount);
				for (int i = 0; i < ArrayCount; i++)
				{
					LoadCommonProperty(arrProperty->Inner, ItemType_Array, i, ArrayHelper.GetRawPtr(i), ItemPropertyData.ContainerData);
				}
				return true;
			}
			else if (auto mapProperty = CastField<FMapProperty>(Property))
			{
				FScriptMapHelper MapHelper(mapProperty, mapProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
				auto count = BitConverter::ToInt32(ItemPropertyData.Data) * 2;//Map element's data is stored as key,value,key,value...
				int mapIndex = 0;
				for (int i = 0; i < count; i++)
				{
					MapHelper.AddDefaultValue_Invalid_NeedsRehash();
					LoadCommonProperty(mapProperty->KeyProp, ItemType_Map, i, MapHelper.GetKeyPtr(mapIndex), ItemPropertyData.ContainerData);//key
					i++;
					LoadCommonProperty(mapProperty->ValueProp, ItemType_Map, i, MapHelper.GetPairPtr(mapIndex), ItemPropertyData.ContainerData);//value. PairPtr of map is the real ptr of value
					mapIndex++;
				}
				MapHelper.Rehash();
				return true;
			}
			else if (auto setProperty = CastField<FSetProperty>(Property))
			{
				FScriptSetHelper SetHelper(setProperty, setProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
				auto count = BitConverter::ToInt32(ItemPropertyData.Data);//Set count
				for (int i = 0; i < count; i++)
				{
					SetHelper.AddDefaultValue_Invalid_NeedsRehash();
					LoadCommonProperty(setProperty->ElementProp, ItemType_Set, i, SetHelper.GetElementPtr(i), ItemPropertyData.ContainerData);
				}
				SetHelper.Rehash();
				return true;
			}
			else if (auto structProperty = CastField<FStructProperty>(Property))
			{
				bool isSimpleProperty = false;
				switch (ItemPropertyData.PropertyType)
				{
				case ELGUIPropertyType::PT_Vector2:
				case ELGUIPropertyType::PT_Vector3:
				case ELGUIPropertyType::PT_Vector4:
				case ELGUIPropertyType::PT_Quat:
				case ELGUIPropertyType::PT_Color:
				case ELGUIPropertyType::PT_LinearColor:
				case ELGUIPropertyType::PT_Rotator:
					isSimpleProperty = true; break;
				}
				if (isSimpleProperty)
				{
					Property->CopyCompleteValue(Property->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex), ItemPropertyData.Data.GetData());
				}
				else
				{
					auto structPtr = Property->ContainerPtrToValuePtr<uint8>(Dest, cppArrayIndex);
					int propertyIndex = 0;
					for (TFieldIterator<FProperty> It(structProperty->Struct); It; ++It)
					{
						if (LoadCommonProperty(*It, ItemType_Normal, propertyIndex, structPtr, ItemPropertyData.ContainerData))
						{
							propertyIndex++;
						}
					}
				}
				return true;
			}

			else if (auto strProperty = CastField<FStrProperty>(Property))//store string as reference
			{
				auto data = ItemPropertyData.Data;
				if (data.Num() == 4)
				{
					auto index = BitConverter::ToInt32(data);
					auto stringValue = FindStringFromListByIndex(index);
					strProperty->SetPropertyValue_InContainer(Dest, stringValue, cppArrayIndex);
				}
				return true;
			}
			else if (auto nameProperty = CastField<FNameProperty>(Property))
			{
				auto data = ItemPropertyData.Data;
				if (data.Num() == 4)
				{
					auto index = BitConverter::ToInt32(data);
					auto nameValue = FindNameFromListByIndex(index);
					nameProperty->SetPropertyValue_InContainer(Dest, nameValue, cppArrayIndex);
				}
				return true;
			}
			else if (auto textProperty = CastField<FTextProperty>(Property))
			{
				auto data = ItemPropertyData.Data;
				if (data.Num() == 4)
				{
					auto index = BitConverter::ToInt32(data);
					auto textValue = FindTextFromListByIndex(index);
					textProperty->SetPropertyValue_InContainer(Dest, textValue, cppArrayIndex);
				}
				return true;
			}
			else if (auto delegateProperty = CastField<FDelegateProperty>(Property))//blueprint dynamic delegate
			{
				//UE_LOG(LGUI, Error, TEXT("[ActorSerializerNotHandled]delegateProperty:%s"), *(Property->GetFName().ToString()));
			}
			else if (auto multicastDelegateProperty = CastField<FMulticastDelegateProperty>(Property))//blueprint dynamic delegate
			{
				//UE_LOG(LGUI, Error, TEXT("[ActorSerializerNotHandled]multicastDelegateProperty:%s"), *(Property->GetFName().ToString()));
			}
			else
			{
				if (auto boolProperty = CastField<FBoolProperty>(Property))
				{
					auto value = BitConverter::ToBoolean(ItemPropertyData.Data);
					boolProperty->SetPropertyValue_InContainer((void*)Dest, value, cppArrayIndex);
				}
				else
				{
					if (Property->GetSize() != ItemPropertyData.Data.Num())
					{
						UE_LOG(LGUI, Error, TEXT("[ActorSerializer/LoadCommonProperty]Load value of Property:%s, but size not match, PropertySize:%d, dataSize:%d.\
 Try rebuild this prefab, and if this problem still exist, please contact the plugin author.")
							, *(Property->GetName()), Property->GetSize(), ItemPropertyData.Data.Num());
					}
					else
					{
						Property->CopyCompleteValue(Property->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex), ItemPropertyData.Data.GetData());
					}
				}
				return true;
			}
		}
		return false;
	}
	return false;
}
bool ActorSerializer::LoadCommonProperty(FProperty* Property, int itemType, int containerItemIndex, uint8* Dest, const TArray<FLGUIPropertyDataForBuild>& PropertyData, int cppArrayIndex, bool isInsideCppArray)
{
	if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_DisableEditOnInstance))return false;//skip property with these flags
	if (Property->IsEditorOnlyProperty())return false;//Build data dont have EditorOnly property
	FLGUIPropertyDataForBuild ItemPropertyData;
	bool HaveData = false;

	switch (itemType)
	{
	case ItemType_Normal:
	{
		if (containerItemIndex < PropertyData.Num())
		{
			ItemPropertyData = PropertyData[containerItemIndex];
			HaveData = true;
		}
	}
	break;
	case ItemType_Array:
	case ItemType_Map:
	case ItemType_Set:
	{
		if (containerItemIndex < PropertyData.Num())
		{
			ItemPropertyData = PropertyData[containerItemIndex];
			HaveData = true;
		}
	}
	break;
	}


	if (HaveData)
	{
		if (Property->ArrayDim > 1 && isInsideCppArray == false)
		{
			for (int i = 0; i < Property->ArrayDim; i++)
			{
				LoadCommonProperty(Property, ItemType_Array, i, Dest, ItemPropertyData.ContainerData, i, true);
			}
			return true;
		}
		else
		{
			if (CastField<FClassProperty>(Property) != nullptr || CastField<FSoftClassProperty>(Property) != nullptr)
			{
				int index = -2;
				if (ItemPropertyData.Data.Num() == 4)
				{
					index = BitConverter::ToInt32(ItemPropertyData.Data);
				}
				if (index <= -1)return true;
				if (auto asset = FindClassFromListByIndex(index))
				{
					auto classProperty = (FObjectPropertyBase*)Property;
					classProperty->SetObjectPropertyValue_InContainer(Dest, asset, cppArrayIndex);
				}
				return true;
			}
			else if (auto objProperty = CastField<FObjectPropertyBase>(Property))
			{
				int index = -2;
				if (ItemPropertyData.Data.Num() == 4)
				{
					index = BitConverter::ToInt32(ItemPropertyData.Data);
				}
				if (index <= -1)
				{
					if (!objProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))//if this property is ActorComponent, if clear the value then it will become ineditable
					{
						objProperty->ClearValue_InContainer(Dest, cppArrayIndex);
					}
					return true;
				}
				if (objProperty->PropertyClass->IsChildOf(AActor::StaticClass()))//is is Actor
				{
					UPropertyMapStruct mapStruct;//store these data, after all actor created, reassign actor reference
					mapStruct.id = index;
					mapStruct.Dest = Dest;
					mapStruct.ObjProperty = objProperty;
					mapStruct.cppArrayIndex = cppArrayIndex;
					ObjectMapStructList.Add(mapStruct);
					return true;
				}
				else
				{
					if (Property->HasAnyPropertyFlags(CPF_InstancedReference))//is instanced object
					{
						UObject* newObj = objProperty->GetObjectPropertyValue_InContainer(Dest, cppArrayIndex);
						if (newObj == nullptr)
						{
							if (auto newObjClass = FindClassFromListByIndex(index))
							{
								newObj = NewObject<UObject>(Outter, newObjClass);
							}
						}
						if (newObj != nullptr)
						{
							LoadProperty(newObj, ItemPropertyData.ContainerData, {});
							objProperty->SetObjectPropertyValue_InContainer(Dest, newObj, cppArrayIndex);
						}
					}
					else if (auto asset = FindAssetFromListByIndex(index))//is asset
					{
						objProperty->SetObjectPropertyValue_InContainer(Dest, asset, cppArrayIndex);
					}
					else
					{
						if (!objProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))//if this property is ActorComponent, if clear the value then it will become ineditable
						{
							objProperty->ClearValue_InContainer(Dest, cppArrayIndex);
						}
					}
					return true;
				}
				return true;
			}
			else if (auto interfaceProperty = CastField<FInterfaceProperty>(Property))
			{
				//UE_LOG(LGUI, Error, TEXT("[ActorSerializerNotHandled]FInterfaceProperty:%s"), *(Property->GetFName().ToString()));
			}

			else if (auto arrProperty = CastField<FArrayProperty>(Property))
			{
				FScriptArrayHelper ArrayHelper(arrProperty, arrProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
				int ArrayCount = BitConverter::ToInt32(ItemPropertyData.Data);//array count
				ArrayHelper.Resize(ArrayCount);
				for (int i = 0; i < ArrayCount; i++)
				{
					LoadCommonProperty(arrProperty->Inner, ItemType_Array, i, ArrayHelper.GetRawPtr(i), ItemPropertyData.ContainerData);
				}
				return true;
			}
			else if (auto mapProperty = CastField<FMapProperty>(Property))
			{
				FScriptMapHelper MapHelper(mapProperty, mapProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
				auto count = BitConverter::ToInt32(ItemPropertyData.Data) * 2;//Map element's data is stored as key,value,key,value...
				int mapIndex = 0;
				for (int i = 0; i < count; i++)
				{
					MapHelper.AddDefaultValue_Invalid_NeedsRehash();
					LoadCommonProperty(mapProperty->KeyProp, ItemType_Map, i, MapHelper.GetKeyPtr(mapIndex), ItemPropertyData.ContainerData);//key
					i++;
					LoadCommonProperty(mapProperty->ValueProp, ItemType_Map, i, MapHelper.GetPairPtr(mapIndex), ItemPropertyData.ContainerData);//value. PairPtr of map is the real ptr of value
					mapIndex++;
				}
				MapHelper.Rehash();
				return true;
			}
			else if (auto setProperty = CastField<FSetProperty>(Property))
			{
				FScriptSetHelper SetHelper(setProperty, setProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
				auto count = BitConverter::ToInt32(ItemPropertyData.Data);//Set count
				for (int i = 0; i < count; i++)
				{
					SetHelper.AddDefaultValue_Invalid_NeedsRehash();
					LoadCommonProperty(setProperty->ElementProp, ItemType_Set, i, SetHelper.GetElementPtr(i), ItemPropertyData.ContainerData);
				}
				SetHelper.Rehash();
				return true;
			}
			else if (auto structProperty = CastField<FStructProperty>(Property))
			{
				bool isSimpleProperty = false;
				switch (ItemPropertyData.PropertyType)
				{
				case ELGUIPropertyType::PT_Vector2:
				case ELGUIPropertyType::PT_Vector3:
				case ELGUIPropertyType::PT_Vector4:
				case ELGUIPropertyType::PT_Quat:
				case ELGUIPropertyType::PT_Color:
				case ELGUIPropertyType::PT_LinearColor:
				case ELGUIPropertyType::PT_Rotator:
					isSimpleProperty = true; break;
				}
				if (isSimpleProperty)
				{
					Property->CopyCompleteValue(Property->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex), ItemPropertyData.Data.GetData());
				}
				else
				{
					auto structPtr = Property->ContainerPtrToValuePtr<uint8>(Dest, cppArrayIndex);
					int propertyIndex = 0;
					for (TFieldIterator<FProperty> It(structProperty->Struct); It; ++It)
					{
						if (LoadCommonProperty(*It, ItemType_Normal, propertyIndex, structPtr, ItemPropertyData.ContainerData))
						{
							propertyIndex++;
						}
					}
				}
				return true;
			}

			else if (auto strProperty = CastField<FStrProperty>(Property))//string store as reference
			{
				auto data = ItemPropertyData.Data;
				if (data.Num() == 4)
				{
					auto index = BitConverter::ToInt32(data);
					auto stringValue = FindStringFromListByIndex(index);
					strProperty->SetPropertyValue_InContainer(Dest, stringValue, cppArrayIndex);
				}
				return true;
			}
			else if (auto nameProperty = CastField<FNameProperty>(Property))
			{
				auto data = ItemPropertyData.Data;
				if (data.Num() == 4)
				{
					auto index = BitConverter::ToInt32(data);
					auto nameValue = FindNameFromListByIndex(index);
					nameProperty->SetPropertyValue_InContainer(Dest, nameValue, cppArrayIndex);
				}
				return true;
			}
			else if (auto textProperty = CastField<FTextProperty>(Property))
			{
				auto data = ItemPropertyData.Data;
				if (data.Num() == 4)
				{
					auto index = BitConverter::ToInt32(data);
					auto textValue = FindTextFromListByIndex(index);
					textProperty->SetPropertyValue_InContainer(Dest, textValue, cppArrayIndex);
				}
				return true;
			}
			else if (auto delegateProperty = CastField<FDelegateProperty>(Property))//blueprint dynamic delegate
			{
				//UE_LOG(LGUI, Error, TEXT("[ActorSerializerNotHandled]delegateProperty:%s"), *(Property->GetFName().ToString()));
			}
			else if (auto multicastDelegateProperty = CastField<FMulticastDelegateProperty>(Property))//blueprint dynamic delegate
			{
				//UE_LOG(LGUI, Error, TEXT("[ActorSerializerNotHandled]multicastDelegateProperty:%s"), *(Property->GetFName().ToString()));
			}
			else
			{
				if (auto boolProperty = CastField<FBoolProperty>(Property))
				{
					auto value = BitConverter::ToBoolean(ItemPropertyData.Data);
					boolProperty->SetPropertyValue_InContainer((void*)Dest, value, cppArrayIndex);
				}
				else
				{
#if WITH_EDITOR
					if (Property->GetSize() != ItemPropertyData.Data.Num())
					{
						auto ErrorMsg = FString::Printf(TEXT("[ActorSerializer/LoadCommonProperty]Error prefab:%s. \n	Load value of Property:%s (type:%s), but size not match, PropertySize:%d, dataSize:%d.\
 Open the prefab and click \"RecreateThis\" can fix it.\
 If this problem still exist, please contact the plugin author.")
							, *(Prefab->GetPathName()), *(Property->GetName()), *(Property->GetClass()->GetPathName()), Property->GetSize(), ItemPropertyData.Data.Num());
						UE_LOG(LGUI, Error, TEXT("%s"), *ErrorMsg);
						LGUIUtils::EditorNotification(FText::FromString(ErrorMsg));
						return false;
					}
#else
					checkf(Property->GetSize() == ItemPropertyData.Data.Num(), TEXT("[ActorSerializer/LoadCommonProperty]Error prefab:%s. \nLoad value of Property:%s (type:%s), but size not match, PropertySize:%d, dataSize:%d.\
 Open the prefab and click \"RecreateThis\" can fix it.\
 If this problem still exist, please contact the plugin author.")
						, *(Prefab->GetPathName()), *(Property->GetName()), *(Property->GetClass()->GetPathName()), Property->GetSize(), ItemPropertyData.Data.Num());
#endif
					Property->CopyCompleteValue(Property->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex), ItemPropertyData.Data.GetData());
				}
				return true;
			}
		}
		return false;
	}
	return false;
}

void ActorSerializer::SaveCommonProperty(FProperty* Property, int itemType, uint8* Dest, TArray<FLGUIPropertyData>& PropertyData, int cppArrayIndex, bool isInsideCppArray)
{
	if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_DisableEditOnInstance))return;//skip property with these flag
	FLGUIPropertyData ItemPropertyData;
	if (Property->IsEditorOnlyProperty())
		ItemPropertyData.IsEditorOnly = true;
	switch (itemType)
	{
	case ItemType_Normal://normal property stored as Name-Value
	{
		ItemPropertyData.Name = Property->GetFName();
	}
	break;
	}
	if (Property->ArrayDim > 1 && isInsideCppArray == false)
	{
		ItemPropertyData.Data = BitConverter::GetBytes(Property->ArrayDim);
		ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Container;
		for (int i = 0; i < Property->ArrayDim; i++)
		{
			SaveCommonProperty(Property, ItemType_Array, Dest, ItemPropertyData.ContainerData, i, true);
		}
		PropertyData.Add(ItemPropertyData);
	}
	else
	{
		if (CastField<FClassProperty>(Property) != nullptr || CastField<FSoftClassProperty>(Property) != nullptr)
		{
			auto classProperty = (FObjectPropertyBase*)Property;
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Reference;
			if (auto object = classProperty->GetObjectPropertyValue_InContainer(Dest, cppArrayIndex))
			{
				auto id = FindClassIdFromList((UClass*)object);
				ItemPropertyData.Data = BitConverter::GetBytes(id);
			}
			else
			{
				ItemPropertyData.Data = BitConverter::GetBytes(-1);
			}
			PropertyData.Add(ItemPropertyData);
		}
		else if (auto objProperty = CastField<FObjectPropertyBase>(Property))
		{
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Reference;
			if (auto object = objProperty->GetObjectPropertyValue_InContainer(Dest, cppArrayIndex))
			{
				if (object->IsAsset() || object->IsA(UClass::StaticClass()))
				{
					auto id = FindAssetIdFromList(object);
					ItemPropertyData.Data = BitConverter::GetBytes(id);
					PropertyData.Add(ItemPropertyData);
				}
				else if (Property->HasAnyPropertyFlags(CPF_InstancedReference))
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_InstancedObject;
					auto id = FindClassIdFromList(object->GetClass());
					ItemPropertyData.Data = BitConverter::GetBytes(id);
					if (object->GetClass()->IsChildOf(USceneComponent::StaticClass()))
					{
						SaveProperty(object, ItemPropertyData.ContainerData, GetComponentExcludeProperties());
					}
					else
					{
						SaveProperty(object, ItemPropertyData.ContainerData, {});
					}
					PropertyData.Add(ItemPropertyData);
				}
				else if (objProperty->PropertyClass->IsChildOf(AActor::StaticClass()))//Actor need remap
				{
					auto actor = Cast<AActor>(object);
					if (auto id = MapActorToID.Find(actor))
					{
						ItemPropertyData.Data = BitConverter::GetBytes(*id);
						PropertyData.Add(ItemPropertyData);
					}
					else//if reference actor is not a child of prefab's root actor, then save index as -1, means null value
					{
						ItemPropertyData.Data = BitConverter::GetBytes(-1);
						PropertyData.Add(ItemPropertyData);
					}
				}
				else
				{
					ItemPropertyData.Data = BitConverter::GetBytes(-1);
					PropertyData.Add(ItemPropertyData);
				}
			}
			else
			{
				ItemPropertyData.Data = BitConverter::GetBytes(-1);
				PropertyData.Add(ItemPropertyData);
			}
		}
		
		else if (auto interfaceProperty = CastField<FInterfaceProperty>(Property))
		{
			//UE_LOG(LGUI, Error, TEXT("[ActorSerializerNotHandled]FInterfaceProperty:%s"), *(Property->GetFName().ToString()));
		}

		else if (auto arrProperty = CastField<FArrayProperty>(Property))
		{
			FScriptArrayHelper ArrayHelper(arrProperty, arrProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
			auto arrayCount = ArrayHelper.Num();
			ItemPropertyData.Data = BitConverter::GetBytes(arrayCount);
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Container;
			for (int i = 0; i < arrayCount; i++)
			{
				SaveCommonProperty(arrProperty->Inner, ItemType_Array, ArrayHelper.GetRawPtr(i), ItemPropertyData.ContainerData);
			}
			PropertyData.Add(ItemPropertyData);
		}
		else if (auto mapProperty = CastField<FMapProperty>(Property))//map element's data stored as key/value/key/value...
		{
			FScriptMapHelper MapHelper(mapProperty, mapProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
			auto count = MapHelper.Num();
			ItemPropertyData.Data = BitConverter::GetBytes(count);
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Container;
			for (int i = 0; i < count; i++)
			{
				SaveCommonProperty(mapProperty->KeyProp, ItemType_Map, MapHelper.GetKeyPtr(i), ItemPropertyData.ContainerData);//key
				SaveCommonProperty(mapProperty->ValueProp, ItemType_Map, MapHelper.GetPairPtr(i), ItemPropertyData.ContainerData);//value
			}
			PropertyData.Add(ItemPropertyData);
		}
		else if (auto setProperty = CastField<FSetProperty>(Property))
		{
			FScriptSetHelper SetHelper(setProperty, setProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
			auto count = SetHelper.Num();
			ItemPropertyData.Data = BitConverter::GetBytes(count);
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Container;
			for (int i = 0; i < count; i++)
			{
				SaveCommonProperty(setProperty->ElementProp, ItemType_Set, SetHelper.GetElementPtr(i), ItemPropertyData.ContainerData);
			}
			PropertyData.Add(ItemPropertyData);
		}
		else if (auto structProperty = CastField<FStructProperty>(Property))
		{
			auto structName = structProperty->Struct->GetFName();
			bool isSimpleData = false;
			if (structName == TEXT("Vector2D"))
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Vector2; isSimpleData = true;
			}
			else if (structName == TEXT("Vector"))
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Vector3; isSimpleData = true;
			}
			else if (structName == TEXT("Vector4"))
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Vector4; isSimpleData = true;
			}
			else if (structName == TEXT("Quat"))
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Quat; isSimpleData = true;
			}
			else if (structName == TEXT("Color"))
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Color; isSimpleData = true;
			}
			else if (structName == TEXT("LinearColor"))
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_LinearColor; isSimpleData = true;
			}
			else if (structName == TEXT("Rotator"))
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Rotator; isSimpleData = true;
			}
			if (isSimpleData)
			{
				auto data = Property->ContainerPtrToValuePtr<uint8>((void*)Dest, cppArrayIndex);
				ItemPropertyData.Data.Reserve(Property->ElementSize);
				for (int i = 0; i < Property->ElementSize; i++)
				{
					ItemPropertyData.Data.Add(data[i]);
				}
			}
			else
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_StructContainer;
				auto structPtr = Property->ContainerPtrToValuePtr<uint8>(Dest, cppArrayIndex);
				for (TFieldIterator<FProperty> It(structProperty->Struct); It; ++It)
				{
					SaveCommonProperty(*It, ItemType_Normal, structPtr, ItemPropertyData.ContainerData);
				}
			}
			PropertyData.Add(ItemPropertyData);
		}

		else if (auto strProperty = CastField<FStrProperty>(Property))
		{
			auto stringValue = strProperty->GetPropertyValue_InContainer(Dest, cppArrayIndex);
			auto id = FindStringIdFromList(stringValue);
			ItemPropertyData.Data = BitConverter::GetBytes(id);
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Reference;
			PropertyData.Add(ItemPropertyData);
		}
		else if (auto nameProperty = CastField<FNameProperty>(Property))
		{
			auto nameValue = nameProperty->GetPropertyValue_InContainer(Dest, cppArrayIndex);
			auto id = FindNameIdFromList(nameValue);
			ItemPropertyData.Data = BitConverter::GetBytes(id);
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Reference;
			PropertyData.Add(ItemPropertyData);
		}
		else if (auto textProperty = CastField<FTextProperty>(Property))
		{
			auto textValue = textProperty->GetPropertyValue_InContainer(Dest, cppArrayIndex);
			auto id = FindTextIdFromList(textValue);
			ItemPropertyData.Data = BitConverter::GetBytes(id);
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Reference;
			PropertyData.Add(ItemPropertyData);
		}
		else if (auto delegateProperty = CastField<FDelegateProperty>(Property))//blueprint dynamic delegate
		{
			//UE_LOG(LGUI, Error, TEXT("[ActorSerializerNotHandled]delegateProperty:%s"), *(Property->GetFName().ToString()));
		}
		else if (auto multicastDelegateProperty = CastField<FMulticastDelegateProperty>(Property))//blueprint dynamic delegate
		{
			//UE_LOG(LGUI, Error, TEXT("[ActorSerializerNotHandled]multicastDelegateProperty:%s"), *(Property->GetFName().ToString()));
		}
		else
		{
			if (auto boolProperty = CastField<FBoolProperty>(Property))//some bool is declared as bit field, so we need to handle it specially
			{
				auto value = boolProperty->GetPropertyValue_InContainer((void*)Dest, cppArrayIndex);
				ItemPropertyData.Data = BitConverter::GetBytes(value);
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_bool;
			}
			else
			{
				if (CastField<FInt8Property>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_int8;
				}
				else if (CastField<FInt16Property>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_int16;
				}
				else if (CastField<FIntProperty>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_int32;
				}
				else if (CastField<FInt64Property>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_int64;
				}
				else if (CastField<FByteProperty>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_uint8;
				}
				else if (CastField<FUInt16Property>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_uint16;
				}
				else if (CastField<FUInt32Property>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_uint32;
				}
				else if (CastField<FUInt64Property>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_uint64;
				}
				else if (CastField<FFloatProperty>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_float;
				}
				else if (CastField<FDoubleProperty>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_double;
				}
				else if (CastField<FEnumProperty>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_uint8;
				}
				auto data = Property->ContainerPtrToValuePtr<uint8>((void*)Dest, cppArrayIndex);
				ItemPropertyData.Data.Reserve(Property->ElementSize);
				for (int i = 0; i < Property->ElementSize; i++)
				{
					ItemPropertyData.Data.Add(data[i]);
				}
			}
			PropertyData.Add(ItemPropertyData);
		}
	}
}
void ActorSerializer::SaveProperty(UObject* Target, TArray<FLGUIPropertyData>& PropertyData, TArray<FName> ExcludeProperties)
{
	auto propertyField = TFieldRange<FProperty>(Target->GetClass());
	int excludePropertyCount = ExcludeProperties.Num();
	for (const auto propertyItem : propertyField)
	{
		if (auto objProperty = CastField<FObjectPropertyBase>(propertyItem))
		{
			if (objProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))//ignore ActorComponent, just need to handle normal properties
			{
				continue;
			}
		}
		auto propertyName = propertyItem->GetFName();
		if (excludePropertyCount != 0)
		{
			int32 index;
			if (ExcludeProperties.Find(propertyName, index))
			{
				ExcludeProperties.RemoveAt(index);
				excludePropertyCount--;
				continue;
			}
		}
		SaveCommonProperty(propertyItem, ItemType_Normal, (uint8*)Target, PropertyData);
	}
}
//serialize single Actor
FLGUIActorSaveData ActorSerializer::SerializeSingleActor(AActor* Actor)
{
	FLGUIActorSaveData ActorRecord;
	ActorRecord.ActorClass = FindClassIdFromList(Actor->GetClass());

	SaveProperty(Actor, ActorRecord.ActorPropertyData, GetActorExcludeProperties());
	auto& Components = Actor->GetComponents();

	TArray<USceneComponent*> AllSceneComponentOfThisActor;
	auto RootComp = Actor->GetRootComponent();
	FLGUIComponentSaveData RootCompData;
	if (RootComp)
	{
		RootCompData.ComponentClass = FindClassIdFromList(RootComp->GetClass());
		RootCompData.ComponentName = RootComp->GetFName();
		SaveProperty(RootComp, RootCompData.PropertyData, GetComponentExcludeProperties());
		AllSceneComponentOfThisActor.Add(RootComp);
	}
	ActorRecord.ComponentPropertyData.Add(RootCompData);//add a data seat for RootComponent, so when deserialize we dont need to check again

	for (auto Comp : Components)
	{
		if (Comp == RootComp)continue;//skip RootComponent
		if (Comp->HasAnyFlags(EObjectFlags::RF_Transient))continue;//skip transient component
		if (Comp->GetClass()->IsChildOf(ULGUIPrefabHelperComponent::StaticClass()))continue;//skip LGUIPrefabHelperComponent, this component is only for editor helper
																							//if (Comp->IsInBlueprint())continue;//skip Blueprint component
		FLGUIComponentSaveData CompData;
		CompData.ComponentClass = FindClassIdFromList(Comp->GetClass());
		CompData.ComponentName = Comp->GetFName();
		if (auto SceneComp = Cast<USceneComponent>(Comp))
		{
			if (auto parentComp = SceneComp->GetAttachParent())
			{
				CompData.SceneComponentParent = parentComp;
			}
			AllSceneComponentOfThisActor.Add(SceneComp);
		}
		SaveProperty(Comp, CompData.PropertyData, GetComponentExcludeProperties());
		ActorRecord.ComponentPropertyData.Add(CompData);
	}
	for (int i = 0, count = ActorRecord.ComponentPropertyData.Num(); i < count; i++)
	{
		auto& componentPropertyData = ActorRecord.ComponentPropertyData[i];
		if (componentPropertyData.SceneComponentParent)
		{
			int32 parentCompIndex;
			if (AllSceneComponentOfThisActor.Find(componentPropertyData.SceneComponentParent, parentCompIndex))
			{
				ActorRecord.ComponentPropertyData[i].SceneComponentParentID = parentCompIndex;
			}
		}
	}

	return ActorRecord;
}
void ActorSerializer::GenerateActorIDRecursive(AActor* Actor, int32& id)
{
	id += 1;
	MapActorToID.Add(Actor, id);

	TArray<AActor*> ChildrenActors;
	Actor->GetAttachedActors(ChildrenActors);
	for (auto ChildActor : ChildrenActors)
	{
		GenerateActorIDRecursive(ChildActor, id);
	}
}
void ActorSerializer::SerializeActorRecursive(AActor* Actor, FLGUIActorSaveData& ActorSaveData)
{
	ActorSaveData = SerializeSingleActor(Actor);
	TArray<AActor*> ChildrenActors;
	Actor->GetAttachedActors(ChildrenActors);
	TArray<FLGUIActorSaveData> ChildSaveDataList;
	for (auto ChildActor : ChildrenActors)
	{
		FLGUIActorSaveData ChildActorSaveData;
		SerializeActorRecursive(ChildActor, ChildActorSaveData);
		ChildSaveDataList.Add(ChildActorSaveData);
	}
	ActorSaveData.ChildActorData = ChildSaveDataList;
}
void ActorSerializer::SavePrefab(AActor* RootActor, ULGUIPrefab* InPrefab)
{
	if (!RootActor || !InPrefab)
	{
		UE_LOG(LGUI, Error, TEXT("[ActorSerializer::SerializeActor]RootActor Or InPrefab is null!"));
		return;
	}
	if (!RootActor->GetWorld())
	{
		UE_LOG(LGUI, Error, TEXT("[ActorSerializer::SerializeActor]Cannot get World from RootActor!"));
		return;
	}
	ActorSerializer serializer(RootActor->GetWorld());
	serializer.SerializeActor(RootActor, InPrefab);
}
void ActorSerializer::SerializeActor(AActor* RootActor, ULGUIPrefab* InPrefab)
{
	Prefab = TWeakObjectPtr<ULGUIPrefab>(InPrefab);
	//clear old reference data
	Prefab->ReferenceAssetList.Empty();
	Prefab->ReferenceStringList.Empty();
	Prefab->ReferenceNameList.Empty();
	Prefab->ReferenceTextList.Empty();
	Prefab->ReferenceClassList.Empty();

	auto StartTime = FDateTime::Now();

	int32 id = 0;
	GenerateActorIDRecursive(RootActor, id);

	FLGUIActorSaveData ActorSaveData;
	SerializeActorRecursive(RootActor, ActorSaveData);

	FLGUIPrefabSaveData SaveData;
	SaveData.SavedActor = ActorSaveData;

#if WITH_EDITORONLY_DATA
	FBufferArchive ToBinary;
	ToBinary << SaveData;

	if (ToBinary.Num() <= 0)
	{
		UE_LOG(LGUI, Warning, TEXT("Save binary length is 0!"));
		return;
	}

	Prefab->BinaryData = ToBinary;
	Prefab->ThumbnailDirty = true;

	ToBinary.FlushCache();
	ToBinary.Empty();

	Prefab->DataCount = Prefab->BinaryData.Num();
#endif
	Prefab->EngineMajorVersion = ENGINE_MAJOR_VERSION;
	Prefab->EngineMinorVersion = ENGINE_MINOR_VERSION;

	ConvertForBuildData(ActorSaveData, InPrefab);

	Prefab->MarkPackageDirty();
	UE_LOG(LGUI, Log, TEXT("SerializeActor, prefab:%s duration:%s"), *(Prefab->GetPathName()), *((FDateTime::Now() - StartTime).ToString()));
}
#if WITH_EDITOR
FLGUIActorSaveData ActorSerializer::CreateActorSaveData(ULGUIPrefab* InPrefab)
{
	FLGUIPrefabSaveData SaveData;
	auto LoadedData = InPrefab->BinaryData;
	if (LoadedData.Num() <= 0)
	{
		UE_LOG(LGUI, Warning, TEXT("Loaded data is empty!"));
		return SaveData.SavedActor;
	}
	auto FromBinary = FMemoryReader(LoadedData, true);
	FromBinary.Seek(0);
	FromBinary << SaveData;
	FromBinary.FlushCache();
	FromBinary.Close();
	LoadedData.Empty();
	return SaveData.SavedActor;
}
#endif
void ActorSerializer::ConvertForBuildData(const FLGUIActorSaveData InSaveData, ULGUIPrefab* InPrefab)
{
	FLGUIActorSaveDataForBuild ActorSaveDataForBuild;
	ActorSaveDataForBuild.FromActorSaveData(InSaveData);
	FBufferArchive ToBinaryForBuild;
	ToBinaryForBuild << ActorSaveDataForBuild;
	if (ToBinaryForBuild.Num() <= 0)
	{
		UE_LOG(LGUI, Warning, TEXT("Save binary for build length is 0!"));
		return;
	}
	InPrefab->BinaryDataForBuild = ToBinaryForBuild;

	ToBinaryForBuild.FlushCache();
	ToBinaryForBuild.Empty();

#if WITH_EDITORONLY_DATA
	InPrefab->DataCountForBuild = InPrefab->BinaryDataForBuild.Num();
#endif
}

TArray<FName> ActorSerializer::GetActorExcludeProperties()
{
	return {
		"Instigator",
		"RootComponent",//this will result in the copied actor have same RootComponent to original actor, and crash. so we need to skip it
		"BlueprintCreatedComponents",
		"InstanceComponents",
#if WITH_EDITORONLY_DATA
		"FolderPath",
#endif
	};
}
TArray<FName> ActorSerializer::GetComponentExcludeProperties()
{
	return {
		"AttachParent",
	};
}


int32 ActorSerializer::FindAssetIdFromList(UObject* AssetObject)
{
	if (!AssetObject)return -1;
	auto& ReferenceAssetList = Prefab->ReferenceAssetList;
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
int32 ActorSerializer::FindStringIdFromList(FString string)//put string into list to avoid same string sotred multiple times
{
	auto& ReferenceStringList = Prefab->ReferenceStringList;
	int32 resultIndex;
	if (ReferenceStringList.Find(string, resultIndex))
	{
		return resultIndex;
	}
	else
	{
		ReferenceStringList.Add(string);
		return ReferenceStringList.Num() - 1;
	}
}
int32 ActorSerializer::FindNameIdFromList(FName name)
{
	auto& ReferenceNameList = Prefab->ReferenceNameList;
	int32 resultIndex;
	if (ReferenceNameList.Find(name, resultIndex))
	{
		return resultIndex;
	}
	else
	{
		ReferenceNameList.Add(name);
		return ReferenceNameList.Num() - 1;
	}
}
int32 ActorSerializer::FindTextIdFromList(FText text)
{
	auto& ReferenceTextList = Prefab->ReferenceTextList;
	int32 Count = ReferenceTextList.Num();
	for (int32 i = 0; i < Count; i++)
	{
		if (ReferenceTextList[i].CompareTo(text, ETextComparisonLevel::Default) == 0)
		{
			return i;
		}
	}

	ReferenceTextList.Add(text);
	return ReferenceTextList.Num() - 1;
}
int32 ActorSerializer::FindClassIdFromList(UClass* uclass)
{
	if (!uclass)return -1;
	auto& ReferenceClassList = Prefab->ReferenceClassList;
	int32 resultIndex;
	if (ReferenceClassList.Find(uclass, resultIndex))
	{
		return resultIndex;
	}
	else
	{
		ReferenceClassList.Add(uclass);
		return ReferenceClassList.Num() - 1;
	}
}

UObject* ActorSerializer::FindAssetFromListByIndex(int32 Id)
{
	auto& ReferenceAssetList = Prefab->ReferenceAssetList;
	int32 count = ReferenceAssetList.Num();
	if (Id >= count || Id < 0)
	{
		return nullptr;
	}
	return ReferenceAssetList[Id];
}
FString ActorSerializer::FindStringFromListByIndex(int32 Id)
{
	auto& ReferenceStringList = Prefab->ReferenceStringList;
	int32 count = ReferenceStringList.Num();
	if (Id >= count || Id < 0)
	{
		return FString();
	}
	return ReferenceStringList[Id];
}
FName ActorSerializer::FindNameFromListByIndex(int32 Id)
{
	auto& ReferenceNameList = Prefab->ReferenceNameList;
	int32 count = ReferenceNameList.Num();
	if (Id >= count || Id < 0)
	{
		return FName();
	}
	return ReferenceNameList[Id];
}
FText ActorSerializer::FindTextFromListByIndex(int32 Id)
{
	auto& ReferenceTextList = Prefab->ReferenceTextList;
	int32 count = ReferenceTextList.Num();
	if (Id >= count || Id < 0)
	{
		return FText();
	}
	return ReferenceTextList[Id];
}
UClass* ActorSerializer::FindClassFromListByIndex(int32 Id)
{
	auto& ReferenceClassList = Prefab->ReferenceClassList;
	int32 count = ReferenceClassList.Num();
	if (Id >= count || Id < 0)
	{
		return nullptr;
	}
	return ReferenceClassList[Id];
}
