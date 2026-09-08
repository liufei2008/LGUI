// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/LexWidgetPresenterComponent.h"
#include "LexUIPrefabPresenterComponent.generated.h"

class ULexWidget;
class UUINavigationInputSelectionHandler;
class ULexCanvas;
class ULexUIPrefab;

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent), DisplayName="LexUI Prefab Presenter Component")
class LGUI_API ULexUIPrefabPresenterComponent : public ULexWidgetPresenterComponent
{
	GENERATED_BODY()

public:
	ULexUIPrefabPresenterComponent();
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void LoadWidget() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
public:
	bool bIsSpawnFromFactory = false;
	void CheckPrefabVersion();
#endif
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=LexWidgetPresenter)
	TObjectPtr<ULexUIPrefab> WidgetPrefab;
	
#if WITH_EDITORONLY_DATA
private:
	UPROPERTY()
	FString OverallVersionMD5;
#endif
public:
	UFUNCTION(BlueprintCallable, Category=LGUI)
	void SetPrefab(ULexUIPrefab* Value);
	UFUNCTION(BlueprintCallable, Category=LGUI)
	ULexUIPrefab* GetPrefab()const{return WidgetPrefab;}
};
