// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/LexWidgetPresenterComponentBase.h"

#include "LexUIMLPresenterComponent.generated.h"

class ULexUIMLBehaviour;
class ULexUIMLResource;

UCLASS(ClassGroup=(Custom), Experimental, meta=(BlueprintSpawnableComponent), DisplayName="LexUI XAML Presenter Component")
class LGUI_API ULexUIMLPresenterComponent : public ULexWidgetPresenterComponentBase
{
	GENERATED_BODY()

public:
	ULexUIMLPresenterComponent();

protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void LoadWidget() override;
#if WITH_EDITOR
	void RegisterEditorFocusCheck();
	void UnregisterEditorFocusCheck();
	FDelegateHandle EditorFocusHandle;
	FString CachedFileMD5;
	/** Re-check XAML file MD5 and reload if changed. Called on editor focus. */
	void CheckAndReloadWidget();
public:
	bool bIsSpawnFromFactory = false;
#endif
	FString GetXAMLFilePath() const;

protected:
	UPROPERTY(EditAnywhere, Category=LexWidgetPresenter)
	TSubclassOf<ULexUIMLBehaviour> XAMLScriptClass;

public:
	UFUNCTION(BlueprintCallable, Category=LGUI)
	void SetScriptClass(TSubclassOf<ULexUIMLBehaviour> Value);
	UFUNCTION(BlueprintCallable, Category=LGUI)
	TSubclassOf<ULexUIMLBehaviour> GetScriptClass()const{return XAMLScriptClass;}
};
