// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "Layout/Margin.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraTypes.h"
#include "LGUICanvas.generated.h"

UENUM(BlueprintType, Category = LGUI)
enum class ELGUIRenderMode :uint8
{
	/**
	 * Render in screen space. If there are multiple screen-space-ui-root in world, they will be sort by SortOrder property.
	 * This mode use LGUI's custom render pipeline.
	 * This mode need a LGUICanvasScaler to control the size and scale.
	 */
	ScreenSpaceOverlay = 0,
	/**
	 * Render in world space by UE default render pipeline.
	 * This mode use engine's default render pieple, so post process will affect ui.
	 */
	WorldSpace=1			UMETA(DisplayName = "World Space - UE Renderer"),
	/**
	 * Render in world space by LGUI's custom render pipeline, 
	 * This mode use LGUI's custom render pipeline, will not be affected by post process.
	 */
	WorldSpace_LGUI = 3		UMETA(DisplayName = "World Space - LGUI Renderer"),
	/**
	 * Render to a custom render target.
	 */
	RenderTarget=2		UMETA(DisplayName = "Render Target (Experimental)"),
};

UENUM(BlueprintType, Category = LGUI)
enum class ELGUICanvasClipType :uint8
{
	None,
	/** Clip content by a rectange area, with edge feather. Support nested rect clip. Support input hit test. */
	Rect,
	/** Clip content with a black-white texture (acturally the red channel of the texture). Not support nested clip. Not support input hit test. */
	Texture,
};

UENUM(BlueprintType, meta = (Bitflags), Category = LGUI)
enum class ELGUICanvasAdditionalChannelType :uint8
{
	/** Lit shader may need this */
	Normal,
	/** Lit and normalMap may need this */
	Tangent,
	/** Additional textureCoordinate at uv1(first uv is uv0, which is used by LGUI. Second uv is uv1) */
	UV1,
	UV2,
	UV3,
};
ENUM_CLASS_FLAGS(ELGUICanvasAdditionalChannelType)

UENUM(BlueprintType, meta = (Bitflags), Category = LGUI)
enum class ELGUICanvasOverrideParameters :uint8
{
	DefaltMaterials,
	PixelPerfect,
	DynamicPixelsPerUnit,
	ClipType,
	AdditionalShaderChannels,
	BlendDepth,
};
ENUM_CLASS_FLAGS(ELGUICanvasOverrideParameters);

USTRUCT()
struct FLGUIMaterialArrayContainer
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = LGUI)
		TArray<UMaterialInstanceDynamic*> MaterialList;
};

class FTransform2D;

struct FLGUICacheTransformContainer
{
public:
	FTransform Transform;

	FVector2D BoundsMin2D;
	FVector2D BoundsMax2D;
};

class UUIItem;
class UUIBaseRenderable;
class UUIBatchGeometryRenderable;
class UUIDirectMeshRenderable;
class ULGUIMeshComponent;
class UUIDrawcall;
class FUIPostProcessRenderProxy;
class UTextureRenderTarget2D;

#define LGUI_DEFAULT_MATERIAL_COUNT 3

/**
 * Canvas is for render and update all UI elements.
 */
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API ULGUICanvas : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULGUICanvas();
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
#if WITH_EDITOR
public:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostLoad()override;
#endif
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy)override;
private:
	/** clear drawcalls */
	void ClearDrawcall();
	void RemoveFromViewExtension();
	ULGUIMeshComponent* FindNextValidMeshInDrawcallList(int32 StartIndex);
public:
	/** mark canvas layout dirty */
	void MarkCanvasLayoutDirty();
	/**
	 * Mark update this Canvas. Canvas dont need to update every frame, only update when need to.
	 * Some rules if update could trigger drawcall's rebuild:
	 *		1. Commonly material & texture change and UI item's active state change
	 *		2. Transform & vertex position change, drawcall could overlap with eachother
	 *		3. Hierarchy order change, this is directly related to render order
	 * And about drawcall's rebuild, it's not actually force rebuild, it will check and reuse prev drawcall if possible.
	 * @param	bForceRebuildDrawcall	Mark it rebuild no matter what parameter change.
	 */
	void MarkCanvasUpdate(bool bMaterialOrTextureChanged, bool bTransformOrVertexPositionChanged, bool bHierarchyOrderChanged, bool bForceRebuildDrawcall = false);

	/** is point visible in Canvas. may not visible if use clip. texture clip just return true. rect clip will ignore feather value */
	bool IsPointVisible(FVector worldPoint);
	/** calculate rect clip range */
	void ConditionalCalculateRectRange();
	const FVector2D& GetClipRectMin() { ConditionalCalculateRectRange(); return clipRectMin; }
	const FVector2D& GetClipRectMax() { ConditionalCalculateRectRange(); return clipRectMax; }

	void BuildProjectionMatrix(FIntPoint InViewportSize, ECameraProjectionMode::Type InProjectionType, float FOV, FMatrix& OutProjectionMatrix)const;
	FMatrix GetViewProjectionMatrix()const;
	FMatrix GetProjectionMatrix()const;
	FVector GetViewLocation()const;
	FRotator GetViewRotator()const;
	FIntPoint GetViewportSize()const;
	/** get scale value of canvas. only valid for root canvas. */
	FORCEINLINE float GetCanvasScale()const { return canvasScale; }
