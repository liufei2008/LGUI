// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "LGUIImageSequencePlayer.h"
#include "UISpriteSheetTexturePlayer.generated.h"

class ULGUISpriteData;

/** Play spritesheet texture, need UITexture component. */
UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent))
class LGUI_API UUISpriteSheetTexturePlayer : public ULGUIImageSequencePlayer
{
	GENERATED_BODY()
protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
#endif
#if WITH_EDITORONLY_DATA
	/** preview in editor. -1 means no preview, just show the origin texture. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (UIMin = "0.0", UIMax = "1.0"))
		float preview = -1;
#endif
	UPROPERTY(Transient)
		TWeakObjectPtr<class UUITexture> texture;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int widthCount = 8;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int heightCount = 8;

	float widthUVInterval, heightUVInterval;
	virtual bool CanPlay()override;
	virtual float GetDuration()const override;
	virtual void PrepareForPlay()override;
	virtual void OnUpdateAnimation(int frameNumber)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetWidthCount()const { return widthCount; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetHeightCount()const { return heightCount; }
	/** Will take effect on nexe cycle. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetWidthCount(int value);
	/** Will take effect on nexe cycle. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetHeightCount(int value);
};
