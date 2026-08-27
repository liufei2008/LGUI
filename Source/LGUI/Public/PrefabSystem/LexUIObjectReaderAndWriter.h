// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/ObjectWriter.h"
#include "Serialization/ObjectReader.h"
#include "Serialization/ArchiveSerializedPropertyChain.h"

namespace LexUIPrefabSystem
{
	class WidgetSerializerBase;

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
		NativeSerializeForDuplicate,
	};

	/** 
	 * Tell if it is a UObject's member property
	 */
	bool LexUIPrefab_CurrentIsMemberProperty(const FMemoryArchive* InMemAr);
	bool LexUIPrefab_ShouldSkipProperty(const FProperty* InProperty);

	class LGUI_API FLexUIObjectWriter : public FObjectWriter
	{
	public:
		FLexUIObjectWriter(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer);
		virtual void DoSerialize(UObject* Object);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(FName& N) override;
		virtual FArchive& operator<<(FText& Value) override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FObjectPtr& Value) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FArchive& operator<<(FLazyObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPath& Value) override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject* Object);
	protected:
		WidgetSerializerBase& Serializer;
	};
	class LGUI_API FLexUIObjectReader : public FObjectReader
	{
	public:
		FLexUIObjectReader(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer);
		virtual void DoSerialize(UObject* Object);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FArchive& operator<<(FName& N) override;
		virtual FArchive& operator<<(FText& Value) override;
		virtual FArchive& operator<<(UObject*& Res) override;
		virtual FArchive& operator<<(FObjectPtr& Value) override;
		virtual FArchive& operator<<(FWeakObjectPtr& Value) override;
		virtual FArchive& operator<<(FLazyObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
		virtual FArchive& operator<<(FSoftObjectPath& Value) override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject*& Object, bool CanSerializeClass);
	protected:
		WidgetSerializerBase& Serializer;
	};

	class LGUI_API FLexUIDuplicateObjectWriter : public FLexUIObjectWriter
	{
	public:
		FLexUIDuplicateObjectWriter(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject* Object)override;
	};
	class LGUI_API FLexUIDuplicateObjectReader : public FLexUIObjectReader
	{
	public:
		FLexUIDuplicateObjectReader(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject*& Object, bool CanSerializeClass)override;
	};



	class LGUI_API FLexUIOverrideParameterObjectWriter : public FLexUIObjectWriter
	{
	public:
		FLexUIOverrideParameterObjectWriter(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject* Object);
	protected:
		mutable TSet<FName> OverridePropertyNames;
	};
	class LGUI_API FLexUIOverrideParameterObjectReader : public FLexUIObjectReader
	{
	public:
		FLexUIOverrideParameterObjectReader(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject*& Object, bool CanSerializeClass);
	protected:
		mutable TSet<FName> OverridePropertyNames;
	};


	class LGUI_API FLexUIDuplicateOverrideParameterObjectWriter : public FLexUIOverrideParameterObjectWriter
	{
	public:
		FLexUIDuplicateOverrideParameterObjectWriter(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject* Object)override;
	};
	class LGUI_API FLexUIDuplicateOverrideParameterObjectReader : public FLexUIOverrideParameterObjectReader
	{
	public:
		FLexUIDuplicateOverrideParameterObjectReader(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject*& Object, bool CanSerializeClass)override;
	};


	/**
	 * Serializes a single property value rather than a whole object, so one nested leaf can be stored on its own.
	 * Derives from the override archives to inherit the reference-table remapping for FName/FText/UObject and the
	 * SerializeObject that refuses to mint new guids for objects outside the prefab.
	 */
	class LGUI_API FLexUIOverrideSubPropertyValueWriter : public FLexUIOverrideParameterObjectWriter
	{
	public:
		FLexUIOverrideSubPropertyValueWriter(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer);
		void SerializeValue(FProperty* InLeafProperty, void* InLeafValuePtr);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
	};
	class LGUI_API FLexUIOverrideSubPropertyValueReader : public FLexUIOverrideParameterObjectReader
	{
	public:
		FLexUIOverrideSubPropertyValueReader(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer);
		void SerializeValue(FProperty* InLeafProperty, void* InLeafValuePtr);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
	};


	class LGUI_API FLexUIImmediateOverrideParameterObjectWriter : public FObjectWriter
	{
	public:
		FLexUIImmediateOverrideParameterObjectWriter(UObject* Object, TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
	private:
		mutable TSet<FName> OverridePropertyNames;
	};
	class LGUI_API FLexUIImmediateOverrideParameterObjectReader : public FObjectReader
	{
	public:
		FLexUIImmediateOverrideParameterObjectReader(UObject* Object, TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, const TArray<FName>& InOverridePropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
	private:
		mutable TSet<FName> OverridePropertyNames;
	};
}