private:
	friend class ULGUICanvasScaler;
	float canvasScale = 1.0f;//for screen space UI, screen size / root canvas size

	FDelegateHandle UIHierarchyChangedDelegateHandle;
	/** hierarchy changed */
	void OnUIHierarchyChanged();

	FDelegateHandle UIActiveStateChangedDelegateHandle;
	void OnUIActiveStateChanged();
public:
	/** get root LGUICanvas on hierarchy */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	ULGUICanvas* GetRootCanvas()const;
	bool IsRootCanvas()const;

	bool IsRenderToScreenSpace();
	bool IsRenderToRenderTarget();
	bool IsRenderToWorldSpace();
	bool IsRenderByLGUIRendererOrUERenderer();

	FORCEINLINE UUIItem* GetUIItem()const { CheckUIItem(); return UIItem.Get(); }
	bool GetIsUIActive()const;
	TWeakObjectPtr<ULGUICanvas> GetParentCanvas() { return ParentCanvas; }

	void SortDrawcall(int32& InOutRenderPriority);

	void SetParentCanvas(ULGUICanvas* InParentCanvas);
protected:
	/** Root LGUICanvas on hierarchy. LGUI's update start from the RootCanvas, and goes all down to every UI elements under it */
	UPROPERTY(Transient) mutable TWeakObjectPtr<ULGUICanvas> RootCanvas = nullptr;
	void CheckRenderMode();
	/** chekc RootCanvas. search for it if not valid */
	bool CheckRootCanvas()const;
	/** nearest up parent Canvas */
	UPROPERTY(Transient) TWeakObjectPtr<ULGUICanvas> ParentCanvas = nullptr;
	
	UMaterialInterface** GetMaterials();

	UPROPERTY(Transient) mutable TWeakObjectPtr<UUIItem> UIItem = nullptr;
	bool CheckUIItem()const;
