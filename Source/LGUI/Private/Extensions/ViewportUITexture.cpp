// Copyright 2019 LexLiu. All Rights Reserved.

#include "Extensions/ViewportUITexture.h"
#include "Event/Rayemitter/LGUI_SceneCapture2DMouseRayemitter.h"
#include "Engine/TextureRenderTarget2D.h"

UViewportUITexture::UViewportUITexture()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UViewportUITexture::BeginPlay()
{
	Super::BeginPlay();	
	CheckSize();
}

void UViewportUITexture::WidthChanged()
{
	CheckSize();
}
void UViewportUITexture::HeightChanged()
{
	CheckSize();
}
void UViewportUITexture::CheckSize()
{
	if (UTextureRenderTarget2D* rt = Cast<UTextureRenderTarget2D>(texture))
	{
		rt->SizeX = FMath::Clamp(widget.width, 2.0f, (float)GetMax2DTextureDimension());
		rt->SizeY = FMath::Clamp(widget.height, 2.0f, (float)GetMax2DTextureDimension());
		rt->UpdateResource();
	}
}


AViewportUITextureActor::AViewportUITextureActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UITextureComponent = CreateDefaultSubobject<UViewportUITexture>(TEXT("UITextureComponent"));
	RootComponent = UITextureComponent;
}

void UViewportUITexture::ViewPointToWorld(const FVector2D& InViewPoint, FVector& OutWorldLocation, FVector& OutWorldDirection)const
{
	if (SceneCaptureActor == nullptr)return;
	auto viewPoint01 = InViewPoint;
	viewPoint01.X /= widget.width;
	viewPoint01.Y /= widget.height;
	ULGUI_SceneCapture2DMouseRayEmitter::DeprojectViewPointToWorldForSceneCapture2D(SceneCaptureActor->GetCaptureComponent2D(), viewPoint01, OutWorldLocation, OutWorldDirection);
}
bool UViewportUITexture::WorldToViewPoint(const FVector& InWorldLocation, FVector2D& OutViewPoint)const
{
	if (SceneCaptureActor == nullptr)return false;
	if (ULGUI_SceneCapture2DMouseRayEmitter::ProjectWorldToViewPointForSceneCapture2D(SceneCaptureActor->GetCaptureComponent2D(), InWorldLocation, OutViewPoint))
	{
		OutViewPoint.X *= widget.width;
		OutViewPoint.Y *= widget.height;
		return true;
	}
	return false;
}