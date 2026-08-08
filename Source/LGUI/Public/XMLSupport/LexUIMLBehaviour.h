// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/LexUIBehaviour.h"
#include "LexUIMLBehaviour.generated.h"

enum class ELexRenderMode : uint8;
class ULexUIMLResource;
/**
 * 
 */
UCLASS(Abstract, Experimental)
class LGUI_API ULexUIMLBehaviour : public ULexUIBehaviour
{
	GENERATED_BODY()
public:
	ULexUIMLBehaviour();
	
	void GetUIMLData(FString& XAMLFilePath, ULexUIMLResource*& XAMLResource)const;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category=LexWidgetPresenter)
	ELexRenderMode DefaultRenderMode;
#endif
	
protected:
	/**
	 * Return the absolute path of the XAML file.
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, meta=(DisplayName="GetUIMLData"))
	void ReceiveGetUIMLData(FString& XAMLFilePath, ULexUIMLResource*& XAMLResource)const;

public:
	static ULexUIMLBehaviour* CreateByClass(
		TSubclassOf<ULexUIMLBehaviour> Class, UWorld* World, ULexWidget* Parent
		, ULexUIMLResource* Resources, bool IsSubTemplate
		, const TFunction<void(const TArray<ULexWidget*>&)>& OnAllWidgetsCreated);
};
