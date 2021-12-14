// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabOverrideParameter.h"
#include "LGUI.h"
#include "Serialization/BufferArchive.h"

PRAGMA_DISABLE_OPTIMIZATION

bool ULGUIPrefabOverrideParameterHelper::IsSupportedProperty(const FProperty* InProperty, ELGUIPrefabOverrideParameterType& OutParameterType)
{
	if (!InProperty)
	{
		return false;
	}

	if (auto boolProperty = CastField<FBoolProperty>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::Bool;
		return true;
	}
	else if (auto floatProperty = CastField<FFloatProperty>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::Float;
		return true;
	}
	else if (auto doubleProperty = CastField<FDoubleProperty>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::Double;
		return true;
	}
	else if (auto int8Property = CastField<FInt8Property>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::Int8;
		return true;
	}
	else if (auto uint8Property = CastField<FByteProperty>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::UInt8;
		return true;
	}
	else if (auto int16Property = CastField<FInt16Property>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::Int16;
		return true;
	}
	else if (auto uint16Property = CastField<FUInt16Property>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::UInt16;
		return true;
	}
	else if (auto int32Property = CastField<FIntProperty>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::Int32;
		return true;
	}
	else if (auto uint32Property = CastField<FUInt32Property>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::UInt32;
		return true;
	}
	else if (auto int64Property = CastField<FInt64Property>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::Int64;
		return true;
	}
	else if (auto uint64Property = CastField<FUInt64Property>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::UInt64;
		return true;
	}
	else if (auto enumProperty = CastField<FEnumProperty>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::UInt8;
		return true;
	}
	else if (auto structProperty = CastField<FStructProperty>(InProperty))
	{
		auto structName = structProperty->Struct->GetFName();
		if (structName == TEXT("Vector2D"))
		{
			OutParameterType = ELGUIPrefabOverrideParameterType::Vector2; return true;
		}
		else if (structName == TEXT("Vector"))
		{
			OutParameterType = ELGUIPrefabOverrideParameterType::Vector3; return true;
		}
		else if (structName == TEXT("Vector4"))
		{
			OutParameterType = ELGUIPrefabOverrideParameterType::Vector4; return true;
		}
		else if (structName == TEXT("Color"))
		{
			OutParameterType = ELGUIPrefabOverrideParameterType::Color; return true;
		}
		else if (structName == TEXT("LinearColor"))
		{
			OutParameterType = ELGUIPrefabOverrideParameterType::LinearColor; return true;
		}
		else if (structName == TEXT("Quat"))
		{
			OutParameterType = ELGUIPrefabOverrideParameterType::Quaternion; return true;
		}
		else if (structName == TEXT("Rotator"))
		{
			OutParameterType = ELGUIPrefabOverrideParameterType::Rotator; return true;
		}
		else
		{
			return false;
		}
	}

	else if (auto classProperty = CastField<FClassProperty>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::Class;
		return true;
	}
	else if (auto objectProperty = CastField<FObjectProperty>(InProperty))//if object property
	{
		if (objectProperty->PropertyClass->IsChildOf(AActor::StaticClass()))//if is Actor
		{
			OutParameterType = ELGUIPrefabOverrideParameterType::Actor;
		}
		else if (objectProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))
		{
			return false;
		}
		else
		{
			OutParameterType = ELGUIPrefabOverrideParameterType::Object;
		}
		return true;
	}

	else if (auto strProperty = CastField<FStrProperty>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::String;
		return true;
	}
	else if (auto nameProperty = CastField<FNameProperty>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::Name;
		return true;
	}
	else if (auto textProperty = CastField<FTextProperty>(InProperty))
	{
		OutParameterType = ELGUIPrefabOverrideParameterType::Text;
		return true;
	}

	return false;
}

UClass* ULGUIPrefabOverrideParameterHelper::GetObjectParameterClass(const FProperty* InProperty)
{
	if (auto objProperty = CastField<FObjectProperty>(InProperty))
	{
		return objProperty->PropertyClass;
	}
	return nullptr;
}

