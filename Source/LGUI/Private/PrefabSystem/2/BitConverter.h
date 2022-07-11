// Copyright 2019-2022 LexLiu. All Rights Reserved.

#if WITH_EDITOR
#pragma once

#include "CoreMinimal.h"
#include "Utils/LGUIUtils.h"
#include "LGUI.h"

#define BITCONVERTER_ERRORCHECK(func, bytes, byteCountNeeded, returnValueIfError)\
if(bytes.Num() < byteCountNeeded)\
{\
	auto ErrorMsg = FString::Printf(TEXT("[BitConvert/%s]bytes.Num %d not enough!"), TEXT(#func), bytes.Num());\
	UE_LOG(LGUI, Error, TEXT("%s"), *ErrorMsg);\
	outSucceed = false;\
	return returnValueIfError; \
}\

//@todo: use build-in instead of BitConverter
class LGUI_API BitConverter
{
public:
	static bool ToBoolean(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToBoolean, bytes, 1, false)
		outSucceed = true;
		return bytes[0] == 1;
	}
	static int8 ToInt8(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToInt8, bytes, 1, 0)
		int8 result;
		FMemory::Memcpy(&result, bytes.GetData(), 1);
		outSucceed = true;
		return result;
	}
	static uint8 ToUInt8(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToUInt8, bytes, 1, 0)
		uint8 result;
		FMemory::Memcpy(&result, bytes.GetData(), 1);
		return result;
	}
	static int16 ToInt16(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToInt16, bytes, 2, 0)
		int16 result;
		FMemory::Memcpy(&result, bytes.GetData(), 2);
		outSucceed = true;
		return result;
	}
	static uint16 ToUInt16(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToUInt16, bytes, 2, 0)
		uint16 result;
		FMemory::Memcpy(&result, bytes.GetData(), 2);
		outSucceed = true;
		return result;
	}
	static int32 ToInt32(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToInt32, bytes, 4, 0)
		int32 result;
		FMemory::Memcpy(&result, bytes.GetData(), 4);
		outSucceed = true;
		return result;
	}
	static uint32 ToUInt32(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToUInt32, bytes, 4, 0)
		uint32 result;
		FMemory::Memcpy(&result, bytes.GetData(), 4);
		outSucceed = true;
		return result;
	}
	static int64 ToInt64(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToInt64, bytes, 8, 0)
		int64 result;
		FMemory::Memcpy(&result, bytes.GetData(), 8);
		outSucceed = true;
		return result;
	}
	static uint64 ToUInt64(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToUInt64, bytes, 8, 0)
		uint64 result;
		FMemory::Memcpy(&result, bytes.GetData(), 8);
		outSucceed = true;
		return result;
	}
	static float ToFloat(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToFloat, bytes, 4, 0)
		float result;
		FMemory::Memcpy(&result, bytes.GetData(), 4);
		outSucceed = true;
		return result;
	}
	static double ToDouble(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToDouble, bytes, 8, 0)
		double result;
		FMemory::Memcpy(&result, bytes.GetData(), 8);
		outSucceed = true;
		return result;
	}
	static FVector2D ToVector2(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToVector2, bytes, 8, FVector2D::ZeroVector)
		FVector2D result;
		FMemory::Memcpy(&result, bytes.GetData(), 8);
		outSucceed = true;
		return result;
	}
	static FVector ToVector3(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToVector3, bytes, 12, FVector::ZeroVector)
		FVector result;
		FMemory::Memcpy(&result, bytes.GetData(), 12);
		outSucceed = true;
		return result;
	}
	static FVector4 ToVector4(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToVector4, bytes, 16, FVector4(EForceInit::ForceInitToZero))
		FVector4 result;
		FMemory::Memcpy(&result, bytes.GetData(), 16);
		outSucceed = true;
		return result;
	}
	static FQuat ToQuat(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToBoolean, bytes, 16, FQuat::Identity)
		FQuat result;
		FMemory::Memcpy(&result, bytes.GetData(), 16);
		outSucceed = true;
		return result;
	}
	static FColor ToColor(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToColor, bytes, 4, FColor::White)
		FColor result;
		FMemory::Memcpy(&result, bytes.GetData(), 4);
		outSucceed = true;
		return result;
	}
	static FLinearColor ToLinearColor(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToLinearColor, bytes, 16, FLinearColor::White)
		FLinearColor result;
		FMemory::Memcpy(&result, bytes.GetData(), 16);
		outSucceed = true;
		return result;
	}
	static FRotator ToRotator(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToRotator, bytes, 12, FRotator::ZeroRotator)
		FRotator result;
		FMemory::Memcpy(&result, bytes.GetData(), 12);
		outSucceed = true;
		return result;
	}
	static FGuid ToGuid(const TArray<uint8>& bytes, bool& outSucceed)
	{
		BITCONVERTER_ERRORCHECK(BitConverter::ToGuid, bytes, 16, FGuid())
		FGuid result;
		FMemory::Memcpy(&result, bytes.GetData(), 16);
		outSucceed = true;
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
	static TArray<uint8> GetBytes(FGuid InValue)
	{
		return GetBytes((uint8*)&InValue, 16);
	}
private:
	FORCEINLINE static TArray<uint8> GetBytes(uint8* InData, uint32 count)
	{
		TArray<uint8> result;
		result.AddUninitialized(count);
		FMemory::Memcpy(result.GetData(), InData, count);
		return result;
	}
};

#endif
