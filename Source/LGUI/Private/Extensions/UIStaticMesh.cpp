// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Extensions/UIStaticMesh.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "StaticMeshResources.h"
#include "Rendering/ColorVertexBuffer.h"
#include "LGUI.h"
#include "Core/LGUIMesh/LGUIMeshComponent.h"
#include "Core/UIDrawcall.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Utils/LGUIUtils.h"

#define LOCTEXT_NAMESPACE "UIStaticMesh"

static void StaticMeshToLGUIMeshRenderData(const UStaticMesh* DataSource, TArray<FLGUIStaticMeshVertex>& OutVerts, TArray<uint32>& OutIndexes)
{
	const FStaticMeshLODResources& LOD = DataSource->GetRenderData()->LODResources[0];
	const int32 NumSections = LOD.Sections.Num();
	if (NumSections > 1)
	{
		auto WarningText = FText::Format(LOCTEXT("StaticMeshHasMultipleSections", "StaticMesh {0} has {1} sections. UIStaticMesh expects a static mesh with 1 section."), FText::FromString(DataSource->GetName()), NumSections);
#if WITH_EDITOR
		LGUIUtils::EditorNotification(WarningText, 10);
#endif
		UE_LOG(LGUI, Warning, TEXT("[%s].%d %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *WarningText.ToString());
		//@todo: support multiple sections
	}

	// Populate Vertex Data
	{
		const uint32 NumVerts = LOD.VertexBuffers.PositionVertexBuffer.GetNumVertices();
		OutVerts.Empty();
		OutVerts.Reserve(NumVerts);

		static const int32 MAX_SUPPORTED_UV_SETS = 4;
		const int32 TexCoordsPerVertex = LOD.GetNumTexCoords();
		if (TexCoordsPerVertex > MAX_SUPPORTED_UV_SETS)
		{
			auto WarningText = FText::Format(LOCTEXT("StaticMeshHasTooManyUVSets", "StaticMesh {0} has {1} UV sets; LGUI vertex data supports at most {2}."), FText::FromString(DataSource->GetName()), TexCoordsPerVertex, MAX_SUPPORTED_UV_SETS);
#if WITH_EDITOR
			LGUIUtils::EditorNotification(WarningText, 10);
#endif
			UE_LOG(LGUI, Warning, TEXT("[%s].%d %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *WarningText.ToString());
		}

		for (uint32 i = 0; i < NumVerts; ++i)
		{
			// Copy Position
			const FVector3f& Position = LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(i);

			// Copy Color
			FColor Color = (LOD.VertexBuffers.ColorVertexBuffer.GetNumVertices() > 0) ? LOD.VertexBuffers.ColorVertexBuffer.VertexColor(i) : FColor::White;

			// Copy all the UVs that we have, and as many as we can fit.
			const FVector2f& UV0 = (TexCoordsPerVertex > 0) ? LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(i, 0) : FVector2f(1, 1);

			const FVector2f& UV1 = (TexCoordsPerVertex > 1) ? LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(i, 1) : FVector2f(1, 1);

			const FVector2f& UV2 = (TexCoordsPerVertex > 2) ? LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(i, 2) : FVector2f(1, 1);

			const FVector2f& UV3 = (TexCoordsPerVertex > 3) ? LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(i, 3) : FVector2f(1, 1);

			const FVector3f TangentX = FVector3f(LOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentX(i));
			const FVector3f TangentZ = FVector3f(LOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(i));

			OutVerts.Add(FLGUIStaticMeshVertex(
				FVector(Position),
				FVector(TangentX),
				FVector(TangentZ),
				Color,
				FVector2D(UV0),
				FVector2D(UV1),
				FVector2D(UV2),
				FVector2D(UV3)
			));
		}
	}

	// Populate Index data
	{
		FIndexArrayView SourceIndexes = LOD.IndexBuffer.GetArrayView();
		const int32 NumIndexes = SourceIndexes.Num();
		OutIndexes.Empty();
		OutIndexes.Reserve(NumIndexes);
		for (int32 i = 0; i < NumIndexes; ++i)
		{
			OutIndexes.Add(SourceIndexes[i]);
		}


		// Sort the index buffer such that verts are drawn in Z-order.
		// Assume that all triangles are coplanar with Z == SomeValue.
		ensure(NumIndexes % 3 == 0);
		for (int32 a = 0; a < NumIndexes; a += 3)
		{
			for (int32 b = 0; b < NumIndexes; b += 3)
			{
				const float VertADepth = LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(OutIndexes[a]).Z;
				const float VertBDepth = LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(OutIndexes[b]).Z;
				if (VertADepth < VertBDepth)
				{
					// Swap the order in which triangles will be drawn
					Swap(OutIndexes[a + 0], OutIndexes[b + 0]);
					Swap(OutIndexes[a + 1], OutIndexes[b + 1]);
					Swap(OutIndexes[a + 2], OutIndexes[b + 2]);
				}
			}
		}
	}
}



const TArray<FLGUIStaticMeshVertex>& ULGUIStaticMeshCacheData::GetVertexData() const
{
	return VertexData;
}

const TArray<uint32>& ULGUIStaticMeshCacheData::GetIndexData() const
{
	return IndexData;
}

UMaterialInterface* ULGUIStaticMeshCacheData::GetMaterial() const
{
	return Material;
}

void ULGUIStaticMeshCacheData::EnsureValidData()
{
#if WITH_EDITORONLY_DATA
	if (IsValid(MeshAsset))
	{
		InitFromStaticMesh(MeshAsset);
	}
#endif
}

#include "UObject/ObjectSaveContext.h"
void ULGUIStaticMeshCacheData::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
}
#if WITH_EDITOR
void ULGUIStaticMeshCacheData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropName = Property->GetFName();
		if (
			PropName == GET_MEMBER_NAME_CHECKED(ULGUIStaticMeshCacheData, MeshAsset)
			)
		{
			if (IsValid(MeshAsset))
			{
				EnsureValidData();
			}
			else
			{
				ClearMeshData();
			}
		}
	}
}

void ULGUIStaticMeshCacheData::InitFromStaticMesh(const UStaticMesh* InSourceMesh)
{
	if (SourceMaterial != InSourceMesh->GetMaterial(0))
	{
		SourceMaterial = InSourceMesh->GetMaterial(0);
		Material = SourceMaterial;
	}

	ensureMsgf(Material != nullptr, TEXT("[%s].%d Expected %s to have a material assigned."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *InSourceMesh->GetFullName());

	StaticMeshToLGUIMeshRenderData(InSourceMesh, VertexData, IndexData);
	OnMeshDataChange.Broadcast();
}
void ULGUIStaticMeshCacheData::ClearMeshData()
{
	VertexData.Empty();
	IndexData.Empty();
}
#endif




UUIStaticMesh::UUIStaticMesh(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

#define ONE_DIVIDE_255 0.0039215686274509803921568627451f

void UUIStaticMesh::UpdateGeometry()
{
	if (GetIsUIActiveInHierarchy() == false)return;
	if (!RenderCanvas.IsValid())return;
	if (!IsValid(meshCache))return;

	Super::UpdateGeometry();

#if WITH_EDITOR
	if (!OnMeshDataChangeDelegateHandle.IsValid())
	{
		OnMeshDataChangeDelegateHandle = meshCache->OnMeshDataChange.AddUObject(this, &UUIStaticMesh::OnStaticMeshDataChange);
	}
#endif
	if (GetUIMesh() != nullptr && GetMeshSection() != nullptr)
	{
		if (bColorChanged)
		{
			UpdateMeshColor(false);
		}
		if (bLocalVertexPositionChanged || bTransformChanged)
		{
			UpdateMeshTransform(false);
		}
		if (bColorChanged || bLocalVertexPositionChanged || bTransformChanged)
		{
			drawcall->DrawcallMesh->UpdateMeshSectionRenderData(drawcall->DrawcallRenderSection.Pin(), bLocalVertexPositionChanged || bTransformChanged, RenderCanvas->GetActualAdditionalShaderChannelFlags());
			bColorChanged = false;
			bLocalVertexPositionChanged = false;
			bTransformChanged = false;
		}
	}
}

void UUIStaticMesh::UpdateMeshColor(bool updateToDrawcallMesh)
{
	if (vertexColorType == UIStaticMeshVertexColorType::NotAffectByUIColor)return;
	const auto& sourceVertexData = meshCache->GetVertexData();
	const auto& sourceIndexData = meshCache->GetIndexData();
	auto numVertices = sourceVertexData.Num();
	auto numIndices = sourceIndexData.Num();
	if (numVertices > 0 && numIndices > 0)
	{
		auto MeshSection = (FLGUIMeshSection*)drawcall->DrawcallRenderSection.Pin().Get();
		auto& VertexData = MeshSection->vertices;

		VertexData.SetNumUninitialized(numVertices);
		auto tempVertexColorType = vertexColorType;

		for (int i = 0; i < numVertices; i++)
		{
			auto& sourceVert = sourceVertexData[i];
			auto& vert = VertexData[i];
			vert.Position = FVector3f(sourceVert.Position);
			switch (tempVertexColorType)
			{
			case UIStaticMeshVertexColorType::MultiplyWithUIColor:
			{
				vert.Color = sourceVert.Color;
				auto uiFinalColor = GetFinalColor();
				vert.Color.R = (uint8)((float)vert.Color.R * uiFinalColor.R * ONE_DIVIDE_255);
				vert.Color.G = (uint8)((float)vert.Color.G * uiFinalColor.G * ONE_DIVIDE_255);
				vert.Color.B = (uint8)((float)vert.Color.B * uiFinalColor.B * ONE_DIVIDE_255);
				vert.Color.A = (uint8)((float)vert.Color.A * uiFinalColor.A * ONE_DIVIDE_255);
			}
			break;
			case UIStaticMeshVertexColorType::NotAffectByUIColor:
			{
				vert.Color = sourceVert.Color;
			}
			break;
			case UIStaticMeshVertexColorType::ReplaceByUIColor:
			{
				vert.Color = GetFinalColor();
			}
			break;
			}
		}
	}

	if (updateToDrawcallMesh)
	{
		drawcall->DrawcallMesh->UpdateMeshSectionRenderData(drawcall->DrawcallRenderSection.Pin(), false, RenderCanvas->GetActualAdditionalShaderChannelFlags());
	}
}
void UUIStaticMesh::CreateGeometry()
{
	const auto& sourceVertexData = meshCache->GetVertexData();
	const auto& sourceIndexData = meshCache->GetIndexData();
	auto numVertices = sourceVertexData.Num();
	auto numIndices = sourceIndexData.Num();
	if (numVertices > 0 && numIndices > 0)
	{
		auto MeshSection = (FLGUIMeshSection*)drawcall->DrawcallRenderSection.Pin().Get();
		auto& VertexData = MeshSection->vertices;

		VertexData.SetNumUninitialized(numVertices);
		bool needNormal = RenderCanvas->GetRequireNormal();
		bool needTangent = RenderCanvas->GetRequireTangent();
		bool needUV1 = RenderCanvas->GetRequireUV1();
		bool needUV2 = RenderCanvas->GetRequireUV2();
		bool needUV3 = RenderCanvas->GetRequireUV3();
		auto tempVertexColorType = vertexColorType;

		for (int i = 0; i < numVertices; i++)
		{
			auto& sourceVert = sourceVertexData[i];
			auto& vert = VertexData[i];
			vert.Position = FVector3f(sourceVert.Position);
			switch (tempVertexColorType)
			{
			case UIStaticMeshVertexColorType::MultiplyWithUIColor:
			{
				vert.Color = sourceVert.Color;
			auto uiFinalColor = GetFinalColor();
			vert.Color.R = (uint8)((float)vert.Color.R * uiFinalColor.R * ONE_DIVIDE_255);
			vert.Color.G = (uint8)((float)vert.Color.G * uiFinalColor.G * ONE_DIVIDE_255);
			vert.Color.B = (uint8)((float)vert.Color.B * uiFinalColor.B * ONE_DIVIDE_255);
			vert.Color.A = (uint8)((float)vert.Color.A * uiFinalColor.A * ONE_DIVIDE_255);
			}
			break;
			case UIStaticMeshVertexColorType::NotAffectByUIColor:
			{
				vert.Color = sourceVert.Color;
			}
			break;
			case UIStaticMeshVertexColorType::ReplaceByUIColor:
			{
				vert.Color = GetFinalColor();
			}
			break;
			}

			vert.TextureCoordinate[0] = FVector2f(sourceVert.UV0);
			if (needUV1)
			{
				vert.TextureCoordinate[1] = FVector2f(sourceVert.UV1);
			}
			if (needUV2)
			{
				vert.TextureCoordinate[2] = FVector2f(sourceVert.UV2);
			}
			if (needUV3)
			{
				vert.TextureCoordinate[3] = FVector2f(sourceVert.UV3);
			}

			if (needNormal)
			{
				vert.TangentZ = sourceVert.TangentZ;
			}
			if (needTangent)
			{
				vert.TangentZ = sourceVert.TangentX;
			}
		}

		auto& IndexData = MeshSection->triangles;
		IndexData.SetNumUninitialized(numIndices);
		for (int i = 0; i < numIndices; i++)
		{
			IndexData[i] = sourceIndexData[i];
		}
	}
	drawcall->DrawcallMesh->CreateRenderSectionRenderData(drawcall->DrawcallRenderSection.Pin());
	drawcall->bMaterialNeedToReassign = true;
	drawcall->bMaterialChanged = true;

	UpdateMeshTransform(true);

	MarkCanvasUpdate(true, false, false);
}

void UUIStaticMesh::UpdateMeshTransform(bool updateToDrawcallMesh)
{
	FTransform itemToCanvasTf;
	auto canvasUIItem = RenderCanvas->GetUIItem();
	auto inverseCanvasTf = canvasUIItem->GetComponentTransform().Inverse();
	const auto& itemTf = this->GetComponentTransform();
	FTransform::Multiply(&itemToCanvasTf, &itemTf, &inverseCanvasTf);

	auto MeshSection = (FLGUIMeshSection*)drawcall->DrawcallRenderSection.Pin().Get();

	const auto& sourceVertexData = meshCache->GetVertexData();
	auto numVertices = sourceVertexData.Num();
	{
		auto& VertexData = MeshSection->vertices;
		VertexData.SetNumUninitialized(numVertices);
		bool needNormal = RenderCanvas->GetRequireNormal();
		bool needTangent = RenderCanvas->GetRequireTangent();
		for (int i = 0; i < numVertices; i++)
		{
			auto& vert = VertexData[i];
			vert.Position = FVector3f(sourceVertexData[i].Position);
			if (needNormal)
			{
				vert.TangentZ = sourceVertexData[i].TangentZ;
			}
			if (needTangent)
			{
				vert.TangentX = sourceVertexData[i].TangentX;
			}
		}
	}

	auto& vertices = MeshSection->vertices;
	auto vertexCount = vertices.Num();
	for (int i = 0; i < vertexCount; i++)
	{
		vertices[i].Position = FVector3f(itemToCanvasTf.TransformPosition(FVector(vertices[i].Position)));
	}
	if (RenderCanvas->GetRequireNormal())
	{
		for (int i = 0; i < vertexCount; i++)
		{
			vertices[i].TangentZ = itemToCanvasTf.TransformVector(vertices[i].TangentZ.ToFVector());
			vertices[i].TangentZ.Vector.W = -127;
		}
	}
	if (RenderCanvas->GetRequireTangent())
	{
		for (int i = 0; i < vertexCount; i++)
		{
			vertices[i].TangentX = itemToCanvasTf.TransformVector(vertices[i].TangentX.ToFVector());
		}
	}

	if (updateToDrawcallMesh)
	{
		drawcall->DrawcallMesh->UpdateMeshSectionRenderData(drawcall->DrawcallRenderSection.Pin(), true, RenderCanvas->GetActualAdditionalShaderChannelFlags());
	}
}

#if WITH_EDITOR
void UUIStaticMesh::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);
	auto PropName = PropertyAboutToChange->GetFName();
	if (PropName == GET_MEMBER_NAME_CHECKED(UUIStaticMesh, meshCache))
	{
		if (IsValid(meshCache) && OnMeshDataChangeDelegateHandle.IsValid())
		{
			meshCache->OnMeshDataChange.Remove(OnMeshDataChangeDelegateHandle);
			OnMeshDataChangeDelegateHandle.Reset();
		}
	}
}
void UUIStaticMesh::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropName = Property->GetFName();
		if (
			PropName == GET_MEMBER_NAME_CHECKED(UUIStaticMesh, meshCache)
			|| PropName == GET_MEMBER_NAME_CHECKED(UUIStaticMesh, vertexColorType)
			)
		{
			if (IsValid(meshCache))
			{
				OnMeshDataChangeDelegateHandle = meshCache->OnMeshDataChange.AddUObject(this, &UUIStaticMesh::OnStaticMeshDataChange);
				if (drawcall.IsValid() && drawcall->DrawcallRenderSection.IsValid())
				{
					if (HaveValidData())
					{
						CreateGeometry();
					}
				}
			}
			else
			{
				ClearMeshData();
			}
		}
		else if (PropName == GET_MEMBER_NAME_CHECKED(UUIStaticMesh, ReplaceMaterial))
		{
			if (drawcall.IsValid() && drawcall->DrawcallRenderSection.IsValid())
			{
				if (HaveValidData())
				{
					CreateGeometry();
				}
			}
		}
	}
}
void UUIStaticMesh::OnStaticMeshDataChange()
{
	if (IsValid(meshCache))
	{
		if (drawcall.IsValid() && drawcall->DrawcallRenderSection.IsValid())
		{
			if (HaveValidData())
			{
				CreateGeometry();
			}
		}
	}
}
#endif

