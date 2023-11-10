// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseRenderable.h"
#include "UIBatchGeometryRenderable.generated.h"

class UIGeometry;
class UMaterialInterface;

USTRUCT(BlueprintType)
struct LGUI_API FLGUIGeometryVertex
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector position = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FColor color = FColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector2D uv0 = FVector2D::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", AdvancedDisplay)
		FVector2D uv1 = FVector2D::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", AdvancedDisplay)
		FVector2D uv2 = FVector2D::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", AdvancedDisplay)
		FVector2D uv3 = FVector2D::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", AdvancedDisplay)
		FVector normal = FVector(-1, 0, 0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", AdvancedDisplay)
		FVector tangent = FVector(0, 1, 0);
};
/** a helper class for make LGUI geometry */
UCLASS(BlueprintType)
class LGUI_API ULGUIGeometryHelper : public UObject
{
	GENERATED_BODY()
public:
	UIGeometry* UIGeo = nullptr;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void AddVertexSimple(FVector position, FColor color, FVector2D uv0);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void AddVertexFull(FVector position, FColor color, FVector2D uv0, FVector2D uv1, FVector2D uv2, FVector2D uv3, FVector normal, FVector tangent);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void AddVertexStruct(FLGUIGeometryVertex vertex);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void AddTriangle(int index0, int index1, int index2);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMesh(const TArray<FLGUIGeometryVertex>& InVertices, const TArray<int>& InIndices);

	/**
	 * Remove vertices and triangle indices data, left the geometry empty.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void Clear();
	/**
	 * Add a list of triangles.
	 * @param	InVertexTriangleStream	Vertices to add, length should be divisible by 3.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void AddVertexTriangleStream(const TArray<FLGUIGeometryVertex>& InVertexTriangleStream);
	/**
	 * Get a stream of vertex in triangles.
	 * @param	OutVertexTriangleStream		Vertices stream, length is divisible by 3.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void GetVertexTriangleStream(TArray<FLGUIGeometryVertex>& OutVertexTriangleStream);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		static FVector2D CalculatePivotOffset(float InWidth, float InHeight, const FVector2D& InPivot);
};

/** UI element which have render geometry, and can be batched and renderred by LGUICanvas */
UCLASS(Abstract, Blueprintable, ClassGroup=(LGUI))
class LGUI_API UUIBatchGeometryRenderable : public UUIBaseRenderable
{
	GENERATED_BODY()

public:	
	UUIBatchGeometryRenderable(const FObjectInitializer& ObjectInitializer);

public:
	static const FName GetCustomUIMaterialPropertyName()
	{
		return GET_MEMBER_NAME_CHECKED(UUIBatchGeometryRenderable, CustomUIMaterial);
	}
protected:
	friend class FUIBatchGeometryRenderableCustomization;
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnRegister()override;
	virtual void OnUnregister()override;

	TSharedPtr<UIGeometry> geometry = nullptr;

