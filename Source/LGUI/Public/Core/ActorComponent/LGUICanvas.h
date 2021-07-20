// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "Layout/Margin.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraTypes.h"
#include "LGUICanvas.generated.h"

UENUM(BlueprintType, Category = LGUI)
enum class ELGUIRenderMode :uint8
{
	/**
	 * Render in screen space. There should be only one screen-space-ui-root in world.
	 * This mode use LGUI's custom render pipeline.
	 * This mode need a LGUICanvasScaler to control the size and scale.
	 */
	ScreenSpaceOverlay,
	/**
	 * Render in world space.
	 * This mode use engine's default render pieple, so post process will affect ui.
	 */
	WorldSpace,
	/**
	 * Render to a custom render target
	 */
	RenderTarget		UMETA(DisplayName = "RenderTarget(Experimental)"),
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
	OnlyOwnerSee,
	OwnerNoSee,
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
class UUIDrawcallMesh;
class UUIDrawcall;
class FUIPostProcessRenderProxy;
class UTextureRenderTarget2D;

/**
 * Canvas is for render and update all UI elements.
 */
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API ULGUICanvas : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULGUICanvas();
	/** Called from LGUIManagerActor */
	void UpdateCanvas(float DeltaTime);
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
	/** canvas array belong to this canvas, include self. for update children's geometry */
	TArray<TWeakObjectPtr<ULGUICanvas>> manageCanvasArray;
	/** for top most canvas only */
	void UpdateRootCanvas();
	/** update canvas's layout */
	void UpdateCanvasLayout(bool parentLayoutChanged);
	/** update Canvas's geometry */
	void UpdateCanvasGeometry();
	/** clear drawcalls */
	void ClearDrawcall();
public:
	/** mark update this Canvas. Canvas dont need to update every frame, only when need to */
	void MarkCanvasUpdate();
	/** mark any child's layout change */
	void MarkCanvasUpdateLayout();
	/** mark sort render priority when next frame update */
	void MarkSortRenderPriority();

	/** is point visible in Canvas. may not visible if use clip. texture clip just return true. rect clip will ignore feather value */
	bool IsPointVisible(FVector worldPoint);
	/** calculate rect clip range */
	void CalculateRectRange();
	FVector2D GetClipRectMin()const { return clipRectMin; }
	FVector2D GetClipRectMax()const { return clipRectMax; }

	void BuildProjectionMatrix(FIntPoint InViewportSize, ECameraProjectionMode::Type InProjectionType, float FOV, float InOrthoWidth, float InOrthoHeight, FMatrix& OutProjectionMatrix)const;
	FMatrix GetViewProjectionMatrix()const;
	FMatrix GetProjectionMatrix()const;
	FVector GetViewLocation()const;
	FMatrix GetViewRotationMatrix()const;
	FRotator GetViewRotator()const;
	FIntPoint GetViewportSize()const;
	/** get scale value of canvas. only valid for root canvas. */
	FORCEINLINE float GetCanvasScale()const { return canvasScale; }
private:
	friend class ULGUICanvasScaler;
	float canvasScale = 1.0f;//for screen space UI, screen size / root canvas size
public:
	/** get root LGUICanvas on hierarchy */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	ULGUICanvas* GetRootCanvas()const;
	bool IsRootCanvas()const;
	/** hierarchy changed */
	void OnUIHierarchyChanged();
	void OnUIHierarchyIndexChanged();
	void OnUIActiveStateChanged(bool active);

	void SetDefaultMaterials(UMaterialInterface* InMaterials[3]);

	bool IsRenderToScreenSpace();
	bool IsRenderToScreenSpaceOrRenderTarget();
	bool IsRenderToRenderTarget();
	bool IsRenderToWorldSpace();
	TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> GetViewExtension();

	FORCEINLINE UUIItem* GetUIItem()const { CheckUIItem(); return UIItem.Get(); }
	bool GetIsUIActive()const;
	TWeakObjectPtr<ULGUICanvas> GetParentCanvas() { CheckParentCanvas(); return ParentCanvas; }

