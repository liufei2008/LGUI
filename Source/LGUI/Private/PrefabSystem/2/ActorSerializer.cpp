// Copyright 2019-Present LexLiu. All Rights Reserved.

#if WITH_EDITOR
#include "PrefabSystem/2/ActorSerializer.h"
#include "BitConverter.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "Components/PrimitiveComponent.h"
#include "UObject/TextProperty.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Core/Actor/LGUIManagerActor.h"

using namespace LGUIPrefabSystem;

ActorSerializer::ActorSerializer(ULGUIPrefab* InPrefab)
{
	Prefab = InPrefab;
}
ActorSerializer::ActorSerializer(UWorld* InTargetWorld)
{
	TargetWorld = TWeakObjectPtr<UWorld>(InTargetWorld);
}
#if WITH_EDITOR
AActor* ActorSerializer::LoadPrefabForEdit(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
	, TFunction<AActor* (FGuid)> InGetExistingActorFunction
	, TFunction<void(AActor*, FGuid)> InCreateNewActorFunction
	, TArray<AActor*>& OutCreatedActors, TArray<FGuid>& OutActorsGuid)
{
	auto Time = FDateTime::Now();
	ActorSerializer serializer(InWorld);
	serializer.editMode = EPrefabEditMode::EditInLevel;
	serializer.GetExistingActorFunction = InGetExistingActorFunction;
	serializer.CreateNewActorFunction = InCreateNewActorFunction;
	auto rootActor = serializer.DeserializeActor(Parent, InPrefab, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
	OutCreatedActors = serializer.CreatedActors;
	OutActorsGuid = serializer.CreatedActorsGuid;
	return rootActor;
}

AActor* ActorSerializer::LoadPrefabInEditor(UWorld *InWorld, ULGUIPrefab *InPrefab, USceneComponent *Parent, bool SetRelativeTransformToIdentity)
{
	ActorSerializer serializer(InWorld);
	serializer.editMode = EPrefabEditMode::NotEditable;
	if (SetRelativeTransformToIdentity)
	{
		return serializer.DeserializeActor(Parent, InPrefab, true, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
	}
	else
	{
		return serializer.DeserializeActor(Parent, InPrefab);
	}
}

void ActorSerializer::RenewActorGuidForDuplicate(ULGUIPrefab* InPrefab)
{
	ActorSerializer serializer(InPrefab);
	serializer.RenewActorGuidForDuplicate_Implement();
}
void ActorSerializer::RenewActorGuidForDuplicate_Implement()
{
	FLGUIPrefabSaveData SaveData;
	{
		auto LoadedData = Prefab->BinaryData;
		if (LoadedData.Num() <= 0)
		{
			UE_LOG(LGUI, Warning, TEXT("Loaded data is empty!"));
			return;
		}
		auto FromBinary = FMemoryReader(LoadedData, true);
		FromBinary.Seek(0);
		FromBinary << SaveData;
		FromBinary.FlushCache();
		FromBinary.Close();
		LoadedData.Empty();
	}

	RenewActorGuidRecursiveForDuplicate(SaveData.SavedActor);
	DeserializeActorRecursiveForDuplicate(SaveData.SavedActor);

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
}
#endif
AActor* ActorSerializer::LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity, TFunction<void(AActor*)> CallbackBeforeAwake)
{
	ActorSerializer serializer(InWorld);
	serializer.CallbackBeforeAwake = CallbackBeforeAwake;
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
AActor* ActorSerializer::LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, FVector RelativeLocation, FQuat RelativeRotation, FVector RelativeScale, TFunction<void(AActor*)> CallbackBeforeAwake)
{
	ActorSerializer serializer(InWorld);
	serializer.CallbackBeforeAwake = CallbackBeforeAwake;
	return serializer.DeserializeActor(Parent, InPrefab, true, RelativeLocation, RelativeRotation, RelativeScale);
}

AActor* ActorSerializer::DeserializeActor(USceneComponent* Parent, ULGUIPrefab* InPrefab, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale)
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

	auto StartTime = FDateTime::Now();

#if WITH_EDITOR
	if (!TargetWorld->IsGameWorld())
	{

	}
	else
#endif
	{
		LGUIManagerActor = ALGUIManagerActor::GetInstance(TargetWorld.Get(), true);
	}

#if WITH_EDITORONLY_DATA
	FLGUIPrefabSaveData SaveData;
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
#else
	FLGUIActorSaveDataForBuild SaveDataForBuild;
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
	int32 id = 0;
	AActor *CreatedRootActor = nullptr;
#if WITH_EDITOR
	if (editMode == EPrefabEditMode::EditInLevel)
	{
		CreatedRootActor = DeserializeActorRecursiveForEdit(Parent, SaveData.SavedActor, id);
	}
	else
	{
		CreatedRootActor = DeserializeActorRecursiveForUseInEditor(Parent, SaveData.SavedActor, id);
	}
#else
	{
		CreatedRootActor = DeserializeActorRecursiveForUseInRuntime(Parent, SaveDataForBuild, id);
	}
#endif
	if (CreatedRootActor != nullptr)
	{
		if (USceneComponent* rootComp = CreatedRootActor->GetRootComponent())
		{
			rootComp->UpdateComponentToWorld();
			if (ReplaceTransform)
			{
				rootComp->SetRelativeLocationAndRotation(InLocation, InRotation);
				rootComp->SetRelativeScale3D(InScale);
			}
		}
	}

	//reassign actor reference after all actor are created
	for (auto item : ObjectMapStructList)
	{
#if WITH_EDITOR
		if (Prefab->PrefabVersion < 2)
		{
			if (auto obj = MapIDToActor.Find(item.id))
			{
				auto objProperty = (FObjectPropertyBase*)item.ObjProperty;
				objProperty->SetObjectPropertyValue_InContainer(item.Dest, *obj, item.cppArrayIndex);
			}
		}
		else
		{
			if (auto obj = MapGuidToActor.Find(item.guid))
			{
				auto objProperty = (FObjectPropertyBase*)item.ObjProperty;
				objProperty->SetObjectPropertyValue_InContainer(item.Dest, *obj, item.cppArrayIndex);
			}
		}
#else
		if (auto obj = MapIDToActor.Find(item.id))
		{
			auto objProperty = (FObjectPropertyBase*)item.ObjProperty;
			objProperty->SetObjectPropertyValue_InContainer(item.Dest, *obj, item.cppArrayIndex);
		}
#endif
	}
	//finish Actor Spawn
#if WITH_EDITOR
	if (editMode == EPrefabEditMode::EditInLevel)
	{
		for (auto Actor : CreatedActorsNeedToFinishSpawn)
		{
			if (IsValid(Actor))
			{
				Actor->FinishSpawning(FTransform::Identity, true);
			}
		}
	}
	else
	{
		for (auto Actor : CreatedActors)
		{
			if (IsValid(Actor))
			{
#if WITH_EDITOR
				if (TargetWorld->IsGameWorld())
				{
					Actor->bIsEditorPreviewActor = false;//make this to false, or BeginPlay won't be called
				}
#endif
				Actor->FinishSpawning(FTransform::Identity, true);
			}
		}
	}
#else
	{
		for (auto Actor : CreatedActors)
		{
			if (Actor->IsValidLowLevel() && !Actor->IsPendingKill())//check, incase some actor is destroyed by other actor when BeginPlay
			{
				Actor->FinishSpawning(FTransform::Identity, true);//BeginPlay is called at this point
			}
		}
	}
#endif
	//assign parent for blueprint actor after actor spawn, otherwise blueprint actor will attach to world root
	for (auto item : BlueprintAndParentArray)
	{
		if (item.BlueprintActor->IsValidLowLevel() && item.ParentActor->IsValidLowLevel())
		{
			item.BlueprintActor->GetRootComponent()->AttachToComponent(item.ParentActor, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}

	if (CallbackBeforeAwake != nullptr)
	{
		CallbackBeforeAwake(CreatedRootActor);
	}

	for (auto item : CreatedActors)
	{
		LGUIManagerActor->RemoveActorForPrefabSystem(item, DeserializationSessionId);
	}
	if (DeserializationSessionId.IsValid())
	{
		LGUIManagerActor->EndPrefabSystemProcessingActor(DeserializationSessionId);
	}

	auto TimeSpan = FDateTime::Now() - StartTime;
	UE_LOG(LGUI, Log, TEXT("Use %f seconds to LoadPrefab: %s"), TimeSpan.GetTotalSeconds(), *InPrefab->GetName());

	return CreatedRootActor;
}


#include "Engine/Engine.h"
void ActorSerializer::LogForBitConvertFail(bool success, FProperty* Property)
{
	if (!success)
	{
		auto msg = FString::Printf(TEXT("bitconvert fail, property:%s, propertyType:%s, prefab:%s"), *(Property->GetPathName()), *(Property->GetClass()->GetName()), *Prefab->GetPathName());
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, msg);
		}
		UE_LOG(LGUI, Error, TEXT("%s"), *msg);
	}
}

