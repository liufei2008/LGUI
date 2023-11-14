// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PrefabSystem/ILGUIPrefabInterface.h"
#include "Core/ActorComponent/UICustomMesh.h"
#include "Components/WidgetComponent.h"
#include "Core/Actor/UIBaseActor.h"
#include "UIWidget.generated.h"

class ULGUICustomMesh;

/**
 * LGUI Widget can render a UMG widget as LGUI's element, and interact with it by UIWidgetInteraction component.
 */
UCLASS(ClassGroup = (LGUI), NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIWidget : public UUICustomMesh, public ILGUIPrefabInterface
{
	GENERATED_BODY()
	
public:	
	UUIWidget(const FObjectInitializer& ObjectInitializer);
protected:
	virtual void OnBeforeCreateOrUpdateGeometry()override;
	virtual UTexture* GetTextureToCreateGeometry()override;
	virtual void OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	virtual void BeginPlay() override;
	// Begin ILGUIPrefabInterface
	virtual void Awake_Implementation()override;
	// End ILGUIPrefabInterface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void DestroyComponent(bool bPromoteChildren = false) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:
	/** Ensures the user widget is initialized */
	virtual void InitWidget();

	/** Release resources associated with the widget. */
	virtual void ReleaseResources();

	/** Ensures the 3d window is created its size and content. */
	virtual void UpdateWidget();

	/** Ensure the render target is initialized and updates it if needed. */
	virtual void UpdateRenderTarget(FIntPoint DesiredRenderTargetSize);

	/**
	 * Converts a world-space hit result to a hit location on the widget
	 */
	virtual void GetLocalHitLocation(int32 InHitFaceIndex, const FVector& InWorldHitLocation, const FVector& InLineStart, const FVector& InLineEnd, FVector2D& OutLocalHitLocation) const;

	/** Gets the last local location that was hit */
	FVector2D GetLastLocalHitLocation() const
	{
		return LastLocalHitLocation;
	}

	/** Returns the class of the user widget displayed by this component */
	TSubclassOf<UUserWidget> GetWidgetClass() const { return WidgetClass; }

	/** Returns the user widget object displayed by this component */
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (UnsafeDuringActorConstruction = true))
	UUserWidget* GetUserWidgetObject() const;

	/** Returns the Slate widget that was assigned to this component, if any */
	const TSharedPtr<SWidget>& GetSlateWidget() const;

	/** Returns the list of widgets with their geometry and the cursor position transformed into this Widget space. The widget space is expressed as a Vector2D. */
	TArray<FWidgetAndPointer> GetHitWidgetPath(FVector2D WidgetSpaceHitCoordinate, bool bIgnoreEnabledStatus, float CursorRadius = 0.0f);

	/** Returns the render target to which the user widget is rendered */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	UTextureRenderTarget2D* GetRenderTarget() const;

	/** Returns the window containing the user widget content */
	TSharedPtr<SWindow> GetSlateWindow() const;

	/**
	 *  Gets the widget that is used by this Widget Component. It will be null if a Slate Widget was set using SetSlateWidget function.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	virtual UUserWidget* GetWidget() const;

	/**
	 *  Sets the widget to use directly. This function will keep track of the widget till the next time it's called
	 *	with either a newer widget or a nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	virtual void SetWidget(UUserWidget* Widget);

	/**
	 *  Sets a Slate widget to be rendered.  You can use this to draw native Slate widgets instead of drawing user widgets.
	 */
	virtual void SetSlateWidget(const TSharedPtr<SWidget>& InSlateWidget);

	/** @see bManuallyRedraw */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool GetManuallyRedraw() const
	{
		return bManuallyRedraw;
	};

	/** @see bManuallyRedraw */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetManuallyRedraw(bool bUseManualRedraw);

	/** Returns the "actual" draw size of the quad in the world */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	FVector2D GetCurrentDrawSize() const;

	/** Requests that the widget have it's render target updated, if TickMode is disabled, this will force a tick to happen to update the render target. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	virtual void RequestRenderUpdate();

	/** Gets whether the widget ticks when offscreen or not */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool GetTickWhenOffscreen() const
	{
		return TickWhenOffscreen;
	};

	/** Sets whether the widget ticks when offscreen or not */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetTickWhenOffscreen(const bool bWantTickWhenOffscreen)
	{
		TickWhenOffscreen = bWantTickWhenOffscreen;
	};

	/** Sets the background color and opacityscale for this widget */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetBackgroundColor(const FLinearColor NewBackgroundColor);

	/**  */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	float GetRedrawTime() const { return RedrawTime; }

	/**  */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetRedrawTime(float InRedrawTime) { RedrawTime = InRedrawTime; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
	float GetResolutionScale()const { return ResolutionScale; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetResolutionScale(float value) { ResolutionScale = value; }

	/** Get the fake window we create for widgets displayed in the world. */
	TSharedPtr< SWindow > GetVirtualWindow() const;

	/** Sets the widget class used to generate the widget for this component */
	void SetWidgetClass(TSubclassOf<UUserWidget> InWidgetClass);

	bool GetEditTimeUsable() const { return bEditTimeUsable; }

	void SetEditTimeUsable(bool Value) { bEditTimeUsable = Value; }

	/** @see bWindowFocusable */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool GetWindowFocusable() const
	{
		return bWindowFocusable;
	};

	/** @see bWindowFocusable */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetWindowFocusable(bool bInWindowFocusable);

	/** Gets the visibility of the virtual window created to host the widget focusable. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	EWindowVisibility GetWindowVisiblility() const
	{
		return WindowVisibility;
	}

	/** Sets the visibility of the virtual window created to host the widget focusable. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetWindowVisibility(EWindowVisibility InVisibility);

	/** Sets the Tick mode of the Widget Component.*/
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetTickMode(ETickMode InTickMode);

	/** Returns true if the the Slate window is visible and that the widget is also visible, false otherwise. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool IsWidgetVisible() const;

	/** Hook to allow this component modify the local position of the widget after it has been projected from world space to screen space. */
	virtual FVector2D ModifyProjectedLocalPosition(const FGeometry& ViewportGeometry, const FVector2D& LocalPosition) { return LocalPosition; }

protected:
	void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);

	void RegisterWindow();
	void UnregisterWindow();

	/** Allows subclasses to control if the widget should be drawn.  Called right before we draw the widget. */
	virtual bool ShouldDrawWidget() const;

	/** Draws the current widget to the render target if possible. */
	virtual void DrawWidgetToRenderTarget(float DeltaTime);

protected:
	/** How this widget should deal with timing, pausing, etc. */
	UPROPERTY(EditAnywhere, Category = LGUI)
	EWidgetTimingPolicy TimingPolicy;

	/** The class of User Widget to create and display an instance of */
	UPROPERTY(EditAnywhere, Category = LGUI)
	TSubclassOf<UUserWidget> WidgetClass;

	/** Should we wait to be told to redraw to actually draw? */
	UPROPERTY(EditAnywhere, Category = LGUI)
	bool bManuallyRedraw;

	/** Has anyone requested we redraw? */
	bool bRedrawRequested;

	/**
	 * The time in between draws, if 0 - we would redraw every frame.  If 1, we would redraw every second.
	 * This will work with bManuallyRedraw as well.  So you can say, manually redraw, but only redraw at this
	 * maximum rate.
	 */
	UPROPERTY(EditAnywhere, Category = LGUI)
	float RedrawTime;

	/** What was the last time we rendered the widget? */
	double LastWidgetRenderTime;

	/** Returns current absolute time, respecting TimingPolicy. */
	double GetCurrentTime() const;

	/**
	 * Scale draw size for the widget.
	 */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (ClampMin = "0.01"))
	float ResolutionScale = 1.0f;
	/**
	 * The actual draw size, this changes based on this UI element's size.
	 */
	FIntPoint CurrentDrawSize;

	/** Is the virtual window created to host the widget focusable? */
	UPROPERTY(EditAnywhere, Category = LGUI)
	bool bWindowFocusable = true;

	/** The visibility of the virtual window created to host the widget */
	UPROPERTY(EditAnywhere, Category = LGUI)
	EWindowVisibility WindowVisibility = EWindowVisibility::SelfHitTestInvisible;

	/**
	 * Widget components that appear in the world will be gamma corrected by the 3D renderer.
	 * In some cases, widget components are blitted directly into the backbuffer, in which case gamma correction should be enabled.
	 */
	UPROPERTY(EditAnywhere, Category = LGUI, AdvancedDisplay)
	bool bApplyGammaCorrection = false;

	/** The background color of the component */
	UPROPERTY(EditAnywhere, Category = LGUI)
	FLinearColor BackgroundColor = FLinearColor::Transparent;

	/** Should the component tick the widget when it's off screen? */
	UPROPERTY(EditAnywhere, Category = LGUI)
	bool TickWhenOffscreen;

	/** The target to which the user widget is rendered */
	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	/**
	 * Allows the widget component to be used at editor time.  For use in the VR-Editor.
	 */
	UPROPERTY()
	bool bEditTimeUsable;

protected:

	UPROPERTY(EditAnywhere, Category = LGUI)
	ETickMode TickMode = ETickMode::Enabled;

	/** The slate window that contains the user widget content */
	TSharedPtr<class SVirtualWindow> SlateWindow;

	/** The relative location of the last hit on this component */
	FVector2D LastLocalHitLocation;

	/** The hit tester to use for this component */
	static TSharedPtr<class FWidget3DHitTester> WidgetHitTester;

	/** Helper class for drawing widgets to a render target. */
	class FWidgetRenderer* WidgetRenderer = nullptr;

private:
	bool ShouldReenableComponentTickWhenWidgetBecomesVisible() const;

	/** The User Widget object displayed and managed by this component */
	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<UUserWidget> Widget;

	/** The Slate widget to be displayed by this component.  Only one of either Widget or SlateWidget can be used */
	TSharedPtr<SWidget> SlateWidget;

	/** The slate widget currently being drawn. */
	TWeakPtr<SWidget> CurrentSlateWidget;

	static EVisibility ConvertWindowVisibilityToVisibility(EWindowVisibility visibility);

	void OnWidgetVisibilityChanged(ESlateVisibility InVisibility);

	/** Set to true after a draw of an empty component.*/
	bool bRenderCleared;
	bool bOnWidgetVisibilityChangedRegistered;
};

/**
 * LGUI Widget can render a UMG widget as LGUI's element, and interact with it by UIWidgetInteraction component.
 */
UCLASS(ClassGroup = LGUI)
class LGUI_API AUIWidgetActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()

public:
	AUIWidgetActor();

	virtual UUIItem* GetUIItem()const override { return UIWidget; }
	virtual class UUIBaseRenderable* GetUIRenderable()const override { return UIWidget; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIWidget* GetUIWidget()const { return UIWidget; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UUIWidget> UIWidget;

};