UEnum* ULGUIPrefabOverrideParameterHelper::GetEnumParameter(const FProperty* InProperty)
{
	if (auto uint8Property = CastField<FByteProperty>(InProperty))
	{
		if (uint8Property->IsEnum())
		{
			return uint8Property->Enum;
		}
	}
	if (auto enumProperty = CastField<FEnumProperty>(InProperty))
	{
		return enumProperty->GetEnum();
	}
	return nullptr;
}
UClass* ULGUIPrefabOverrideParameterHelper::GetClassParameterClass(const FProperty* InProperty)
{
	if (auto classProperty = CastField<FClassProperty>(InProperty))
	{
		return classProperty->MetaClass;
	}
	return nullptr;
}

bool ULGUIPrefabOverrideParameterHelper::IsStillSupported(const FProperty* Target, ELGUIPrefabOverrideParameterType InParamType)
{
	ELGUIPrefabOverrideParameterType ParamType;
	if (IsSupportedProperty(Target, ParamType))
	{
		if (ParamType == InParamType)
		{
			return true;
		}
	}
	return false;
}

FString ULGUIPrefabOverrideParameterHelper::ParameterTypeToName(ELGUIPrefabOverrideParameterType paramType, const FProperty* InProperty)
{
	FString ParamTypeString = "";
	switch (paramType)
	{
	case ELGUIPrefabOverrideParameterType::Bool:
		ParamTypeString = "Bool";
		break;
	case ELGUIPrefabOverrideParameterType::Float:
		ParamTypeString = "Float";
		break;
	case ELGUIPrefabOverrideParameterType::Double:
		ParamTypeString = "Double";
		break;
	case ELGUIPrefabOverrideParameterType::Int8:
		ParamTypeString = "Int8";
		break;
	case ELGUIPrefabOverrideParameterType::UInt8:
	{
		if (auto enumValue = GetEnumParameter(InProperty))
		{
			ParamTypeString = enumValue->GetName() + "(Enum)";
		}
		else
		{
			ParamTypeString = "UInt8";
		}
	}
	break;
	case ELGUIPrefabOverrideParameterType::Int16:
		ParamTypeString = "Int16";
		break;
	case ELGUIPrefabOverrideParameterType::UInt16:
		ParamTypeString = "UInt16";
		break;
	case ELGUIPrefabOverrideParameterType::Int32:
		ParamTypeString = "Int32";
		break;
	case ELGUIPrefabOverrideParameterType::UInt32:
		ParamTypeString = "UInt32";
		break;
	case ELGUIPrefabOverrideParameterType::Int64:
		ParamTypeString = "Int64";
		break;
	case ELGUIPrefabOverrideParameterType::UInt64:
		ParamTypeString = "UInt64";
		break;
	case ELGUIPrefabOverrideParameterType::Vector2:
		ParamTypeString = "Vector2";
		break;
	case ELGUIPrefabOverrideParameterType::Vector3:
		ParamTypeString = "Vector3";
		break;
	case ELGUIPrefabOverrideParameterType::Vector4:
		ParamTypeString = "Vector4";
		break;
	case ELGUIPrefabOverrideParameterType::Quaternion:
		ParamTypeString = "Quaternion";
		break;
	case ELGUIPrefabOverrideParameterType::Color:
		ParamTypeString = "Color";
		break;
	case ELGUIPrefabOverrideParameterType::LinearColor:
		ParamTypeString = "LinearColor";
		break;
	case ELGUIPrefabOverrideParameterType::String:
		ParamTypeString = "String";
		break;
	case ELGUIPrefabOverrideParameterType::Object:
	{
		if (auto ObjProperty = CastField<FObjectProperty>(InProperty))
		{
			if (ObjProperty->PropertyClass != UObject::StaticClass())
			{
				ParamTypeString = ObjProperty->PropertyClass->GetName() + "(Object)";
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
	case ELGUIPrefabOverrideParameterType::Actor:
	{
		if (auto ObjProperty = CastField<FObjectProperty>(InProperty))
		{
			if (ObjProperty->PropertyClass != AActor::StaticClass())
			{
				ParamTypeString = ObjProperty->PropertyClass->GetName() + "(Actor)";
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
	case ELGUIPrefabOverrideParameterType::Class:
		ParamTypeString = "Class";
		break;
	case ELGUIPrefabOverrideParameterType::Rotator:
		ParamTypeString = "Rotator";
		break;
	case ELGUIPrefabOverrideParameterType::Name:
		ParamTypeString = "Name";
		break;
	case ELGUIPrefabOverrideParameterType::Text:
		ParamTypeString = "Text";
		break;
	default:
		break;
	}
	return ParamTypeString;
}

bool FLGUIPrefabOverrideParameterData::ApplyPropertyParameter(UObject* InTarget, FProperty* InProperty)
{
	if (!InProperty || !InTarget)
	{
		return false;
	}

	if (auto boolProperty = CastField<FBoolProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Bool, InProperty))
		{
			//some bool is declared as bit field, so we need to handle it specially
			auto Value = ParamBuffer[0] == 1;
			boolProperty->SetPropertyValue_InContainer(InTarget, Value);
			return true;
		}
	}
	else if (auto floatProperty = CastField<FFloatProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Float, InProperty))
		{
			InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
			return true;
		}
	}
	else if (auto doubleProperty = CastField<FDoubleProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Double, InProperty))
		{
			InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
			return true;
		}
	}
	else if (auto int8Property = CastField<FInt8Property>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Int8, InProperty))
		{
			InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
			return true;
		}
	}
	else if (auto uint8Property = CastField<FByteProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::UInt8, InProperty))
		{
			InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
			return true;
		}
	}
	else if (auto int16Property = CastField<FInt16Property>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Int16, InProperty))
		{
			InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
			return true;
		}
	}
	else if (auto uint16Property = CastField<FUInt16Property>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::UInt16, InProperty))
		{
			InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
			return true;
		}
	}
	else if (auto int32Property = CastField<FIntProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Int32, InProperty))
		{
			InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
			return true;
		}
	}
	else if (auto uint32Property = CastField<FUInt32Property>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::UInt32, InProperty))
		{
			InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
			return true;
		}
	}
	else if (auto int64Property = CastField<FInt64Property>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Int64, InProperty))
		{
			InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
			return true;
		}
	}
	else if (auto uint64Property = CastField<FUInt64Property>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::UInt64, InProperty))
		{
			InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
			return true;
		}
	}
	else if (auto enumProperty = CastField<FEnumProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::UInt8, InProperty))
		{
			InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
			return true;
		}
	}
	else if (auto structProperty = CastField<FStructProperty>(InProperty))
	{
		auto structName = structProperty->Struct->GetFName();
		if (structName == TEXT("Vector2D"))
		{
			if (CheckDataType(ELGUIPrefabOverrideParameterType::Vector2, InProperty))
			{
				InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
				return true;
			}
		}
		else if (structName == TEXT("Vector"))
		{
			if (CheckDataType(ELGUIPrefabOverrideParameterType::Vector3, InProperty))
			{
				InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
				return true;
			}
		}
		else if (structName == TEXT("Vector4"))
		{
			if (CheckDataType(ELGUIPrefabOverrideParameterType::Vector4, InProperty))
			{
				InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
				return true;
			}
		}
		else if (structName == TEXT("Color"))
		{
			if (CheckDataType(ELGUIPrefabOverrideParameterType::Color, InProperty))
			{
				InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
				return true;
			}
		}
		else if (structName == TEXT("LinearColor"))
		{
			if (CheckDataType(ELGUIPrefabOverrideParameterType::LinearColor, InProperty))
			{
				InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
				return true;
			}
		}
		else if (structName == TEXT("Quat"))
		{
			if (CheckDataType(ELGUIPrefabOverrideParameterType::Quaternion, InProperty))
			{
				InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
				return true;
			}
		}
		else if (structName == TEXT("Rotator"))
		{
			if (CheckDataType(ELGUIPrefabOverrideParameterType::Rotator, InProperty))
			{
				InProperty->CopyCompleteValue(InProperty->ContainerPtrToValuePtr<void>(InTarget), ParamBuffer.GetData());
				return true;
			}
		}
		else
		{
			return false;
		}
	}

	else if (auto objectProperty = CastField<FObjectProperty>(InProperty))//if object property
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Object))
		{
			if (objectProperty->PropertyClass->IsAsset())
			{
				objectProperty->SetPropertyValue_InContainer(InTarget, ReferenceObject);
				return true;
			}
		}
	}

	else if (auto strProperty = CastField<FStrProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::String))
		{
			FString Value;
			auto FromBinary = FMemoryReader(ParamBuffer, false);
			FromBinary << Value;
			strProperty->SetPropertyValue_InContainer(InTarget, Value);
			return true;
		}
	}
	else if (auto nameProperty = CastField<FNameProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Name))
		{
			FName Value;
			auto FromBinary = FMemoryReader(ParamBuffer, false);
			FromBinary << Value;
			nameProperty->SetPropertyValue_InContainer(InTarget, Value);
			return true;
		}
	}
	else if (auto textProperty = CastField<FTextProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Text))
		{
			FText Value;
			auto FromBinary = FMemoryReader(ParamBuffer, false);
			FromBinary << Value;
			textProperty->SetPropertyValue_InContainer(InTarget, Value);
			return true;
		}
	}

	return false;
}

