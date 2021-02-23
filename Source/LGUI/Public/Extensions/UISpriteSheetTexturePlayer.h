// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once
#include "LGUIImageSequencePlayer.h"
#include "UISpriteSheetTexturePlayer.generated.h"

class ULGUISpriteData;

UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent))
class LGUI_API UUISpriteSheetTexturePlayer : public ULGUIImageSequencePlayer
{
	GENERATED_BODY()
protected:
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
};
