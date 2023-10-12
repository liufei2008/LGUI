// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Event/LGUIEventDelegate.h"
#include "LGUI.h"
#include "Serialization/MemoryReader.h"
#if WITH_EDITOR
#include "Utils/LGUIUtils.h"
#endif

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

#define LOCTEXT_NAMESPACE "LGUIEventDelegate"

bool ULGUIEventDelegateParameterHelper::IsFunctionCompatible(const UFunction* InFunction, ELGUIEventDelegateParameterType& OutParameterType)
{
	if (InFunction->GetReturnProperty() != nullptr)return false;//not support return value for ProcessEvent
	TFieldIterator<FProperty> IteratorA(InFunction);
	TArray<ELGUIEventDelegateParameterType> ParameterTypeArray;
	while (IteratorA && (IteratorA->PropertyFlags & CPF_Parm))
	{
		FProperty* PropA = *IteratorA;
		ELGUIEventDelegateParameterType ParamType;
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
		OutParameterType = ELGUIEventDelegateParameterType::Empty;
		return true;
	}
	return false;
}
bool ULGUIEventDelegateParameterHelper::IsPropertyCompatible(const FProperty* InFunctionProperty, ELGUIEventDelegateParameterType& OutParameterType)
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
		OutParameterType = ELGUIEventDelegateParameterType::Bool;
		return true;
	}
	case NAME_FloatProperty:
	{
		OutParameterType = ELGUIEventDelegateParameterType::Float;
		return true;
	}
	case NAME_DoubleProperty:
	{
		OutParameterType = ELGUIEventDelegateParameterType::Double;
		return true;
	}
	case NAME_Int8Property:
	{
		OutParameterType = ELGUIEventDelegateParameterType::Int8;
		return true;
	}
	case NAME_ByteProperty:
	{
		OutParameterType = ELGUIEventDelegateParameterType::UInt8;
		return true;
	}
	case NAME_Int16Property:
	{
		OutParameterType = ELGUIEventDelegateParameterType::Int16;
		return true;
	}
	case NAME_UInt16Property:
	{
		OutParameterType = ELGUIEventDelegateParameterType::UInt16;
		return true;
	}
	case NAME_IntProperty:
	{
		OutParameterType = ELGUIEventDelegateParameterType::Int32;
		return true;
	}
	case NAME_UInt32Property:
	{
		OutParameterType = ELGUIEventDelegateParameterType::UInt32;
		return true;
	}
	case NAME_Int64Property:
	{
		OutParameterType = ELGUIEventDelegateParameterType::Int64;
		return true;
	}
	case NAME_UInt64Property:
	{
		OutParameterType = ELGUIEventDelegateParameterType::UInt64;
		return true;
	}
	case NAME_EnumProperty:
	{
		OutParameterType = ELGUIEventDelegateParameterType::UInt8;
		return true;
	}
	case NAME_StructProperty:
	{
		auto structProperty = (FStructProperty*)InFunctionProperty;
		auto structName = structProperty->Struct->GetFName();
		if (structName == NAME_Vector2D)
		{
			OutParameterType = ELGUIEventDelegateParameterType::Vector2; return true;
		}
		else if (structName == NAME_Vector)
		{
			OutParameterType = ELGUIEventDelegateParameterType::Vector3; return true;
		}
		else if (structName == NAME_Vector4)
		{
			OutParameterType = ELGUIEventDelegateParameterType::Vector4; return true;
		}
		else if (structName == NAME_Color)
		{
			OutParameterType = ELGUIEventDelegateParameterType::Color; return true;
		}
		else if (structName == NAME_LinearColor)
		{
			OutParameterType = ELGUIEventDelegateParameterType::LinearColor; return true;
		}
		else if (structName == NAME_Quat)
		{
			OutParameterType = ELGUIEventDelegateParameterType::Quaternion; return true;
		}
		else if (structName == NAME_Rotator)
		{
			OutParameterType = ELGUIEventDelegateParameterType::Rotator; return true;
		}
		return false;
	}

	case NAME_ObjectProperty:
	{
		if (auto classProperty = CastField<FClassProperty>(InFunctionProperty))
		{
			OutParameterType = ELGUIEventDelegateParameterType::Class;
			return true;
		}
		else if (auto objectProperty = CastField<FObjectProperty>(InFunctionProperty))//if object property
		{
			if (objectProperty->PropertyClass->IsChildOf(AActor::StaticClass()))//if is Actor
			{
				OutParameterType = ELGUIEventDelegateParameterType::Actor;
			}
			else if (objectProperty->PropertyClass->IsChildOf(ULGUIPointerEventData::StaticClass()))
			{
				OutParameterType = ELGUIEventDelegateParameterType::PointerEvent;
			}
			else if (objectProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))
			{
				return false;
			}
			else
			{
				OutParameterType = ELGUIEventDelegateParameterType::Object;
			}
			return true;
		}
	}

	case NAME_StrProperty:
	{
		OutParameterType = ELGUIEventDelegateParameterType::String;
		return true;
	}
	case NAME_NameProperty:
	{
		OutParameterType = ELGUIEventDelegateParameterType::Name;
		return true;
	}
	case NAME_TextProperty:
	{
		OutParameterType = ELGUIEventDelegateParameterType::Text;
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

bool ULGUIEventDelegateParameterHelper::IsSupportedFunction(UFunction* Target, ELGUIEventDelegateParameterType& OutParamType)
{
	return IsFunctionCompatible(Target, OutParamType);
}

bool ULGUIEventDelegateParameterHelper::IsStillSupported(UFunction* Target, ELGUIEventDelegateParameterType InParamType)
{
	ELGUIEventDelegateParameterType ParamType;
	if (IsSupportedFunction(Target, ParamType))
	{
		if (ParamType == InParamType)
		{
			return true;
		}
	}
	return false;
}

FString ULGUIEventDelegateParameterHelper::ParameterTypeToName(ELGUIEventDelegateParameterType paramType, const UFunction* InFunction)
{
	FString ParamTypeString = "";
	switch (paramType)
	{
	case ELGUIEventDelegateParameterType::Empty:
		break;
	case ELGUIEventDelegateParameterType::Bool:
		ParamTypeString = "Bool";
		break;
	case ELGUIEventDelegateParameterType::Float:
		ParamTypeString = "Float";
		break;
	case ELGUIEventDelegateParameterType::Double:
		ParamTypeString = "Double";
		break;
	case ELGUIEventDelegateParameterType::Int8:
		ParamTypeString = "Int8";
		break;
	case ELGUIEventDelegateParameterType::UInt8:
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
	case ELGUIEventDelegateParameterType::Int16:
		ParamTypeString = "Int16";
		break;
	case ELGUIEventDelegateParameterType::UInt16:
		ParamTypeString = "UInt16";
		break;
	case ELGUIEventDelegateParameterType::Int32:
		ParamTypeString = "Int32";
		break;
	case ELGUIEventDelegateParameterType::UInt32:
		ParamTypeString = "UInt32";
		break;
	case ELGUIEventDelegateParameterType::Int64:
		ParamTypeString = "Int64";
		break;
	case ELGUIEventDelegateParameterType::UInt64:
		ParamTypeString = "UInt64";
		break;
	case ELGUIEventDelegateParameterType::Vector2:
		ParamTypeString = "Vector2";
		break;
	case ELGUIEventDelegateParameterType::Vector3:
		ParamTypeString = "Vector3";
		break;
	case ELGUIEventDelegateParameterType::Vector4:
		ParamTypeString = "Vector4";
		break;
	case ELGUIEventDelegateParameterType::Quaternion:
		ParamTypeString = "Quaternion";
		break;
	case ELGUIEventDelegateParameterType::Color:
		ParamTypeString = "Color";
		break;
	case ELGUIEventDelegateParameterType::LinearColor:
		ParamTypeString = "LinearColor";
		break;
	case ELGUIEventDelegateParameterType::String:
		ParamTypeString = "String";
		break;

	case ELGUIEventDelegateParameterType::Object:
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
	case ELGUIEventDelegateParameterType::Actor:
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
	case ELGUIEventDelegateParameterType::PointerEvent:
		ParamTypeString = "PointerEvent";
		break;
	case ELGUIEventDelegateParameterType::Class:
		ParamTypeString = "Class";
		break;
	case ELGUIEventDelegateParameterType::Rotator:
		ParamTypeString = "Rotator";
		break;
	case ELGUIEventDelegateParameterType::Name:
		ParamTypeString = "Name";
		break;
	case ELGUIEventDelegateParameterType::Text:
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
	for (int i = 0; i < AllEventDelegateDataArray.Num(); i++)
	{
		auto& Item = AllEventDelegateDataArray[i];
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
		auto errMsg = LOCTEXT("NativeParameterError", "LGUIEventDelegateData.Execute, If use NativeParameter, you must FireEvent with your own parameter!");
#if WITH_EDITOR
		LGUIUtils::EditorNotification(errMsg, 10);
#endif
		UE_LOG(LGUI, Error, TEXT("[%s].%d %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *errMsg.ToString());
		return;
	}
	if (ParamType == ELGUIEventDelegateParameterType::None)
	{
		auto errMsg = LOCTEXT("NotValid", "LGUIEventDelegateData.Execute, Not valid LGUIEventDelegate.");
#if WITH_EDITOR
		LGUIUtils::EditorNotification(errMsg, 10);
#endif
		UE_LOG(LGUI, Error, TEXT("[%s].%d %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *errMsg.ToString());
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
void FLGUIEventDelegateData::Execute(void* InParam, ELGUIEventDelegateParameterType InParameterType)
{
	if (ParamType == ELGUIEventDelegateParameterType::None)
	{
		auto errMsg = LOCTEXT("NotValid", "LGUIEventDelegateData.Execute, Not valid LGUIEventDelegate.");
#if WITH_EDITOR
		LGUIUtils::EditorNotification(errMsg, 10);
#endif
		UE_LOG(LGUI, Error, TEXT("[%s].%d %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *errMsg.ToString());
		return;
	}

	if (UseNativeParameter)//should use native parameter (pass in param)
	{
		if (ParamType != InParameterType)//function's supported parameter is equal to event's parameter
		{
			if (InParameterType == ELGUIEventDelegateParameterType::Double && ParamType == ELGUIEventDelegateParameterType::Float)
			{
				auto InValue = *((double*)InParam);
				auto ConvertValue = (float)InValue;
				InParam = &ConvertValue;
				auto errMsg = LOCTEXT("ParameterTypeNotEqual_DoubleToFloat", "LGUIEventDelegateData.Execute, Parameter type not equal, LGUI will automatic convert it from double to float.");
#if WITH_EDITOR
				LGUIUtils::EditorNotification(errMsg, 10);
#endif
				UE_LOG(LGUI, Error, TEXT("[%s].%d %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *errMsg.ToString());
			}
			else if (InParameterType == ELGUIEventDelegateParameterType::Float && ParamType == ELGUIEventDelegateParameterType::Double)
			{
				auto InValue = *((float*)InParam);
				auto ConvertValue = (double)InValue;
				InParam = &ConvertValue;
				auto errMsg = LOCTEXT("ParameterTypeNotEqual_FloatToDouble", "LGUIEventDelegateData.Execute, Parameter type not equal, LGUI will automatic convert it from float to double.");
#if WITH_EDITOR
				LGUIUtils::EditorNotification(errMsg, 10);
#endif
				UE_LOG(LGUI, Error, TEXT("[%s].%d %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *errMsg.ToString());
			}
			else
			{
				auto errMsg = LOCTEXT("ParameterTypeNotEqual", "LGUIEventDelegateData.Execute, Parameter type not equal!");
#if WITH_EDITOR
				LGUIUtils::EditorNotification(errMsg, 10);
#endif
				UE_LOG(LGUI, Error, TEXT("[%s].%d %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *errMsg.ToString());
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
	if (ParamType == ELGUIEventDelegateParameterType::None)
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
			auto errMsg = FText::Format(LOCTEXT("FunctionNotSupport", "LGUIEventDelegateData.FindAndExecute, Target function: {0} not supported!"), FText::FromName(functionName));
#if WITH_EDITOR
			LGUIUtils::EditorNotification(errMsg, 10);
#endif
			UE_LOG(LGUI, Error, TEXT("[%s].%d %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *errMsg.ToString());
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
		auto errMsg = FText::Format(LOCTEXT("FunctionNotExist", "LGUIEventDelegateData.FindAndExecute, Target function: {0} not exist!"), FText::FromName(functionName));
#if WITH_EDITOR
		LGUIUtils::EditorNotification(errMsg, 10);
#endif
		UE_LOG(LGUI, Error, TEXT("[%s].%d %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *errMsg.ToString());
	}
}
void FLGUIEventDelegateData::ExecuteTargetFunction(UObject* Target, UFunction* Func)
{
	switch (ParamType)
	{
	case ELGUIEventDelegateParameterType::String:
	{
		FString TempString;
		auto FromBinary = FMemoryReader(ParamBuffer, false);
		FromBinary << TempString;
		Target->ProcessEvent(Func, &TempString);
	}
	break;
	case ELGUIEventDelegateParameterType::Name:
	{
		FName TempName;
		auto FromBinary = FMemoryReader(ParamBuffer, false);
		FromBinary << TempName;
		Target->ProcessEvent(Func, &TempName);
	}
	break;
	case ELGUIEventDelegateParameterType::Text:
	{
		FText TempText;
		auto FromBinary = FMemoryReader(ParamBuffer, false);
		FromBinary << TempText;
		Target->ProcessEvent(Func, &TempText);
	}
	break;
	case ELGUIEventDelegateParameterType::Object:
	case ELGUIEventDelegateParameterType::Actor:
	case ELGUIEventDelegateParameterType::Class:
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
FLGUIEventDelegate::FLGUIEventDelegate(ELGUIEventDelegateParameterType InParameterType)
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
	if (supportParameterType == ELGUIEventDelegateParameterType::Empty)
	{
		for (auto& item : eventList)
		{
			item.Execute();
		}
	}
	else
		LogParameterError(ELGUIEventDelegateParameterType::Empty);
}
void FLGUIEventDelegate::LogParameterError(ELGUIEventDelegateParameterType WrongParamType)const
{
	auto enumObject = FindObject<UEnum>(nullptr, TEXT("/Script/LGUI.ELGUIEventDelegateParameterType"), true);
	auto errMsg = FText::Format(LOCTEXT("ParameterTypeMismatch", "LGUIEventDelegate parameter type must be the same as your declaration. support parameter type: {0}, execute parameter type: {1}")
		, enumObject->GetDisplayNameTextByValue((int64)supportParameterType)
		, enumObject->GetDisplayNameTextByValue((int64)WrongParamType)
	);
#if WITH_EDITOR
	LGUIUtils::EditorNotification(errMsg, 10);
#endif
	UE_LOG(LGUI, Error, TEXT("[%s].%d %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *errMsg.ToString());
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
	if (supportParameterType == ELGUIEventDelegateParameterType::Bool)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Bool);
}
void FLGUIEventDelegate::FireEvent(float InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Float)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Float);
}
void FLGUIEventDelegate::FireEvent(double InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Double)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Double);
}
void FLGUIEventDelegate::FireEvent(int8 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Int8)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Int8);
}
void FLGUIEventDelegate::FireEvent(uint8 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::UInt8)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::UInt8);
}
void FLGUIEventDelegate::FireEvent(int16 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Int16)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Int16);
}
void FLGUIEventDelegate::FireEvent(uint16 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::UInt16)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::UInt16);
}
void FLGUIEventDelegate::FireEvent(int32 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Int32)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Int32);
}
void FLGUIEventDelegate::FireEvent(uint32 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::UInt32)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::UInt32);
}
void FLGUIEventDelegate::FireEvent(int64 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Int64)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Int64);
}
void FLGUIEventDelegate::FireEvent(uint64 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::UInt64)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::UInt64);
}
void FLGUIEventDelegate::FireEvent(FVector2D InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Vector2)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Vector2);
}
void FLGUIEventDelegate::FireEvent(FVector InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Vector3)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Vector3);
}
void FLGUIEventDelegate::FireEvent(FVector4 InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Vector4)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Vector4);
}
void FLGUIEventDelegate::FireEvent(FColor InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Color)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Color);
}
void FLGUIEventDelegate::FireEvent(FLinearColor InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::LinearColor)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::LinearColor);
}
void FLGUIEventDelegate::FireEvent(FQuat InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Quaternion)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Quaternion);
}
void FLGUIEventDelegate::FireEvent(const FString& InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::String)
	{
		FireEvent((void*)&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::String);
}
void FLGUIEventDelegate::FireEvent(UObject* InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Object)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Object);
}
void FLGUIEventDelegate::FireEvent(AActor* InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Actor)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Actor);
}
void FLGUIEventDelegate::FireEvent(ULGUIPointerEventData* InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::PointerEvent)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::PointerEvent);
}
void FLGUIEventDelegate::FireEvent(UClass* InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Class)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Class);
}
void FLGUIEventDelegate::FireEvent(FRotator InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Rotator)
	{
		FireEvent(&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Rotator);
}
void FLGUIEventDelegate::FireEvent(const FName& InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Name)
	{
		FireEvent((void*)&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Name);
}
void FLGUIEventDelegate::FireEvent(const FText& InParam)const
{
	if (eventList.Num() == 0)return;
	if (supportParameterType == ELGUIEventDelegateParameterType::Text)
	{
		FireEvent((void*)&InParam);
	}
	else LogParameterError(ELGUIEventDelegateParameterType::Text);
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

#undef LOCTEXT_NAMESPACE

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
