// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class LGUI_API BitConverter
{
public:
	static bool ToBoolean(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 1, TEXT("[BitConvert/ToBoolen]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		return bytes[0] == 1;
	}
	static int8 ToInt8(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 1, TEXT("[BitConvert/ToInt8]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		int8 result;
		FMemory::Memcpy(&result, bytes.GetData(), 1);
		return result;
	}
	static uint8 ToUInt8(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 1, TEXT("[BitConvert/ToUInt8]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		uint8 result;
		FMemory::Memcpy(&result, bytes.GetData(), 1);
		return result;
	}
	static int16 ToInt16(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 2, TEXT("[BitConvert/ToInt16]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		int16 result;
		FMemory::Memcpy(&result, bytes.GetData(), 2);
		return result;
	}
	static uint16 ToUInt16(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 2, TEXT("[BitConvert/ToUInt16]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		uint16 result;
		FMemory::Memcpy(&result, bytes.GetData(), 2);
		return result;
	}
	static int32 ToInt32(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 4, TEXT("[BitConvert/ToInt32]bytes.Num:%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		int32 result;
		FMemory::Memcpy(&result, bytes.GetData(), 4);
		return result;
	}
	static uint32 ToUInt32(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 4, TEXT("[BitConvert/ToUInt32]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		uint32 result;
		FMemory::Memcpy(&result, bytes.GetData(), 4);
		return result;
	}
	static int64 ToInt64(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 8, TEXT("[BitConvert/ToInt64]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		int64 result;
		FMemory::Memcpy(&result, bytes.GetData(), 8);
		return result;
	}
	static uint64 ToUInt64(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 8, TEXT("[BitConvert/ToUInt64]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		uint64 result;
		FMemory::Memcpy(&result, bytes.GetData(), 8);
		return result;
	}
	static float ToFloat(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 4, TEXT("[BitConvert/ToFloat]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		float result;
		FMemory::Memcpy(&result, bytes.GetData(), 4);
		return result;
	}
	static double ToDouble(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 8, TEXT("[BitConvert/ToDouble]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		double result;
		FMemory::Memcpy(&result, bytes.GetData(), 8);
		return result;
	}
	static FVector2D ToVector2(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 8, TEXT("[BitConvert/ToVector2]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		FVector2D result;
		FMemory::Memcpy(&result, bytes.GetData(), 8);
		return result;
	}
	static FVector ToVector3(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 12, TEXT("[BitConvert/ToVector3]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		FVector result;
		FMemory::Memcpy(&result, bytes.GetData(), 12);
		return result;
	}
	static FVector4 ToVector4(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 16, TEXT("[BitConvert/ToVector4]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		FVector4 result;
		FMemory::Memcpy(&result, bytes.GetData(), 16);
		return result;
	}
	static FQuat ToQuat(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 16, TEXT("[BitConvert/ToQuat]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		FQuat result;
		FMemory::Memcpy(&result, bytes.GetData(), 16);
		return result;
	}
	static FColor ToColor(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 4, TEXT("[BitConvert/ToColor]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		FColor result;
		FMemory::Memcpy(&result, bytes.GetData(), 4);
		return result;
	}
	static FLinearColor ToLinearColor(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 16, TEXT("[BitConvert/ToLinearColor]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
		FLinearColor result;
		FMemory::Memcpy(&result, bytes.GetData(), 16);
		return result;
	}
	static FRotator ToRotator(const TArray<uint8>& bytes)
	{
		checkf(bytes.Num() >= 12, TEXT("[BitConvert/ToRotator]bytes.Num%d is not enough! If this happened when loading a LGUIPrefab, then recreate that prefab may fix it."), bytes.Num());
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
};