// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Utils/LGUIUtils.h"
#include "LGUI.h"

#define BITCONVERTER_ERRORCHECK_EDITOR(func, bytes, byteCountNeeded, returnValueIfError)\
if(bytes.Num() < byteCountNeeded)\
{\
	auto ErrorMsg = FString::Printf(ErrorMsgFormat, TEXT(#func), bytes.Num());\
	UE_LOG(LGUI, Error, TEXT("%s"), *ErrorMsg);\
	LGUIUtils::EditorNotification(FText::FromString(ErrorMsg));\
	return returnValueIfError; \
}\

#define BITCONVERTER_ERRORCHECK_RUNTIME(func, bytes, byteCountNeeded)\
checkf(bytes.Num() >= byteCountNeeded, ErrorMsgFormat, TEXT(#func), bytes.Num());

class LGUI_API BitConverter
{
public:
	static bool ToBoolean(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToBoolean, bytes, 1, false)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToBoolean, bytes, 1)
#endif
		return bytes[0] == 1;
	}
	static int8 ToInt8(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToInt8, bytes, 1, 0)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToInt8, bytes, 1)
#endif
		int8 result;
		FMemory::Memcpy(&result, bytes.GetData(), 1);
		return result;
	}
	static uint8 ToUInt8(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToUInt8, bytes, 1, 0)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToUInt8, bytes, 1)
#endif
		uint8 result;
		FMemory::Memcpy(&result, bytes.GetData(), 1);
		return result;
	}
	static int16 ToInt16(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToInt16, bytes, 2, 0)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToInt16, bytes, 2)
#endif
		int16 result;
		FMemory::Memcpy(&result, bytes.GetData(), 2);
		return result;
	}
	static uint16 ToUInt16(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToUInt16, bytes, 2, 0)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToUInt16, bytes, 2)
#endif
		uint16 result;
		FMemory::Memcpy(&result, bytes.GetData(), 2);
		return result;
	}
	static int32 ToInt32(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToInt32, bytes, 4, 0)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToInt32, bytes, 4)
#endif
		int32 result;
		FMemory::Memcpy(&result, bytes.GetData(), 4);
		return result;
	}
	static uint32 ToUInt32(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToUInt32, bytes, 4, 0)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToUInt32, bytes, 4)
#endif
		uint32 result;
		FMemory::Memcpy(&result, bytes.GetData(), 4);
		return result;
	}
	static int64 ToInt64(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToInt64, bytes, 8, 0)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToInt64, bytes, 8)
#endif
		int64 result;
		FMemory::Memcpy(&result, bytes.GetData(), 8);
		return result;
	}
	static uint64 ToUInt64(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToUInt64, bytes, 8, 0)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToUInt64, bytes, 8)
#endif
		uint64 result;
		FMemory::Memcpy(&result, bytes.GetData(), 8);
		return result;
	}
	static float ToFloat(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToFloat, bytes, 4, 0)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToFloat, bytes, 4)
#endif
		float result;
		FMemory::Memcpy(&result, bytes.GetData(), 4);
		return result;
	}
	static double ToDouble(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToDouble, bytes, 8, 0)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToDouble, bytes, 8)
#endif
		double result;
		FMemory::Memcpy(&result, bytes.GetData(), 8);
		return result;
	}
	static FVector2D ToVector2(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToVector2, bytes, 8, FVector2D::ZeroVector)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToVector2, bytes, 8)
#endif
		FVector2D result;
		FMemory::Memcpy(&result, bytes.GetData(), 8);
		return result;
	}
	static FVector ToVector3(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToVector3, bytes, 12, FVector::ZeroVector)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToVector3, bytes, 12)
#endif
		FVector result;
		FMemory::Memcpy(&result, bytes.GetData(), 12);
		return result;
	}
	static FVector4 ToVector4(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToVector4, bytes, 16, FVector4(EForceInit::ForceInitToZero))
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToVector4, bytes, 16)
#endif
		FVector4 result;
		FMemory::Memcpy(&result, bytes.GetData(), 16);
		return result;
	}
	static FQuat ToQuat(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToBoolean, bytes, 16, FQuat::Identity)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToBoolean, bytes, 16)
