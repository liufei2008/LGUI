// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LexUIBPLibrary.h"

#include "LexUIDelegateHandleWrapper.h"
#include "Framework/Application/SlateApplication.h"
#include "LGUI.h"
#include "Core/Components/LexWidget.h"
#include "Event/LexScreenSpaceRaycaster.h"
#include "PrefabSystem/LexUIPrefab.h"

#include LEXUIPREFAB_SERIALIZER_NEWEST_INCLUDE

ULexWidget* ULexUIBPLibrary::DuplicateWidget(UObject* WorldContextObject, ULexWidget* Target, ULexWidget* Parent)
{
	if (auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return LEXUIPREFAB_SERIALIZER_NEWEST_NAMESPACE::WidgetSerializer::DuplicateWidget(World, Parent->GetOuter(), Target, Parent);
	}
	return nullptr;
}
void ULexUIBPLibrary::PrepareDuplicateData(ULexWidget* Target, FLexUIDuplicateDataContainer& DataContainer)
{
	DataContainer.bIsValid = LEXUIPREFAB_SERIALIZER_NEWEST_NAMESPACE::WidgetSerializer::PrepareDataForDuplicate(Target, DataContainer.DuplicateData);
}
ULexWidget* ULexUIBPLibrary::DuplicateWidgetWithPreparedData(UObject* WorldContextObject, FLexUIDuplicateDataContainer& Data, ULexWidget* Parent)
{
	if (Data.bIsValid)
	{
		if (auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			return LEXUIPREFAB_SERIALIZER_NEWEST_NAMESPACE::WidgetSerializer::DuplicateWidgetWithPreparedData(World, Parent->GetOuter(), Data.DuplicateData, Parent);
		}
	}
	return nullptr;
}

ULexVisual* ULexUIBPLibrary::CreateWidgetWithVisual(UObject* WorldContextObject, ULexWidget* Parent, TSubclassOf<ULexVisual> VisualClass)
{
	if (auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		auto Widget = NewObject<ULexWidget>(World);
		Widget->SetFlags(EObjectFlags::RF_Transient);
		Widget->SetParent(Parent, false);
		return Widget->CreateNewVisual(VisualClass);
	}
	return nullptr;
}

UActorComponent* ULexUIBPLibrary::LexUICompRef_GetComponent(const FLexUIComponentReference& InLexUIComponentReference, TSubclassOf<UActorComponent> InComponentType)
{
	auto comp = InLexUIComponentReference.GetComponent();
	if (comp == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::GetComponent]Target actor:%s dont have this kind of component:%s"), *(InLexUIComponentReference.GetActor()->GetPathName()), *(InComponentType->GetPathName()));
		return nullptr;
	}
	if (comp->GetClass() != InComponentType)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIBPLibrary::GetComponent]InComponentType must be the same as InLGUIComponentReference's component type!"));
		return nullptr;
	}
	return comp;
}

AActor* ULexUIBPLibrary::LexUICompRef_GetActor(const FLexUIComponentReference& InLexUIComponentReference)
{
	return InLexUIComponentReference.GetActor();
}

void ULexUIBPLibrary::K2_LexUICompRef_GetComponent(const FLexUIComponentReference& InLexUICompRef, UActorComponent*& OutResult)
{
	OutResult = InLexUICompRef.GetComponent();
}


#pragma region LTween

void ULexUIBPLibrary::LexUIExecuteControllerInputAxis(FKey inputKey, float value)
{
	if (inputKey.IsValid())
	{
		FPlatformUserId UserId = IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser();
		FInputDeviceId DeviceId = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
		const FGamepadKeyNames::Type keyName = inputKey.GetFName();
		FSlateApplication::Get().OnControllerAnalog(keyName, UserId, DeviceId, value);
	}
}
void ULexUIBPLibrary::LexUIExecuteControllerInputAction(FKey inputKey, bool pressOrRelease)
{
	if (inputKey.IsValid())
	{
		FPlatformUserId UserId = IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser();
		FInputDeviceId DeviceId = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
		const FGamepadKeyNames::Type keyName = inputKey.GetFName();
		if (pressOrRelease)
		{
			FSlateApplication::Get().OnControllerButtonPressed(keyName, UserId, DeviceId, false);
		}
		else
		{
			FSlateApplication::Get().OnControllerButtonReleased(keyName, UserId, DeviceId, false);
		}
	}
}
#pragma endregion

#pragma region EventDelegate
#define IMPLEMENT_EVENTDELEGATE_BP(EventDelegateParamType, ParamType)\
FLexUIDelegateHandleWrapper ULexUIBPLibrary::LexUIEventDelegate_##EventDelegateParamType##_Register(const FLexUIEventDelegate_##EventDelegateParamType& InEvent, FLexUIEventDelegate_##EventDelegateParamType##_DynamicDelegate InDelegate)\
{\
	auto delegateHandle = InEvent.Register([InDelegate](ParamType value) {\
		if (InDelegate.IsBound())\
		{\
			InDelegate.Execute(value);\
		}\
		});\
	return FLexUIDelegateHandleWrapper(delegateHandle);\
}\
void ULexUIBPLibrary::LexUIEventDelegate_##EventDelegateParamType##_Unregister(const FLexUIEventDelegate_##EventDelegateParamType& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle)\
{\
	InEvent.Unregister(InDelegateHandle.DelegateHandle);\
}

