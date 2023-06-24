// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LGUIImageSequencePlayer.h"
#include "UISpriteSequencePlayer.generated.h"

class ULGUISpriteData_BaseObject;

/** Play sprite sequence, need UISprite component. */
UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent))
class LGUI_API UUISpriteSequencePlayer : public ULGUIImageSequencePlayer
{
	GENERATED_BODY()
protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
#endif
	UPROPERTY(Transient)
		TWeakObjectPtr<class UUISpriteBase> sprite;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<TObjectPtr<ULGUISpriteData_BaseObject>> spriteSequence;
	/** should also set size to sprite-data? */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool snapSpriteSize = true;

	virtual bool CanPlay()override;
	virtual float GetDuration()const override;
	virtual void PrepareForPlay()override;
	virtual void OnUpdateAnimation(int frameNumber)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const TArray<ULGUISpriteData_BaseObject*> GetSpriteSequence()const { return spriteSequence; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetSnapSpriteSize()const { return snapSpriteSize; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSpriteSequence(TArray<ULGUISpriteData_BaseObject*> value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSnapSpriteSize(bool value);
};
