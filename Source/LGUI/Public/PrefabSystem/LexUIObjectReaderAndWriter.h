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
	bool LexUIPrefab_ShouldSkipProperty(const FProperty* InProperty);

	class LGUI_API FLexUIObjectWriter : public FObjectWriter
	{
	public:
		FLexUIObjectWriter(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames);
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
		TSet<FName> SkipPropertyNames;
	};
	class LGUI_API FLexUIObjectReader : public FObjectReader
	{
	public:
		FLexUIObjectReader(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames);
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
		TSet<FName> SkipPropertyNames;
	};

	class LGUI_API FLexUIDuplicateObjectWriter : public FLexUIObjectWriter
	{
	public:
		FLexUIDuplicateObjectWriter(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames);

		virtual bool ShouldSkipProperty(const FProperty* InProperty) const override;
		virtual FString GetArchiveName() const override;
		virtual bool SerializeObject(UObject* Object)override;
	};
	class LGUI_API FLexUIDuplicateObjectReader : public FLexUIObjectReader
	{
	public:
		FLexUIDuplicateObjectReader(TArray< uint8 >& Bytes, WidgetSerializerBase& InSerializer, TSet<FName> InSkipPropertyNames);

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