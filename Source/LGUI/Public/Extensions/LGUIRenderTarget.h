// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIComponentReference.h"
#include "PrefabSystem/ILGUIPrefabInterface.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "Core/Actor/UIBaseActor.h"
#include "LGUIRenderTargetInteraction.h"
#include "LGUIRenderTarget.generated.h"

class ULGUICanvas;

/**
 * LGUI Render Target provide a solution to display a LGUICanvas with RenderMode of RenderTarget, just like "Retainer Box", and interact it with LGUIRenderTargetInteraction component.
 */
UCLASS(ClassGroup = LGUI, Blueprintable, meta = (BlueprintSpawnableComponent), hidecategories = (Object, Activation, "Components|Activation"))
class LGUI_API ULGUIRenderTarget : public UUIBatchGeometryRenderable, public ILGUIPrefabInterface, public ILGUIRenderTargetInteractionSourceInterface
{
	GENERATED_BODY()
	
public:	
	ULGUIRenderTarget(const FObjectInitializer& ObjectInitializer);
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

	// Begin ILGUIRenderTargetInteractionSourceInterface
	virtual ULGUICanvas* GetTargetCanvas_Implementation()const override;
	virtual bool PerformLineTrace_Implementation(const int32& InHitFaceIndex, const FVector& InHitPoint, const FVector& InLineStart, const FVector& InLineEnd, FVector2D& OutHitUV)override;
	// End ILGUIRenderTargetInteractionSourceInterface
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
 * LGUI Render Target provide a solution to display a LGUICanvas with RenderMode of RenderTarget, just like "Retainer Box", and interact it with LGUIRenderTargetInteraction component.
 */
UCLASS(ClassGroup = LGUI)
class LGUI_API ALGUIRenderTargetActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()

public:
	ALGUIRenderTargetActor();

	virtual UUIItem* GetUIItem()const override { return LGUIRenderTarget; }
	virtual class UUIBaseRenderable* GetUIRenderable()const override { return LGUIRenderTarget; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	ULGUIRenderTarget* Get2DLineRing()const { return LGUIRenderTarget; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULGUIRenderTarget> LGUIRenderTarget;

};