TArray<FName> ActorSerializer::GetActorExcludeProperties(bool instigator, bool actorGuid)
{
	TArray<FName> result;
	result.Reserve(6);
	if (instigator)
	{
		result.Add("Instigator");
	}
	result.Add("RootComponent"); //this will result in the copied actor have same RootComponent to original actor, and crash. so we need to skip it
	result.Add("BlueprintCreatedComponents");
	result.Add("InstanceComponents");
#if WITH_EDITORONLY_DATA
	if (actorGuid)
	{
		result.Add(Name_ActorGuid); //ActorGuid is generated when spawn in world, should be unique
	}
#endif
	return result;
}
TArray<FName> ActorSerializer::GetComponentExcludeProperties()
{
	TArray<FName> result;
	result.Reserve(1);
	result.Add("AttachParent");
	return result;
}


int32 ActorSerializer::FindOrAddAssetIdFromList(UObject* AssetObject)
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
int32 ActorSerializer::FindOrAddStringFromList(FString string)//put string into list to avoid same string sotred multiple times
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
int32 ActorSerializer::FindOrAddNameFromList(FName name)
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
int32 ActorSerializer::FindOrAddTextFromList(FText text)
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
int32 ActorSerializer::FindOrAddClassFromList(UClass* uclass)
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

UObject* ActorSerializer::FindAssetFromListByIndex(int32 Id, ULGUIPrefab* InPrefab)
{
	auto& ReferenceAssetList = InPrefab->ReferenceAssetList;
	int32 count = ReferenceAssetList.Num();
	if (Id >= count || Id < 0)
	{
		return nullptr;
	}
	return ReferenceAssetList[Id];
}
FString ActorSerializer::FindStringFromListByIndex(int32 Id, ULGUIPrefab* InPrefab)
{
	auto& ReferenceStringList = InPrefab->ReferenceStringList;
	int32 count = ReferenceStringList.Num();
	if (Id >= count || Id < 0)
	{
		return FString();
	}
	return ReferenceStringList[Id];
}
FName ActorSerializer::FindNameFromListByIndex(int32 Id, ULGUIPrefab* InPrefab)
{
	auto& ReferenceNameList = InPrefab->ReferenceNameList;
	int32 count = ReferenceNameList.Num();
	if (Id >= count || Id < 0)
	{
		return FName();
	}
	return ReferenceNameList[Id];
}
FText ActorSerializer::FindTextFromListByIndex(int32 Id, ULGUIPrefab* InPrefab)
{
	auto& ReferenceTextList = InPrefab->ReferenceTextList;
	int32 count = ReferenceTextList.Num();
	if (Id >= count || Id < 0)
	{
		return FText();
	}
	return ReferenceTextList[Id];
}
UClass* ActorSerializer::FindClassFromListByIndex(int32 Id, ULGUIPrefab* InPrefab)
{
	auto& ReferenceClassList = InPrefab->ReferenceClassList;
	int32 count = ReferenceClassList.Num();
	if (Id >= count || Id < 0)
	{
		return nullptr;
	}
	return ReferenceClassList[Id];
}
void ActorSerializer::RegisterComponent(AActor* Actor, UActorComponent* Comp)
{
	switch (Comp->CreationMethod)
	{
	default:
	case EComponentCreationMethod::SimpleConstructionScript:
	case EComponentCreationMethod::UserConstructionScript:
	{
		Actor->FinishAndRegisterComponent(Comp);
	}
	break;
	case EComponentCreationMethod::Instance:
	{
		Comp->RegisterComponent();
		Actor->AddInstanceComponent(Comp);
	}
	break;
	}
}

