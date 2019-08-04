// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "LGUI.h"
#include "CoreMinimal.h"
#include "Engine/SceneCapture2D.h"
#include "Core/ActorComponent/UITexture.h"
#include "Core/Actor/UIBaseActor.h"
#include "ViewportUITexture.generated.h"


UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UViewportUITexture : public UUITexture
{
	GENERATED_BODY()

public:	
	UViewportUITexture();

protected:
	virtual void BeginPlay() override;

	virtual void WidthChanged()override;
	virtual void HeightChanged()override;
	void CheckSize();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "LGUI")
		ASceneCapture2D* SceneCaptureActor;
public:
	/*
	 Convert viewpoint position to world ray, useful for linecast
	 InViewPoint: SceneCapture's RenderTarget2D left bottom is (0,0), right top is (width, height)
	*/
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void ViewPointToWorld(const FVector2D& InViewPoint, FVector& OutWorldLocation, FVector& OutWorldDirection)const;
	/*
	 Convert world point to viewpoint position
	 OutViewPoint: SceneCapture's RenderTarget2D left bottom is (0,0), right top is (width, height)
	*/
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool WorldToViewPoint(const FVector& InWorldLocation, FVector2D& OutViewPoint)const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
	USceneCaptureComponent2D* GetSceneCapture()const{return SceneCaptureActor->GetCaptureComponent2D();}
};

UCLASS()
class LGUI_API AViewportUITextureActor : public AUIBaseActor
{
	GENERATED_BODY()

public:
	AViewportUITextureActor();

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UViewportUITexture* GetUITexture() { return UITextureComponent; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UViewportUITexture* UITextureComponent;

};
