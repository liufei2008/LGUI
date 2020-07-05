// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "Layout/Margin.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraTypes.h"
#include "LGUICanvas.generated.h"

UENUM(BlueprintType)
enum class ELGUIRenderMode :uint8
{
	//render in screen space. 
	//this mode need a LGUICanvasScaler to control the size and scale.
	ScreenSpaceOverlay,
	//render in world space, so post process effect will affect ui
	WorldSpace,
	//render to a custom render target
	RenderTarget		UMETA(DisplayName = "RenderTarget(Experimental)"),
};

UENUM(BlueprintType)
enum class ELGUICanvasClipType :uint8
{
	None		 		UMETA(DisplayName = "None"),
	Rect		 		UMETA(DisplayName = "Rect"),
	Texture				UMETA(DisplayName = "Texture"),
};

UENUM(BlueprintType, meta = (Bitflags))
enum class ELGUICanvasAdditionalChannelType :uint8
{
	//Lit shader may need this
	Normal,
	//Lit and normalMap may need this
	Tangent,
	//Additional textureCoordinate at uv1(first uv is uv0, which is used by LGUI. Second uv is uv1)
	UV1,
	UV2,
	UV3,
};
ENUM_CLASS_FLAGS(ELGUICanvasAdditionalChannelType)

UENUM(BlueprintType, meta = (Bitflags))
enum class ELGUICanvasOverrideParameters :uint8
{
	DefaltMaterials,
	PixelPerfect,
	DynamicPixelsPerUnit,
	ClipType,
	AdditionalShaderChannels,
};
ENUM_CLASS_FLAGS(ELGUICanvasOverrideParameters);

class UUIItem;
class UUIRenderable;
class UTextureRenderTarget2D;

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API ULGUICanvas : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULGUICanvas();
	void CustomTick(float DeltaTime);
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
public:
	//update Canvas's geometry
	void UpdateCanvasGeometry();
	//mark update this Canvas. Canvas dont need to update every frame, only when need to
	void MarkCanvasUpdate();

	//mark need to rebuild all drawcall
	void MarkRebuildAllDrawcall();
	//mark specific drawcall need to rebuild
	void MarkRebuildSpecificDrawcall(int drawcallIndex);
	//set specific drawcall's texture. eg when UIText's texture expend, just need to change the texture
	void SetDrawcallTexture(int drawcallIndex, UTexture* drawcallTexture);
	//mark update specific drawcall vertex, when vertex position/uv/color etc change
	void MarkUpdateSpecificDrawcallVertex(int drawcallIndex, bool vertexPositionChanged = true);
	//insert a UI element into an existing drawcall. if all existing drawcall cannot fit in the element, create new drawcall.
	void InsertIntoDrawcall(UUIRenderable* item);
	//remove a UI element from drawcall list
	void RemoveFromDrawcall(UUIRenderable* item);
	//UI element's depth change
	void OnUIElementDepthChange(UUIRenderable* item);
	//return created MaterialInstanceDynamic for target drawcall, return nullptr if drawcallIndex is -1
	UMaterialInstanceDynamic* GetMaterialInstanceDynamicForDrawcall(int drawcallIndex);
	
	//is point visible in Canvas. may not visible if use clip. texture clip just return true. rect clip will ignore feather value
	bool IsPointVisible(FVector worldPoint);

	void BuildProjectionMatrix(FIntPoint InViewportSize, ECameraProjectionMode::Type InProjectionType, float FOV, float InOrthoWidth, float InOrthoHeight, FMatrix& OutProjectionMatrix);
	FMatrix GetViewProjectionMatrix();
	FMatrix GetProjectionMatrix();
	FVector GetViewLocation();
	FMatrix GetViewRotationMatrix();
	FRotator GetViewRotator();
	FIntPoint GetViewportSize();
