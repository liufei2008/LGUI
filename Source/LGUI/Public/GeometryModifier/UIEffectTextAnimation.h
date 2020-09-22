// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "UIGeometryModifierBase.h"
#include "UIEffectTextAnimation.generated.h"

USTRUCT(BlueprintType)
struct FUIEffectTextAnimation_SelectResult
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere) 
		float lerpValue;
};

UCLASS(ClassGroup = (LGUI), Abstract, BlueprintType, DefaultToInstanced, EditInlineNew)
class LGUI_API UUIEffectTextAnimation_Selector : public UObject
{
	GENERATED_BODY()
public:
	virtual bool Select(class UUIText* InUIText, TArray<FUIEffectTextAnimation_SelectResult>& OutSelection) PURE_VIRTUAL(UUIEffectTextAnimation_Selector::Select, return false;);
protected:
	class UUIText* GetUIText();
};

UCLASS(ClassGroup = (LGUI), Abstract, BlueprintType, DefaultToInstanced, EditInlineNew)
class LGUI_API UUIEffectTextAnimation_Property : public UObject
{
	GENERATED_BODY()
public:
	virtual void ApplyProperty(class UUIText* InUIText, const TArray<FUIEffectTextAnimation_SelectResult>& InSelection, TSharedPtr<UIGeometry> OutGeometry) PURE_VIRTUAL(UUIEffectTextAnimation_Property::ApplyEffect, );
protected:
	class UUIText* GetUIText();
};

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
	TArray<FUIEffectTextAnimation_SelectResult> selection;
	bool CheckUIText();
public:
	virtual void ModifyUIGeometry(TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged)override;
	class UUIText* GetUIText();

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIEffectTextAnimation_Selector* GetSelector()const { return selector; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const TArray<UUIEffectTextAnimation_Property*> GetProperties()const { return properties; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIEffectTextAnimation_Property* GetProperty(int index)const;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSelector(UUIEffectTextAnimation_Selector* value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetProperties(const TArray<UUIEffectTextAnimation_Property*>& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetProperty(int index, UUIEffectTextAnimation_Property* value);
};
