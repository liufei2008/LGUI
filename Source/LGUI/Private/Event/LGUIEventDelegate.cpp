// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Event/LGUIEventDelegate.h"
#include "LGUI.h"

//PRAGMA_DISABLE_OPTIMIZATION

bool ULGUIEventDelegateParameterHelper::IsFunctionCompatible(const UFunction* InFunction, LGUIEventDelegateParameterType& OutParameterType)
{
	if (InFunction->GetReturnProperty() != nullptr)return false;//not support return value for ProcessEvent
	TFieldIterator<FProperty> IteratorA(InFunction);
	TArray<LGUIEventDelegateParameterType> ParameterTypeArray;
	while (IteratorA && (IteratorA->PropertyFlags & CPF_Parm))
	{
		FProperty* PropA = *IteratorA;
		LGUIEventDelegateParameterType ParamType;
		if (IsPropertyCompatible(PropA, ParamType))
		{
			ParameterTypeArray.Add(ParamType);
		}
		else
		{
			// Type mismatch between an argument of A and B
			return false;
		}
		++IteratorA;
	}
	if (ParameterTypeArray.Num() == 1)
	{
		OutParameterType = ParameterTypeArray[0];
		return true;
	}
	return false;
}
bool ULGUIEventDelegateParameterHelper::IsPropertyCompatible(const FProperty* InFunctionProperty, LGUIEventDelegateParameterType& OutParameterType)
{
	if (!InFunctionProperty )
	{
		return false;
	}

	if (auto boolProperty = CastField<FBoolProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::Bool;
		return true;
	}
	else if (auto floatProperty = CastField<FFloatProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::Float;
		return true;
	}
	else if (auto doubleProperty = CastField<FDoubleProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::Double;
		return true;
	}
	else if (auto int8Property = CastField<FInt8Property>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::Int8;
		return true;
	}
	else if (auto uint8Property = CastField<FByteProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::UInt8;
		return true;
	}
	else if (auto int16Property = CastField<FInt16Property>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::Int16;
		return true;
	}
	else if (auto uint16Property = CastField<FUInt16Property>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::UInt16;
		return true;
	}
	else if (auto int32Property = CastField<FIntProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::Int32;
		return true;
	}
	else if (auto uint32Property = CastField<FUInt32Property>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::UInt32;
		return true;
	}
	else if (auto int64Property = CastField<FInt64Property>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::Int64;
		return true;
	}
	else if (auto uint64Property = CastField<FUInt64Property>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::UInt64;
		return true;
	}
	else if (auto enumProperty = CastField<FEnumProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::UInt8;
		return true;
	}
	else if (auto structProperty = CastField<FStructProperty>(InFunctionProperty))
	{
		auto structName = structProperty->Struct->GetFName();
		if (structName == TEXT("Vector2D"))
		{
			OutParameterType = LGUIEventDelegateParameterType::Vector2; return true;
		}
		else if (structName == TEXT("Vector"))
		{
			OutParameterType = LGUIEventDelegateParameterType::Vector3; return true;
		}
		else if (structName == TEXT("Vector4"))
		{
			OutParameterType = LGUIEventDelegateParameterType::Vector4; return true;
		}
		else if (structName == TEXT("Color"))
		{
			OutParameterType = LGUIEventDelegateParameterType::Color; return true;
		}
		else if (structName == TEXT("LinearColor"))
		{
			OutParameterType = LGUIEventDelegateParameterType::LinearColor; return true;
		}
		else if (structName == TEXT("Quat"))
		{
			OutParameterType = LGUIEventDelegateParameterType::Quaternion; return true;
		}
		else if (structName == TEXT("Rotator"))
		{
			OutParameterType = LGUIEventDelegateParameterType::Rotator; return true;
		}
		else
		{
			return false;
		}
	}

	else if (auto classProperty = CastField<FClassProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::Class;
		return true;
	}
	else if (auto objectProperty = CastField<FObjectProperty>(InFunctionProperty))//if object property
	{
		if (objectProperty->PropertyClass->IsChildOf(AActor::StaticClass()))//if is Actor
		{
			OutParameterType = LGUIEventDelegateParameterType::Actor;
		}
		else if (objectProperty->PropertyClass->IsChildOf(ULGUIPointerEventData::StaticClass()))
		{
			OutParameterType = LGUIEventDelegateParameterType::PointerEvent;
		}
		else if (objectProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))
		{
			return false;
		}
		else
		{
			OutParameterType = LGUIEventDelegateParameterType::Object;
		}
		return true;
	}

	else if (auto strProperty = CastField<FStrProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::String;
		return true;
	}
	else if (auto nameProperty = CastField<FNameProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::Name;
		return true;
	}
	else if (auto textProperty = CastField<FTextProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIEventDelegateParameterType::Text;
		return true;
	}

	return false;
}