protected:
	friend class FLGUICanvasCustomization;
	friend class FUIItemCustomization;

	ECameraProjectionMode::Type ProjectionType = ECameraProjectionMode::Perspective;
	float FOVAngle = 90;
	float NearClipPlane = GNearClippingPlane;
	float FarClipPlane = GNearClippingPlane;
	int32 EditorPreview_ViewIndex = 0;

	float CalculateDistanceToCamera()const;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUIRenderMode renderMode = ELGUIRenderMode::WorldSpace;
	/** Render to RenderTargets. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UTextureRenderTarget2D* renderTarget;
	/** This can avoid half-pixel render */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool pixelPerfect = false;
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
		int16 sortOrder = 0;

	/** 
	 * Clip content UI elements. 
	 * The best way to do clip is use stencil, but haven't find a way.
	 */
	UPROPERTY(EditAnywhere, Category = LGUI)
		ELGUICanvasClipType clipType = ELGUICanvasClipType::None;
	UPROPERTY(EditAnywhere, Category = LGUI)
		FVector2D clipFeather = FVector2D(4, 4);
	UPROPERTY(EditAnywhere, Category = LGUI)
		FMargin clipRectOffset = FMargin(0);
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (DisplayThumbnail = "false"))
		UTexture* clipTexture = nullptr;

	/** if inherit parent's rect clip value. only valid if self is RectClip */
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool inheritRectClip = true;

	/**
	 * By default canvas should render by up parent canvas (parent canvas will go up for render canvas too), if this set to true then canvas will render by itself.
	 * So by default, if canvas A is render by canvas B, when something need to update in canvas A, it still need to start from canvas B.
	 * This parameter is useful for the case:
	 *		Canvas A have something that need to update very frequently, then make this to true so canvas A can update and render by itself, without bothering canvas B.
	 */
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bForceSelfRender = false;

	/**
	 * The amount of pixels per unit to use for dynamically created bitmaps in the UI, such as UIText. 
	 * But!!! Do not set this value too large if you already have large font size of UIText, because that will result in extreamly large texture! 
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float dynamicPixelsPerUnit = 1.0f;

	/** Flags to enable/disable shader channels. Default only provide Position/Color/UV0, you can check Normal/Tangent/UV1/UV2/UV3 for your own use. */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (Bitmask, BitmaskEnum = "ELGUICanvasAdditionalChannelType"))
		int8 additionalShaderChannels = 0;

	/** Default materials, for render default UI elements. */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (DisplayThumbnail = "false"))
		UMaterialInterface* DefaultMaterials[LGUI_DEFAULT_MATERIAL_COUNT];

	/** For "World Space - LGUI Renderer" only, render with blend depth, 0-occlude by scene depth, 1-all visible. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float blendDepth = 0.0f;

	/** For not root canvas, inherit or override parent canvas parameters. */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (Bitmask, BitmaskEnum = "ELGUICanvasOverrideParameters"))
		int8 overrideParameters;

	/**
	 * LGUICanvas create mesh for render UI elements, this property can give us opportunity to use custom type of mesh for render.
	 * You can set "OwnerNoSee" "CastShadow" properties for your mesh.
	 * @todo: override this property from parent canvas?
	 */
	UPROPERTY(EditAnywhere, Category = LGUI, AdvancedDisplay, meta = (AllowAbstract = "true"))
		TSubclassOf<ULGUIMeshComponent> DefaultMeshType;

	FORCEINLINE FLinearColor GetRectClipOffsetAndSize();
	FORCEINLINE FLinearColor GetRectClipFeather();
	FORCEINLINE FLinearColor GetTextureClipOffsetAndSize();

	FORCEINLINE bool GetOverrideDefaultMaterials()const				{ return overrideParameters & (1 << 0); }
	FORCEINLINE bool GetOverridePixelPerfect()const					{ return overrideParameters & (1 << 1); }
	FORCEINLINE bool GetOverrideDynamicPixelsPerUnit()const			{ return overrideParameters & (1 << 2); }
	FORCEINLINE bool GetOverrideClipType()const						{ return overrideParameters & (1 << 3); }
	FORCEINLINE bool GetOverrideAddionalShaderChannel()const		{ return overrideParameters & (1 << 4); }
	FORCEINLINE bool GetOverrideBlendDepth()const					{ return overrideParameters & (1 << 5); }