public:
	//get top most LGUICanvas on hierarchy
	ULGUICanvas* GetRootCanvas();
	bool IsRootCanvas()const;
	//hierarchy changed
	void OnUIHierarchyChanged();
	void OnWidthChanged();
	void OnHeightChanged();
	void OnUIActiveStateChange(bool active);

	void SetDefaultMaterials(UMaterialInterface* InMaterials[3]);

	UE_DEPRECATED(4.23, "Use IsRenderToScreenSpace instead.")
	bool IsScreenSpaceOverlayUI();
	FORCEINLINE bool IsRenderToScreenSpace();
	FORCEINLINE bool IsRenderToScreenSpaceOrRenderTarget();
	FORCEINLINE bool IsRenderToRenderTarget();
	FORCEINLINE bool IsRenderToWorldSpace();
	FORCEINLINE TSharedPtr<class FLGUIViewExtension, ESPMode::ThreadSafe> GetViewExtension();

	FORCEINLINE UUIItem* CheckAndGetUIItem() { CheckUIItem(); return UIItem; }
protected:
	//consider this as a cache to IsScreenSpaceOverlayUI(). eg: when attach to other canvas, this will tell which render mode in old canvas
	bool currentIsRenderToRenderTargetOrWorld = false;
	//top most LGUICanvas on hierarchy. LGUI's update start from the TopMostCanvas, and goes all down to every UI elements under it
	UPROPERTY(Transient) ULGUICanvas* TopMostCanvas = nullptr;
	//chekc TopMostCanvas. search for it if not valid
	FORCEINLINE bool CheckTopMostCanvas();
	//nearest up parent Canvas
	UPROPERTY(Transient) ULGUICanvas* ParentCanvas = nullptr;
	//check parent Canvas. search for it if not valid
	FORCEINLINE bool CheckParentCanvas();
	//sort drawcall
	void SortDrawcallRenderPriority();
	//@param	return	drawcall count
	int32 SortDrawcall(int32 InStartRenderPriority);
	
	UMaterialInterface** GetMaterials();

	UPROPERTY(Transient)UUIItem* UIItem = nullptr;
	bool CheckUIItem();

	TSharedPtr<class FLGUIViewExtension, ESPMode::ThreadSafe> ViewExtension;
