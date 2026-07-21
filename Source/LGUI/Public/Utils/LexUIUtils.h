// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Package.h"

class ULexWidget;
class ULexCanvas;
class UTexture2D;

#if !UE_BUILD_SHIPPING
//Check UObject valid, if not then return, to prevent crash.
//Removed in shipping build.
#define LGUI_CHECK_VALID(UObjectPtr, ReturnValue)\
if (!IsValid(UObjectPtr))\
{\
	auto HeaderString = FString::Printf(TEXT("Check IsValid fail! %s"), ANSI_TO_TCHAR(__FUNCTION__));\
	FDebug::DumpStackTraceToLog(*HeaderString, ELogVerbosity::Error);\
	return ReturnValue;\
}
#else
#define LGUI_CHECK_VALID(UObjectPtr, ReturnValue)
#endif

class LGUI_API FLexUIUtils
{
public:	
	static FColor MultiplyColor(FColor A, FColor B);
#if WITH_EDITOR
	//notify information text in editor with sound
	static void EditorNotification(const FText& NotifyText, bool bSuccessOrFailureSound, float ExpireDuration = 5.0f);
#endif
	static UTexture2D* CreateTexture(int32 InSize, FColor InDefaultColor = FColor::Transparent, class UObject* InOuter = GetTransientPackage(), FName InDefaultName = NAME_None);

	static TArray<uint8> GetMD5(const FString& InString);
	static TArray<uint8> GetMD5(uint8* InData, uint64 InSize);
	static FString GetMD5String(const TArray<uint8>& InMD5Digits);
#if WITH_EDITOR
	static void NotifyPropertyChanged(UObject* Object, FProperty* Property);
	static void NotifyPropertyChanged(UObject* Object, FName PropertyName);
	static void ChangePropertyWithNotify(UObject* Object, FProperty* Property, TFunctionRef<void()> ChangePropertyFunction);
	static void ChangePropertyWithNotify(UObject* Object, FName PropertyName, TFunctionRef<void()> ChangePropertyFunction);
	static void NotifyPropertyPreChange(UObject* Object, FProperty* Property);
	static void NotifyPropertyPreChange(UObject* Object, FName PropertyName);
#endif

	static void LogObjectFlags(UObject* obj);
	static void LogClassFlags(UClass* cls);

	//convert a 0-255 byte color component to a 0-1 float.
	//replaces the old Color255To1_Table[256] lookup: identical IEEE result to V/255.f, no static data, no cache footprint.
	static FORCEINLINE float ByteToFloat01(uint8 V)
	{
		return (float)V * (1.0f / 255.0f);
	}

	static UTexture2D* GetDefaultWhiteTexture();
	static int CeilPowerOfTwo(int v)
	{
		if (v <= 1)
			return 1;
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}
public:
	static FColor ColorHSVDataToColorRGB(const FVector& InHSVColor);
	static FVector ColorRGBToColorHSVData(const FColor& InRGBColor);
	static void StaticMeshToLexUIMeshRenderData(const UStaticMesh* InMesh, TArray<struct FLexUIMeshVertex>& OutVerts, TArray<uint16>& OutIndexes);
};