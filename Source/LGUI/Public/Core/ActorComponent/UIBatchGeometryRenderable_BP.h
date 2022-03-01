// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIBatchGeometryRenderable.h"
#include "UIBatchGeometryRenderable_BP.generated.h"

class UIGeometry;

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
		FVector normal = FVector(1, 0, 0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", AdvancedDisplay)
		FVector tangent = FVector(0, 1, 0);
};
/** a helper class for create LGUI geometry */
UCLASS(BlueprintType)
class LGUI_API ULGUICreateGeometryHelper : public UObject
{
	GENERATED_BODY()
public:
	TWeakObjectPtr<UUIBatchGeometryRenderable> UIBatchGeometryRenderable = nullptr;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void AddVertexSimple(FVector position, FColor color, FVector2D uv0);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void AddVertexFull(FVector position, FColor color, FVector2D uv0, FVector2D uv1, FVector2D uv2, FVector2D uv3, FVector normal, FVector tangent);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void AddVertexStruct(FLGUIGeometryVertex vertex);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void AddTriangle(int index0, int index1, int index2);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetGeometry(const TArray<FLGUIGeometryVertex>& InVertices, const TArray<int>& InIndices);
};
/** a helper class for update LGUI geometry */
UCLASS(BlueprintType)
class LGUI_API ULGUIUpdateGeometryHelper : public UObject
{
	GENERATED_BODY()
public:
	TWeakObjectPtr<UUIBatchGeometryRenderable> UIBatchGeometryRenderable = nullptr;
	/** DO NOT modify this vertices array's size!!! DO NOT add or remove any element of this array. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "LGUI")
		TArray<FLGUIGeometryVertex> cacheVertices;
	/** DO NOT modify this vertices array's size!!! DO NOT add or remove any element of this array. */
	void BeginUpdateVertices();
	void EndUpdateVertices();

	UE_DEPRECATED(4.24, "No need to call this anymore.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DisplayName = "BeginUpdateVertices", DeprecatedFunction, DeprecationMessage = "No need to call this anymore."))
		void BeginUpdateVertices_BP() {};
	UE_DEPRECATED(4.24, "No need to call this anymore.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DisplayName = "EndUpdateVertices", DeprecatedFunction, DeprecationMessage = "No need to call this anymore."))
		void EndUpdateVertices_BP() {};
};

/** a wrapper class, blueprint can use this to create custom UI type */
UCLASS(ClassGroup = (LGUI), Abstract, Blueprintable)
class LGUI_API UUIBatchGeometryRenderable_BP : public UUIBatchGeometryRenderable
{
	GENERATED_BODY()

public:	
	UUIBatchGeometryRenderable_BP(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
protected:
	virtual UTexture* GetTextureToCreateGeometry()override;

	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "GetTextureToCreateGeometry"))
		UTexture* ReceiveGetTextureToCreateGeometry();

	virtual void OnBeforeCreateOrUpdateGeometry()override;
	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnBeforeCreateOrUpdateGeometry"))
		void ReceiveOnBeforeCreateOrUpdateGeometry();
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnCreateGeometry"))
		void ReceiveOnCreateGeometry(ULGUICreateGeometryHelper* InCreateGeometryHelper);
	/** update geometry data. Do Not add or remove any vertex or triangles in this function */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnUpdateGeometry"))
		void ReceiveOnUpdateGeometry(ULGUIUpdateGeometryHelper* InUpdateGoemetryHelper, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);
protected:
	UPROPERTY(Transient)ULGUICreateGeometryHelper* createGeometryHelper = nullptr;
	UPROPERTY(Transient)ULGUIUpdateGeometryHelper* updateGeometryHelper = nullptr;
};
