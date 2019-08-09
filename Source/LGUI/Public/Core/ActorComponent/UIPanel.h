// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "UIItem.h"
#include "UIMesh.h"
#include "Core/UIDrawcall.h"
#include "UIRenderable.h"
#include "Layout/Margin.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UIPanel.generated.h"

UENUM(BlueprintType)
enum class UIPanelClipType :uint8
{
	None		 		UMETA(DisplayName = "None"),
	Rect		 		UMETA(DisplayName = "Rect"),
	Texture				UMETA(DisplayName = "Texture"),
};

UENUM(BlueprintType, meta = (Bitflags))
enum class UIPanelAdditionalChannelType :uint8
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
ENUM_CLASS_FLAGS(UIPanelAdditionalChannelType)

UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent))
class LGUI_API UUIPanel : public UUIItem
{
	GENERATED_BODY()

public:	
	UUIPanel();
	void CustomTick(float DeltaTime);
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
#if WITH_EDITOR
public:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void EditorForceUpdateImmediately() override;
	virtual void PostLoad()override;
#endif
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
public:
	//update panel's geometry
	void UpdatePanelGeometry();
	//mark need update this UIPanel. UIPanel dont need to update every frame, only when need to
	FORCEINLINE void MarkNeedUpdate();

	//mark need to rebuild all drawcall
	void MarkRebuildAllDrawcall();
	//mark specific drawcall need to rebuild
	void MarkRebuildSpecificDrawcall(int drawcallIndex);
	//set specific drawcall's texture. eg when UIText's texture expend, just need to change the texture
	void SetDrawcallTexture(int drawcallIndex, UTexture* drawcallTexture, bool isFontTexture);
	//mark update specific drawcall vertex, when vertex position/uv/color etc change
	void MarkUpdateSpecificDrawcallVertex(int drawcallIndex, bool vertexPositionChanged = true);
	//insert a UI element into an existing drawcall. if all existing drawcall cannot fit in the element, create new drawcall.
	void InsertIntoDrawcall(UUIRenderable* item);
	//remove a UI element from drawcall list
	void RemoveFromDrawcall(UUIRenderable* item);
	//UI element's depth change
	void DepthChangeForDrawcall(UUIRenderable* item);
	//return created MaterialInstanceDynamic for target drawcall, return nullptr if drawcallIndex is -1
	UMaterialInstanceDynamic* GetMaterialInstanceDynamicForDrawcall(int drawcallIndex);
	
	//is point visible in UIPanel. may not visible if use clip. texture clip just return true. rect clip will ignore feather value
	bool IsPointVisible(FVector worldPoint);
public:
	//get top most UIPanel on hierarchy
	UUIPanel* GetFirstUIPanel();
	//hierarchy changed
	virtual void UIHierarchyChanged()override;

	void SetDefaultMaterials(UMaterialInterface* InMaterials[3]);
	void SetUIOnlyOwnerSee(bool InValue);
	void SetUIOwnerNoSee(bool InValue);

	//return SnapPixel of UIRoot. false if not have UIRoot
	FORCEINLINE bool ShouldSnapPixel();
	//up hierarchy to find panel which have UIRoot
	FORCEINLINE UUIPanel* GetUIRootPanel();
protected:
	//top most UIPanel on hierarchy. UIPanel's update start from the FirstUIPanel, and goes all down to every UI elements under it
	UPROPERTY(Transient) UUIPanel* FirstUIPanel = nullptr;
	//chekc first UIPanel is valid. if not, find it
	FORCEINLINE bool CheckFirstUIPanel();
	//nearest up parent UIPanel
	UPROPERTY(Transient) UUIPanel* ParentUIPanel = nullptr;
	//check parent UIpanel is valid. if not, find it
	FORCEINLINE bool CheckParentUIPanel();
	//sort drawcall
	void SortDrawcallRenderPriority();
	//@param	return	drawcall count
	int32 SortDrawcall(int32 InOutStartRenderPriority);
	
	//default materials, for render default UI elements
	UPROPERTY(EditAnywhere, Category = LGUI)
		UMaterialInterface* DefaultMaterials[3];
	void CheckMaterials();
	//check and find UIRoot
	void CheckUIRoot();
	UPROPERTY(Transient)class UUIRoot* uiRootComp = nullptr;

	virtual void ApplyUIActiveState()override;
protected:
	friend class FUIPanelCustomization;
	friend class FUIItemCustomization;
	UPROPERTY(EditAnywhere, Category = LGUI)
		UIPanelClipType clipType = UIPanelClipType::None;
	UPROPERTY(EditAnywhere, Category = LGUI)
		FVector2D clipFeather = FVector2D(4, 4);
	UPROPERTY(EditAnywhere, Category = LGUI)
		FMargin clipRectOffset = FMargin(0);
	UPROPERTY(EditAnywhere, Category = LGUI)
		UTexture* clipTexture = nullptr;
	//if inherit parent's rect clip value. only valid if self is RectClip
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool inheritRectClip = true;

	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bOwnerNoSee = false;
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bOnlyOwnerSee = false;