#define SET_BUFFER_ON_VALUE()\
ParamBuffer.Empty();\
ParamBuffer.AddUninitialized(InProperty->ElementSize);\
InProperty->CopyCompleteValue(ParamBuffer.GetData(), InProperty->ContainerPtrToValuePtr<void>(InTarget));\

bool FLGUIPrefabOverrideParameterData::SavePropertyParameter(UObject* InTarget, FProperty* InProperty)
{
	if (!InProperty || !InTarget)
	{
		return false;
	}

	if (auto boolProperty = CastField<FBoolProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Bool))
		{
			SET_BUFFER_ON_VALUE();
			return true;
		}
	}
	else if (auto floatProperty = CastField<FFloatProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Float))
		{
			SET_BUFFER_ON_VALUE();
			return true;
		}
	}
	else if (auto doubleProperty = CastField<FDoubleProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Double))
		{
			SET_BUFFER_ON_VALUE();
			return true;
		}
	}
	else if (auto int8Property = CastField<FInt8Property>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Int8))
		{
			SET_BUFFER_ON_VALUE();
			return true;
		}
	}
	else if (auto uint8Property = CastField<FByteProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::UInt8))
		{
			SET_BUFFER_ON_VALUE();
			return true;
		}
	}
	else if (auto int16Property = CastField<FInt16Property>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Int16))
		{
			SET_BUFFER_ON_VALUE();
			return true;
		}
	}
	else if (auto uint16Property = CastField<FUInt16Property>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::UInt16))
		{
			SET_BUFFER_ON_VALUE();
			return true;
		}
	}
	else if (auto int32Property = CastField<FIntProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Int32))
		{
			SET_BUFFER_ON_VALUE();
			return true;
		}
	}
	else if (auto uint32Property = CastField<FUInt32Property>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::UInt32))
		{
			SET_BUFFER_ON_VALUE();
			return true;
		}
	}
	else if (auto int64Property = CastField<FInt64Property>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Int64))
		{
			SET_BUFFER_ON_VALUE();
			return true;
		}
	}
	else if (auto uint64Property = CastField<FUInt64Property>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::UInt64))
		{
			SET_BUFFER_ON_VALUE();
			return true;
		}
	}
	else if (auto enumProperty = CastField<FEnumProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::UInt8))
		{
			SET_BUFFER_ON_VALUE();
			return true;
		}
	}
	else if (auto structProperty = CastField<FStructProperty>(InProperty))
	{
		auto structName = structProperty->Struct->GetFName();
		if (structName == TEXT("Vector2D"))
		{
			if (CheckDataType(ELGUIPrefabOverrideParameterType::Vector2))
			{
				SET_BUFFER_ON_VALUE();
				return true;
			}
		}
		else if (structName == TEXT("Vector"))
		{
			if (CheckDataType(ELGUIPrefabOverrideParameterType::Vector3))
			{
				SET_BUFFER_ON_VALUE();
				return true;
			}
		}
		else if (structName == TEXT("Vector4"))
		{
			if (CheckDataType(ELGUIPrefabOverrideParameterType::Vector4))
			{
				SET_BUFFER_ON_VALUE();
				return true;
			}
		}
		else if (structName == TEXT("Color"))
		{
			if (CheckDataType(ELGUIPrefabOverrideParameterType::Color))
			{
				SET_BUFFER_ON_VALUE();
				return true;
			}
		}
		else if (structName == TEXT("LinearColor"))
		{
			if (CheckDataType(ELGUIPrefabOverrideParameterType::LinearColor))
			{
				SET_BUFFER_ON_VALUE();
				return true;
			}
		}
		else if (structName == TEXT("Quat"))
		{
			if (CheckDataType(ELGUIPrefabOverrideParameterType::Quaternion))
			{
				SET_BUFFER_ON_VALUE();
				return true;
			}
		}
		else if (structName == TEXT("Rotator"))
		{
			if (CheckDataType(ELGUIPrefabOverrideParameterType::Rotator))
			{
				SET_BUFFER_ON_VALUE();
				return true;
			}
		}
		else
		{
			return false;
		}
	}

	else if (auto objectProperty = CastField<FObjectProperty>(InProperty))//if object property
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Object))
		{
			if (objectProperty->PropertyClass->IsAsset())
			{
				ReferenceObject = objectProperty->GetPropertyValue_InContainer(InTarget);
				return true;
			}
		}
	}

	else if (auto strProperty = CastField<FStrProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::String))
		{
			auto Value = strProperty->GetPropertyValue_InContainer(InTarget);
			FBufferArchive ToBinary;
			ToBinary << Value;
			ParamBuffer = ToBinary;
			return true;
		}
	}
	else if (auto nameProperty = CastField<FNameProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Name))
		{
			auto Value = nameProperty->GetPropertyValue_InContainer(InTarget);
			FBufferArchive ToBinary;
			ToBinary << Value;
			ParamBuffer = ToBinary;
			return true;
		}
	}
	else if (auto textProperty = CastField<FTextProperty>(InProperty))
	{
		if (CheckDataType(ELGUIPrefabOverrideParameterType::Text))
		{
			FText Value = textProperty->GetPropertyValue_InContainer(InTarget);
			FBufferArchive ToBinary;
			ToBinary << Value;
			ParamBuffer = ToBinary;
			return true;
		}
	}

	return false;
}