	/** if have GeometryModifier component */
	bool HaveGeometryModifier(bool includeDisabled = true);
	/** Will any geometry modifier change these data? */
	void GeometryModifierWillChangeVertexData(bool& OutTriangleIndices, bool& OutVertexPosition, bool& OutUV, bool& OutColor);
	/** 
	 * use GeometryModifier to modify geometry 
	 */
	void ApplyGeometryModifier(bool triangleChanged, bool uvChanged, bool colorChanged, bool vertexPositionChanged);
	TInlineComponentArray<class UUIGeometryModifierBase*> GeometryModifierComponentArray;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UMaterialInterface* GetCustomUIMaterial()const { return CustomUIMaterial; }
	/** 
	 * if inMat is a UMaterialInstanceDynamic, then it will directly use for render.
	 * if not, then a new MaterialInstanceDynamic will be created to render this UI item, and the created MaterialInstanceDynamic may shared with others UI items.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetCustomUIMaterial(UMaterialInterface* inMat);
	/** 
	 * If CustomUIMaterial is a UMaterialInstanceDynamic, then will return it directly.
	 * If not, then return a created MaterialInstanceDynamic that renderring this UI item, may shared by other UI item. if this UI item is not renderred yet, then return nullptr.
	 * LGUI only create MaterialInstanceDynamic when specified material have one of these LGUI material parameter: [MainTexture, RectClipOffsetAndSize, RectClipFeather, ClipTexture, TextureClipOffsetAndSize].
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UMaterialInstanceDynamic* GetMaterialInstanceDynamic()const;
protected:
	virtual void OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache = true)override;
public:
	void MarkVertexPositionDirty();
	void MarkUVDirty();
	UE_DEPRECATED(4.24, "Use MarkVerticesDirty(true, true, true, true) instead")
		void MarkTriangleDirty() { MarkVerticesDirty(true, true, true, true); }
	virtual void MarkTextureDirty();
	virtual void MarkMaterialDirty();
	virtual void MarkVerticesDirty(bool InTriangleDirty, bool InVertexPositionDirty, bool InVertexUVDirty, bool InVertexColorDirty);

	/** 
	 * Mark vertices dirty, then LGUI will trigger UpdateGeometry process, and OnUpdateGeometry will executed in next render update.
	 * Call this if you want to update vertex data. 
	 * For blueprint easily use.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void MarkVerticesDirty();

	void AddGeometryModifier(class UUIGeometryModifierBase* InModifier);
	void RemoveGeometryModifier(class UUIGeometryModifierBase* InModifier);
	void SortGeometryModifier();

	virtual void MarkAllDirty()override;
	UIGeometry* GetGeometry()const { return geometry.Get(); }

	virtual bool LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)override;
protected:
	virtual bool LineTraceVisiblePixel(float InAlphaThreshold, FHitResult& OutHit, const FVector& Start, const FVector& End);
	virtual bool ReadPixelFromMainTexture(const FVector2D& InUV, FColor& OutPixel)const { return false; }
protected:
	friend class FUIGeometryRenderableCustomization;
	/** Use custom material to render this element */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayThumbnail = "false"))
		TObjectPtr<UMaterialInterface> CustomUIMaterial = nullptr;

	/** texture for render this UI element */
	virtual UTexture* GetTextureToCreateGeometry();
	/** material to render this UI element. if CustomUIMaterial is not valid, then use this material. */
	virtual UMaterialInterface* GetMaterialToCreateGeometry();

	/** do anything before acturally create or update geometry */
	virtual void OnBeforeCreateOrUpdateGeometry();
	/** fill and update ui geometry */
	virtual void OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);

	virtual void UpdateGeometry()override final;
	virtual void GetGeometryBoundsInLocalSpace(FVector2D& OutMinPoint, FVector2D& OutMaxPoint)const override;
#if WITH_EDITOR
	virtual void GetGeometryBounds3DInLocalSpace(FVector& OutMinPoint, FVector& OutMaxPoint)const override;
#endif

	/** texture for render this UI element */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "GetTextureToCreateGeometry"))
		UTexture* ReceiveGetTextureToCreateGeometry();
	/** material to render this UI element. if CustomUIMaterial is not valid, then use this material. */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "GetMaterialToCreateGeometry"))
		UMaterialInterface* ReceiveGetMaterialToCreateGeometry();
	/** do anything before acturally create or update geometry */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnBeforeCreateOrUpdateGeometry"))
		void ReceiveOnBeforeCreateOrUpdateGeometry();
	/** fill and update ui geometry */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnUpdateGeometry"))
		void ReceiveOnUpdateGeometry(ULGUIGeometryHelper* InGeometryHelper, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);

private:
	/** local space vertex position changed */
	uint8 bLocalVertexPositionChanged : 1;
	/** vertex's uv change */
	uint8 bUVChanged:1;
	/** triangle index change */
	uint8 bTriangleChanged:1;
	FVector2D LocalMinPoint = FVector2D(0, 0), LocalMaxPoint = FVector2D(0, 0);
#if WITH_EDITORONLY_DATA
	FVector LocalMinPoint3D = FVector::ZeroVector, LocalMaxPoint3D = FVector::ZeroVector;
#endif
	void CalculateLocalBounds();
	UPROPERTY(Transient)TObjectPtr<ULGUIGeometryHelper> GeometryHelper = nullptr;
};
