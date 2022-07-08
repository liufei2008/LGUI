// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Event/LGUIEventDelegate.h"
#include "LGUI.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

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
	if (ParameterTypeArray.Num() == 0)
	{
		OutParameterType = LGUIEventDelegateParameterType::Empty;
		return true;
	}
	return false;
}
bool ULGUIEventDelegateParameterHelper::IsPropertyCompatible(const FProperty* InFunctionProperty, LGUIEventDelegateParameterType& OutParameterType)
{
	if (!InFunctionProperty)
	{
		return false;
	}

	auto PropertyID = InFunctionProperty->GetID();
	switch (*PropertyID.ToEName())
	{
	case NAME_BoolProperty:
	{
		OutParameterType = LGUIEventDelegateParameterType::Bool;
		return true;
	}
	case NAME_FloatProperty:
	{
		OutParameterType = LGUIEventDelegateParameterType::Float;
		return true;
	}
	case NAME_DoubleProperty:
	{
		OutParameterType = LGUIEventDelegateParameterType::Double;
		return true;
	}
	case NAME_Int8Property:
	{
		OutParameterType = LGUIEventDelegateParameterType::Int8;
		return true;
	}
	case NAME_ByteProperty:
	{
		OutParameterType = LGUIEventDelegateParameterType::UInt8;
		return true;
	}
	case NAME_Int16Property:
	{
		OutParameterType = LGUIEventDelegateParameterType::Int16;
		return true;
	}
	case NAME_UInt16Property:
	{
		OutParameterType = LGUIEventDelegateParameterType::UInt16;
		return true;
	}
	case NAME_IntProperty:
	{
		OutParameterType = LGUIEventDelegateParameterType::Int32;
		return true;
	}
	case NAME_UInt32Property:
	{
		OutParameterType = LGUIEventDelegateParameterType::UInt32;
		return true;
	}
	case NAME_Int64Property:
	{
		OutParameterType = LGUIEventDelegateParameterType::Int64;
		return true;
	}
	case NAME_UInt64Property:
	{
		OutParameterType = LGUIEventDelegateParameterType::UInt64;
		return true;
	}
	case NAME_EnumProperty:
	{
		OutParameterType = LGUIEventDelegateParameterType::UInt8;
		return true;
	}
	case NAME_StructProperty:
	{
		auto structProperty = (FStructProperty*)InFunctionProperty;
		auto structName = structProperty->Struct->GetFName();
		if (structName == NAME_Vector2D)
		{
			OutParameterType = LGUIEventDelegateParameterType::Vector2; return true;
		}
		else if (structName == NAME_Vector)
		{
			OutParameterType = LGUIEventDelegateParameterType::Vector3; return true;
		}
		else if (structName == NAME_Vector4)
		{
			OutParameterType = LGUIEventDelegateParameterType::Vector4; return true;
		}
		else if (structName == NAME_Color)
		{
			OutParameterType = LGUIEventDelegateParameterType::Color; return true;
		}
		else if (structName == NAME_LinearColor)
		{
			OutParameterType = LGUIEventDelegateParameterType::LinearColor; return true;
		}
		else if (structName == NAME_Quat)
		{
			OutParameterType = LGUIEventDelegateParameterType::Quaternion; return true;
		}
		else if (structName == NAME_Rotator)
		{
			OutParameterType = LGUIEventDelegateParameterType::Rotator; return true;
		}
		return false;
	}

	case NAME_ObjectProperty:
	{
		if (auto classProperty = CastField<FClassProperty>(InFunctionProperty))
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
	}

	case NAME_StrProperty:
	{
		OutParameterType = LGUIEventDelegateParameterType::String;
		return true;
	}
	case NAME_NameProperty:
	{
		OutParameterType = LGUIEventDelegateParameterType::Name;
		return true;
	}
	case NAME_TextProperty:
	{
		OutParameterType = LGUIEventDelegateParameterType::Text;
		return true;
	}
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



#if WITH_EDITOR
TArray<FLGUIEventDelegateData*> FLGUIEventDelegateData::AllEventDelegateDataArray;
FLGUIEventDelegateData::FLGUIEventDelegateData()
{
	AllEventDelegateDataArray.Add(this);
}
FLGUIEventDelegateData::~FLGUIEventDelegateData()
{
	AllEventDelegateDataArray.Remove(this);
}
void FLGUIEventDelegateData::RefreshAllOnBlueprintRecompile()
{
	for (auto Item : AllEventDelegateDataArray)
	{
		Item->RefreshOnBlueprintRecompile();
	}
}
void FLGUIEventDelegateData::RefreshOnBlueprintRecompile()
{
	if (!IsValid(TargetObject))
	{
		if (IsValid(HelperActor) && IsValid(HelperClass))
		{
			if (HelperClass == AActor::StaticClass())
			{
				TargetObject = HelperActor;
			}
			else
			{
				TArray<UActorComponent*> Components;
				HelperActor->GetComponents(HelperClass, Components);
				if (Components.Num() == 1)
				{
					TargetObject = Components[0];
				}
				else if(Components.Num() > 1)
				{
					for (auto Comp : Components)
					{
						if (Comp->GetFName() == HelperComponentName)
						{
							TargetObject = Comp;
						}
					}
				}
			}
		}
	}
}
#endif

void FLGUIEventDelegateData::Execute()
{
	if (UseNativeParameter)
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
		if (IsValid(TargetObject))
		{
			FindAndExecute(TargetObject);
		}
	}
}
void FLGUIEventDelegateData::Execute(void* InParam, LGUIEventDelegateParameterType InParameterType)
{
	if (ParamType == LGUIEventDelegateParameterType::None)
	{
		UE_LOG(LGUI, Error, TEXT("[FLGUIEventDelegateData::Execute]Not valid event"));
		return;
	}

	if (UseNativeParameter)//should use native parameter (pass in param)
	{
		if (ParamType != InParameterType)//function's supported parameter is equal to event's parameter
		{
			if (InParameterType == LGUIEventDelegateParameterType::Double && ParamType == LGUIEventDelegateParameterType::Float)
			{
				auto InValue = *((double*)InParam);
				auto ConvertValue = (float)InValue;
				InParam = &ConvertValue;
				UE_LOG(LGUI, Warning, TEXT("[FLGUIEventDelegateData::Execute]Parameter type not equal, LGUI will automatic convert it from double to float."));
			}
			else if (InParameterType == LGUIEventDelegateParameterType::Float && ParamType == LGUIEventDelegateParameterType::Double)
			{
				auto InValue = *((float*)InParam);
				auto ConvertValue = (double)InValue;
				InParam = &ConvertValue;
				UE_LOG(LGUI, Warning, TEXT("[FLGUIEventDelegateData::Execute]Parameter type not equal, LGUI will automatic convert it from float to double."));
			}
			else
			{
				UE_LOG(LGUI, Error, TEXT("[FLGUIEventDelegateData::Execute]Parameter type not equal!"));
				return;
			}
		}
		if (CacheTarget != nullptr && CacheFunction != nullptr)
		{
			ExecuteTargetFunction(CacheTarget, CacheFunction, InParam);
		}
		else
		{
			if (IsValid(TargetObject))
			{
				FindAndExecute(TargetObject, InParam);
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
			if (IsValid(TargetObject))
			{
				FindAndExecute(TargetObject);
			}
		}
	}
}

#if WITH_EDITOR
bool FLGUIEventDelegateData::CheckFunctionParameter()const
{
	if (ParamType == LGUIEventDelegateParameterType::None)
	{
		return false;
	}

	auto TargetFunction = TargetObject->FindFunction(functionName);
	if (!TargetFunction)
	{
		return false;
	}
	if (!ULGUIEventDelegateParameterHelper::IsStillSupported(TargetFunction, ParamType))
	{
		return false;
	}

	return true;
}
#endif

void FLGUIEventDelegateData::FindAndExecute(UObject* Target, void* ParamData)
{
	CacheTarget = Target;
	CacheFunction = Target->FindFunction(functionName);
	if (CacheFunction)
	{
		if (!ULGUIEventDelegateParameterHelper::IsStillSupported(CacheFunction, ParamType))
		{
			UE_LOG(LGUI, Error, TEXT("[LGUIEventDelegateData/FindAndExecute]Target function:%s not supported!"), *(functionName.ToString()));
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
		UE_LOG(LGUI, Error, TEXT("[LGUIEventDelegateData/FindAndExecute]Target function:%s not found!"), *(functionName.ToString()));
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
		FromBinary << TempString;
		Target->ProcessEvent(Func, &TempString);
	}
	break;
	case LGUIEventDelegateParameterType::Name:
	{
		FName TempName;
		auto FromBinary = FMemoryReader(ParamBuffer, false);
		FromBinary << TempName;
		Target->ProcessEvent(Func, &TempName);
	}
	break;
	case LGUIEventDelegateParameterType::Text:
	{
		FText TempText;
		auto FromBinary = FMemoryReader(ParamBuffer, false);
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
	supportParameterType = InParameterType;
}

bool FLGUIEventDelegate::IsBound()const
{
	return eventList.Num() != 0;
}
void FLGUIEventDelegate::FireEvent()const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Empty)
	{
		for (auto& item : eventList)
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
	UE_LOG(LGUI, Error, TEXT("[LGUIEventDelegate/FireEvent]Parameter type must be the same as your declaration. supportParameterType:%s"), *(enumObject->GetDisplayNameTextByValue((int64)supportParameterType)).ToString());
}
void FLGUIEventDelegate::FireEvent(void* InParam)const
{
	for (auto& item : eventList)
	{
		item.Execute(InParam, supportParameterType);
	}
}

void FLGUIEventDelegate::FireEvent(bool InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Bool)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(float InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Float)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(double InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Double)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(int8 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Int8)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(uint8 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::UInt8)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(int16 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Int16)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(uint16 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::UInt16)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(int32 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Int32)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(uint32 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::UInt32)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(int64 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Int64)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(uint64 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::UInt64)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(FVector2D InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Vector2)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(FVector InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Vector3)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(FVector4 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Vector4)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(FColor InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Color)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(FLinearColor InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::LinearColor)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(FQuat InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Quaternion)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(const FString& InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::String)
	{
		FireEvent((void*)&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(UObject* InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Object)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(AActor* InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Actor)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(ULGUIPointerEventData* InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::PointerEvent)
	{
		FireEvent((void*)&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(FRotator InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Rotator)
	{
		FireEvent(&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(const FName& InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Name)
	{
		FireEvent((void*)&InParam);
	}
	else LogParameterError();
}
void FLGUIEventDelegate::FireEvent(const FText& InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == LGUIEventDelegateParameterType::Text)
	{
		FireEvent((void*)&InParam);
	}
	else LogParameterError();
}

#if WITH_EDITOR
bool FLGUIEventDelegate::CheckFunctionParameter()const
{
	for (auto& item : eventList)
	{
		if (!item.CheckFunctionParameter())
		{
			return false;
		}
	}
	return true;
}
#endif

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