FLGUIPrefabOverrideParameterData::FLGUIPrefabOverrideParameterData()
{
	Guid = FGuid::NewGuid();
}

void FLGUIPrefabOverrideParameterData::SaveDefaultValue()
{
	if (TargetObject.IsValid())
	{
		if (auto FoundProperty = FindFProperty<FProperty>(TargetObject->GetClass(), PropertyName))
		{
			SavePropertyParameter(TargetObject.Get(), FoundProperty);
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[FLGUIPrefabOverrideParameterData::ApplyDefaultValue]Property '%s' not found on object '%s"), *(PropertyName.ToString()), *(TargetObject->GetClass()->GetPathName()));
		}
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[FLGUIPrefabOverrideParameterData::ApplyDefaultValue]TargetObject not valid!"));
	}
}

bool FLGUIPrefabOverrideParameterData::CheckDataType(ELGUIPrefabOverrideParameterType PropertyType)
{
	if (ParamType != PropertyType)
	{
		UE_LOG(LGUI, Error, TEXT("[FLGUIPrefabOverrideParameterData::CheckDataType]Parameter type not equal!"));
		return false;
	}
	return true;
}

bool FLGUIPrefabOverrideParameterData::CheckDataType(ELGUIPrefabOverrideParameterType PropertyType, FProperty* InProperty)
{
	if (ParamType != PropertyType)
	{
		UE_LOG(LGUI, Error, TEXT("[FLGUIPrefabOverrideParameterData::CheckDataType]Parameter type not equal!"));
		return false;
	}
	if (InProperty->GetSize() != ParamBuffer.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[FLGUIPrefabOverrideParameterData::CheckDataType]Parameter buffer size not match! Property size:%d, buffer size:%d"), InProperty->GetSize(), ParamBuffer.Num());
		return false;
	}
	return true;
}