	/** @return	drawcall count */
	int32 SortDrawcall(int32 InStartRenderPriority);
protected:
	/** consider this as a cache to IsRenderToScreenSpaceOrRenderTarget(). eg: when attach to other canvas, this will tell which render mode in old canvas */
	bool currentIsRenderToRenderTargetOrWorld = false;
	/** Root LGUICanvas on hierarchy. LGUI's update start from the RootCanvas, and goes all down to every UI elements under it */
	mutable TWeakObjectPtr<ULGUICanvas> RootCanvas = nullptr;
	void CheckRenderMode();
	/** chekc RootCanvas. search for it if not valid */
	bool CheckRootCanvas()const;
	/** nearest up parent Canvas */
	TWeakObjectPtr<ULGUICanvas> ParentCanvas = nullptr;
	/** check parent Canvas. search for it if not valid */
	bool CheckParentCanvas();
	const TArray<TWeakObjectPtr<ULGUICanvas>>& GetAllCanvasArray();
	
	UMaterialInterface** GetMaterials();

	mutable TWeakObjectPtr<UUIItem> UIItem = nullptr;
	bool CheckUIItem()const;

	TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> ViewExtension;
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
	/** Canvas with larger order will render on top of lower one */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int32 sortOrder = 0;

	/** Clip content UI elements. */
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
		UMaterialInterface* DefaultMaterials[3];

	/** Just like StaticMesh's OwnerNoSee property, only valid on world space UI. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ownerNoSee = false;
	/** Just like StaticMesh's OnlyOwnerSee property, only valid on world space UI. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool onlyOwnerSee = false;

	/** For not root canvas, inherit or override parent canvas parameters. */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (Bitmask, BitmaskEnum = "ELGUICanvasOverrideParameters"))
		int8 overrideParameters;

	FORCEINLINE FLinearColor GetRectClipOffsetAndSize();
	FORCEINLINE FLinearColor GetRectClipFeather();
	FORCEINLINE FLinearColor GetTextureClipOffsetAndSize();

	FORCEINLINE bool GetOverrideDefaultMaterials()const				{ return overrideParameters & (1 << 0); }
	FORCEINLINE bool GetOverridePixelPerfect()const					{ return overrideParameters & (1 << 1); }
	FORCEINLINE bool GetOverrideDynamicPixelsPerUnit()const			{ return overrideParameters & (1 << 2); }
	FORCEINLINE bool GetOverrideClipType()const						{ return overrideParameters & (1 << 3); }
	FORCEINLINE bool GetOverrideAddionalShaderChannel()const		{ return overrideParameters & (1 << 4); }
	FORCEINLINE bool GetOverrideOwnerNoSee()const					{ return overrideParameters & (1 << 5); }
	FORCEINLINE bool GetOverrideOnlyOwnerSee()const					{ return overrideParameters & (1 << 6); }
public:
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
	 * @param	propagateToChildrenCanvas	if true, set this Canvas's SortOrder and all Canvases that is attached to this Canvas, not just set absolute value, but keep child Canvas's relative order to this one
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrder(int32 newValue, bool propagateToChildrenCanvas = true);
	/** Set SortOrder to highest, so this canvas will render on top of all canvas that belong to same hierarchy. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrderToHighestOfHierarchy(bool propagateToChildrenCanvas = true);
	/** Set SortOrder to lowest, so this canvas will render behide all canvas that belong to same hierarchy. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrderToLowestOfHierarchy(bool propagateToChildrenCanvas = true);
	/** Set SortOrder to highest of all Canvas, so this canvas will render on top of all other canvas no matter if it belongs to different hierarchy. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrderToHighestOfAll(bool propagateToChildrenCanvas = true);
	/** Set SortOrder to lowest of all Canvas, so this canvas will render behide all other canvas no matter if it belongs to different hierarchy. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrderToLowestOfAll(bool propagateToChildrenCanvas = true);

	/** Get actural render mode of canvas. Actually canvas's render mode property is inherit from root canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUIRenderMode GetActualRenderMode()const;
	/** Get render mode of this canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUIRenderMode GetRenderMode()const { return renderMode; }
	/** Get pixel perfect of canvas. Actually canvas's pixel perfect property is inherit from root canvas. */
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

	/** Get OwnerNoSee of this canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetOwnerNoSee()const { return ownerNoSee; }
	/** Get OnlyOwnerSee of this canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetOnlyOwnerSee()const { return onlyOwnerSee; }
	/** Get OwnerNoSee of this canvas. Actually canvas's OwnerNoSee property is inherit from root canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetActualOwnerNoSee()const;
	/** Get OnlyOwnerSee of this canvas. Actually canvas's OnlyOwnerSee property is inherit from root canvas. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetActualOnlyOwnerSee()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetOwnerNoSee(bool value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetOnlyOwnerSee(bool value);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		int32 GetSortOrder()const { return sortOrder; }
	/** Get clip type of canvas. Actually canvas's clip type property is inherit from root canvas. */
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
	/** Get the shader channel flags of canvas. Actually canvas's additional shader channel flags property is inherit from root canvas. */
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

	void AddUIRenderable(UUIBaseRenderable* InUIRenderable);
	void RemoveUIRenderable(UUIBaseRenderable* InUIRenderable);