UClass* ULGUIEventDelegateParameterHelper::GetObjectParameterClass(const UFunction* InFunction)
{
	TFieldIterator<FProperty> paramsIterator(InFunction);
	FProperty* firstProperty = *paramsIterator;
	if (auto objProperty = CastField<FObjectProperty>(firstProperty))
	{
		return objProperty->PropertyClass;
	}
	return nullptr;
}

UEnum* ULGUIEventDelegateParameterHelper::GetEnumParameter(const UFunction* InFunction)
{
	TFieldIterator<FProperty> paramsIterator(InFunction);
	FProperty* firstProperty = *paramsIterator;
	if (auto uint8Property = CastField<FByteProperty>(firstProperty))
	{
		if (uint8Property->IsEnum())
		{
			return uint8Property->Enum;
		}
	}
	if (auto enumProperty = CastField<FEnumProperty>(firstProperty))
	{
		return enumProperty->GetEnum();
	}
	return nullptr;
}
UClass* ULGUIEventDelegateParameterHelper::GetClassParameterClass(const UFunction* InFunction)
{
	TFieldIterator<FProperty> paramsIterator(InFunction);
	FProperty* firstProperty = *paramsIterator;
	if (auto classProperty = CastField<FClassProperty>(firstProperty))
	{
		return classProperty->MetaClass;
	}
	return nullptr;
}

bool ULGUIEventDelegateParameterHelper::IsSupportedFunction(UFunction* Target, LGUIEventDelegateParameterType& OutParamType)
{
	return IsFunctionCompatible(Target, OutParamType);
}

bool ULGUIEventDelegateParameterHelper::IsStillSupported(UFunction* Target, LGUIEventDelegateParameterType InParamType)
{
	LGUIEventDelegateParameterType ParamType;
	if (IsSupportedFunction(Target, ParamType))
	{
		if (ParamType == InParamType)
		{
			return true;
		}
	}
	return false;
}