void FLGUIPrefabOverrideParameterData::ApplyParameter()
{
	if (TargetObject.IsValid())
	{
		if (auto FoundProperty = FindFProperty<FProperty>(TargetObject->GetClass(), PropertyName))
		{
			ApplyPropertyParameter(TargetObject.Get(), FoundProperty);
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[FLGUIPrefabOverrideParameterData::ApplyParameter]Property '%s' not found on object '%s"), *(PropertyName.ToString()), *(TargetObject->GetClass()->GetPathName()));
		}
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[FLGUIPrefabOverrideParameterData::ApplyParameter]TargetObject not valid!"));
	}
}

void FLGUIPrefabOverrideParameterData::SetParameterReferenceFromTemplate(const FLGUIPrefabOverrideParameterData& InTemplate)
{
#if WITH_EDITORONLY_DATA
	this->HelperActor = InTemplate.HelperActor;//copy source reference from template
#endif
	this->TargetObject = InTemplate.TargetObject;//copy source reference from template

	//this->ReferenceObject = InTemplate.ReferenceObject;//ReferenceObject is parameter value, which should reference current prefab's object
}

bool FLGUIPrefabOverrideParameterData::IsReferenceParameterEqual(const FLGUIPrefabOverrideParameterData& Other)const
{
	return
#if WITH_EDITORONLY_DATA
		this->HelperActor == Other.HelperActor &&
#endif
		this->TargetObject == Other.TargetObject &&
		this->PropertyName == Other.PropertyName;
}

