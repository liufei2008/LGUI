// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LexWidgetPresenterComponentBase.generated.h"

class ULexWidget;
class UUINavigationInputSelectionHandler;
class ULexCanvas;
class ULexUIPrefab;

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API ULexWidgetPresenterComponentBase : public USceneComponent
{
	GENERATED_BODY()

public:
	ULexWidgetPresenterComponentBase();
	friend class FLexWidgetPresenterBaseCustomization;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostInitProperties() override;
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None) override;
	virtual void LoadWidget()PURE_VIRTUAL(ULexWidgetPresenterComponentBase::LoadWidget, );
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	static bool bNeedCheckEventSystem;
	static bool bNeverCheckEventSystem;
	static bool bNeedCheckRaycasterSource;
	static bool bNeverCheckRaycasterSource;
public:
	void CheckNecessaryObjects();
	static void MarkNeedCheckNecessaryObjects();
	
	void ReloadWidget();
#endif
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=LexWidgetPresenter)
	TObjectPtr<ULexCanvas> CanvasTemplate;
	
	UPROPERTY(Transient)
	TWeakObjectPtr<ULexCanvas> RootCanvas;
	UPROPERTY(Transient)
	TWeakObjectPtr<ULexWidget> LoadedWidget;
	/**
	 * For navigation input, show a selection widget
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=LexWidgetPresenter)
	TObjectPtr<ULexUIPrefab> NavigationSelectionPrefab;

	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category=LexWidgetPresenter, AdvancedDisplay)
	TWeakObjectPtr<UUINavigationInputSelectionHandler> NavigationSelection;

public:
	UFUNCTION(BlueprintCallable, Category=LGUI)
	UUINavigationInputSelectionHandler* GetNavigationSelection();
	UFUNCTION(BlueprintCallable, Category=LGUI)
	ULexCanvas* GetLoadedCanvas()const{return RootCanvas.Get();}
	UFUNCTION(BlueprintCallable, Category=LGUI)
	ULexWidget* GetLoadedWidget()const{return LoadedWidget.Get();}
};
