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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LexUI")
	float AnimDuration = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LexUI")
	TWeakObjectPtr<ULexCanvas> ThisCanvas = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LexUI", AdvancedDisplay)
	TWeakObjectPtr<ULexWidget> CurrentSelected = nullptr;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "SelectWidget"), Category = "LexUI")
	void ReceiveSelectWidget(ULexWidget* InSelected);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "SelectNone"), Category = "LexUI")
	void ReceiveSelectNone();

	UPROPERTY(VisibleAnywhere, Category = "LexUI", AdvancedDisplay)
	TArray<TWeakObjectPtr<ULTweener>> TweenerCollection;

	/** True after SelectNone() is called and before the widget is actually destroyed. Prevents re-entry. */
	bool bIsDestroyPending = false;
public:
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	virtual void SelectWidget(ULexWidget* InSelected);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	virtual void SelectNone();
};