public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		TArray<UMaterialInterface*> GetDefaultMaterials()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetDefaultMaterials(const TArray<UMaterialInterface*>& InMaterialArray);

	/** Set render mode of this canvas. This may not take effect if the canvas is not a root cnavas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRenderMode(ELGUIRenderMode value);
	/** Set pixel perfect of this canvas. This may not take effect if the canvas is not a root canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetPixelPerfect(bool value);
	/** Set parameters for calculating projection matrix. Only valid for ScreenSpace/RenderTarget mode. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetProjectionParameters(TEnumAsByte<ECameraProjectionMode::Type> InProjectionType, float InFovAngle, float InNearClipPlane, float InFarClipPlane);
	/** if renderMode is RenderTarget, then this will change the renderTarget */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRenderTarget(UTextureRenderTarget2D* value);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetForceSelfRender()const { return bForceSelfRender; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetClipType(ELGUICanvasClipType newClipType);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRectClipFeather(FVector2D newFeather);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRectClipOffset(FMargin newOffset);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetClipTexture(UTexture* newTexture);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetInheriRectClip(bool newBool);
	/** 
	 * Set LGUICanvas SortOrder
	 * @param	propagateToChildrenCanvas	if true, set this Canvas's SortOrder and all children Canvas, not just set absolute value, but keep child Canvas's relative order to this one
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrder(int32 newValue, bool propagateToChildrenCanvas = true);
	/** Set SortOrder to highest, so this canvas will render on top of all canvas that belong to same hierarchy. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrderToHighestOfHierarchy(bool propagateToChildrenCanvas = true);
	/** Set SortOrder to lowest, so this canvas will render behide all canvas that belong to same hierarchy. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrderToLowestOfHierarchy(bool propagateToChildrenCanvas = true);
	void GetMinMaxSortOrderOfHierarchy(int32& OutMin, int32& OutMax);

	/** Get actural render mode of canvas. Actually canvas's render mode property is inherit from parent canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUIRenderMode GetActualRenderMode()const;
	/** Get render mode of this canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUIRenderMode GetRenderMode()const { return renderMode; }
	/** Get pixel perfect of canvas. Actually canvas's pixel perfect property is inherit from parent canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetActualPixelPerfect()const;
	/** Get pixel perfect of this canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetPixelPerfect()const { return pixelPerfect; }
	/** Get render target of canvas if render mode is RenderTarget. Actually canvas's render target property is inherit from root canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		UTextureRenderTarget2D* GetActualRenderTarget()const;
	/** Get render target of this canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		UTextureRenderTarget2D* GetRenderTarget()const { return renderTarget; }

	/** Get blendDepth value of canvas. Actually canvas's blendDepth property is inherit from parent canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetActualBlendDepth()const;
	/** Get blendDepth value of this canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetBlendDepth()const { return blendDepth; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetBlendDepth(float value);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetOverrideSorting()const { return bOverrideSorting; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetOverrideSorting(bool value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		int32 GetSortOrder()const { return sortOrder; }
	/** Get SortOrder of this canvas. Actually canvas's SortOrder property may inherit from parent canvas depend on OverrideSorting property. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		int32 GetActualSortOrder()const;
	/** Get clip type of canvas. Actually canvas's clip type property is inherit from parent canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUICanvasClipType GetActualClipType()const;
	/** Get clip type of this canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUICanvasClipType GetClipType()const { return clipType; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FVector2D GetClipFeather()const { return clipFeather; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		UTexture* GetClipTexture()const { return clipTexture; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetInheritRectClip()const { return inheritRectClip; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetRequireNormal()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetRequireTangent()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetRequireUV1()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetRequireUV2()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetRequireUV3()const;
	/** Get the shader channel flags of canvas. Actually canvas's additional shader channel flags property is inherit from parent canvas. */
	int8 GetActualAdditionalShaderChannelFlags()const;
	/** return calculated additional-shaderchannel-flags, not just the property value, but take consider the overrideParameters */
	int8 GetAdditionalShaderChannelFlags()const { return additionalShaderChannels; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetActualDynamicPixelsPerUnit()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetDynamicPixelsPerUnit()const { return dynamicPixelsPerUnit; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetDynamicPixelsPerUnit(float newValue);

	int GetDrawcallCount()const { return UIDrawcallList.Num(); }

	/** Override LGUI's screen space UI render's camera location. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetOverrideViewLoation(bool InOverride, FVector InValue);
	/** Override LGUI's screen space UI render's camera rotation. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetOverrideViewRotation(bool InOverride, FRotator InValue);
	/**
	 * Override LGUI's screen space UI render's camera's fov in degree, will affect projection matrix.
	 * If SetOverrideProjectionMatrix is true, then this will not take effect.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetOverrideFovAngle(bool InOverride, float InValue);
	/**
	 * Override LGUI's screen space UI render's camera's projection matrix.
	 * If this is set to true, then SetOverrideFovAngle will not take effect.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetOverrideProjectionMatrix(bool InOverride, FMatrix InValue);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		TSubclassOf<ULGUIMeshComponent> GetDefaultMeshType()const { return DefaultMeshType; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetDefaultMeshType(TSubclassOf<ULGUIMeshComponent> InValue);

	void AddUIRenderable(UUIBaseRenderable* InUIRenderable);
	void RemoveUIRenderable(UUIBaseRenderable* InUIRenderable);
public:
	static FName LGUI_MainTextureMaterialParameterName;
	static FName LGUI_RectClipOffsetAndSize_MaterialParameterName;
	static FName LGUI_RectClipFeather_MaterialParameterName;
	static FName LGUI_TextureClip_MaterialParameterName;
	static FName LGUI_TextureClipOffsetAndSize_MaterialParameterName;
	bool IsMaterialContainsLGUIParameter(UMaterialInterface* InMaterial);
private:
	void SetSortOrderAdditionalValueRecursive(int32 InAdditionalValue);
public:
	/** Called from LGUIManagerActor */
	void UpdateCanvas();
	/** Check if any invalid in list. Currently use in editor after undo check. */
	void EnsureDrawcallObjectReference();
private:
	uint32 bHasAddToLGUIManager : 1;
	uint32 bClipTypeChanged:1;
	uint32 bRectClipParameterChanged:1;
	uint32 bTextureClipParameterChanged:1;

	uint32 bCanTickUpdate:1;//if Canvas can update from tick
	uint32 bShouldRebuildDrawcall : 1;
	uint32 bShouldSortRenderableOrder : 1;//if any renderable UIItem's hierarchy change, then we need to sort renderable list
	uint32 bRectRangeCalculated:1;
	uint32 bNeedToSortRenderPriority : 1;
	uint32 bHasAddToLGUIScreenSpaceRenderer : 1;//is this canvas added to LGUI screen space renderer
	uint32 bHasAddToLGUIWorldSpaceRenderer : 1;//is this canvas added to LGUI world space renderer
	/**
	 * RenderMode can affect UI's renderer, basically WorldSpace use UE's buildin renderer, others use LGUI's renderer. Different renderers cannot share render data.
	 * eg: when attach to other canvas, this will tell which render mode in old canvas, and if not compatible then recreate render data.
	 */
	uint32 bCurrentIsLGUIRendererOrUERenderer : 1;

	uint32 bOverrideViewLocation:1, bOverrideViewRotation:1, bOverrideProjectionMatrix:1, bOverrideFovAngle :1;

	mutable uint32 bIsViewProjectionMatrixDirty : 1;
	mutable FMatrix cacheViewProjectionMatrix = FMatrix::Identity;//cache to prevent multiple calculation in same frame

	FVector OverrideViewLocation;
	FRotator OverrideViewRotation;
	float OverrideFovAngle;
	FMatrix OverrideProjectionMatrix;

	UPROPERTY(Transient, VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
	TArray<TWeakObjectPtr<ULGUIMeshComponent>> PooledUIMeshList;//unuse UIMesh pool. PooledUIMeshList is managed by actual render canvas.
	UPROPERTY(Transient, VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
	TArray<TWeakObjectPtr<ULGUIMeshComponent>> UsingUIMeshList;//current using UIMesh list. UsingUIMeshList is managed by actual render canvas.
	UPROPERTY(Transient, VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
	TArray<FLGUIMaterialArrayContainer> PooledUIMaterialList;//Default material pool. PooledMaterialList is managed by self canvas, not the actual render canvas.
	TArray<TSharedPtr<UUIDrawcall>> UIDrawcallList;//Drawcall collection of this Canvas. UIDrawcallList is managed by actual render canvas.
	TArray<TSharedPtr<UUIDrawcall>> CacheUIDrawcallList;//Cached Drawcall collection. CacheUIDrawcallList is managed by actual render canvas.
	UPROPERTY(Transient, VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
	TArray<UUIItem*> UIRenderableList;//UIRenderableList is managed by self canvas, not the actual render canvas.

	/** rect clip's min position */
	FVector2D clipRectMin = FVector2D(0, 0);
	/** rect clip's max position */
	FVector2D clipRectMax = FVector2D(0, 0);

	TMap<UUIBaseRenderable*, FLGUICacheTransformContainer> CacheUIItemToCanvasTransformMap;//UI element relative to canvas transform
public:
	bool GetCacheUIItemToCanvasTransform(UUIBaseRenderable* item, bool createIfNotExist, FLGUICacheTransformContainer& outResult);
	/** Is this canvas renderred by other canvas? */
	bool IsRenderByOtherCanvas()const;
	/** If this canvas is renderred by other canvas, then use this to get that canvas. */
	ULGUICanvas* GetActualRenderCanvas()const;
private:
	FTransform2D ConvertTo2DTransform(const FTransform& Transform);
	void CalculateUIItem2DBounds(UUIBaseRenderable* item, const FTransform2D& transform, FVector2D& min, FVector2D& max);

	/** canvas array belong to this canvas in hierarchy. */
	UPROPERTY(Transient) TArray<TWeakObjectPtr<ULGUICanvas>> ChildrenCanvasArray;
	/** update Canvas's drawcall */
	void UpdateCanvasDrawcallRecursive();
	/** mark render finish */
	void MarkFinishRenderFrameRecursive();

	void UpdateGeometry_Implement();
	void UpdateDrawcall_Implement(ULGUICanvas* InRenderCanvas, TArray<TSharedPtr<UUIDrawcall>>& InUIDrawcallList, TArray<TSharedPtr<UUIDrawcall>>& InCacheUIDrawcallList, bool& OutNeedToSortRenderPriority);
	void UpdateDrawcallMesh_Implement();
	void UpdateDrawcallMaterial_Implement();
	static bool Is2DUITransform(const FTransform& Transform);
private:
	UMaterialInstanceDynamic* GetUIMaterialFromPool(ELGUICanvasClipType inClipType);
	void AddUIMaterialToPool(UMaterialInstanceDynamic* uiMat);
	TWeakObjectPtr<ULGUIMeshComponent> GetUIMeshFromPool();
	void AddUIMeshToPool(TWeakObjectPtr<ULGUIMeshComponent> InUIMesh);
};
