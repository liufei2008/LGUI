// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/ObjectWriter.h"
#include "Serialization/ObjectReader.h"

namespace LGUIPrefabSystem3
{
	class ActorSerializer3;

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

	class FLGUIObjectWriter : public FObjectWriter
	{
	public:
		FLGUIObjectWriter(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, TSet<FName> InSkipPropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(class FName& N) override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FLazyObjectPtr& LazyObjectPtr) override;
		virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPath& Value) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FString GetArchiveName() const override;
		bool SerializeObject(UObject* Object);
	private:
		ActorSerializer3& Serializer;
		TSet<FName> SkipPropertyNames;
	};
	class FLGUIObjectReader : public FObjectReader
	{
	public:
		FLGUIObjectReader(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, TSet<FName> InSkipPropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(class FName& N) override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FLazyObjectPtr& LazyObjectPtr) override;
		virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPath& Value) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FString GetArchiveName() const override;
		bool SerializeObject(UObject*& Object, bool CanSerializeClass);
	private:
		ActorSerializer3& Serializer;
		TSet<FName> SkipPropertyNames;
	};

	class FLGUIDuplicateObjectWriter : public FObjectWriter
	{
	public:
		FLGUIDuplicateObjectWriter(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, TSet<FName> InSkipPropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FString GetArchiveName() const override;
		bool SerializeObject(UObject* Object);
	private:
		ActorSerializer3& Serializer;
		TSet<FName> SkipPropertyNames;
	};
	class FLGUIDuplicateObjectReader : public FObjectReader
	{
	public:
		FLGUIDuplicateObjectReader(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, TSet<FName> InSkipPropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FString GetArchiveName() const override;
		bool SerializeObject(UObject*& Object, bool CanSerializeClass);
	private:
		ActorSerializer3& Serializer;
		TSet<FName> SkipPropertyNames;
	};



	class FLGUIOverrideParameterObjectWriter : public FObjectWriter
	{
	public:
		FLGUIOverrideParameterObjectWriter(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, const TSet<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(class FName& N) override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FLazyObjectPtr& LazyObjectPtr) override;
		virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPath& Value) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FString GetArchiveName() const override;
		bool SerializeObject(UObject* Object);
	private:
		ActorSerializer3& Serializer;
		mutable TSet<FName> OverridePropertyNames;
	};
	class FLGUIOverrideParameterObjectReader : public FObjectReader
	{
	public:
		FLGUIOverrideParameterObjectReader(UObject* Object, TArray< uint8 >& Bytes, ActorSerializer3& InSerializer, const TSet<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(class FName& N) override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FLazyObjectPtr& LazyObjectPtr) override;
		virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPath& Value) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FString GetArchiveName() const override;
		bool SerializeObject(UObject*& Object, bool CanSerializeClass);
	private:
		ActorSerializer3& Serializer;
		mutable TSet<FName> OverridePropertyNames;
	};

	class FLGUIImmediateOverrideParameterObjectWriter : public FObjectWriter
	{
	public:
		FLGUIImmediateOverrideParameterObjectWriter(UObject* Object, TArray< uint8 >& Bytes, const TSet<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
	private:
		mutable TSet<FName> OverridePropertyNames;
	};
	class FLGUIImmediateOverrideParameterObjectReader : public FObjectReader
	{
	public:
		FLGUIImmediateOverrideParameterObjectReader(UObject* Object, TArray< uint8 >& Bytes, const TSet<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
	private:
		mutable TSet<FName> OverridePropertyNames;
	};
}