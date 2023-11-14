// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIComponentReference.h"
#include "PrefabSystem/ILGUIPrefabInterface.h"
#include "Core/ActorComponent/UICustomMesh.h"
#include "Core/Actor/UIBaseActor.h"
#include "LGUIRenderTargetInteraction.h"
#include "UIRenderTarget.generated.h"

class ULGUICanvas;
class ULGUICustomMesh;

/**
 * LGUI Render Target provide a solution to display a LGUICanvas with RenderMode of RenderTarget, just like "Retainer Box", and interact it with UIRenderTargetInteraction component.
 */
UCLASS(ClassGroup = LGUI, Blueprintable, meta = (BlueprintSpawnableComponent), hidecategories = (Object, Activation, "Components|Activation"))
class LGUI_API UUIRenderTarget : public UUICustomMesh, public ILGUIPrefabInterface, public ILGUIRenderTargetInteractionSourceInterface
{
	GENERATED_BODY()
	
public:	
	UUIRenderTarget(const FObjectInitializer& ObjectInitializer);
protected:
	virtual void OnBeforeCreateOrUpdateGeometry()override;
	virtual UTexture* GetTextureToCreateGeometry()override;
	virtual void OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	virtual void BeginPlay()override;
	virtual void EndPlay(EEndPlayReason::Type Reason)override;

	UPROPERTY(EditAnywhere, Category = LGUI)
		FLGUIComponentReference TargetCanvas;

	mutable TWeakObjectPtr<class ULGUICanvas> TargetCanvasObject = nullptr;
public:
	// Begin ILGUIPrefabInterface
	virtual void Awake_Implementation()override;
	// End ILGUIPrefabInterface

	// Begin IUIRenderTargetInteractionSourceInterface
	virtual ULGUICanvas* GetTargetCanvas_Implementation()const override;
	virtual bool PerformLineTrace_Implementation(const int32& InHitFaceIndex, const FVector& InHitPoint, const FVector& InLineStart, const FVector& InLineEnd, FVector2D& OutHitUV)override;
	// End IUIRenderTargetInteractionSourceInterface
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	UFUNCTION(BlueprintCallable, Category = LGUI)
	ULGUICanvas* GetCanvas()const;

	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetCanvas(ULGUICanvas* Value);
private:
};

/**
 * LGUI Render Target provide a solution to display a LGUICanvas with RenderMode of RenderTarget, just like "Retainer Box", and interact it with UIRenderTargetInteraction component.
 */
UCLASS(ClassGroup = LGUI)
class LGUI_API AUIRenderTargetActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()

public:
	AUIRenderTargetActor();

	virtual UUIItem* GetUIItem()const override { return UIRenderTarget; }
	virtual class UUIBaseRenderable* GetUIRenderable()const override { return UIRenderTarget; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	UUIRenderTarget* Get2DLineRing()const { return UIRenderTarget; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UUIRenderTarget> UIRenderTarget;

};
