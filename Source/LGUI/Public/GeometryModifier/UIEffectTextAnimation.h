// Copyright 2019-Present LexLiu. All Rights Reserved.

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
	/** 
	 * 0 means *Properties* will have no effect, 1 means *Properties* have full effect, and middle value is interplation.
	 * So we can set this "offset" property to make animation.
	 */
	UPROPERTY(EditAnywhere, Category = "Property", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float offset = 0.5f;
	class UUIText* GetUIText()const;
	class UUIEffectTextAnimation* GetUIEffectTextAnimation()const;
private:
	mutable TWeakObjectPtr<class UUIEffectTextAnimation> UIEffectTextAnimation = nullptr;
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
	void MarkUITextPositionDirty();
public:
	virtual void Init() {};
	virtual void Deinit() {};
	virtual void ApplyProperty(class UUIText* InUIText, const FUIEffectTextAnimation_SelectResult& InSelection, UIGeometry* InGeometry) PURE_VIRTUAL(UUIEffectTextAnimation_Property::ApplyEffect, );
};

//per character animation control for UIText
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIEffectTextAnimation : public UUIGeometryModifierBase
{
	GENERATED_BODY()

public:	
	UUIEffectTextAnimation();
protected:
	/** Selector defines the method to select characters in text */
	UPROPERTY(EditAnywhere, Category = "LGUI", Instanced)
		TObjectPtr<UUIEffectTextAnimation_Selector> selector;
	/** Properties defines which property will affect and how it affect */
	UPROPERTY(EditAnywhere, Category = "LGUI", Instanced)
		TArray<TObjectPtr<UUIEffectTextAnimation_Property>> properties;
	/** This is just a agent to selector's offset property, for Sequencer access it. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		mutable float selectorOffset = 0.0f;

	UPROPERTY(Transient)TObjectPtr<class UUIText> uiText;
	FUIEffectTextAnimation_SelectResult selection;
	bool CheckUIText();
	virtual void BeginPlay()override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
#endif
public:
	virtual void ModifyUIGeometry(UIGeometry& InGeometry
		, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
	)override;
	class UUIText* GetUIText();

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIEffectTextAnimation_Selector* GetSelector()const { return selector; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const TArray<UUIEffectTextAnimation_Property*>& GetProperties()const { return properties; }
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
