// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "UIRenderable.h"
#include "UIRenderable_BP.generated.h"

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
//a helper class for create LGUI geometry
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
//a helper class for update LGUI geometry
UCLASS(BlueprintType)
class LGUI_API ULGUIUpdateGeometryHelper : public UObject
{
	GENERATED_BODY()
public:
	TSharedPtr<UIGeometry> uiGeometry = nullptr;
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void BeginUpdateVertices(TArray<FLGUIGeometryVertex>& outVertices);
	//do not midify this vertices array's size!!!
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void EndUpdateVertices(UPARAM(ref) TArray< FLGUIGeometryVertex>& inVertices);
};

//a wrapper class, blueprint can use this to create custom UI type
UCLASS(ClassGroup = (LGUI), Abstract, Blueprintable)
class LGUI_API UUIRenderable_BP : public UUIRenderable
{
	GENERATED_BODY()

public:	
	UUIRenderable_BP(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
protected:
	virtual bool HaveDataToCreateGeometry()override { return true; }
	virtual bool NeedTextureToCreateGeometry()override { return false; }
	virtual UTexture* GetTextureToCreateGeometry()override { return nullptr; }

	virtual void OnBeforeCreateOrUpdateGeometry()override;
	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnBeforeCreateOrUpdateGeometry"))
		void OnBeforeCreateOrUpdateGeometry_BP();
	//
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnCreateGeometry"))
		void OnCreateGeometry_BP(ULGUICreateGeometryHelper* InCreateGeometryHelper);
	//update geometry data. Do Not add or remove any vertex or triangles in this function
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "OnUpdateGeometry"))
		void OnUpdateGeometry_BP(ULGUIUpdateGeometryHelper* InUpdateGoemetryHelper, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged);
public:
	//if vertex data change and vertex count not change.
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DisplayName = "MarkVertexChanged"))
		void MarkVertexChanged_BP();
	//if vertex count change or triangle count change, call this
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DisplayName = "MarkRebuildGeometry"))
		void MarkRebuildGeometry_BP();
private:
	UPROPERTY(Transient)ULGUICreateGeometryHelper* createGeometryHelper;
	UPROPERTY(Transient)ULGUIUpdateGeometryHelper* updateGeometryHelper;
};
