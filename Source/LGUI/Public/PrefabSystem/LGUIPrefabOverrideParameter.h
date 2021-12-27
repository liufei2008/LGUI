// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIPrefabOverrideParameter.generated.h"


UENUM()
enum class ELGUIPrefabOverrideParameterType :uint8
{
	Bool		UMETA(DisplayName = "Boolean"),
	Float,
	Double		UMETA(Hidden),
	Int8		UMETA(Hidden),
	UInt8		UMETA(DisplayName = "UInt8\Enum\Byte"),
	Int16		UMETA(Hidden),
	UInt16		UMETA(Hidden),
	Int32		UMETA(DisplayName = "Integer"),
	UInt32		UMETA(Hidden),
	Int64		UMETA(Hidden),
	UInt64		UMETA(Hidden),
	Vector2		UMETA(DisplayName = "Vector2"),
	Vector3		UMETA(DisplayName = "Vector3"),
	Vector4		UMETA(DisplayName = "Vector4"),
	Color,
	LinearColor,
	Quaternion,
	String,	
	Rotator,
	Name,
	Text,

	/** for asset reference */
	Object,
	/** for actor reference in level */
	Actor,
	/** for UClass reference */
	Class,

	/** for structs that only store data, not include any uobjects */
	DataStruct,
};
/** helper class for finding function */
class LGUI_API ULGUIPrefabOverrideParameterHelper
{
public:
	static bool IsSupportedProperty(const FProperty* Target, bool IncludeDataStruct, ELGUIPrefabOverrideParameterType& OutParamType);
	static bool IsStillSupported(const FProperty* Target, bool IncludeDataStruct, ELGUIPrefabOverrideParameterType InParamType);
	static FString ParameterTypeToName(ELGUIPrefabOverrideParameterType paramType, const FProperty* InProperty = nullptr);
	/** if first parameter is an object type, then return it's objectclass */
	static UClass* GetObjectParameterClass(const FProperty* InProperty);
	static UEnum* GetEnumParameter(const FProperty* InProperty);
	static UClass* GetClassParameterClass(const FProperty* InProperty);
private:
};

#define LGUIActorSelfName "(ActorSelf)"

USTRUCT(NotBlueprintType)
struct LGUI_API FLGUIPrefabOverrideParameterData
{
	GENERATED_BODY()
public:
	FLGUIPrefabOverrideParameterData();

private:
	friend class FLGUIPrefabOverrideParameterCustomization;
	friend struct FLGUIPrefabOverrideParameter;
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")bool BoolValue = false;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")float FloatValue = 0;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")double DoubleValue = 0;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")int8 Int8Value = 0;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")uint8 UInt8Value = 0;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")int16 Int16Value = 0;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")uint16 UInt16Value = 0;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")int32 Int32Value = 0;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")uint32 UInt32Value = 0;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")int64 Int64Value = 0;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")uint64 UInt64Value = 0;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")FVector2D Vector2Value = FVector2D::ZeroVector;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")FVector Vector3Value = FVector::ZeroVector;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")FVector4 Vector4Value = FVector4(0, 0, 0, 0);
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")FQuat QuatValue = FQuat::Identity;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")FColor ColorValue = FColor::White;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")FLinearColor LinearColorValue = FLinearColor::White;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")FRotator RotatorValue = FRotator::ZeroRotator;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")FString StringValue = TEXT("");
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")FName NameValue = NAME_None;
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")FText TextValue;
#endif
#if WITH_EDITORONLY_DATA
	/** Editor helper actor, for direct reference actor */
	UPROPERTY(EditAnywhere, Category = "LGUI", DisplayName = "Target")
		TWeakObjectPtr<AActor> HelperActor = nullptr;
	/** Editor helper, target object class. If class is actor then TargetObject is HelperActor, if class is ActorComponent then TargetObject is the component. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI-old")
		UClass* HelperClass = nullptr;
	/** Editor helper, if TargetObject is actor component and HelperActor have multiple components, then select by component name. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI-old")
		FName HelperComponentName;
#endif
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TWeakObjectPtr<UObject> TargetObject;
	/** target function name */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FName PropertyName;
	/** target function supported parameter type */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUIPrefabOverrideParameterType ParamType = ELGUIPrefabOverrideParameterType::Bool;

	/** data buffer stores parameter's actural data */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<uint8> ParamBuffer;
	/** Object reference, can reference actor/class/asset */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UObject* ReferenceObject = nullptr;