bool FLGUIPrefabOverrideParameterData::IsParameter_Type_Name_Guid_Equal(const FLGUIPrefabOverrideParameterData& Other)const
{
	return
#if WITH_EDITORONLY_DATA
		this->Guid == Other.Guid &&
#endif
		this->ParamType == Other.ParamType &&
		this->PropertyName == Other.PropertyName;
}


void FLGUIPrefabOverrideParameter::SaveDefaultValue()
{
	for (auto& item : ParameterList)
	{
		item.SaveDefaultValue();
	}
}


void FLGUIPrefabOverrideParameter::ApplyParameter()
{
	for (auto& item : ParameterList)
	{
		item.ApplyParameter();
	}
}

void FLGUIPrefabOverrideParameter::SetParameterReferenceFromTemplate(const FLGUIPrefabOverrideParameter& InTemplate)
{
	for (int i = 0; i < ParameterList.Num(); i++)
	{
		ParameterList[i].SetParameterReferenceFromTemplate(InTemplate.ParameterList[i]);
	}
}
bool FLGUIPrefabOverrideParameter::RefreshParameterOnTemplate(const FLGUIPrefabOverrideParameter& InTemplate)
{
	//compare, find by key, keep original value
	//compare first, check if order or reference change
	bool ShouldCheck = false;
	if (this->ParameterList.Num() != InTemplate.ParameterList.Num())
	{
		ShouldCheck = true;
	}
	if (!ShouldCheck)
	{
		for (int i = 0; i < this->ParameterList.Num(); i++)
		{
			if (!this->ParameterList[i].IsParameter_Type_Name_Guid_Equal(InTemplate.ParameterList[i]))
			{
				ShouldCheck = true;
				break;
			}
		}
	}
	if (ShouldCheck)
	{
		auto OldThisParameterList = this->ParameterList;
		this->ParameterList.Reset();
		this->ParameterList.Reserve(InTemplate.ParameterList.Num());
		for (int i = 0; i < InTemplate.ParameterList.Num(); i++)
		{
			auto& TemplateItem = InTemplate.ParameterList[i];
			auto FoundIndex = OldThisParameterList.IndexOfByPredicate([&](const FLGUIPrefabOverrideParameterData& Item) {
				return 
					Item.Guid == TemplateItem.Guid//if guid equal then they are the same property
					&& Item.ParamType == TemplateItem.ParamType;//but if property's value type change then consider it as new
				});
			if (FoundIndex != INDEX_NONE)
			{
				auto Item = OldThisParameterList[FoundIndex];
#if WITH_EDITORONLY_DATA
				Item.DisplayName = TemplateItem.DisplayName;
#endif
				this->ParameterList.Add(Item);
			}
			else
			{
				this->ParameterList.Add(TemplateItem);
			}
		}
		return true;
	}
	return false;
}