FLexUIDelegateHandleWrapper ULexUIBPLibrary::LexUIEventDelegate_Empty_Register(const FLexUIEventDelegate_Empty& InEvent, FLexUIEventDelegate_Empty_DynamicDelegate InDelegate)
{
	auto delegateHandle = InEvent.Register([InDelegate]() {
		if (InDelegate.IsBound())
		{
			InDelegate.Execute();
		}
		});
	return FLexUIDelegateHandleWrapper(delegateHandle);
}
void ULexUIBPLibrary::LexUIEventDelegate_Empty_Unregister(const FLexUIEventDelegate_Empty& InEvent, const FLexUIDelegateHandleWrapper& InDelegateHandle)
{
	InEvent.Unregister(InDelegateHandle.DelegateHandle);
}

IMPLEMENT_EVENTDELEGATE_BP(Bool, bool);
IMPLEMENT_EVENTDELEGATE_BP(Float, float);
IMPLEMENT_EVENTDELEGATE_BP(Double, double);
//IMPLEMENT_EVENTDELEGATE_BP(Int8, int8);
IMPLEMENT_EVENTDELEGATE_BP(UInt8, uint8);
//IMPLEMENT_EVENTDELEGATE_BP(Int16, int16);
//IMPLEMENT_EVENTDELEGATE_BP(UInt16, uint16);
IMPLEMENT_EVENTDELEGATE_BP(Int32, int32);
//IMPLEMENT_EVENTDELEGATE_BP(UInt32, uint32);
IMPLEMENT_EVENTDELEGATE_BP(Int64, int64);
//IMPLEMENT_EVENTDELEGATE_BP(UInt64, uint64);
IMPLEMENT_EVENTDELEGATE_BP(Vector2, FVector2D);
IMPLEMENT_EVENTDELEGATE_BP(Vector3, FVector);
IMPLEMENT_EVENTDELEGATE_BP(Vector4, FVector4);
IMPLEMENT_EVENTDELEGATE_BP(Color, FColor);
IMPLEMENT_EVENTDELEGATE_BP(LinearColor, FLinearColor);
IMPLEMENT_EVENTDELEGATE_BP(Quaternion, FQuat);
IMPLEMENT_EVENTDELEGATE_BP(String, FString);
IMPLEMENT_EVENTDELEGATE_BP(Asset, UObject*);
IMPLEMENT_EVENTDELEGATE_BP(LexWidget, ULexWidget*);
IMPLEMENT_EVENTDELEGATE_BP(PointerEvent, ULexPointerEventData*);
IMPLEMENT_EVENTDELEGATE_BP(Class, UClass*);
IMPLEMENT_EVENTDELEGATE_BP(Rotator, FRotator);
IMPLEMENT_EVENTDELEGATE_BP(Text, FText);
IMPLEMENT_EVENTDELEGATE_BP(Name, FName);

#pragma endregion

void ULexUIBPLibrary::GetSpriteSize(const FLexUISpriteInfo& SpriteInfo, int32& width, int32& height)
{
	width = SpriteInfo.Width;
	height = SpriteInfo.Height;
}
void ULexUIBPLibrary::GetSpriteBorderSize(const FLexUISpriteInfo& SpriteInfo, int32& borderLeft, int32& borderRight, int32& borderTop, int32& borderBottom)
{
	borderLeft = SpriteInfo.Border.Left;
	borderRight = SpriteInfo.Border.Right;
	borderTop = SpriteInfo.Border.Top;
	borderBottom = SpriteInfo.Border.Bottom;
}
void ULexUIBPLibrary::GetSpriteUV(const FLexUISpriteInfo& SpriteInfo, float& UV0X, float& UV0Y, float& UV3X, float& UV3Y)
{
	UV0X = SpriteInfo.MinUV.X;
	UV0Y = SpriteInfo.MaxUV.Y;
	UV3X = SpriteInfo.MaxUV.X;
	UV3Y = SpriteInfo.MinUV.Y;
}
void ULexUIBPLibrary::GetSpriteBorderUV(const FLexUISpriteInfo& SpriteInfo, float& borderUV0X, float& borderUV0Y, float& borderUV3X, float& borderUV3Y)
{
	borderUV0X = SpriteInfo.MinUV.X;
	borderUV0Y = SpriteInfo.MaxUV.Y;
	borderUV3X = SpriteInfo.MaxUV.X;
	borderUV3Y = SpriteInfo.MinUV.Y;
}
