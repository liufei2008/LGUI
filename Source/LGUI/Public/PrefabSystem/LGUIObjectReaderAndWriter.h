// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/ObjectWriter.h"
#include "Serialization/ObjectReader.h"
#include "Serialization/ArchiveSerializedPropertyChain.h"

namespace LGUIPrefabSystem
{
	class ActorSerializerBase;

	enum class EObjectType :uint8
	{
		None,
		/** Asset resource */
		Asset,
		/** UClass */
		Class,
		/** UObject reference(Not asset), include actor/ component/ uobject */
		ObjectReference,
		/** Only for duplicate, use native ObjectWriter/ObjectReader serialization method */
		NativeSerailizeForDuplicate,
	};

	/** 
	 * If not have valid property chain, then it is member property.
	 * Why use a template instead of FArchiveState? Because FArchiveState's construcion is prirvate, I can't convert FLGUIObjectWriterXXX to FArchiveState.
	 */
	template<class T>
	bool CurrentIsMemberProperty(const T& t)
	{
		auto PropertyChain = t.GetSerializedPropertyChain();
		if (PropertyChain == nullptr || PropertyChain->GetNumProperties() == 0)
		{
			return true;
		}
		return false;
	}
	bool LGUIPrefab_ShouldSkipProperty(const FProperty* InProperty);

	class LGUI_API FLGUIObjectWriter : public FObjectWriter
	{
	public:
		FLGUIObjectWriter(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames);
		virtual void DoSerialize(UObject* Object);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(class FName& N) override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FArchive& operator<<(FLazyObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPath& Value) override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject* Object);
	protected:
		ActorSerializerBase& Serializer;
		TSet<FName> SkipPropertyNames;
	};
	class LGUI_API FLGUIObjectReader : public FObjectReader
	{
	public:
		FLGUIObjectReader(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames);
		virtual void DoSerialize(UObject* Object);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(class FName& N) override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FArchive& operator<<(FLazyObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPath& Value) override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject*& Object, bool CanSerializeClass);
	protected:
		ActorSerializerBase& Serializer;
		TSet<FName> SkipPropertyNames;
	};

	class LGUI_API FLGUIDuplicateObjectWriter : public FLGUIObjectWriter
	{
	public:
		FLGUIDuplicateObjectWriter(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames);

		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject* Object)override;
	};
	class LGUI_API FLGUIDuplicateObjectReader : public FLGUIObjectReader
	{
	public:
		FLGUIDuplicateObjectReader(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames);

		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject*& Object, bool CanSerializeClass)override;
	};



	class LGUI_API FLGUIOverrideParameterObjectWriter : public FObjectWriter
	{
	public:
		FLGUIOverrideParameterObjectWriter(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TSet<FName>& InOverridePropertyNames);
		virtual void DoSerialize(UObject* Object);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(class FName& N) override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FArchive& operator<<(FLazyObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPath& Value) override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject* Object);
	protected:
		ActorSerializerBase& Serializer;
		mutable TSet<FName> OverridePropertyNames;
	};
	class LGUI_API FLGUIOverrideParameterObjectReader : public FObjectReader
	{
	public:
		FLGUIOverrideParameterObjectReader(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TSet<FName>& InOverridePropertyNames);
		virtual void DoSerialize(UObject* Object);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(class FName& N) override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FArchive& operator<<(FLazyObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPath& Value) override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject*& Object, bool CanSerializeClass);
	protected:
		ActorSerializerBase& Serializer;
		mutable TSet<FName> OverridePropertyNames;
	};


	class LGUI_API FLGUIDuplicateOverrideParameterObjectWriter : public FLGUIOverrideParameterObjectWriter
	{
	public:
		FLGUIDuplicateOverrideParameterObjectWriter(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames);

		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject* Object)override;
	};
	class LGUI_API FLGUIDuplicateOverrideParameterObjectReader : public FLGUIOverrideParameterObjectReader
	{
	public:
		FLGUIDuplicateOverrideParameterObjectReader(TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames);

		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject*& Object, bool CanSerializeClass)override;
	};


	class LGUI_API FLGUIImmediateOverrideParameterObjectWriter : public FObjectWriter
	{
	public:
		FLGUIImmediateOverrideParameterObjectWriter(UObject* Object, TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TSet<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
	private:
		mutable TSet<FName> OverridePropertyNames;
	};
	class LGUI_API FLGUIImmediateOverrideParameterObjectReader : public FObjectReader
	{
	public:
		FLGUIImmediateOverrideParameterObjectReader(UObject* Object, TArray< uint8 >& Bytes, ActorSerializerBase& InSerializer, const TSet<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
	private:
		mutable TSet<FName> OverridePropertyNames;
	};
}