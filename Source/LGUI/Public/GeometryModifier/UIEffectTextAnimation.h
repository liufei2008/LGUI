// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIGeometryModifierBase.h"
#include "LTweener.h"
#include "UIEffectTextAnimation.generated.h"

struct FUIEffectTextAnimation_SelectResult
{
public:
	//start index
	int startCharIndex = 0;
	//end index + 1
	int endCharCount = 0;
	TArray<float> lerpValueArray;
};

UCLASS(ClassGroup = (LGUI), Abstract, BlueprintType, DefaultToInstanced, EditInlineNew)
class LGUI_API UUIEffectTextAnimation_Selector : public UObject
{
	GENERATED_BODY()
protected:
	//editor clamp this value between 0-1, but you can assign it more than 0-1 for PositionWave/RotationWave/ScaleWave/XXXWave
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float offset = 0.5f;
	class UUIText* GetUIText();
public:
	virtual bool Select(class UUIText* InUIText, FUIEffectTextAnimation_SelectResult& OutSelection) PURE_VIRTUAL(UUIEffectTextAnimation_Selector::Select, return false;);
	
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetOffset()const { return offset; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOffset(float value);
};

UCLASS(ClassGroup = (LGUI), Abstract, BlueprintType, DefaultToInstanced, EditInlineNew)
class LGUI_API UUIEffectTextAnimation_Property : public UObject
{
	GENERATED_BODY()
protected:
	class UUIText* GetUIText();
	FORCEINLINE void MarkUITextPositionDirty();
public:
	virtual void Init() {};
	virtual void Deinit() {};
	virtual void ApplyProperty(class UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, TSharedPtr<UIGeometry> OutGeometry) PURE_VIRTUAL(UUIEffectTextAnimation_Property::ApplyEffect, );
};

//per character animation control for UIText
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIEffectTextAnimation : public UUIGeometryModifierBase
{
	GENERATED_BODY()

public:	
	UUIEffectTextAnimation();
protected:
	UPROPERTY(EditAnywhere, Category = "LGUI", Instanced)
		UUIEffectTextAnimation_Selector* selector;
	UPROPERTY(EditAnywhere, Category = "LGUI", Instanced)
		TArray<UUIEffectTextAnimation_Property*> properties;

	UPROPERTY(Transient)class UUIText* uiText;
	FUIEffectTextAnimation_SelectResult selection;
	bool CheckUIText();
	virtual void BeginPlay()override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
public:
	virtual void ModifyUIGeometry(
		TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged,
		bool uvChanged, bool colorChanged, bool vertexPositionChanged, bool layoutChanged
		)override;
	class UUIText* GetUIText();

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIEffectTextAnimation_Selector* GetSelector()const { return selector; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const TArray<UUIEffectTextAnimation_Property*> GetProperties()const { return properties; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIEffectTextAnimation_Property* GetProperty(int index)const;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetSelectorOffset()const;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSelector(UUIEffectTextAnimation_Selector* value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetProperties(const TArray<UUIEffectTextAnimation_Property*>& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetProperty(int index, UUIEffectTextAnimation_Property* value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSelectorOffset(float value);
};
