// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/LexUIBehaviour.h"
#include "UINavigationInputSelectionHandler.generated.h"


class ULexCanvas;
class ULTweener;

UCLASS(ClassGroup=(LGUI), Blueprintable, meta=(BlueprintSpawnableComponent))
class LGUI_API UUINavigationInputSelectionHandler : public ULexUIBehaviour
{
	GENERATED_BODY()
public:
	UUINavigationInputSelectionHandler();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
	float AnimDuration = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LGUI")
	TWeakObjectPtr<ULexCanvas> ThisCanvas = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LGUI", AdvancedDisplay)
	TWeakObjectPtr<ULexWidget> CurrentSelected = nullptr;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "SelectWidget"), Category = "LGUI")
	void ReceiveSelectWidget(ULexWidget* InSelected);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "SelectNone"), Category = "LGUI")
	void ReceiveSelectNone();

	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
	TArray<TWeakObjectPtr<ULTweener>> TweenerCollection;

	/** True after SelectNone() is called and before the widget is actually destroyed. Prevents re-entry. */
	bool bIsDestroyPending = false;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	virtual void SelectWidget(ULexWidget* InSelected);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	virtual void SelectNone();
};
