// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Camera/CameraTypes.h"
#include "Core/LexCanvasAsyncFunctionRunnable.h"
#include "Core/LexCanvasDrawCallProcessingRunnable.h"
#include "Core/LexCanvasProcessingDrawCallData.h"
#include "Core/LexUIBehaviour.h"
#include "Core/LexUIDrawCall.h"
#include "Math/TransformCalculus2D.h"
#include "LexCanvas.generated.h"

class ULexWidgetPresenterComponentBase;
class FLexUIClipData;
class ULexUIDataAsTexture;

UENUM(BlueprintType, Category = LGUI)
enum class ELexRenderMode :uint8
{
	/**
	 * Render in screen space. If there are multiple screen-space-ui-root in world, they will be sorted by SortOrder property.
	 * This mode use LexUI's custom render pipeline.
	 * This mode need a LexCanvasScaler to control the size and scale.
	 */
	ScreenSpaceOverlay = 0,
	/**
	 * Render in world space by UE default render pipeline.
	 * This mode use engine's default render pipeline, so post process will affect ui.
	 */
	WorldSpace=1			UMETA(DisplayName = "World Space - UE Renderer"),
	/**
	 * Render in world space by LexUI's custom render pipeline, 
	 * This mode use LexUI's custom render pipeline, will not be affected by post process.
	 */
	WorldSpace_LexUI = 3		UMETA(DisplayName = "World Space - LexUI Renderer"),
	/**
	 * Render to a custom render target.
	 */
	RenderTarget = 2		UMETA(DisplayName = "Render Target"),
	
	None = 255				UMETA(Hidden),
};

UENUM(BlueprintType, Category = LGUI)
enum class ELexCanvasRenderTargetSizeMode : uint8
{
	None,
	/** Change LexCanvas's size to fit RenderTarget. */
	CanvasFitToRenderTarget,
	/** Change RenderTarget's size to fit LexCanvas. */
	RenderTargetFitToCanvas,
};

UENUM(BlueprintType, Category = LGUI)
enum class ELexCanvasRenderTargetUpdateMode : uint8
{
	/** LexUI will automatically manage update, only draw to RenderTarget when it detect something change. */
	Automatic,
	/** Always draw to RenderTarget every frame. */
	Always,
	/** Only draw to RenderTarget when call RequestUpdateForRenderTarget. */
	WhenRequest,
};

UENUM(BlueprintType, meta = (Bitflags), Category = LGUI)
enum class ELexCanvasOverrideParameters :uint8
{
	DefaultMaterial,
	RequireNormalAndTangent,
	BlendDepth,
	DepthFade,
};
ENUM_CLASS_FLAGS(ELexCanvasOverrideParameters);

UENUM(BlueprintType, Category = LGUI)
enum class ELexCanvasScaleMode:uint8
{
	/** 1 unit is 1 pixel render in screen*/
	ConstantPixelSize,
	/** scale UI with reference resolution and screen resolution*/
	ScaleWithScreenSize,
	/**
	 * Assign CustomScale parameter to use a custom class calculate resolution and scale.
	 */
	Custom,
};

UENUM(BlueprintType, Category = LGUI)
enum class ELexCanvasScreenMatchMode :uint8
{
	/** Use "MatchFromWidthToHeight" and "ReferenceResolution" properties to control size and scale UI*/
	MatchWidthOrHeight,
	/** If viewport's aspect ratio not match "ReferenceResolution"'s aspect ratio, then expand size and scale UI*/
	Expand,
	/** if viewport's aspect ratio not match "ReferenceResolution"'s aspect ratio, then shrink size and scale UI*/
	Shrink,
};

class ULexCanvas;

UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API ULexCanvasCustomScale: public UObject
{
	GENERATED_BODY()
public:
	/** Initialize, called when LexCanvas Awake. */
	virtual void Init(ULexCanvas* InCanvas);
	/** Called when LexCanvas calculate viewport size and scale. */
	virtual void CalculateSizeAndScale(ULexCanvas* InCanvas, const FIntPoint& InViewportSize, FIntPoint& OutLexCanvasSize, float& OutScale);
	/**
	 * Convert position from viewport to LexCanvas space.
	 * @param InPosition The point's pixel position on viewport.
	 * @param Result LexCanvas space position, left bottom is zero point.
	 * @return convert will fail if this LexCanvas is not root canvas
	 */
	virtual bool ConvertPositionFromViewportToCanvas(const FVector2D& InPosition, FVector2D& Result)const;
	/**
	 * Convert position from LexCanvas space to viewport.
	 * @param InPosition The point's position in LexCanvas space.
	 * @param Result in viewport, pixel unit, left top is zero point.
	 * @return convert will fail if this LexCanvas is not root canvas
	 */
	virtual bool ConvertPositionFromCanvasToViewport(const FVector2D& InPosition, FVector2D& Result)const;
protected:
	/** Initialize, called when LexCanvas Awake. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Init"), Category = "LGUI")
	void ReceiveInit(ULexCanvas* InCanvas);
	/** Called when LexCanvas calculate viewport size and scale. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "CalculateSizeAndScale"), Category = "LGUI")
	void ReceiveCalculateSizeAndScale(ULexCanvas* InCanvas, const FIntPoint& InViewportSize, FIntPoint& OutLexCanvasSize, float& OutScale);
	/**
	 * Convert position from viewport to LexCanvas space.
	 * @param InPosition The point's pixel position on viewport.
	 * @param Result LexCanvas space position, left bottom is zero point.
	 * @return convert will fail if this LexCanvas is not root canvas
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ConvertPositionFromViewportToCanvas"), Category = "LGUI")
	bool ReceiveConvertPositionFromViewportToCanvas(const FVector2D& InPosition, FVector2D& Result)const;
	/**
	 * Convert position from LexCanvas space to viewport.
	 * @param InPosition The point's position in LexCanvas space.
	 * @param Result in viewport, pixel unit, left top is zero point.
	 * @return convert will fail if this LexCanvas is not root canvas
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ConvertPositionFromCanvasToViewport"), Category = "LGUI")
	bool ReceiveConvertPositionFromCanvasToViewport(const FVector2D& InPosition, FVector2D& Result)const;
};

USTRUCT()
struct FLexCanvasDynamicMaterialArrayContainer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = LGUI)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> MaterialArray;

	int CurrentIndex = 0;
};

USTRUCT()
struct FLexCanvasMaterialParameterCache
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, Category=LGUI)
	TWeakObjectPtr<UTexture> Texture = nullptr;
	UPROPERTY(VisibleAnywhere, Category=LGUI)
	TWeakObjectPtr<UTexture> FontTexture = nullptr;
};

class ULexWidget;
class ULexVisual;
class ULexVisualBatchMesh;
class ULexVisualDirectMesh;
class ULexUIMeshComponent;
class FLexVisualPostProcessRenderProxy;
class UTextureRenderTarget2D;

/**
 * Canvas is for render and update all UI elements.
 * Default UV channels-
 *		UV0: Texture coordinate
 *		UV1: X- Widget property data coordinate, including clipData coordinate in data texture; Y- Slice index of texture array, mostly for font rendering
 * Other UV channels are defined by LexVisual, check LexText and LexRectBlock.
 */
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API ULexCanvas : public ULexUIBehaviour
{
	GENERATED_BODY()

public:	
	ULexCanvas();
protected:
	virtual void Awake() override;
#if WITH_EDITOR
public:
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostLoad()override;
	virtual void PostEditUndo()override;
	void EnsureDataForRebuild();
#endif
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
	virtual void PostInitProperties() override;

	static FName GetPropertyName_TraceChannel()
	{
		return GET_MEMBER_NAME_CHECKED(ULexCanvas, TraceChannel);
	}
private:
	/** clear draw-calls */
	void ClearDrawCall();
	void RemoveFromViewExtension(bool PropagateToChildrenCanvas);
	TSharedPtr<class FLexUIRenderer, ESPMode::ThreadSafe> RenderTargetViewExtension = nullptr;
	TSharedPtr<class FLexUIRenderer, ESPMode::ThreadSafe> GetRenderTargetViewExtension();
public:
	/** mark canvas layout dirty */
	void MarkTransformOrDimensionChanged();
	/**
	 * Mark update this Canvas. Canvas don't need to update every frame, only update when need to.
	 * Some rules if update could trigger draw-call's rebuild:
	 *		1. Commonly material & texture change and UI item's active state change
	 *		2. Transform & vertex position change, draw-call could overlap with each other
	 *		3. Hierarchy order change, this is directly related to render order
	 * And about draw-call's rebuild, it's not actually force rebuild, it will check and reuse prev draw-call if possible.
	 * @param	bRebuildDrawCall	When we need rebuild draw-call? Material or texture change, transform or vertex position change, add or remove ui element
	 */
	void MarkCanvasUpdate(bool bRebuildDrawCall);

	static void BuildProjectionMatrix(FIntPoint InViewportSize, ECameraProjectionMode::Type InProjectionType, float FOV, float FarClipPlane, float NearClipPlane, FMatrix& OutProjectionMatrix);
	FMatrix GetViewProjectionMatrix()const;
	FMatrix GetProjectionMatrix()const;
	FVector GetViewLocation()const;
	FRotator GetViewRotator()const;
	FIntPoint GetViewportSize()const;
	/** get scale value of canvas. only valid for root canvas. */
	FORCEINLINE float GetCanvasScale()const { return CanvasScale; }
private:
	friend class ULexCanvasScaler;
	float CanvasScale = 1.0f;//for screen space UI, screen size / root canvas size

	/** hierarchy changed */
	void OnUIHierarchyAttachmentChanged();
	void OnCanvasWidgetActiveChanged(bool WidgetActive);
public:
	/** get root canvas on hierarchy */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	ULexCanvas* GetRootCanvas()const;
	/** is this the root canvas in hierarchy */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool IsRootCanvas()const;
	/** return root SceneComponent if the root canvas is attached to a SceneComponent */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	ULexWidgetPresenterComponentBase* GetWidgetPresenterComponent() const;
	/** Only set on root canvas */
	void AttachToWidgetPresenterComponent(ULexWidgetPresenterComponentBase* InSceneComp) const;

	bool IsRenderToScreenSpace()const;
	bool IsRenderToRenderTarget()const;
	bool IsRenderToWorldSpace()const;
	bool IsRenderByLexUIRendererOrUERenderer()const;

	TWeakObjectPtr<ULexCanvas> GetParentCanvas()const { return ParentCanvas; }

	void SetParentCanvas(ULexCanvas* InParentCanvas);

	static void CollectChildrenCanvas(ULexCanvas* Target, TArray<ULexCanvas*>& OutAllChildrenCanvas, bool IncludeTarget = true);

	DECLARE_EVENT_ThreeParams(ULexCanvas, FRenderModeChangedEvent, ULexCanvas*, ELexRenderMode/*Old*/, ELexRenderMode/*New*/);
	DECLARE_EVENT_OneParam(ULexCanvas, FRenderTargetChangedEvent, UTextureRenderTarget2D*);
protected:
	/** Root LexCanvas on hierarchy. LGUI's update start from the RootCanvas, and goes all down to every UI elements under it */
	UPROPERTY(Transient) mutable TWeakObjectPtr<ULexCanvas> RootCanvas = nullptr;
	void CheckRenderMode(bool PropagateToChildrenCanvas);
	/** check RootCanvas. search for it if not valid */
	bool CheckRootCanvas(bool forceRecheck = false)const;
	/** nearest up parent Canvas */
	UPROPERTY(Transient) TWeakObjectPtr<ULexCanvas> ParentCanvas = nullptr;

protected:
	friend class FLexCanvasCustomization;
	friend class FLexWidgetCustomization;

	float CalculateDistanceToCamera()const;

	/**
	 * Force this canvas render to a TextureRenderTarget, no matter what render mode of the root canvas is.
	 * This will break canvas link and make this canvas as root canvas.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
	bool bForceRenderToTarget = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELexRenderMode RenderMode = ELexRenderMode::WorldSpace;
	/**
	 * Render to RenderTarget, if not specified then LGUI will create a new one.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TObjectPtr<UTextureRenderTarget2D> RenderTarget;
	/** Clear color for TextureRenderTarget */
	UPROPERTY(EditAnywhere, Category = "LGUI")
	FColor RenderTargetClearColor = FColor::Transparent;
	/** Controls how LexCanvas render to RenderTarget. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELexCanvasRenderTargetUpdateMode RenderTargetUpdateMode = ELexCanvasRenderTargetUpdateMode::Automatic;
	/**
	 * How RenderTarget and LexCanvas's size change depend on the other.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELexCanvasRenderTargetSizeMode RenderTargetSizeMode = ELexCanvasRenderTargetSizeMode::RenderTargetFitToCanvas;
	/**
	 * RenderTarget size scale.
	 * Only valid if RenderTargetSizeMode is RenderTargetFitToCanvas.
	 */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (ClampMin = "0.01", EditCondition="RenderTargetSizeMode==ELexCanvasRenderTargetSizeMode::RenderTargetFitToCanvas"))
		float RenderTargetResolutionScale = 1.0f;
	/**
	 * true- Use custom sort order.
	 * false- Use default sort order management, which is based on hierarchy order.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bOverrideSorting = false;
	/**
	 * Canvas with larger order will render on top of lower one.
	 * NOTE! SortOrder value is stored with int16 type, so valid range is -32768 to 32767
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta=(EditCondition="bOverrideSorting"))
		int16 SortOrder = 0;

	/** Enable/disable normal and tangent in vertex data. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
	bool bRequireNormalAndTangent = false;

	/** Default materials, for render default UI elements. */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (DisplayThumbnail = "false"))
	mutable TObjectPtr<UMaterialInterface> DefaultMaterial;

	/** For "World Space - LexUI Renderer" only, render with blend depth, 0-occlude by scene depth, 1-all visible, 0.5-half transparent. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float BlendDepth = 0.0f;
	/** For "World Space - LexUI Renderer" only, render with depth fade effect. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0", ClampMax = "10"))
		int DepthFade = 0;
	/**
	 * Create a depth texture so we can do depth test. This is very useful for UIStaticMesh which use Opaque material.
	 * Only valid for ScreenSpaceOverlay and RenderTarget mode.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bEnableDepthTest = false;
	/** For not root canvas, inherit or override parent canvas parameters. */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (Bitmask, BitmaskEnum = "/Script/LGUI.ELexCanvasOverrideParameters"))
		int8 OverrideParameters = 0;

	/**
	 * TraceChannel for line trace of EventSystem interaction.
	 * Only world space UI need this property.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
	TEnumAsByte<ETraceTypeQuery> TraceChannel = TraceTypeQuery1;

	/**
	 * Allow drop canvas frame when canvas draw-call take too much time. This may cause some delay for UI response, but can improve performance.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI", AdvancedDisplay)
	bool bAllowDropFrame = false;

	/**
	 * LexCanvas create mesh for render UI elements, this property can give us opportunity to use custom type of mesh for render.
	 * You can set "OwnerNoSee" "CastShadow" properties for your mesh.
	 * @todo: override this property from parent canvas?
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI", AdvancedDisplay, meta = (AllowAbstract = "true"))
		TSubclassOf<ULexUIMeshComponent> DefaultMeshType;

#pragma region CanvasScaler
	/** Virtual Camera Projection Type.*/
	UPROPERTY(EditAnywhere, Category = "LGUI-CanvasScaler", AdvancedDisplay, meta = (DisplayName = "Projection Type"))
	TEnumAsByte<ECameraProjectionMode::Type> ProjectionType = ECameraProjectionMode::Perspective;
	/** Virtual Camera field of view (in degrees). */
	UPROPERTY(EditAnywhere, Category = "LGUI-CanvasScaler", AdvancedDisplay, meta = (UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0"))
	float FieldOfView = 60;
	UPROPERTY(EditAnywhere, Category = "LGUI-CanvasScaler", AdvancedDisplay)
	float NearClipPlane = 1;
	UPROPERTY(EditAnywhere, Category = "LGUI-CanvasScaler", AdvancedDisplay)
	float FarClipPlane = 10000;
	
	UPROPERTY(EditAnywhere, Category = "LGUI-CanvasScaler")
	ELexCanvasScaleMode ScaleMode = ELexCanvasScaleMode::ConstantPixelSize;
	UPROPERTY(EditAnywhere, Category = "LGUI-CanvasScaler")
	FVector2D ReferenceResolution = FVector2D(1280, 720);
	UPROPERTY(EditAnywhere, Category = "LGUI-CanvasScaler", meta = (ClampMin = "0.0", ClampMax = "1.0", DisplayName = "Match"))
	float MatchFromWidthToHeight = 1;
	UPROPERTY(EditAnywhere, Category = "LGUI-CanvasScaler")
	ELexCanvasScreenMatchMode ScreenMatchMode = ELexCanvasScreenMatchMode::MatchWidthOrHeight;
#if WITH_EDITORONLY_DATA
public:
	/** When Canvas use ScreenSpaceOverlay, in edit mode it will try to match editor viewport's size. So make this true to use a fixed size. */
	UPROPERTY(EditAnywhere, Category = "LGUI-CanvasScaler")
	bool bFixedSizeInEditMode = false;
	UPROPERTY(EditAnywhere, Category = "LGUI-CanvasScaler", meta = (EditCondition = "bFixedSizeInEditMode"))
	FIntPoint SizeInEditMode = FIntPoint(1920, 1080);
#endif
private:
	/**
	 * Use this to do custom scale. Only valid if ScaleMode = Custom.
	 * Will fallback to "ConstantPixelSize" if not assign this value.
	 */
	UPROPERTY(EditAnywhere, Instanced, Category = "LGUI-CanvasScaler")
	TObjectPtr<ULexCanvasCustomScale> CustomScale;
	/** Current viewport size*/
	FIntPoint ViewportSize = FIntPoint(2, 2);
#pragma endregion
	FRenderModeChangedEvent OnRenderModeChanged;
	FRenderTargetChangedEvent OnRenderTargetChanged;

public:
	FORCEINLINE bool GetOverrideDefaultMaterial()const						{ return OverrideParameters & (1 << (int)ELexCanvasOverrideParameters::DefaultMaterial); }
	FORCEINLINE bool GetOverrideRequireNormalAndTangent()const				{ return OverrideParameters & (1 << (int)ELexCanvasOverrideParameters::RequireNormalAndTangent); }
	FORCEINLINE bool GetOverrideBlendDepth()const							{ return OverrideParameters & (1 << (int)ELexCanvasOverrideParameters::BlendDepth); }
	FORCEINLINE bool GetOverrideDepthFade()const							{ return OverrideParameters & (1 << (int)ELexCanvasOverrideParameters::DepthFade); }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		UMaterialInterface* GetDefaultMaterial()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetDefaultMaterial(UMaterialInterface* InMaterial);

	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetTraceChannel(TEnumAsByte<ETraceTypeQuery> InTraceChannel);
	UFUNCTION(BlueprintCallable, Category = LGUI)
	TEnumAsByte<ETraceTypeQuery> GetTraceChannel()const { return TraceChannel; }

	/** Set render mode of this canvas. This may not take effect if the canvas is not a root canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRenderMode(ELexRenderMode Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetForceRenderToTarget(bool Value);
	/** Set parameters for calculating projection matrix. Only valid for ScreenSpace/RenderTarget mode. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetProjectionParameters(TEnumAsByte<ECameraProjectionMode::Type> InProjectionType, float InFovAngle, float InNearClipPlane, float InFarClipPlane);
	/** if renderMode is RenderTarget, then this will change the renderTarget */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRenderTarget(UTextureRenderTarget2D* Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetRenderTargetClearColor(FColor Value);
	FRenderModeChangedEvent& GetRenderModeChangedEvent(){return OnRenderModeChanged;}
	FRenderTargetChangedEvent& GetRenderTargetChangedEvent(){return OnRenderTargetChanged;}
	
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRenderTargetResolutionScale(float Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRenderTargetSizeMode(ELexCanvasRenderTargetSizeMode Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRenderTargetUpdateMode(ELexCanvasRenderTargetUpdateMode Value);
	/** Only valid when call this on root canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void RequestUpdateForRenderTarget();
	
	/** 
	 * Set LexCanvas SortOrder
	 * @param	PropagateToChildrenCanvas	if true, set this Canvas's SortOrder and all children Canvas, not just set absolute value, but keep child Canvas's relative order to this one
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrder(int32 Value, bool PropagateToChildrenCanvas = true);
	/** Set SortOrder to highest, so this canvas will render on top of all canvas that belong to same hierarchy. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrderToHighestOfHierarchy(bool PropagateToChildrenCanvas = true);
	/** Set SortOrder to lowest, so this canvas will render behind all canvas that belong to same hierarchy. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrderToLowestOfHierarchy(bool PropagateToChildrenCanvas = true);
	void GetMinMaxSortOrderOfHierarchy(int32& OutMin, int32& OutMax);

	/**
	 * Get actual render mode of this canvas.
	 * Normally canvas's render-mode is inherited from parent canvas.
	 * */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELexRenderMode GetActualRenderMode()const;
	/** Get render mode of this canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELexRenderMode GetRenderMode()const { return RenderMode; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool GetForceRenderToTarget()const{return bForceRenderToTarget;}
	/** Get actual render target of this canvas if actual render mode is RenderTarget. Canvas's render-target is inherited from root canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		UTextureRenderTarget2D* GetActualRenderTarget()const;
	/** Get render target of this canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		UTextureRenderTarget2D* GetRenderTarget()const { return RenderTarget; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	FColor GetRenderTargetClearColor()const{return RenderTargetClearColor;}
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetRenderTargetResolutionScale()const { return RenderTargetResolutionScale; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELexCanvasRenderTargetSizeMode GetRenderTargetSizeMode()const { return RenderTargetSizeMode; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELexCanvasRenderTargetUpdateMode GetRenderTargetUpdateMode()const { return RenderTargetUpdateMode; }

	/** Get actual BlendDepth value of canvas. This property may inherit from parent canvas depend on OverrideParameters property. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetActualBlendDepth()const;
	/** Get blendDepth value of this canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetBlendDepth()const { return BlendDepth; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetBlendDepth(float Value);

	/** Get actual DepthFade value of canvas. This property may inherit from parent canvas depend on OverrideParameters property. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		int GetActualDepthFade()const;
	/** Get blendDepth value of this canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		int GetDepthFade()const { return DepthFade; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetDepthFade(int Value);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetEnableDepthTest()const { return bEnableDepthTest; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetEnableDepthTest(bool Value);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetOverrideSorting()const { return bOverrideSorting; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetOverrideSorting(bool Value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		int32 GetSortOrder()const { return SortOrder; }
	/** Get actual SortOrder of canvas. This property may inherit from parent canvas depend on OverrideSorting property. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		int32 GetActualSortOrder()const;

	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool GetActualRequireNormalAndTangent()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool GetRequireNormalAndTangent()const { return bRequireNormalAndTangent; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetRequireNormalAndTangent(bool Value);

	UFUNCTION(BlueprintCallable, Category = LGUI)
	int GetDrawCallCount()const;

	/** Override LexUI's screen space UI render's camera location. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetOverrideViewLocation(bool Override, FVector Value);
	/** Override LexUI's screen space UI render's camera rotation. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetOverrideViewRotation(bool Override, FRotator Value);
	/**
	 * Override LexUI's screen space UI render's camera's fov in degree, will affect projection matrix.
	 * If SetOverrideProjectionMatrix is true, then this will not take effect.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetOverrideFovAngle(bool Override, float Value);
	/**
	 * Override LexUI's screen space UI render's camera's projection matrix.
	 * If this is set to true, then SetOverrideFovAngle will not take effect.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetOverrideProjectionMatrix(bool Override, FMatrix Value);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		TSubclassOf<ULexUIMeshComponent> GetDefaultMeshType()const { return DefaultMeshType; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetDefaultMeshType(TSubclassOf<ULexUIMeshComponent> InValue);

#pragma region CanvasScaler
	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	TEnumAsByte<ECameraProjectionMode::Type> GetProjectionType()const { return ProjectionType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	float GetFieldOfView()const { return FieldOfView; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	float GetNearClipPlane()const { return NearClipPlane; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	float GetFarClipPlane()const { return FarClipPlane; }

	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	void SetProjectionType(TEnumAsByte<ECameraProjectionMode::Type> Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	void SetFieldOfView(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	void SetNearClipPlane(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	void SetFarClipPlane(float Value);

	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	ELexCanvasScaleMode GetScaleMode() { return ScaleMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	FVector2D GetReferenceResolution() { return ReferenceResolution; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	float GetMatchFromWidthToHeight() { return MatchFromWidthToHeight; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	ELexCanvasScreenMatchMode GetScreenMatchMode() { return ScreenMatchMode; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	ULexCanvasCustomScale* GetCustomScale()const { return CustomScale; }

	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	void SetScaleMode(ELexCanvasScaleMode Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	void SetReferenceResolution(FVector2D Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	void SetMatchFromWidthToHeight(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	void SetScreenMatchMode(ELexCanvasScreenMatchMode Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-CanvasScaler")
	void SetCustomScale(ULexCanvasCustomScale* Value);

	/**
	 * Convert position from viewport to LexCanvas space.
	 * @param InPosition The point's pixel position on viewport.
	 * @param Result LexCanvas space position, left bottom is zero point.
	 * @return convert will fail if this LexCanvas is not root canvas
	 */
	UFUNCTION(BlueprintPure, Category = "LGUI-CanvasScaler")
	bool ConvertPositionFromViewportToCanvas(const FVector2D& InPosition, FVector2D& Result)const;
	/**
	 * Convert position from LexCanvas space to viewport.
	 * @param InPosition The point's position in LexCanvas space.
	 * @param Result in viewport, pixel unit, left top is zero point.
	 * @return convert will fail if this LexCanvas is not root canvas
	 */
	UFUNCTION(BlueprintPure, Category = "LGUI-CanvasScaler")
	bool ConvertPositionFromCanvasToViewport(const FVector2D& InPosition, FVector2D& Result)const;
	/**
	 * Project 3D screen-space-UI element's position to 2D screen-space-UI.
	 * NOTE!!! This is only for lex-screen-space-UI, DON'T use this for convert world space!!!
	 * @param	Position3D	GetWorldLocation from the UI element (world location).
	 * @param	OutPosition2D	2D Position in screen-space, left bottom is zero point.
	 * @return 	convert will fail if this LexCanvas is not root canvas, or not screen space.
	 */
	UFUNCTION(BlueprintPure, Category = "LGUI-CanvasScaler")
	bool Project3DToScreen(const FVector& Position3D, FVector2D& OutPosition2D)const;
	/**
	 * Transforms 2D screen coordinates into a 3D world-space origin and direction.
	 * NOTE!!! This is only for lex-screen-space-UI, DON'T use this for convert world space!!!
	 * @param ScreenPos Screen coordinates in pixels, left bottom is zero point.
	 * @param OutWorldOrigin World space origin vector
	 * @param OutWorldDirection World space direction vector
	 * @return convert will fail if this LexCanvas is not root canvas, or not screen space.
	 */
	UFUNCTION(BlueprintPure, Category = "LGUI-CanvasScaler")
	bool DeprojectScreenTo3D(const FVector2D& ScreenPos, FVector& OutWorldOrigin, FVector& OutWorldDirection);
	/**
	 * Project 3D world position to 2D screen-space-UI position with specific player's camera.
	 * This function need player pawn contains a camera component, and use this camera to do projection, so it can be used for world space UI.
	 * @param Player 
	 * @param InPosition 
	 * @param OutPosition2D 
	 * @return 
	 */
	UFUNCTION(BlueprintPure, Category = "LGUI-CanvasScaler")
	static bool ProjectWorldToScreenWithPlayerCamera(APlayerController* Player, class UCameraComponent* PlayerCamera, const FVector& InPosition, FVector2D& OutPosition2D);
	UFUNCTION(BlueprintPure, Category = "LGUI-CanvasScaler")
	static bool BuildViewProjectionMatrixForPlayerCamera(APlayerController* Player, class UCameraComponent* PlayerCamera, FMatrix& OutViewProjectionMatrix);
	UFUNCTION(BlueprintPure, Category = "LGUI-CanvasScaler")
	static bool ProjectWorldToScreenWithViewProjectionMatrix(const FMatrix& InViewProjectionMatrix, const FVector2D& InViewportSize, const FVector& InPosition, FVector2D& OutPosition2D);
	
private:
#if WITH_EDITOR
	FDelegateHandle EditorTickDelegateHandle;
	void DrawVirtualCamera();
	void DrawViewportArea();
	void OnEditorTick(float DeltaTime);
#endif
	void RegisterCanvasScaler();
	void UnregisterCanvasScaler();
	void OnViewportParameterChanged();
	void CheckAndApplyViewportParameter();
	FDelegateHandle ViewportResizeDelegateHandle;
#pragma endregion

public:
	void MarkVisualWillChange(ULexVisual* InOldVisual);
	void RegisterVisual(ULexVisual* InVisual);
	void UnregisterVisual(ULexVisual* InVisual);

	void AddLexWidget(ULexWidget* InWidget);
	void RemoveLexWidget(ULexWidget* InWidget);
	void MarkLexWidgetHierarchyChanged();
	/** return all LexWidget that belongs to this canvas. */
	const TArray<ULexVisual*>& GetVisualArray()const { return VisualList; }
	const TArray<ULexWidget*>& GetWidgetArray()const { return WidgetList; }

	ULexUIMeshComponent* GetUIMesh()const { CheckUIMesh(); return UIMesh.Get(); }
public:
	static FName LexUI_MainTextureMaterialParameterName;
	static FName LexUI_FontTextureMaterialParameterName;
	static FName LexUI_ClipDataTexture_MaterialParameterName;
	static FName LexUI_WidgetPropertyDataTexture_MaterialParameterName;
	static FName LexUI_IsRenderByLexUIRenderer_MaterialParameterName;
	static bool IsMaterialContainsLexUIParameter(const UMaterialInterface* InMaterial);
private:
	void SetSortOrderAdditionalValueRecursive(int32 InAdditionalValue);
	void UpdateRenderTarget(bool CallEvent);
	void CheckRenderTargetUpdate();
public:
	/** Called from LexUIManagerActor. Update this canvas if it is a RootCanvas */
	void UpdateRootCanvas();
	void UpdateDrawCallBatchData();
	/**  */
	void MarkNeedVerifyMaterials();
private:
	uint32 bCanTickUpdate : 1 = true;//if Canvas can update from tick
	uint32 bShouldRebuildDrawCall : 1 = true;
	uint32 bHasPendingUpdateData : 1 = false;
	uint32 bNeedToSortRenderPriority : 1 = true;
	uint32 bHasAddToLexScreenSpaceRenderer : 1 = false;//is this canvas added to LGUI screen space renderer
	uint32 bRequestUpdateForRenderTarget : 1 = true;//request update when RenderTargetUpdateMode is WhenRequest
	uint32 bAnythingChangedForRenderTarget : 1 = true;//if children canvas anything changed, then mark this property for root canvas, good for RenderTarget mode to update
	uint32 bPrevAnythingChangedForRenderTarget : 1 = true;//same as upper one, but the prev frame
	uint32 bHasSetInitialStateForLexWorldSpaceRenderer : 1 = false;//is LGUI world space renderer's initial state set
	uint32 bNeedToVerifyMaterials : 1 = true;
	uint32 bNeedToGenerateWidgetList : 1 = true;
	uint32 bWidgetPropertyDataAsTextureChanged : 1 = true;
	uint32 bClipDataAsTextureChanged : 1 = true;

	uint32 bPrevIsVisible : 1 = true;//is LexWidget active in prev frame?

	uint32 bOverrideViewLocation:1=false, bOverrideViewRotation:1=false, bOverrideProjectionMatrix:1=false, bOverrideFovAngle:1=false;

	mutable uint32 bUIMeshNeedToSetInitialParameters : 1 = true;//after clear UIMesh, it will need to set initial parameters to use again
	mutable uint32 bIsViewProjectionMatrixDirty : 1 = true;
	mutable FMatrix CacheViewProjectionMatrix = FMatrix::Identity;//cache to prevent multiple calculation in same frame
	mutable float LastRenderTime = 0;
	friend class FLexUIRenderSceneProxy;
	/**
	 * RenderMode can affect UI's renderer, basically WorldSpace use UE's built-in renderer, others use LGUI's renderer. Different renderers cannot share same render data.
	 * eg: when attach to other canvas, this will tell which render mode in old canvas, and if not compatible then recreate render data.
	 */
	ELexRenderMode CurrentRenderMode = ELexRenderMode::None;
	FORCEINLINE bool RenderModeIsLexRendererOrUERenderer(ELexRenderMode InRenderMode)const
	{
		if (bForceRenderToTarget)return true;
		return 
			InRenderMode != ELexRenderMode::WorldSpace
			;
	}

	FVector OverrideViewLocation = FVector::ZeroVector;
	FRotator OverrideViewRotation = FRotator::ZeroRotator;
	float OverrideFovAngle = 0;
	FMatrix OverrideProjectionMatrix = FMatrix::Identity;

	UPROPERTY(Transient)
	mutable TObjectPtr<ULexUIMeshComponent> UIMesh;//current using UIMesh.
	//DefaultMaterial created MaterialInstanceDynamic pool 
	UPROPERTY(Transient, VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> PooledDefaultMaterialList;
	//Currently using material inside PooledDefaultMaterialList from this start index to end
	UPROPERTY(Transient, VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
	int UsingMaterialStartIndex = 0;
	UPROPERTY(Transient, VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
	TMap<TObjectPtr<UMaterialInterface>, FLexCanvasDynamicMaterialArrayContainer> MapSrcMatToDynamicMat;//@todo: delete not using material
	UPROPERTY(Transient, VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
	TMap<TObjectPtr<UMaterialInterface>, FLexCanvasMaterialParameterCache> MapMatToParamCache;//@todo: delete not using material
	uint64 NewestDrawCallFrameNumber = 0;
	FLexCanvasPendingDrawCallData CurrentDrawCallData;//current drawing draw-call
	TUniquePtr<FLexCanvasDrawCallProcessingRunnable> DrawCallProcessingRunnable;
	TUniquePtr<FLexCanvasAsyncFunctionRunnable> TransformVerticesAsyncFunctionRunnable;
	UPROPERTY(Transient)
	TArray<TObjectPtr<ULexVisual>> VisualList;//Use LexWidget instead of LexVisual, because we need LexWidget to get sub-canvas.
	UPROPERTY(Transient)
	TArray<TObjectPtr<ULexWidget>> WidgetList;//All LexWidget that belongs to this canvas
	TSharedPtr<FLexUIDrawCall> DrawCallAsChildCanvas = nullptr;//DrawCall that represent this canvas when the canvas is render as child.

	UPROPERTY(Transient)
	mutable TWeakObjectPtr<ULexWidgetPresenterComponentBase> WidgetPresenterComponent = nullptr;
	
	//clip data is stored in root canvas
	TArray<TSharedPtr<FLexUIClipData>> ClipDataList;
	UPROPERTY(Transient, VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
	TObjectPtr<ULexUIDataAsTexture> ClipDataAsTexture;//clip coordinate stored in UV1.x
	void OnClipDataTextureChanged(UTexture* NewTexture);
	//widget property data is stored in each canvas (not only root canvas)
	UPROPERTY(Transient, VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
	TObjectPtr<ULexUIDataAsTexture> WidgetPropertyDataAsTexture;//widget properties coordinate stored in UV1.y
	void OnWidgetPropertyDataTextureChanged(UTexture* NewTexture);
	void CheckWidgetPropertyData();
public:
	void PushAsyncFunction_TransformVertices(TFunction<void()> InFunction);
	/** Called by LexWidget to delete clip data */
	void RemoveClipData(const TSharedPtr<FLexUIClipData>& InClipData);
	UTexture* GetClipDataTexture()const;
	ULexUIDataAsTexture* GetWidgetPropertyDataAsTexture()const{return WidgetPropertyDataAsTexture;}
	
	const TArray<TWeakObjectPtr<ULexCanvas>>& GetChildrenCanvasArray()const{return ChildrenCanvasArray;}
	
	static FTransform2D ConvertTo2DTransform(const FTransform& Transform);
	static void CalculateVisual2DBounds(ULexVisual* Visual, const FTransform2D& OutTransform2D, FVector2D& OutMin, FVector2D& OutMax);
private:

	/** canvas array belong to this canvas in hierarchy. */
	UPROPERTY(Transient) TArray<TWeakObjectPtr<ULexCanvas>> ChildrenCanvasArray;
	/** update Canvas's draw-call */
	void UpdateCanvasDrawCall();
	/** mark render finish */
	void MarkFinishUpdateCanvasDrawCall();

	void PrepareDrawCallBatchingData(TArray<FLexUIRenderData>& OutRenderDataArray);
	void UpdateDrawCallMesh();
	void UpdateDrawCallMaterial();
	void SortDrawCall();
public:
	static void BatchDrawCallAsync(const FVector2D& InCanvasLeftBottom, const FVector2D& InCanvasRightTop, const TArray<FLexUIRenderData>& InRenderDataArray, TArray<FLexUIDrawCall>& InOutUIDrawCallList);
	static bool Is2DUITransform(const FTransform& Transform);
private:
	void CheckUIMesh()const;
};
