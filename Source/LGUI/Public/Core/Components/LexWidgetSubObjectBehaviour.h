// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "LexWidgetSubObjectBehaviour.generated.h"

class ULexWidget;
UCLASS(BlueprintType, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API ULexWidgetSubObjectBehaviour : public UObject
{
	GENERATED_BODY()

public:
	void Call_OnRegister();
	void Call_OnUnregister();
	virtual void BeginPlay(){};
	virtual void EndPlay(){};
	bool IsRegistered()const{return bIsRegistered;}

	virtual void PostInitProperties() override;

	UFUNCTION()
	ULexWidget* GetWidget()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
	FString GetPathDisplayName(const UObject* StopOuter = nullptr) const;
protected:
	virtual void OnRegister(){};
	virtual void OnUnregister(){};
private:
	bool bIsRegistered = false;
	UPROPERTY(Transient, BlueprintReadOnly, Category = LGUI, Getter=GetWidget, meta = (AllowPrivateAccess = true), DisplayName=Widget)
	mutable TObjectPtr<ULexWidget> OwnerWidget = nullptr;
};