#if WITH_EDITORONLY_DATA
	/** Property's friendly name to display for outer prefab */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FString DisplayName;
	/** Use Guid as unique key, so we can find the same property. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		FGuid Guid;
	//@todo: add a bool, for store if the property's value is changed
#endif
public:
	bool ApplyParameter();
	/** Check Actorserializer3_Deserialize */
	void SetParameterReferenceFromTemplate(const FLGUIPrefabOverrideParameterData& InTemplate);
	/** Save current value as default */
	void SaveCurrentValueAsDefault();
	/** Check if reference parameter equal, not include value */
	bool IsReferenceParameterEqual(const FLGUIPrefabOverrideParameterData& Other)const;
	bool IsParameter_Type_Name_Guid_DisplayName_Equal(const FLGUIPrefabOverrideParameterData& Other)const;
private:
	/** Apply value to property from stored data */
	bool ApplyPropertyParameter(UObject* InTarget, FProperty* InProperty);
	/** Store parameter value from property */
	bool SavePropertyParameter(UObject* InTarget, FProperty* InProperty);
	bool CheckDataType(ELGUIPrefabOverrideParameterType PropertyType);
	bool CheckDataType(ELGUIPrefabOverrideParameterType PropertyType, FProperty* InProperty);
};

/**
 * Sub prefab's override parameter
 */
USTRUCT(NotBlueprintType)
struct LGUI_API FLGUIPrefabOverrideParameter
{
	GENERATED_BODY()

public:
	FLGUIPrefabOverrideParameter();
	~FLGUIPrefabOverrideParameter();
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, EditAnywhere, Category = "LGUI")
		bool bIsTemplate = true;
private:
	static TArray<FLGUIPrefabOverrideParameter*> AllLGUIPrefabOverrideParameterArray;
#endif
#if WITH_EDITOR
	/** if TargetObject is blueprint ActorComponent, when hit compile, TargetObject will become null, so get it again */
	void RefreshOnBlueprintCompiled();
public:
	static void RefreshAll_OnBlueprintCompiled();
#endif
private:
	friend class FLGUIPrefabOverrideParameterCustomization;
	friend class ULGUIPrefabOverrideParameterObject;
	/** parameter list */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<FLGUIPrefabOverrideParameterData> ParameterList;
public:
	bool ApplyParameter();
	void SetParameterReferenceFromTemplate(const FLGUIPrefabOverrideParameter& InTemplate);
#if WITH_EDITOR
	/** @return	true if anything change, false nothing change */
	bool RefreshParameterOnTemplate(const FLGUIPrefabOverrideParameter& InTemplate);
	bool RefreshAutomaticParameter();
	void SaveCurrentValueAsDefault();
	void SetListItemDefaultNameWhenAddNewToList();
	bool HasRepeatedParameter();

	/** Add parameter to list or update parameter's value */
	void AddOrUpdateParameter(AActor* InActor, UObject* InObject, FProperty* InProperty, ELGUIPrefabOverrideParameterType InParamType);
#endif
};

UCLASS(NotBlueprintType)
class LGUI_API ULGUIPrefabOverrideParameterObject : public UObject
{
	GENERATED_BODY()
public:
	ULGUIPrefabOverrideParameterObject() {}
private:
	friend class FLGUIPrefabOverrideParameterCustomization;
	/** override parameter that set manually */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FLGUIPrefabOverrideParameter Parameter;
	/** override parameter that set automatically */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FLGUIPrefabOverrideParameter AutomaticParameter;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:
	/** Apply recored data to property value */
	void ApplyParameter();
	/** Check Actorserializer3_Deserialize */
	void SetParameterReferenceFromTemplate(ULGUIPrefabOverrideParameterObject* InTemplate);
	virtual void BeginDestroy()override;
#if WITH_EDITOR
	/**
	 * For sub prefab, refresh parameters by compare with template.
	 * @param	InTemplate	the sub prefab's origin OverrideParameterObject
	 */
	bool RefreshParameterOnTemplate(ULGUIPrefabOverrideParameterObject* InTemplate);
	/** Refresh automatic parameters, check if parameter still exist, if not then remove it. */
	bool RefreshAutomaticParameter();
	/** Set display type, template or editable instance */
	void SetParameterDisplayType(bool InIsTemplate);
	/** For template, when select property in editor, it will get the property's value and sotre it as default value. */
	void SaveCurrentValueAsDefault();
	bool HasRepeatedParameter();
	int32 GetParameterCount()const;
	bool GetIsIsTemplate()const { return Parameter.bIsTemplate; }
	/** Add or update parameter to AutomaticParameter, will record value aswell */
	void AddOrUpdateParameterToAutomaticParameters(AActor* InActor, UObject* InObject, FProperty* InProperty, ELGUIPrefabOverrideParameterType InParamType);
#endif
};
