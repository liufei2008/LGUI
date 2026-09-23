// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Core/LexUIMeshIndex.h"
#include "Core/LexUIMeshVertex.h"

enum class ELexUIGizmoMeshPrimitiveType
{
	Line, Triangle,
};

class LGUI_API FLexUIGizmoMesh : public TSharedFromThis<FLexUIGizmoMesh>
{
public:
	FLexUIGizmoMesh(){}
	FLexUIGizmoMesh(const TArray<FLexUIMeshVertex>& InVertexArray, const TArray<FLexUIMeshIndex>& InIndexArray, ELexUIGizmoMeshPrimitiveType InPrimitiveType);
	~FLexUIGizmoMesh();

	void UpdateVertices(TArray<FLexUIMeshVertex> InVertexArray);
	void UpdateIndices(TArray<FLexUIMeshIndex> InIndexArray);
	void SetColor(const FColor& InColor);
	void UpdateLocalBounds();
	void Render(TSharedPtr<class FLexUIRenderer> LexUIRenderer, bool ScreenSpaceOrWorldSpace);
	
	TStrongObjectPtr<UMaterialInterface> Material = nullptr;
	FMatrix LocalToWorldMatrix = FMatrix::Identity;
	FBoxSphereBounds LocalBounds = FBoxSphereBounds(EForceInit::ForceInit);
	ELexUIGizmoMeshPrimitiveType GetPrimitiveType()const { return PrimitiveType; }
	const FLexUIMeshVertexBuffer& GetVertexBuffer() { return VertexBuffer; }
	uint32 GetNumVertices()const { return VertexBuffer.Vertices.Num(); }
	const FLexUIMeshIndexBuffer& GetIndexBuffer() { return IndexBuffer; }
private:
	ELexUIGizmoMeshPrimitiveType PrimitiveType = ELexUIGizmoMeshPrimitiveType::Triangle;
	FLexUIMeshVertexBuffer VertexBuffer;
	/** Index buffer for this section */
	FLexUIMeshIndexBuffer IndexBuffer;
	/** cache current color to skip redundant vertex buffer update */
	FColor CurrentColor = FColor(0, 0, 0, 0);
	bool bCurrentColorValid = false;
	/** vertex color was changed before RHI init finished, update it after initialized */
	bool bNeedToUpdateAfterInitRHI = false;
};
