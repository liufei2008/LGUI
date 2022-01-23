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
		FVector position;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FColor color;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector2D uv0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", AdvancedDisplay)
		FVector2D uv1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", AdvancedDisplay)
		FVector2D uv2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", AdvancedDisplay)
		FVector2D uv3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", AdvancedDisplay)
		FVector normal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", AdvancedDisplay)
		FVector tagent;
};
/** a helper class for create LGUI geometry */
UCLASS(BlueprintType)
class LGUI_API ULGUICreateGeometryHelper : public UObject
{
	GENERATED_BODY()
public:
	TSharedPtr<UIGeometry> uiGeometry = nullptr;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void AddVertexSimple(FVector position, FColor color, FVector2D uv0);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void AddVertexFull(FVector position, FColor color, FVector2D uv0, FVector2D uv1, FVector2D uv2, FVector2D uv3, FVector normal, FVector tangent);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void AddVertexStruct(FLGUIGeometryVertex vertex);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void AddTriangle(int index0, int index1, int index2);
};
/** a helper class for update LGUI geometry */
UCLASS(BlueprintType)
class LGUI_API ULGUIUpdateGeometryHelper : public UObject
{
	GENERATED_BODY()
public:
	TSharedPtr<UIGeometry> uiGeometry = nullptr;
	/** do not midify this vertices array's size!!! */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "LGUI")
		TArray<FLGUIGeometryVertex> cacheVertices;
	/** do not midify this vertices array's size!!! */
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
	virtual bool HaveDataToCreateGeometry()override { return true; }
	virtual bool NeedTextureToCreateGeometry()override;
	virtual UTexture* GetTextureToCreateGeometry()override;

	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "NeedTextureToCreateGeometry"))
		bool ReceiveNeedTextureToCreateGeometry();
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
public:
	/** if vertex data change and vertex count not change. */
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DisplayName = "MarkVertexChanged"))
		void MarkVertexChanged();
	/** if vertex count change or triangle count change, call this */
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DisplayName = "MarkRebuildGeometry"))
		void MarkRebuildGeometry();
private:
	UPROPERTY(Transient)ULGUICreateGeometryHelper* createGeometryHelper = nullptr;
	UPROPERTY(Transient)ULGUIUpdateGeometryHelper* updateGeometryHelper = nullptr;
};