protected:
	friend class FLGUICanvasCustomization;
	friend class FUIItemCustomization;

	ECameraProjectionMode::Type ProjectionType = ECameraProjectionMode::Perspective;
	float FOVAngle = 90;
	float NearClipPlane = GNearClippingPlane;
	float FarClipPlane = GNearClippingPlane;

	float CalculateDistanceToCamera();

	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUIRenderMode renderMode = ELGUIRenderMode::WorldSpace;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UTextureRenderTarget2D* renderTarget;
	//This can avoid half-pixel render
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool pixelPerfect = false;
	//Canvas with larger order will render on top of lower one
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int32 sortOrder = 0;

	UPROPERTY(EditAnywhere, Category = LGUI)
		ELGUICanvasClipType clipType = ELGUICanvasClipType::None;
	UPROPERTY(EditAnywhere, Category = LGUI)
		FVector2D clipFeather = FVector2D(4, 4);
	UPROPERTY(EditAnywhere, Category = LGUI)
		FMargin clipRectOffset = FMargin(0);
	UPROPERTY(EditAnywhere, Category = LGUI)
		UTexture* clipTexture = nullptr;

	//if inherit parent's rect clip value. only valid if self is RectClip
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool inheritRectClip = true;

	//The amount of pixels per unit to use for dynamically created bitmaps in the UI, such as UIText. 
	//But!!! Do not set this value too large if you already have large font size of UIText, because that will result in extreamly large texture! 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float dynamicPixelsPerUnit = 1.0f;

	UPROPERTY(EditAnywhere, Category = LGUI, meta = (Bitmask, BitmaskEnum = "ELGUICanvasAdditionalChannelType"))
		int8 additionalShaderChannels = 0;

	//default materials, for render default UI elements
	UPROPERTY(EditAnywhere, Category = LGUI)
		UMaterialInterface* DefaultMaterials[3];

	//for not root canvas, inherit or override parent canvas parameters
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
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRenderMode(ELGUIRenderMode value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetPixelPerfect(bool value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetProjectionParameters(TEnumAsByte<ECameraProjectionMode::Type> InProjectionType, float InFovAngle, float InNearClipPlane, float InFarClipPlane);
	//if renderMode is RenderTarget, then this will change the renderTarget
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRenderTarget(UTextureRenderTarget2D* value);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetClipType(ELGUICanvasClipType newClipType);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRectClipFeather(FVector2D newFeather);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetClipTexture(UTexture* newTexture);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetInheriRectClip(bool newBool);
	/*
	Set LGUICanvas SortOrder
	@param	propagateToChildrenCanvas	if true, set this Canvas's SortOrder and all Canvases that is attached to this Canvas, not just set absolute value, but keep child Canvas's relative order to this one
	*/
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrder(int32 newValue, bool propagateToChildrenCanvas = true);
	//Set SortOrder to highest, so this canvas will render on top of all canvas that belong to same hierarchy.
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrderToHighestOfHierarchy(bool propagateToChildrenCanvas = true);
	//Set SortOrder to lowest, so this canvas will render behide all canvas that belong to same hierarchy.
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrderToLowestOfHierarchy(bool propagateToChildrenCanvas = true);
	//Set SortOrder to highest of all Canvas, so this canvas will render on top of all other canvas no matter if it belongs to different hierarchy.
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrderToHighestOfAll(bool propagateToChildrenCanvas = true);
	//Set SortOrder to lowest of all Canvas, so this canvas will render behide all other canvas no matter if it belongs to different hierarchy.
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSortOrderToLowestOfAll(bool propagateToChildrenCanvas = true);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUIRenderMode GetRenderMode()const { return renderMode; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetPixelPerfect()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		UTextureRenderTarget2D* GetRenderTarget()const { return renderTarget; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		int32 GetSortOrder()const { return sortOrder; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUICanvasClipType GetClipType()const;
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


	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetDynamicPixelsPerUnit()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetDynamicPixelsPerUnit(float newValue);

	//return UIRenderables that belong to this canvas
	const TArray<UUIRenderable*> GetUIRenderables()const { return UIRenderableItemList; }
	int GetDrawcallCount()const { return UIDrawcallList.Num(); }
private:
	int8 GetAdditionalShaderChannelFlags()const;
private:
	bool bClipTypeChanged = true;
	bool bRectClipParameterChanged = true;
	bool bTextureClipParameterChanged = true;
	//prev frame number, we can tell if we enter to a new render frame
	uint32 prevFrameNumber = 0;

	uint32 cacheViewProjectionMatrixFrameNumber = 0;
	FMatrix cacheViewProjectionMatrix = FMatrix::Identity;//cache to prevent multiple calculation in same frame

	UPROPERTY(Transient)TArray<UUIRenderable*> UIRenderableItemList;//all renderable UI element collection
	UPROPERTY(Transient)TArray<class UUIDrawcallMesh*> UIMeshList;//UIDrawcallMesh collection of this Canvas
	TArray<TSharedPtr<class UUIDrawcall>> UIDrawcallList;//Drawcall collection of this Canvas
	UPROPERTY(Transient)TArray<UMaterialInstanceDynamic*> UIMaterialList;//material collection for UIDrawcallMesh

	bool bCanTickUpdate = false;//if Canvas can update from tick
	bool bShouldRebuildAllDrawcall = false;//if Canvas need to rebuild all drawcall

	bool bRectRangeCalculated = false;
	//rect clip's min position
	FVector2D rectMin = FVector2D(0, 0);
	//rect clip's max position
	FVector2D rectMax = FVector2D(0, 0);
	//calculate rect range
	void CalculateRectRange();
private:
	void UpdateChildRecursive(UUIItem* target, bool parentTransformChanged, bool parentLayoutChanged);
	void UpdateAndApplyMaterial();
	void SetParameterForStandard(int drawcallCount);
	void SetParameterForRectClip(int drawcallCount);
	void SetParameterForTextureClip(int drawcallCount);
};