private:
	void CombineDrawcall();
	void ApplyOwnerSeeRecursive();
private:
	uint8 bClipTypeChanged:1;
	uint8 bRectClipParameterChanged:1;
	uint8 bTextureClipParameterChanged:1;

	uint8 bCanTickUpdate:1;//if Canvas can update from tick
	uint8 bShouldUpdateLayout:1;//if any child layout changed
	uint8 bRectRangeCalculated:1;
	uint8 bNeedToSortRenderPriority : 1;

	uint8 cacheForThisUpdate_ShouldUpdateLayout:1
		, cacheForThisUpdate_ClipTypeChanged:1, cacheForThisUpdate_RectClipParameterChanged:1, cacheForThisUpdate_TextureClipParameterChanged:1;
	/** prev frame number, we can tell if we enter to a new render frame */
	uint32 prevFrameNumber = 0;

	mutable uint32 cacheViewProjectionMatrixFrameNumber = 0;
	mutable FMatrix cacheViewProjectionMatrix = FMatrix::Identity;//cache to prevent multiple calculation in same frame

	TArray<TWeakObjectPtr<UUIDrawcallMesh>> PooledUIMeshList;//UIDrawcallMesh pool
	UPROPERTY(Transient) TArray<FLGUIMaterialArrayContainer> PooledUIMaterialList;//Default material pool
	TDoubleLinkedList<TSharedPtr<UUIDrawcall>> UIDrawcallList;//Drawcall collection of this Canvas

	/** rect clip's min position */
	FVector2D clipRectMin = FVector2D(0, 0);
	/** rect clip's max position */
	FVector2D clipRectMax = FVector2D(0, 0);

	TMap<UUIItem*, FLGUICacheTransformContainer> CacheUIItemToCanvasTransformMap;//UI element relative to canvas transform
#ifdef LGUI_DRAWCALLMODE_AUTO
public:
	bool GetCacheUIItemToCanvasTransform(UUIItem* item, bool createIfNotExist, FLGUICacheTransformContainer& outResult);
private:
	FTransform2D ConvertTo2DTransform(const FTransform& Transform);
	void CalculateUIItem2DBounds(UUIItem* item, const FTransform2D& transform, FVector2D& min, FVector2D& max);
	void GetMinMax(float a, float b, float c, float d, float& min, float& max);
#endif
private:
	void UpdateChildRecursive(UUIItem* target, bool parentLayoutChanged);
	void UpdateAndApplyMaterial();
	void SetParameterForStandard();
	void SetParameterForRectClip();
	void SetParameterForTextureClip();
	UMaterialInstanceDynamic* GetUIMaterialFromPool(ELGUICanvasClipType inClipType);
	void AddUIMaterialToPool(UMaterialInstanceDynamic* uiMat);
	void AddUIMeshToPool(UUIDrawcallMesh* uiMesh);
};