FString ULGUIEventDelegateParameterHelper::ParameterTypeToName(LGUIEventDelegateParameterType paramType, const UFunction* InFunction)
{
	FString ParamTypeString = "";
	switch (paramType)
	{
	case LGUIEventDelegateParameterType::Empty:
		break;
	case LGUIEventDelegateParameterType::Bool:
		ParamTypeString = "Bool";
		break;
	case LGUIEventDelegateParameterType::Float:
		ParamTypeString = "Float";
		break;
	case LGUIEventDelegateParameterType::Double:
		ParamTypeString = "Double";
		break;
	case LGUIEventDelegateParameterType::Int8:
		ParamTypeString = "Int8";
		break;
	case LGUIEventDelegateParameterType::UInt8:
	{
		if (auto enumValue = GetEnumParameter(InFunction))
		{
			ParamTypeString = enumValue->GetName() + "(Enum)";
		}
		else
		{
			ParamTypeString = "UInt8";
		}
	}
		break;
	case LGUIEventDelegateParameterType::Int16:
		ParamTypeString = "Int16";
		break;
	case LGUIEventDelegateParameterType::UInt16:
		ParamTypeString = "UInt16";
		break;
	case LGUIEventDelegateParameterType::Int32:
		ParamTypeString = "Int32";
		break;
	case LGUIEventDelegateParameterType::UInt32:
		ParamTypeString = "UInt32";
		break;
	case LGUIEventDelegateParameterType::Int64:
		ParamTypeString = "Int64";
		break;
	case LGUIEventDelegateParameterType::UInt64:
		ParamTypeString = "UInt64";
		break;
	case LGUIEventDelegateParameterType::Vector2:
		ParamTypeString = "Vector2";
		break;
	case LGUIEventDelegateParameterType::Vector3:
		ParamTypeString = "Vector3";
		break;
	case LGUIEventDelegateParameterType::Vector4:
		ParamTypeString = "Vector4";
		break;
	case LGUIEventDelegateParameterType::Quaternion:
		ParamTypeString = "Quaternion";
		break;
	case LGUIEventDelegateParameterType::Color:
		ParamTypeString = "Color";
		break;
	case LGUIEventDelegateParameterType::LinearColor:
		ParamTypeString = "LinearColor";
		break;
	case LGUIEventDelegateParameterType::String:
		ParamTypeString = "String";
		break;
		//case LGUIEventDelegateParameterType::Name:
		//	ParamTypeString = "Name";
		//	break;
	case LGUIEventDelegateParameterType::Object:
	{
		TFieldIterator<FProperty> ParamIterator(InFunction);
		if (auto firstProperty = CastField<FObjectProperty>(*ParamIterator))
		{
			if (firstProperty->PropertyClass != UObject::StaticClass())
			{
				ParamTypeString = firstProperty->PropertyClass->GetName() + "(Object)";
			}
			else
			{
				ParamTypeString = "Object";
			}
		}
		else
		{
			ParamTypeString = "Object";
		}
	}
		break;
	case LGUIEventDelegateParameterType::Actor:
	{
		TFieldIterator<FProperty> ParamIterator(InFunction);
		if (auto firstProperty = CastField<FObjectProperty>(*ParamIterator))
		{
			if (firstProperty->PropertyClass != AActor::StaticClass())
			{
				ParamTypeString = firstProperty->PropertyClass->GetName() + "(Actor)";
			}
			else
			{
				ParamTypeString = "Actor";
			}
		}
		else
		{
			ParamTypeString = "Actor";
		}
	}
		break;
	case LGUIEventDelegateParameterType::PointerEvent:
		ParamTypeString = "PointerEvent";
		break;
	case LGUIEventDelegateParameterType::Class:
		ParamTypeString = "Class";
		break;
	case LGUIEventDelegateParameterType::Rotator:
		ParamTypeString = "Rotator";
		break;
	case LGUIEventDelegateParameterType::Name:
		ParamTypeString = "Name";
		break;
	case LGUIEventDelegateParameterType::Text:
		ParamTypeString = "Text";
		break;
	default:
		break;
	}
	return ParamTypeString;
}



void FLGUIEventDelegateData::Execute()
{
	if (bUseNativeParameter)
	{
		UE_LOG(LGUI, Error, TEXT("[FLGUIEventDelegateData::Execute]If use NativeParameter, you must FireEvent with your own parameter!"));
		return;
	}
	if (ParamType == LGUIEventDelegateParameterType::None)
	{
		UE_LOG(LGUI, Error, TEXT("[FLGUIEventDelegateData::Execute]Not valid event"));
		return;
	}
	if (CacheTarget != nullptr && CacheFunction != nullptr)
	{
		ExecuteTargetFunction(CacheTarget, CacheFunction);
	}
	else
	{
		if (TargetObject.IsValid())
		{
			FindAndExecute(TargetObject.Get());
		}
	}
}
void FLGUIEventDelegateData::Execute(void* InParam, LGUIEventDelegateParameterType InParameterType)
{
	if (ParamType == LGUIEventDelegateParameterType::None)
	{
		UE_LOG(LGUI, Error, TEXT("[LGUIEventDelegate]Not valid event"));
		return;
	}

	if (ParamType == InParameterType//function's supported parameter is equal to event's parameter
		&& bUseNativeParameter)//and use native parameter
	{
		if (CacheTarget != nullptr && CacheFunction != nullptr)
		{
			ExecuteTargetFunction(CacheTarget, CacheFunction, InParam);
		}
		else
		{
			if (TargetObject.IsValid())
			{
				FindAndExecute(TargetObject.Get(), InParam);
			}
		}
	}
	else
	{
		if (CacheTarget != nullptr && CacheFunction != nullptr)
		{
			ExecuteTargetFunction(CacheTarget, CacheFunction);
		}
		else
		{
			if (TargetObject.IsValid())
			{
				FindAndExecute(TargetObject.Get());
			}
		}
	}
}
void FLGUIEventDelegateData::FindAndExecute(UObject* Target, void* ParamData)
{
	CacheTarget = Target;
	CacheFunction = Target->FindFunction(FunctionName);
	if (CacheFunction)
	{
		if (!ULGUIEventDelegateParameterHelper::IsStillSupported(CacheFunction, { ParamType }))
		{
			UE_LOG(LGUI, Error, TEXT("[LGUIEventDelegateData/FindAndExecute]Target function:%s not supported!"), *(FunctionName.ToString()));
			CacheFunction = nullptr;
		}
		else
		{
			if (ParamData == nullptr)
			{
				ExecuteTargetFunction(Target, CacheFunction);
			}
			else
			{
				ExecuteTargetFunction(Target, CacheFunction, ParamData);
			}
		}
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[LGUIEventDelegateData/FindAndExecute]Target function:%s not found!"), *(FunctionName.ToString()));
	}
}
void FLGUIEventDelegateData::ExecuteTargetFunction(UObject* Target, UFunction* Func)
{
	switch (ParamType)
	{
	case LGUIEventDelegateParameterType::String:
	{
		FString TempString;
		auto FromBinary = FMemoryReader(ParamBuffer, false);
		FromBinary.Seek(0);
		FromBinary << TempString;
		Target->ProcessEvent(Func, &TempString);
	}
	break;
	case LGUIEventDelegateParameterType::Name:
	{
		FName TempName;
		auto FromBinary = FMemoryReader(ParamBuffer, false);
		FromBinary.Seek(0);
		FromBinary << TempName;
		Target->ProcessEvent(Func, &TempName);
	}
	break;
	case LGUIEventDelegateParameterType::Text:
	{
		FText TempText;
		auto FromBinary = FMemoryReader(ParamBuffer, false);
		FromBinary.Seek(0);
		FromBinary << TempText;
		Target->ProcessEvent(Func, &TempText);
	}
	break;
	case LGUIEventDelegateParameterType::Object:
	case LGUIEventDelegateParameterType::Actor:
	case LGUIEventDelegateParameterType::Class:
	{
		Target->ProcessEvent(Func, &ReferenceObject);
	}
	break;
	default:
	{
		Target->ProcessEvent(Func, ParamBuffer.GetData());
	}
	break;
	}
}
void FLGUIEventDelegateData::ExecuteTargetFunction(UObject* Target, UFunction* Func, void* ParamData)
{
	Target->ProcessEvent(Func, ParamData);
}


