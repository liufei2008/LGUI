// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Event/LGUIDrawableEvent.h"
#include "Utils/BitConverter.h"
#include "LGUI.h"


bool ULGUIDrawableEventParameterHelper::IsFunctionCompatible(const UFunction* InFunction, TArray<LGUIDrawableEventParameterType>& OutParameterTypeArray)
{
	if (InFunction->GetReturnProperty() != nullptr)return false;//not support return value for ProcessEvent
	TFieldIterator<FProperty> IteratorA(InFunction);
	while (IteratorA && (IteratorA->PropertyFlags & CPF_Parm))
	{
		FProperty* PropA = *IteratorA;
		LGUIDrawableEventParameterType ParamType;
		if (IsPropertyCompatible(PropA, ParamType))
		{
			OutParameterTypeArray.Add(ParamType);
		}
		else
		{
			// Type mismatch between an argument of A and B
			return false;
		}
		++IteratorA;
	}
	return true;
}
bool ULGUIDrawableEventParameterHelper::IsPropertyCompatible(const FProperty* InFunctionProperty, LGUIDrawableEventParameterType& OutParameterType)
{
	if (!InFunctionProperty )
	{
		return false;
	}

	if (auto boolProperty = CastField<FBoolProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIDrawableEventParameterType::Bool;
		return true;
	}
	else if (auto floatProperty = CastField<FFloatProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIDrawableEventParameterType::Float;
		return true;
	}
	else if (auto doubleProperty = CastField<FDoubleProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIDrawableEventParameterType::Double;
		return true;
	}
	else if (auto int8Property = CastField<FInt8Property>(InFunctionProperty))
	{
		OutParameterType = LGUIDrawableEventParameterType::Int8;
		return true;
	}
	else if (auto uint8Property = CastField<FByteProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIDrawableEventParameterType::UInt8;
		return true;
	}
	else if (auto int16Property = CastField<FInt16Property>(InFunctionProperty))
	{
		OutParameterType = LGUIDrawableEventParameterType::Int16;
		return true;
	}
	else if (auto uint16Property = CastField<FUInt16Property>(InFunctionProperty))
	{
		OutParameterType = LGUIDrawableEventParameterType::UInt16;
		return true;
	}
	else if (auto int32Property = CastField<FIntProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIDrawableEventParameterType::Int32;
		return true;
	}
	else if (auto uint32Property = CastField<FUInt32Property>(InFunctionProperty))
	{
		OutParameterType = LGUIDrawableEventParameterType::UInt32;
		return true;
	}
	else if (auto int64Property = CastField<FInt64Property>(InFunctionProperty))
	{
		OutParameterType = LGUIDrawableEventParameterType::Int64;
		return true;
	}
	else if (auto uint64Property = CastField<FUInt64Property>(InFunctionProperty))
	{
		OutParameterType = LGUIDrawableEventParameterType::UInt64;
		return true;
	}
	else if (auto enumProperty = CastField<FEnumProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIDrawableEventParameterType::UInt8;
		return true;
	}
	else if (auto structProperty = CastField<FStructProperty>(InFunctionProperty))
	{
		auto structName = structProperty->Struct->GetFName();
		if (structName == TEXT("Vector2D"))
		{
			OutParameterType = LGUIDrawableEventParameterType::Vector2; return true;
		}
		else if (structName == TEXT("Vector"))
		{
			OutParameterType = LGUIDrawableEventParameterType::Vector3; return true;
		}
		else if (structName == TEXT("Vector4"))
		{
			OutParameterType = LGUIDrawableEventParameterType::Vector4; return true;
		}
		else if (structName == TEXT("Color"))
		{
			OutParameterType = LGUIDrawableEventParameterType::Color; return true;
		}
		else if (structName == TEXT("LinearColor"))
		{
			OutParameterType = LGUIDrawableEventParameterType::LinearColor; return true;
		}
		else if (structName == TEXT("Quat"))
		{
			OutParameterType = LGUIDrawableEventParameterType::Quaternion; return true;
		}
		else if (structName == TEXT("Rotator"))
		{
			OutParameterType = LGUIDrawableEventParameterType::Rotator; return true;
		}
		else
		{
			return false;
		}
	}

	else if (auto classProperty = CastField<FClassProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIDrawableEventParameterType::Class;
		return true;
	}
	else if (auto objectProperty = CastField<FObjectProperty>(InFunctionProperty))//if object property
	{
		if (objectProperty->PropertyClass->IsChildOf(AActor::StaticClass()))//if is Actor
		{
			OutParameterType = LGUIDrawableEventParameterType::Actor;
		}
		else if (objectProperty->PropertyClass->IsChildOf(ULGUIPointerEventData::StaticClass()))
		{
			OutParameterType = LGUIDrawableEventParameterType::PointerEvent;
		}
		else if (objectProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))
		{
			return false;
		}
		else
		{
			OutParameterType = LGUIDrawableEventParameterType::Object;
		}
		return true;
	}

	else if (auto strProperty = CastField<FStrProperty>(InFunctionProperty))
	{
		OutParameterType = LGUIDrawableEventParameterType::String;
		return true;
	}

	return false;
}