FString ActorSerializer::GetValueAsString(const FLGUIPropertyData &ItemPropertyData)
{
	bool bitConvertSuccess;
	switch (ItemPropertyData.PropertyType)
	{

	case ELGUIPropertyType::PT_bool:
	{
		auto value = BitConverter::ToBoolean(ItemPropertyData.Data, bitConvertSuccess);
		return value ? FString(TEXT("true")) : FString(TEXT("false"));
	}
	break;

	case ELGUIPropertyType::PT_int8:
	{
		auto value = BitConverter::ToInt8(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("%i"), value);
	}
	break;
	case ELGUIPropertyType::PT_int16:
	{
		auto value = BitConverter::ToInt16(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("%i"), value);
	}
	break;
	case ELGUIPropertyType::PT_int32:
	{
		auto value = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("%i"), value);
	}
	break;
	case ELGUIPropertyType::PT_int64:
	{
		auto value = BitConverter::ToInt64(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("%i"), value);
	}
	break;

	case ELGUIPropertyType::PT_uint8:
	{
		auto value = BitConverter::ToUInt8(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("%u"), value);
	}
	break;
	case ELGUIPropertyType::PT_uint16:
	{
		auto value = BitConverter::ToUInt16(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("%u"), value);
	}
	break;
	case ELGUIPropertyType::PT_uint32:
	{
		auto value = BitConverter::ToUInt32(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("%u"), value);
	}
	break;
	case ELGUIPropertyType::PT_uint64:
	{
		auto value = BitConverter::ToUInt64(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("%u"), value);
	}
	break;

	case ELGUIPropertyType::PT_float:
	{
		auto value = BitConverter::ToFloat(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("%f"), value);
	}
	break;
	case ELGUIPropertyType::PT_double:
	{
		auto value = BitConverter::ToDouble(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("%f"), value);
	}
	break;

	case ELGUIPropertyType::PT_Vector2:
	{
		auto value = BitConverter::ToVector2(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("X=%f Y=%f"), value.X, value.Y);
	}
	break;
	case ELGUIPropertyType::PT_Vector3:
	{
		auto value = BitConverter::ToVector3(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("X=%f Y=%f Z=%f"), value.X, value.Y, value.Z);
	}
	break;
	case ELGUIPropertyType::PT_Vector4:
	{
		auto value = BitConverter::ToVector4(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("X=%f Y=%f Z=%f W=%f"), value.X, value.Y, value.Z, value.W);
	}
	break;
	case ELGUIPropertyType::PT_Quat:
	{
		auto value = BitConverter::ToQuat(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("X=%f Y=%f Z=%f W=%f"), value.X, value.Y, value.Z, value.W);
	}
	break;
	case ELGUIPropertyType::PT_Color:
	{
		auto value = BitConverter::ToColor(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("(R=%i,G=%i,B=%i,A=%i)"), value.R, value.G, value.B, value.A);
	}
	break;
	case ELGUIPropertyType::PT_LinearColor:
	{
		auto value = BitConverter::ToLinearColor(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("(R=%f,G=%f,B=%f,A=%f)"), value.R, value.G, value.B, value.A);
	}
	break;
	case ELGUIPropertyType::PT_Rotator:
	{
		auto value = BitConverter::ToRotator(ItemPropertyData.Data, bitConvertSuccess);
		return FString::Printf(TEXT("P=%f Y=%f R=%f"), value.Pitch, value.Yaw, value.Roll);
	}
	break;

	case ELGUIPropertyType::PT_Name:
	{
		if (ItemPropertyData.Data.Num() == 4)
		{
			auto index = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);
			return FindNameFromListByIndex(index, Prefab.Get()).ToString();
		}
	}
	break;
	case ELGUIPropertyType::PT_String:
	{
		if (ItemPropertyData.Data.Num() == 4)
		{
			auto index = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);
			return FindStringFromListByIndex(index, Prefab.Get());
		}
	}
	break;
	case ELGUIPropertyType::PT_Text:
	{
		if (ItemPropertyData.Data.Num() == 4)
		{
			auto index = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);
			return FindTextFromListByIndex(index, Prefab.Get()).ToString();
		}
	}
	break;

	default:
		break;
	}
	return FString();
}