#endif
		FQuat result;
		FMemory::Memcpy(&result, bytes.GetData(), 16);
		return result;
	}
	static FColor ToColor(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToColor, bytes, 4, FColor::White)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToColor, bytes, 4)
#endif
		FColor result;
		FMemory::Memcpy(&result, bytes.GetData(), 4);
		return result;
	}
	static FLinearColor ToLinearColor(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToLinearColor, bytes, 16, FLinearColor::White)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToLinearColor, bytes, 16)
#endif
		FLinearColor result;
		FMemory::Memcpy(&result, bytes.GetData(), 16);
		return result;
	}
	static FRotator ToRotator(const TArray<uint8>& bytes)
	{
#if WITH_EDITOR
		BITCONVERTER_ERRORCHECK_EDITOR(BitConverter::ToRotator, bytes, 12, FRotator::ZeroRotator)
#else
		BITCONVERTER_ERRORCHECK_RUNTIME(BitConverter::ToRotator, bytes, 12)
#endif
		FRotator result;
		FMemory::Memcpy(&result, bytes.GetData(), 12);
		return result;
	}

	static TArray<uint8> GetBytes(bool InBool)
	{
		TArray<uint8> result;
		result.Add(InBool ? 1 : 0);
		return result;
	}
	static TArray<uint8> GetBytes(int8 InInt)
	{
		return GetBytes((uint8*)&InInt, 1);
	}
	static TArray<uint8> GetBytes(uint8 InInt)
	{
		return GetBytes((uint8*)&InInt, 1);
	}
	static TArray<uint8> GetBytes(int16 InInt)
	{
		return GetBytes((uint8*)&InInt, 2);
	}
	static TArray<uint8> GetBytes(uint16 InInt)
	{
		return GetBytes((uint8*)&InInt, 2);
	}
	static TArray<uint8> GetBytes(int32 InInt)
	{
		return GetBytes((uint8*)&InInt, 4);
	}
	static TArray<uint8> GetBytes(uint32 InInt)
	{
		return GetBytes((uint8*)&InInt, 4);
	}
	static TArray<uint8> GetBytes(int64 InInt)
	{
		return GetBytes((uint8*)&InInt, 8);
	}
	static TArray<uint8> GetBytes(uint64 InInt)
	{
		return GetBytes((uint8*)&InInt, 8);
	}
	static TArray<uint8> GetBytes(float InFloat)
	{
		return GetBytes((uint8*)&InFloat, 4);
	}
	static TArray<uint8> GetBytes(double InDouble)
	{
		return GetBytes((uint8*)&InDouble, 8);
	}
	static TArray<uint8> GetBytes(FVector2D InValue)
	{
		return GetBytes((uint8*)&InValue, 8);
	}
	static TArray<uint8> GetBytes(FVector InValue)
	{
		return GetBytes((uint8*)&InValue, 12);
	}
	static TArray<uint8> GetBytes(FVector4 InValue)
	{
		return GetBytes((uint8*)&InValue, 16);
	}
	static TArray<uint8> GetBytes(FQuat InValue)
	{
		return GetBytes((uint8*)&InValue, 16);
	}
	static TArray<uint8> GetBytes(FColor InValue)
	{
		return GetBytes((uint8*)&InValue, 4);
	}
	static TArray<uint8> GetBytes(FLinearColor InValue)
	{
		return GetBytes((uint8*)&InValue, 16);
	}
	static TArray<uint8> GetBytes(FRotator InValue)
	{
		return GetBytes((uint8*)&InValue, 12);
	}
private:
	FORCEINLINE static TArray<uint8> GetBytes(uint8* InData, uint32 count)
	{
		TArray<uint8> result;
		result.AddUninitialized(count);
		FMemory::Memcpy(result.GetData(), InData, count);
		return result;
	}
	static const TCHAR ErrorMsgFormat[];
};