UClass* ULGUIDrawableEventParameterHelper::GetObjectParameterClass(const UFunction* InFunction)
{
	TFieldIterator<FProperty> paramsIterator(InFunction);
	FProperty* firstProperty = *paramsIterator;
	if (auto objProperty = CastField<FObjectProperty>(firstProperty))
	{
		return objProperty->PropertyClass;
	}
	return nullptr;
}

UEnum* ULGUIDrawableEventParameterHelper::GetEnumParameter(const UFunction* InFunction)
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
UClass* ULGUIDrawableEventParameterHelper::GetClassParameterClass(const UFunction* InFunction)
{
	TFieldIterator<FProperty> paramsIterator(InFunction);
	FProperty* firstProperty = *paramsIterator;
	if (auto classProperty = CastField<FClassProperty>(firstProperty))
	{
		return classProperty->MetaClass;
	}
	return nullptr;
}

bool ULGUIDrawableEventParameterHelper::IsSupportedFunction(UFunction* Target, TArray<LGUIDrawableEventParameterType>& OutParamTypeArray)
{
	return IsFunctionCompatible(Target, OutParamTypeArray);
}

bool ULGUIDrawableEventParameterHelper::IsStillSupported(UFunction* Target, const TArray<LGUIDrawableEventParameterType>& InParamTypeArray)
{
	TArray<LGUIDrawableEventParameterType> ParamTypeArray;
	if (IsSupportedFunction(Target, ParamTypeArray))
	{
		if (ParamTypeArray.Num() == 0)
		{
			ParamTypeArray.Add(LGUIDrawableEventParameterType::Empty);
		}
		if (ParamTypeArray == InParamTypeArray)
		{
			return true;
		}
	}
	return false;
}

FString ULGUIDrawableEventParameterHelper::ParameterTypeToName(LGUIDrawableEventParameterType paramType, const UFunction* InFunction)
{
	FString ParamTypeString = "";
	switch (paramType)
	{
	case LGUIDrawableEventParameterType::Empty:
		break;
	case LGUIDrawableEventParameterType::Bool:
		ParamTypeString = "Bool";
		break;
	case LGUIDrawableEventParameterType::Float:
		ParamTypeString = "Float";
		break;
	case LGUIDrawableEventParameterType::Double:
		ParamTypeString = "Double";
		break;
	case LGUIDrawableEventParameterType::Int8:
		ParamTypeString = "Int8";
		break;
	case LGUIDrawableEventParameterType::UInt8:
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
	case LGUIDrawableEventParameterType::Int16:
		ParamTypeString = "Int16";
		break;
	case LGUIDrawableEventParameterType::UInt16:
		ParamTypeString = "UInt16";
		break;
	case LGUIDrawableEventParameterType::Int32:
		ParamTypeString = "Int32";
		break;
	case LGUIDrawableEventParameterType::UInt32:
		ParamTypeString = "UInt32";
		break;
	case LGUIDrawableEventParameterType::Int64:
		ParamTypeString = "Int64";
		break;
	case LGUIDrawableEventParameterType::UInt64:
		ParamTypeString = "UInt64";
		break;
	case LGUIDrawableEventParameterType::Vector2:
		ParamTypeString = "Vector2";
		break;
	case LGUIDrawableEventParameterType::Vector3:
		ParamTypeString = "Vector3";
		break;
	case LGUIDrawableEventParameterType::Vector4:
		ParamTypeString = "Vector4";
		break;
	case LGUIDrawableEventParameterType::Quaternion:
		ParamTypeString = "Quaternion";
		break;
	case LGUIDrawableEventParameterType::Color:
		ParamTypeString = "Color";
		break;
	case LGUIDrawableEventParameterType::LinearColor:
		ParamTypeString = "LinearColor";
		break;
	case LGUIDrawableEventParameterType::String:
		ParamTypeString = "String";
		break;
		//case LGUIDrawableEventParameterType::Name:
		//	ParamTypeString = "Name";
		//	break;
	case LGUIDrawableEventParameterType::Object:
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
	case LGUIDrawableEventParameterType::Actor:
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
	case LGUIDrawableEventParameterType::PointerEvent:
		ParamTypeString = "PointerEvent";
		break;
	case LGUIDrawableEventParameterType::Class:
		ParamTypeString = "Class";
		break;
	case LGUIDrawableEventParameterType::Rotator:
		ParamTypeString = "Rotator";
		break;
	default:
		break;
	}
	return ParamTypeString;
}