void FLGUIPrefabOverrideParameter::SetListItemDefaultNameWhenAddNewToList()
{
	if (bIsTemplate)
	{
		int index = 0;
		for (auto& item : ParameterList)
		{
			if (item.DisplayName.IsEmpty())
			{
				item.DisplayName = FString::Printf(TEXT("Property_%d"), index);
			}
			index++;
		}
	}
}
bool FLGUIPrefabOverrideParameter::HasRepeatedParameter()
{
	for (int i = 0; i < ParameterList.Num() - 1; i++)
	{
		auto& A = ParameterList[i];
		for (int j = i + 1; j < ParameterList.Num(); j++)
		{
			auto& B = ParameterList[j];
			if (A.IsReferenceParameterEqual(B))
			{
				return true;
			}
		}
	}
	return false;
}

void ULGUIPrefabOverrideParameterObject::ApplyParameter()
{
	Parameter.ApplyParameter();
}

void ULGUIPrefabOverrideParameterObject::SaveDefaultValue()
{
	Parameter.SaveDefaultValue();
}

void ULGUIPrefabOverrideParameterObject::SetParameterDisplayType(bool InIsTemplate)
{
	Parameter.bIsTemplate = InIsTemplate;
}
void ULGUIPrefabOverrideParameterObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	Parameter.SetListItemDefaultNameWhenAddNewToList();
}

void ULGUIPrefabOverrideParameterObject::SetParameterReferenceFromTemplate(ULGUIPrefabOverrideParameterObject* InTemplate)
{
	Parameter.SetParameterReferenceFromTemplate(InTemplate->Parameter);
}
bool ULGUIPrefabOverrideParameterObject::RefreshParameterOnTemplate(ULGUIPrefabOverrideParameterObject* InTemplate)
{
	return Parameter.RefreshParameterOnTemplate(InTemplate->Parameter);
}

bool ULGUIPrefabOverrideParameterObject::HasRepeatedParameter()
{
	return Parameter.HasRepeatedParameter();
}

int32 ULGUIPrefabOverrideParameterObject::GetParameterCount()const
{
	return Parameter.ParameterList.Num();
}

PRAGMA_ENABLE_OPTIMIZATION