#if WITH_EDITOR
FGuid FLGUIActorSaveData::GetActorGuid(FGuid fallbackGuid) const
{
	FGuid guid;
	FLGUIPropertyData guidData;
	if (FLGUIPropertyData::GetPropertyDataFromArray(ActorSerializer::Name_ActorGuid, ActorPropertyData, guidData))
	{
		if (guidData.ContainerData.Num() == 4)
		{
			bool succeedA, succeedB, succeedC, succeedD;
			uint32 A = BitConverter::ToUInt32(guidData.ContainerData[0].Data, succeedA);
			uint32 B = BitConverter::ToUInt32(guidData.ContainerData[1].Data, succeedB);
			uint32 C = BitConverter::ToUInt32(guidData.ContainerData[2].Data, succeedC);
			uint32 D = BitConverter::ToUInt32(guidData.ContainerData[3].Data, succeedD);
			if (succeedA && succeedB && succeedC && succeedD)
			{
				guid = FGuid(A, B, C, D);
				return guid;
			}
		}
	}
	UE_LOG(LGUI, Warning, TEXT("[FLGUIActorSaveData::GetActorGuid] Failed to get ActorGuid, maybe this Prefab is created by previous UE version."));
	guid = fallbackGuid;
	return guid;
}
void FLGUIActorSaveData::SetActorGuid(FGuid guid)
{
	for (int i = 0; i < ActorPropertyData.Num(); i++)
	{
		if (ActorPropertyData[i].Name == ActorSerializer::Name_ActorGuid)
		{
			FLGUIPropertyData& guidData = ActorPropertyData[i];
			if (guidData.ContainerData.Num() == 4
				&& guidData.ContainerData[0].Name == FName(TEXT("A"))
				&& guidData.ContainerData[1].Name == FName(TEXT("B"))
				&& guidData.ContainerData[2].Name == FName(TEXT("C"))
				&& guidData.ContainerData[3].Name == FName(TEXT("D"))
				)
			{
				guidData.ContainerData[0].Data = BitConverter::GetBytes(guid.A);
				guidData.ContainerData[1].Data = BitConverter::GetBytes(guid.B);
				guidData.ContainerData[2].Data = BitConverter::GetBytes(guid.C);
				guidData.ContainerData[3].Data = BitConverter::GetBytes(guid.D);
			}
		}
	}
}
#endif

#if WITH_EDITOR
FName ActorSerializer::Name_ActorGuid = FName(TEXT("ActorGuid"));
#endif

#endif