	//The amount of pixels per unit to use for dynamically created bitmaps in the UI, such as UIText. 
	//But!!! Do not set this value too large if you already have large font size of UIText, because that will result in extreamly large texture! 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float dynamicPixelsPerUnit = 1.0f;

	UPROPERTY(EditAnywhere, Category = LGUI, meta = (Bitmask, BitmaskEnum = "UIPanelAdditionalChannelType"))
		int8 additionalShaderChannels = 0;

	virtual void WidthChanged()override;
	virtual void HeightChanged()override;

	FORCEINLINE FLinearColor GetRectClipOffsetAndSize();
	FORCEINLINE FLinearColor GetRectClipFeather();
	FORCEINLINE FLinearColor GetTextureClipOffsetAndSize();

	FORCEINLINE virtual void MarkPanelUpdate()override;
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetClipType(UIPanelClipType newClipType);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRectClipFeather(FVector2D newFeather);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetClipTexture(UTexture* newTexture);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetInheriRectClip(bool newBool);
	/*
	Set panel render depth
	@param	propagateToChildrenPanel	if true, set this panel's depth and all panels that is attached to this panel, not just set absolute value, but keep child panel's relative depth to this panel
	*/
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetUIPanelDepth(int32 newDepth, bool propagateToChildrenPanel = true);
	//Set panel's depth to highest, so this panel will render on top of all panels that belong to a same UIPanel's hierarchy.
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetUIPanelDepthToHighestOfHierarchy(bool propagateToChildrenPanel = true);
	//Set panel's depth to lowest, so this panel will render behide of all panels that belong to a same UIPanel's hierarchy.
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetUIPanelDepthToLowestOfHierarchy(bool propagateToChildrenPanel = true);
	//Set panel's depth to highest of all panel, so this panel will render on top of all other panels no matter if it belongs to different hierarchy.
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetUIPanelDepthToHighestOfAll(bool propagateToChildrenPanel = true);
	//Set panel's depth to lowest of all panel, so this panel will render behide of all other panels no matter if it belongs to different hierarchy.
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetUIPanelDepthToLowestOfAll(bool propagateToChildrenPanel = true);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		UIPanelClipType GetClipType()const { return clipType; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FVector2D GetClipFeather()const { return clipFeather; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		UTexture* GetClipTexture()const { return clipTexture; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetInheritRectClip()const { return inheritRectClip; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetRequireNormal()const { return (additionalShaderChannels & (1 << 0)) != 0; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetRequireTangent()const { return (additionalShaderChannels & (1 << 1)) != 0; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetRequireUV1()const { return (additionalShaderChannels & (1 << 2)) != 0; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetRequireUV2()const { return (additionalShaderChannels & (1 << 3)) != 0; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetRequireUV3()const { return (additionalShaderChannels & (1 << 4)) != 0; }


	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetDynamicPixelsPerUnit() { return dynamicPixelsPerUnit; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetDynamicPixelsPerUnit(float newValue);
private:
	bool bClipTypeChanged = true;
	bool bRectClipParameterChanged = true;
	bool bTextureClipParameterChanged = true;
	//prev frame number, we can tell if we enter to a new render frame
	uint32 prevFrameNumber = 0;

	TArray<UUIRenderable*> UIRenderableItemList;//all renderable UI element collection
	TArray<UUIMesh*> UIMeshList;//UIMesh collection of this UIPanel
	TArray<UUIDrawcall*> UIDrawcallList;//Drawcall collection of this UIPanel
	UPROPERTY(Transient)TArray<UMaterialInstanceDynamic*> UIMaterialList;//material collection for UIMesh

	bool bCanTickUpdate = false;//if UIPanel can update from tick
	bool bShouldRebuildAllDrawcall = false;//if UIPanel need to rebuild all drawcall

	bool bRectRangeCalculated = false;
	//rect clip's min position
	FVector2D rectMin = FVector2D(0, 0);
	//rect clip's max position
	FVector2D rectMax = FVector2D(0, 0);
	//calculate rect range
	FORCEINLINE void CalculateRectRange();
private:
	void UpdateChildRecursive(UUIItem* target, bool parentTransformChanged, bool parentLayoutChanged);
	void UpdateAndApplyMaterial();
	void SetParameterForStandard(int drawcallCount);
	void SetParameterForRectClip(int drawcallCount);
	void SetParameterForTextureClip(int drawcallCount);
};