void FLGUIDrawableEventData::FindAndExecuteFromActor(void* InParam)
{
	TArray<UActorComponent*> compArray;
	targetActor->GetComponents(componentClass, compArray);
	if (compArray.Num() > 1)
	{
		bool found = false;
		for (auto comp : compArray)
		{
			if (comp->GetFName() == componentName)
			{
				FindAndExecute(comp, functionName, InParam);
				found = true;
				break;
			}
		}
		if (!found)
		{
			if (componentName.IsNone() || !componentName.IsValid())
			{
				FindAndExecute(compArray[0], functionName, InParam);
				UE_LOG(LGUI, Warning, TEXT("[LGUIDrawableEventData/FindAndExecuteFromActor]Pos 0, Target component:%s with name:%s not found! will use the first compatible component."), *(componentClass->GetPathName(), *(componentName.ToString())));
			}
			else
			{
				UE_LOG(LGUI, Error, TEXT("[LGUIDrawableEventData/FindAndExecuteFromActor]Pos 1, Target component:%s with name:%s not found! event will not execute!"), *(componentClass->GetPathName(), *(componentName.ToString())));
			}
		}
	}
	else if (compArray.Num() == 1)
	{
		FindAndExecute(compArray[0], functionName, InParam);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[LGUIDrawableEventData/FindAndExecuteFromActor]Pos 2, Target component:%s not found!"), *(componentClass->GetPathName()));
	}
}
void FLGUIDrawableEventData::Execute()
{
	if (UseNativeParameter)
	{
		UE_LOG(LGUI, Error, TEXT("[FLGUIDrawableEventData::Execute]If use NativeParameter, you must FireEvent with your own parameter!"));
		return;
	}
	if (ParamType == LGUIDrawableEventParameterType::None)
	{
		UE_LOG(LGUI, Error, TEXT("[FLGUIDrawableEventData::Execute]Not valid event"));
		return;
	}
	if (CacheTarget != nullptr && CacheFunction != nullptr)
	{
		ExecuteTargetFunction(CacheTarget, CacheFunction);
	}
	else
	{
		if (targetActor)
		{
			if (componentClass->IsChildOf(AActor::StaticClass()))//target class is actor self
			{
				FindAndExecute(targetActor, functionName);
			}
			else//search from actor's component list
			{
				FindAndExecuteFromActor(nullptr);
			}
		}
	}
}
void FLGUIDrawableEventData::Execute(void* InParam, LGUIDrawableEventParameterType InParameterType)
{
	if (ParamType == LGUIDrawableEventParameterType::None)
	{
		UE_LOG(LGUI, Error, TEXT("[LGUIDrawableEvent]Not valid event"));
		return;
	}

	if (ParamType == InParameterType//function's supported parameter is equal to event's parameter
		&& UseNativeParameter)//and use native parameter
	{
		if (CacheTarget != nullptr && CacheFunction != nullptr)
		{
			ExecuteTargetFunction(CacheTarget, CacheFunction, InParam);
		}
		else
		{
			if (targetActor)
			{
				if (componentClass->IsChildOf(AActor::StaticClass()))//target class is actor self
				{
					FindAndExecute(targetActor, functionName, InParam);
				}
				else//search from actor's component list
				{
					FindAndExecuteFromActor(InParam);
				}
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
			if (targetActor)
			{
				if (componentClass->IsChildOf(AActor::StaticClass()))//target class is actor self
				{
					FindAndExecute(targetActor, functionName);
				}
				else//search from actor's component list
				{
					FindAndExecuteFromActor(nullptr);
				}
			}
		}
	}
}
void FLGUIDrawableEventData::FindAndExecute(UObject* Target, FName FunctionName, void* ParamData)
{
	CacheTarget = Target;
	CacheFunction = Target->FindFunction(functionName);
	if (CacheFunction)
	{
		if (!ULGUIDrawableEventParameterHelper::IsStillSupported(CacheFunction, { ParamType }))
		{
			UE_LOG(LGUI, Error, TEXT("[LGUIDrawableEventData/FindAndExecute]Pos 2, Target function:%s not supported!"), *(FunctionName.ToString()));
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
		UE_LOG(LGUI, Error, TEXT("[LGUIDrawableEventData/FindAndExecute]Pos 3, Target function:%s not found!"), *(FunctionName.ToString()));
	}
}
void FLGUIDrawableEventData::ExecuteTargetFunction(UObject* Target, UFunction* Func)
{
	switch (ParamType)
	{
	case LGUIDrawableEventParameterType::String:
	{
		Target->ProcessEvent(Func, &ReferenceString);
	}
	break;
	case LGUIDrawableEventParameterType::Object:
	{
		Target->ProcessEvent(Func, &ReferenceObject);
	}
	break;
	case LGUIDrawableEventParameterType::Actor:
	{
		Target->ProcessEvent(Func, &ReferenceActor);
	}
	break;
	default:
	{
		Target->ProcessEvent(Func, ParamBuffer.GetData());
	}
	break;
	}
}
void FLGUIDrawableEventData::ExecuteTargetFunction(UObject* Target, UFunction* Func, void* ParamData)
{
	Target->ProcessEvent(Func, ParamData);
}


FLGUIDrawableEvent::FLGUIDrawableEvent()
{
	
}
FLGUIDrawableEvent::FLGUIDrawableEvent(LGUIDrawableEventParameterType InParameterType)
{
	supportParameterType = InParameterType;
}
bool FLGUIDrawableEvent::IsBound()const
{
	return eventList.Num() != 0;
}
void FLGUIDrawableEvent::FireEvent()const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Empty)
	{
		for (auto item : eventList)
		{
			item.Execute();
		}
	}
	else
		LogParameterError();
}
void FLGUIDrawableEvent::LogParameterError()const
{
	auto enumObject = FindObject<UEnum>((UObject*)ANY_PACKAGE, TEXT("LGUIDrawableEventParameterType"), true);
	UE_LOG(LGUI, Error, TEXT("[LGUIDrawableEvent/FireEvent]Parameter type must be the same as your declaration. NativeParameterType:%s"), *(enumObject->GetDisplayNameTextByValue((int64)supportParameterType)).ToString());
}
void FLGUIDrawableEvent::FireEvent(void* InParam)const
{
	for (auto item : eventList)
	{
		item.Execute(InParam, supportParameterType);
	}
}

void FLGUIDrawableEvent::FireEvent(bool InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Bool)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(float InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Float)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(double InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Double)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(int8 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Int8)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(uint8 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::UInt8)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(int16 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Int16)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(uint16 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::UInt16)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(int32 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Int32)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(uint32 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::UInt32)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(int64 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Int64)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(uint64 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::UInt64)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(FVector2D InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Vector2)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(FVector InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Vector3)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(FVector4 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Vector4)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(FColor InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Color)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(FLinearColor InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::LinearColor)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(FQuat InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Quaternion)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(FString InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::String)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(UObject* InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Object)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(AActor* InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Actor)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(ULGUIPointerEventData* InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::PointerEvent)
	{
		FireEvent((void*)&InParam);
	}
	else LogParameterError();
}
void FLGUIDrawableEvent::FireEvent(FRotator InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIDrawableEventParameterType::Rotator)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}