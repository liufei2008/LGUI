// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LexUIMesh/LexUIGizmoMesh.h"

#include "Core/LexUIRender/LexUIRenderer.h"

FLexUIGizmoMesh::FLexUIGizmoMesh(const TArray<FLexUIMeshVertex>& InVertexArray, const TArray<FLexUIMeshIndex>& InIndexArray, ELexUIGizmoMeshPrimitiveType InPrimitiveType)
{
	PrimitiveType = InPrimitiveType;

	VertexBuffer.bAutoClearVerticesAfterInitRHI = false;
	auto& Vertices = VertexBuffer.Vertices;
	Vertices.SetNumUninitialized(InVertexArray.Num());
	FMemory::Memcpy(Vertices.GetData(), InVertexArray.GetData(), InVertexArray.Num() * sizeof(FLexUIMeshVertex));
	auto& Indices = IndexBuffer.Indices;
	Indices.SetNumUninitialized(InIndexArray.Num());
	FMemory::Memcpy(Indices.GetData(), InIndexArray.GetData(), InIndexArray.Num() * sizeof(FLexUIMeshIndex));

	// Enqueue initialization of render resource
	BeginInitResource(&IndexBuffer);
	BeginInitResource(&VertexBuffer);
}

FLexUIGizmoMesh::~FLexUIGizmoMesh()
{
	IndexBuffer.ReleaseResource();
	VertexBuffer.ReleaseResource();
}

void FLexUIGizmoMesh::UpdateVertices(TArray<FLexUIMeshVertex> InVertexArray)
{
	if (VertexBuffer.Vertices.Num() != InVertexArray.Num())
	{
		VertexBuffer.ReleaseResource();
		auto& Vertices = VertexBuffer.Vertices;
		Vertices.SetNumUninitialized(InVertexArray.Num());
		FMemory::Memcpy(Vertices.GetData(), InVertexArray.GetData(), InVertexArray.Num() * sizeof(FLexUIMeshVertex));
		BeginInitResource(&VertexBuffer);
	}
	else
	{
		//capture the RHI ref by value instead of this pointer, so the buffer stays valid even if this object is destroyed
		ENQUEUE_RENDER_COMMAND(FLexUIMeshUpdate)(
		[VertexBufferRHI = VertexBuffer.VertexBufferRHI, InVertexArray = MoveTemp(InVertexArray)](FRHICommandListImmediate& RHICmdList)
		{
			if (!VertexBufferRHI.IsValid())return;
			uint32 VertexDataLength = InVertexArray.Num() * sizeof(FLexUIMeshVertex);
			void* VertexBufferData = RHICmdList.LockBuffer(VertexBufferRHI, 0, VertexDataLength, RLM_WriteOnly);
			FMemory::Memcpy(VertexBufferData, InVertexArray.GetData(), VertexDataLength);
			RHICmdList.UnlockBuffer(VertexBufferRHI);
		});
	}
}

void FLexUIGizmoMesh::UpdateIndices(TArray<FLexUIMeshIndex> InIndexArray)
{
	if (IndexBuffer.Indices.Num() != InIndexArray.Num())
	{
		IndexBuffer.ReleaseResource();
		auto& Indices = IndexBuffer.Indices;
		Indices.SetNumUninitialized(InIndexArray.Num());
		FMemory::Memcpy(Indices.GetData(), InIndexArray.GetData(), InIndexArray.Num() * sizeof(FLexUIMeshIndex));
		BeginInitResource(&IndexBuffer);
	}
	else
	{
		//capture the RHI ref by value instead of this pointer, so the buffer stays valid even if this object is destroyed
		ENQUEUE_RENDER_COMMAND(FLexUIMeshUpdate)(
		[IndexBufferRHI = IndexBuffer.IndexBufferRHI, InIndexArray = MoveTemp(InIndexArray)](FRHICommandListImmediate& RHICmdList)
		{
			if (!IndexBufferRHI.IsValid())return;
			uint32 IndicesDataLength = InIndexArray.Num() * sizeof(FLexUIMeshIndex);
			auto IndexBufferData = RHICmdList.LockBuffer(IndexBufferRHI, 0, IndicesDataLength, RLM_WriteOnly);
			FMemory::Memcpy(IndexBufferData, InIndexArray.GetData(), IndicesDataLength);
			RHICmdList.UnlockBuffer(IndexBufferRHI);
		});
	}
}

void FLexUIGizmoMesh::SetColor(const FColor& InColor)
{
	if (bCurrentColorValid && CurrentColor == InColor)return;
	CurrentColor = InColor;
	bCurrentColorValid = true;
	for (auto& Vertex : VertexBuffer.Vertices)
	{
		Vertex.Color = InColor;
	}
	if (VertexBuffer.IsInitialized())
	{
		UpdateVertices(VertexBuffer.Vertices);
	}
	else
	{
		//InitRHI not finished yet, vertex buffer will be created from Vertices when initialized,
		//but color may keep changing, so mark it to update after initialization
		bNeedToUpdateAfterInitRHI = true;
	}
}

void FLexUIGizmoMesh::UpdateLocalBounds()
{
	FBox Box;
	for (const auto& Vertex : VertexBuffer.Vertices)
	{
		Box += FVector(Vertex.Position);
	}
	LocalBounds = FBoxSphereBounds(Box);
}

void FLexUIGizmoMesh::Render(TSharedPtr<FLexUIRenderer> LexUIRenderer, bool ScreenSpaceOrWorldSpace)
{
#if WITH_EDITOR
	if (bNeedToUpdateAfterInitRHI && VertexBuffer.IsInitialized())
	{
		bNeedToUpdateAfterInitRHI = false;
		UpdateVertices(VertexBuffer.Vertices);
	}
	if (ScreenSpaceOrWorldSpace)
	{
		LexUIRenderer->AddScreenSpaceGizmoMesh(SharedThis(this));
	}
	else
	{
		LexUIRenderer->AddWorldSpaceGizmoMesh(SharedThis(this));
	}
#endif
}