FLGUIEventDelegate::FLGUIEventDelegate()
{
	
}
FLGUIEventDelegate::FLGUIEventDelegate(LGUIEventDelegateParameterType InParameterType)
{
	NativeParameterType = InParameterType;
}
bool FLGUIEventDelegate::IsBound()const
{
	return EventList.Num() != 0;
}
void FLGUIEventDelegate::FireEvent()const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Empty)
	{
		for (auto item : EventList)
		{
			item.Execute();
		}
	}
	else
		LogParameterError();
}
void FLGUIEventDelegate::LogParameterError()const
{
	auto enumObject = FindObject<UEnum>((UObject*)ANY_PACKAGE, TEXT("LGUIEventDelegateParameterType"), true);
	UE_LOG(LGUI, Error, TEXT("[LGUIEventDelegate/FireEvent]Parameter type must be the same as your declaration. NativeParameterType:%s"), *(enumObject->GetDisplayNameTextByValue((int64)NativeParameterType)).ToString());
}
void FLGUIEventDelegate::FireEvent(void* InParam)const
{
	for (auto item : EventList)
	{
		item.Execute(InParam, NativeParameterType);
	}
}

void FLGUIEventDelegate::FireEvent(bool InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Bool)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(float InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Float)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(double InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Double)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(int8 InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Int8)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(uint8 InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::UInt8)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(int16 InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Int16)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(uint16 InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::UInt16)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(int32 InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Int32)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(uint32 InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::UInt32)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(int64 InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Int64)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(uint64 InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::UInt64)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(FVector2D InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Vector2)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(FVector InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Vector3)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(FVector4 InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Vector4)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(FColor InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Color)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(FLinearColor InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::LinearColor)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(FQuat InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Quaternion)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(const FString& InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::String)
	{
		FireEvent((void*)&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(UObject* InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Object)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(AActor* InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Actor)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(ULGUIPointerEventData* InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::PointerEvent)
	{
		FireEvent((void*)&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(FRotator InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Rotator)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(const FName& InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Name)
	{
		FireEvent((void*)&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(const FText& InParam)const
{
	if (EventList.Num() == 0)return;
	if (NativeParameterType == LGUIEventDelegateParameterType::Text)
	{
		FireEvent((void*)&InParam);
	}
	else LogParameterError();
}

//PRAGMA_ENABLE_OPTIMIZATION
