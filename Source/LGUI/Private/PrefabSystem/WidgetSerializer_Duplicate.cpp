// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/WidgetSerializer.h"
#include "PrefabSystem/LexUIObjectReaderAndWriter.h"
#include "Serialization/MemoryReader.h"
#include "Runtime/Launch/Resources/Version.h"
#include "LGUI.h"
#include "Core/LexUIManager.h"
#include "Core/LexUISettings.h"
#include "Core/Components/LexWidget.h"

namespace LEXUIPREFAB_SERIALIZER_NEWEST_NAMESPACE
{
	ULexWidget* WidgetSerializer::DuplicateWidget(UWorld* InWorld, UObject* InOwnerObject, ULexWidget* OriginRootWidget, ULexWidget* Parent)
	{
		if (!OriginRootWidget)
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d OriginRootWidget is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}
		WidgetSerializer serializer;
		serializer.World = InWorld;
		serializer.OwnerObject = InOwnerObject;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = false;

		auto Name = OriginRootWidget->GetDisplayName();
		auto StartTime = FDateTime::Now();

		//serialize
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer) {
			LexUIPrefabSystem::FLexUIDuplicateObjectWriter Writer(InOutBuffer, serializer, {});
			Writer.DoSerialize(InObject);
		};
		FLexUIPrefabSaveData SaveData;
		serializer.SerializeWidgetToData(OriginRootWidget, SaveData);

		//deserialize
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer) {
			LexUIPrefabSystem::FLexUIDuplicateObjectReader Reader(InOutBuffer, serializer, {});
			Reader.DoSerialize(InObject);
		};
		auto CreatedRootWidget = serializer.DeserializeWidgetFromData(SaveData, Parent, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);

		if (GetDefault<ULexUIEditorSettings>()->bLogPrefabLoadTime)
		{
			auto TimeSpan = FDateTime::Now() - StartTime;
			UE_LOG(LGUI, Log, TEXT("Duplicate actor: '%s', total time: %fms"), *Name, TimeSpan.GetTotalMilliseconds());
		}

		return CreatedRootWidget;
	}
	bool WidgetSerializer::PrepareDataForDuplicate(ULexWidget* OriginRootWidget, FDuplicateWidgetDataContainer& OutData)
	{
		if (!OriginRootWidget)
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d OriginRootWidget is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return false;
		}

		auto Name = OriginRootWidget->GetDisplayName();
		auto StartTime = FDateTime::Now();

		auto& serializer = OutData.Serializer;
		serializer.OwnerObject = OriginRootWidget->GetOuter();
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = false;

		//serialize
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer) {
			LexUIPrefabSystem::FLexUIDuplicateObjectWriter Writer(InOutBuffer, serializer, {});
			Writer.DoSerialize(InObject);
		};
		serializer.SerializeWidgetToData(OriginRootWidget, OutData.WidgetData);

		//for deserialize, set once for all use
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer) {
			LexUIPrefabSystem::FLexUIDuplicateObjectReader Reader(InOutBuffer, serializer, {});
			Reader.DoSerialize(InObject);
		};

		if (GetDefault<ULexUIEditorSettings>()->bLogPrefabLoadTime)
		{
			auto TimeSpan = FDateTime::Now() - StartTime;
			UE_LOG(LGUI, Log, TEXT("PrepareData_ForDuplicate, actor: '%s' total time: %fms"), *Name, TimeSpan.GetTotalMilliseconds());
		}
		return true;
	}
	ULexWidget* WidgetSerializer::DuplicateWidgetWithPreparedData(UWorld* InWorld, UObject* InOwnerObject, FDuplicateWidgetDataContainer& InData, ULexWidget* InParent)
	{
		auto StartTime = FDateTime::Now();
		auto& serializer = InData.Serializer;//use copied, incase undesired data
		serializer.World = InWorld;
		serializer.OwnerObject = InOwnerObject;
		//clear these data for deserializer use
		serializer.WillSerializeWidgetArray.Reset();
		serializer.WillSerializeObjectArray.Reset();
		serializer.MapGuidToObject.Reset();
		serializer.MapObjectToGuid.Reset();
		serializer.SubPrefabMap.Reset();
		serializer.AllWidgetArray.Reset();
		serializer.SubPrefabOverrideParameters.Reset();
		serializer.bIsSubPrefab = false;
		serializer.SubPrefabObjectOverrideData.Reset();

		auto CreatedRootWidget = serializer.DeserializeWidgetFromData(InData.WidgetData, InParent, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
		if (GetDefault<ULexUIEditorSettings>()->bLogPrefabLoadTime)
		{
			auto TimeSpan = FDateTime::Now() - StartTime;
			UE_LOG(LGUI, Log, TEXT("DuplicateWidgetWithPreparedData total time: %fms"), TimeSpan.GetTotalMilliseconds());
		}
		return CreatedRootWidget;
	}

	ULexWidget* WidgetSerializer::DuplicateWidgetForEditor(UWorld* InWorld, ULexWidget* OriginRootWidget, ULexWidget* Parent
		, const TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData>& InSubPrefabMap
		, const TMap<UObject*, FGuid>& InMapObjectToGuid
		, TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData>& OutDuplicatedSubPrefabMap
		, TMap<FGuid, TObjectPtr<UObject>>& OutMapGuidToObject
	)
	{
		if (!OriginRootWidget)
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d OriginRootWidget is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}

		auto Name = OriginRootWidget->GetDisplayName();
		UE_LOG(LGUI, Log, TEXT("Begin duplicate actor: '%s'"), *Name);
		auto StartTime = FDateTime::Now();

		WidgetSerializer serializer;
		serializer.World = InWorld;
		serializer.OwnerObject = Parent->GetOuter();
		serializer.MapObjectToGuid = InMapObjectToGuid;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = false;
		//serialize
		serializer.SubPrefabMap = InSubPrefabMap;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer) {
			LexUIPrefabSystem::FLexUIDuplicateObjectWriter Writer(InOutBuffer, serializer, {});
			Writer.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LexUIPrefabSystem::FLexUIDuplicateOverrideParameterObjectWriter Writer(InOutBuffer, serializer, InOverridePropertyNames);
			Writer.DoSerialize(InObject);
		};
		FLexUIPrefabSaveData SaveData;
		serializer.SerializeWidgetToData(OriginRootWidget, SaveData);

		//deserialize
		serializer.SubPrefabMap = {};//clear it for deserializer to fill
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer) {
			LexUIPrefabSystem::FLexUIDuplicateObjectReader Reader(InOutBuffer, serializer, {});
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNameSet) {
			LexUIPrefabSystem::FLexUIDuplicateOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNameSet);
			Reader.DoSerialize(InObject);
		};
		auto CreatedRootWidget = serializer.DeserializeWidgetFromData(SaveData, Parent, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);

		OutDuplicatedSubPrefabMap = serializer.SubPrefabMap;
		OutMapGuidToObject = serializer.MapGuidToObject;
		auto TimeSpan = FDateTime::Now() - StartTime;
		UE_LOG(LGUI, Log, TEXT("End duplicate actor: '%s', total time: %fms"), *Name, TimeSpan.GetTotalMilliseconds());

		return CreatedRootWidget;
	}
}