void UUIStaticMesh::OnMeshDataReady()
{
	Super::OnMeshDataReady();
	if (drawcall.IsValid() && drawcall->DrawcallRenderSection.IsValid())
	{
		if (HaveValidData())
		{
			CreateGeometry();
		}
	}
}

bool UUIStaticMesh::HaveValidData()const
{
	if (IsValid(meshCache))
	{
		return meshCache->GetVertexData().Num() > 0 && meshCache->GetIndexData().Num() > 0;
	}
	return false;
}

UMaterialInterface* UUIStaticMesh::GetMaterial()const
{
	if (IsValid(ReplaceMaterial))
	{
		return ReplaceMaterial;
	}
	else
	{
		return meshCache->GetMaterial();
	}
}

UMaterialInstanceDynamic* UUIStaticMesh::GetOrCreateDynamicMaterialInstance()
{
	UMaterialInterface* MaterialInstance = GetMaterial();
	UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(MaterialInstance);

	if (MaterialInstance && !MID)
	{
		// Create and set the dynamic material instance.
		MID = UMaterialInstanceDynamic::Create(MaterialInstance, this);
		SetReplaceMaterial(MID);
	}
	else if (!MaterialInstance)
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIStaticMesh::GetOrCreateDynamicMaterialInstance]Material is invalid on %s."), *GetPathName());
	}

	return MID;
}

void UUIStaticMesh::SetMesh(ULGUIStaticMeshCacheData* value)
{
	if (meshCache != value)
	{
		meshCache = value;
		if (IsValid(meshCache))
		{
			if (drawcall.IsValid() && drawcall->DrawcallRenderSection.IsValid())
			{
				if (HaveValidData())
				{
					CreateGeometry();
				}
			}
		}
	}
}

void UUIStaticMesh::SetReplaceMaterial(UMaterialInterface* value)
{
	if (ReplaceMaterial != value)
	{
		ReplaceMaterial = value;
		if (RenderCanvas.IsValid())
		{
			if (drawcall.IsValid())
			{
				drawcall->bMaterialChanged = true;
			}
			MarkCanvasUpdate(true, false, false);
		}
	}
}

void UUIStaticMesh::SetVertexColorType(UIStaticMeshVertexColorType value)
{
	if (vertexColorType != value)
	{
		vertexColorType = value;
		MarkColorDirty();
	}
}


AUIStaticMeshActor::AUIStaticMeshActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIStaticMesh = CreateDefaultSubobject<UUIStaticMesh>(TEXT("UIStaticMeshComponent"));
	RootComponent = UIStaticMesh;
}

#undef LOCTEXT_NAMESPACE
