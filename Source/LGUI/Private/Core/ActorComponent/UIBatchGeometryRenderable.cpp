// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Utils/LGUIUtils.h"
#include "GeometryModifier/UIGeometryModifierBase.h"
#include "Core/LGUIMesh/LGUIMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Core/UIDrawcall.h"
#include "Core/Actor/LGUIManagerActor.h"

DECLARE_CYCLE_STAT(TEXT("UIBatchGeometryRenderable GeometryModifier"), STAT_ApplyModifier, STATGROUP_LGUI);

UUIBatchGeometryRenderable::UUIBatchGeometryRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	UIRenderableType = EUIRenderableType::UIBatchGeometryRenderable;
	geometry = TSharedPtr<UIGeometry>(new UIGeometry);

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	bTriangleChanged = true;
}

void UUIBatchGeometryRenderable::BeginPlay()
{
	Super::BeginPlay();

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	bTriangleChanged = true;
}

void UUIBatchGeometryRenderable::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

#if WITH_EDITOR
void UUIBatchGeometryRenderable::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{

	}
}
#endif
void UUIBatchGeometryRenderable::OnRegister()
{
	Super::OnRegister();
}
void UUIBatchGeometryRenderable::OnUnregister()
{
	Super::OnUnregister();
}

void UUIBatchGeometryRenderable::OnAnchorChange(bool InPivotChange, bool InSizeChange, bool InDiscardCache)
{
    Super::OnAnchorChange(InPivotChange, InSizeChange, InDiscardCache);
	if (InPivotChange || InSizeChange)
    {
        MarkVertexPositionDirty();
    }
}

void UUIBatchGeometryRenderable::MarkVerticesDirty()
{
	bTriangleChanged = true;
	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	bColorChanged = true;
	MarkCanvasUpdate(false, false, false, true);
}

void UUIBatchGeometryRenderable::MarkVerticesDirty(bool InTriangleDirty, bool InVertexPositionDirty, bool InVertexUVDirty, bool InVertexColorDirty)
{
	bTriangleChanged = bTriangleChanged || InTriangleDirty;
	bLocalVertexPositionChanged = bLocalVertexPositionChanged || InVertexPositionDirty;
	bUVChanged = bUVChanged || InVertexUVDirty;
	bColorChanged = bColorChanged || InVertexColorDirty;
	MarkCanvasUpdate(false, bLocalVertexPositionChanged, false);
}

void UUIBatchGeometryRenderable::MarkVertexPositionDirty()
{
	MarkVerticesDirty(false, true, false, false);
}
void UUIBatchGeometryRenderable::MarkUVDirty()
{
	MarkVerticesDirty(false, false, true, false);
}

void UUIBatchGeometryRenderable::MarkTextureDirty()
{
	if (RenderCanvas.IsValid())
	{
		if (drawcall.IsValid())
		{
			geometry->texture = GetTextureToCreateGeometry();
			drawcall->bTextureChanged = true;
		}
		MarkCanvasUpdate(true, false, false);
	}
}
void UUIBatchGeometryRenderable::MarkMaterialDirty()
{
	if (RenderCanvas.IsValid())
	{
		if (drawcall.IsValid())
		{
			geometry->material = GetMaterialToCreateGeometry();
			drawcall->bMaterialChanged = true;
		}
		MarkCanvasUpdate(true, false, false);
	}
}

void UUIBatchGeometryRenderable::AddGeometryModifier(class UUIGeometryModifierBase* InModifier)
{
	auto Index = GeometryModifierComponentArray.AddUnique(InModifier);
	if (Index > 0)
	{
		SortGeometryModifier();
	}
	MarkVerticesDirty(true, true, true, true);
}
void UUIBatchGeometryRenderable::RemoveGeometryModifier(class UUIGeometryModifierBase* InModifier)
{
	GeometryModifierComponentArray.Remove(InModifier);
	MarkVerticesDirty(true, true, true, true);
}
void UUIBatchGeometryRenderable::SortGeometryModifier()
{
	GeometryModifierComponentArray.Sort([](const UUIGeometryModifierBase& A, const UUIGeometryModifierBase& B) {
		return A.GetExecuteOrder() < B.GetExecuteOrder();
		});
	MarkVerticesDirty(true, true, true, true);
}

void UUIBatchGeometryRenderable::MarkAllDirty()
{
	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	bTriangleChanged = true;
	if (RenderCanvas.IsValid())
	{
		if (drawcall.IsValid())
		{
			geometry->texture = GetTextureToCreateGeometry();
			drawcall->bTextureChanged = true;

			geometry->material = GetMaterialToCreateGeometry();
			drawcall->bMaterialChanged = true;
		}
		MarkCanvasUpdate(true, false, false);
	}
	Super::MarkAllDirty();
}
void UUIBatchGeometryRenderable::SetCustomUIMaterial(UMaterialInterface* inMat)
{
	if (CustomUIMaterial != inMat)
	{
		CustomUIMaterial = inMat;
		MarkMaterialDirty();
	}
}

UMaterialInstanceDynamic* UUIBatchGeometryRenderable::GetMaterialInstanceDynamic()const
{
	if (IsValid(CustomUIMaterial))
	{
		if (CustomUIMaterial->IsA(UMaterialInstanceDynamic::StaticClass()))
		{
			return (UMaterialInstanceDynamic*)CustomUIMaterial;//if CustomUIMaterial is a MaterialInstanceDynamic then just return it directly
		}
	}
	if (drawcall.IsValid() && drawcall->RenderMaterial.IsValid() && drawcall->bMaterialContainsLGUIParameter)
	{
		return (UMaterialInstanceDynamic*)drawcall->RenderMaterial.Get();
	}
	return nullptr;
}
bool UUIBatchGeometryRenderable::HaveGeometryModifier(bool includeDisabled)
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(GeometryModifierComponentArray, false);
		GeometryModifierComponentArray.Sort([](const UUIGeometryModifierBase& A, const UUIGeometryModifierBase& B) {
			return A.GetExecuteOrder() < B.GetExecuteOrder();
			});
	}
#endif
	if (includeDisabled)
	{
		return GeometryModifierComponentArray.Num() > 0;
	}
	else
	{
		int32 enabledCount = 0;
		if (GeometryModifierComponentArray.Num() > 0)
		{
			for (auto& Item : GeometryModifierComponentArray)
			{
				if (Item->GetEnable())
				{
					enabledCount++;
				}
			}
		}
		return enabledCount > 0;
	}
	return false;
}

void UUIBatchGeometryRenderable::GeometryModifierWillChangeVertexData(bool& OutTriangleIndices, bool& OutVertexPosition, bool& OutUV, bool& OutColor)
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(GeometryModifierComponentArray, false);
		GeometryModifierComponentArray.Sort([](const UUIGeometryModifierBase& A, const UUIGeometryModifierBase& B) {
			return A.GetExecuteOrder() < B.GetExecuteOrder();
			});
	}
#endif

	int count = GeometryModifierComponentArray.Num();
	if (count > 0)
	{
		for (int i = 0; i < count; i++)
		{
			auto modifierComp = GeometryModifierComponentArray[i];
			if (modifierComp->GetEnable())
			{
				bool TempTriangleIndices = false, TempVertexPosition = false, TempUV = false, TempColor = false;
				modifierComp->ModifierWillChangeVertexData(TempTriangleIndices, TempVertexPosition, TempUV, TempColor);
				if (TempTriangleIndices)OutTriangleIndices = true;
				if (TempVertexPosition)OutVertexPosition = true;
				if (TempUV)OutUV = true;
				if (TempColor)OutColor = true;
			}
		}
	}
}

void UUIBatchGeometryRenderable::ApplyGeometryModifier(bool triangleChanged, bool uvChanged, bool colorChanged, bool vertexPositionChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_ApplyModifier);

	int count = GeometryModifierComponentArray.Num();
	if (count > 0)
	{
		for (int i = 0; i < count; i++)
		{
			auto modifierComp = GeometryModifierComponentArray[i];
			if (modifierComp->GetEnable())
			{
				modifierComp->ModifyUIGeometry(*(geometry.Get()), triangleChanged, uvChanged, colorChanged, vertexPositionChanged);
			}
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("UIBatchGeometryRenderable UpdateGeometry"), STAT_UpdateGeometry, STATGROUP_LGUI);
void UUIBatchGeometryRenderable::UpdateGeometry()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateGeometry);

	Super::UpdateGeometry();

	OnBeforeCreateOrUpdateGeometry();
	if (!drawcall.IsValid()//not add to render yet
		)
	{
		geometry->Clear();
		geometry->texture = GetTextureToCreateGeometry();
		geometry->material = GetMaterialToCreateGeometry();
		OnUpdateGeometry(*(geometry.Get()), true, true, true, true);
		ApplyGeometryModifier(true, true, true, true);
		CalculateLocalBounds();//CalculateLocalBounds must stay before TransformVertices, because TransformVertices will also cache bounds for Canvas to check 2d overlap.
		UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry.Get());
	}
	else//if geometry is created, update data
	{
		if (bTriangleChanged || bLocalVertexPositionChanged || bColorChanged || bUVChanged)
		{
			geometry->Clear();
			//check if GeometryModifier will affect vertex data, if so we need to update these data in OnUpdateGeometry
			{
				bool TempTriangleIndices = false, TempVertexPosition = false, TempUV = false, TempColor = false;
				GeometryModifierWillChangeVertexData(TempTriangleIndices, TempVertexPosition, TempUV, TempColor);
				if (TempTriangleIndices)bTriangleChanged = true;
				if (TempVertexPosition)bLocalVertexPositionChanged = true;
				if (TempUV)bUVChanged = true;
				if (TempColor)bColorChanged = true;
			}
			OnUpdateGeometry(*(geometry.Get()), bTriangleChanged, bLocalVertexPositionChanged, bUVChanged, bColorChanged);
			ApplyGeometryModifier(bTriangleChanged, bUVChanged, bColorChanged, bLocalVertexPositionChanged);
			drawcall->bNeedToUpdateVertex = true;
			if (bLocalVertexPositionChanged)
			{
				CalculateLocalBounds();//CalculateLocalBounds must stay before TransformVertices, because TransformVertices will also cache bounds for Canvas to check 2d overlap.
			}
		}
		if (bLocalVertexPositionChanged || bTransformChanged)
		{
			UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry.Get());
			drawcall->bNeedToUpdateVertex = true;
		}
	}
	if (geometry->vertices.Num() >= LGUI_MAX_VERTEX_COUNT)
	{
		auto errorMsg = FText::Format(NSLOCTEXT("UIBatchGeometryRenderable", "TooManyTrianglesInSingleDdrawcall", "[UUIBatchGeometryRenderable::UpdateGeometry] Too many vertex ({0}) in single UI element: {1}")
			, geometry->vertices.Num()
#if WITH_EDITOR
			, FText::FromString(this->GetOwner()->GetActorLabel())
#else
			, FText::FromString(this->GetPathName())
#endif
		);
#if WITH_EDITOR
		LGUIUtils::EditorNotification(errorMsg, 10);
#endif
		UE_LOG(LGUI, Error, TEXT("%s"), *errorMsg.ToString());
	}

	bTriangleChanged = false;
	bLocalVertexPositionChanged = false;
	bUVChanged = false;
	bColorChanged = false;
	bTransformChanged = false;
}

bool UUIBatchGeometryRenderable::LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)
{
	if (RaycastType == EUIRenderableRaycastType::Rect)
	{
		return Super::LineTraceUI(OutHit, Start, End);
	}
	else if (RaycastType == EUIRenderableRaycastType::Geometry)
	{
		return LineTraceUIGeometry(geometry.Get(), OutHit, Start, End);
	}
	else
	{
		return LineTraceUICustom(OutHit, Start, End);
	}
}

void UUIBatchGeometryRenderable::CalculateLocalBounds()
{
	auto& originVertices = geometry->originVertices;
	float horizontalMin = MAX_flt, horizontalMax = -MAX_flt;
	float verticalMin = MAX_flt, verticalMax = -MAX_flt;
#if WITH_EDITOR
	float forwardMin = MAX_flt, forwardMax = -MAX_flt;
#endif
	if (originVertices.Num() == 0)
	{
		horizontalMin = horizontalMax = verticalMin = verticalMax = 0;
#if WITH_EDITOR
		forwardMin = forwardMax = 0;
#endif
	}
	else
	{
		for (auto& Vert : originVertices)
		{
			auto& VertPos = Vert.Position;
			if (VertPos.Y < horizontalMin)
			{
				horizontalMin = VertPos.Y;
			}
			if (VertPos.Y > horizontalMax)
			{
				horizontalMax = VertPos.Y;
			}
			if (VertPos.Z < verticalMin)
			{
				verticalMin = VertPos.Z;
			}
			if (VertPos.Z > verticalMax)
			{
				verticalMax = VertPos.Z;
			}
#if WITH_EDITOR
			if (VertPos.X < forwardMin)
			{
				forwardMin = VertPos.X;
			}
			if (VertPos.X > forwardMax)
			{
				forwardMax = VertPos.X;
			}
#endif
		}
	}
	this->LocalMinPoint = FVector2D(horizontalMin, verticalMin);
	this->LocalMaxPoint = FVector2D(horizontalMax, verticalMax);
#if WITH_EDITOR
	this->LocalMinPoint3D = FVector(forwardMin, horizontalMin, verticalMin);
	this->LocalMaxPoint3D = FVector(forwardMax, horizontalMax, verticalMax);
#endif
}

void UUIBatchGeometryRenderable::GetGeometryBoundsInLocalSpace(FVector2D& OutMinPoint, FVector2D& OutMaxPoint)const
{
	OutMinPoint = this->LocalMinPoint;
	OutMaxPoint = this->LocalMaxPoint;
}

#if WITH_EDITOR
void UUIBatchGeometryRenderable::GetGeometryBounds3DInLocalSpace(FVector& OutMinPoint, FVector& OutMaxPoint)const
{
	OutMinPoint = this->LocalMinPoint3D;
	OutMaxPoint = this->LocalMaxPoint3D;
}
#endif

UTexture* UUIBatchGeometryRenderable::GetTextureToCreateGeometry()
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		return ReceiveGetTextureToCreateGeometry();
	}
	return nullptr;
}

UMaterialInterface* UUIBatchGeometryRenderable::GetMaterialToCreateGeometry()
{
	if (IsValid(CustomUIMaterial))
	{
		return CustomUIMaterial;
	}
	else
	{
		if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
		{
			return ReceiveGetMaterialToCreateGeometry();
		}
	}
	return nullptr;
}

void UUIBatchGeometryRenderable::OnBeforeCreateOrUpdateGeometry()
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnBeforeCreateOrUpdateGeometry();
	}
}

DECLARE_CYCLE_STAT(TEXT("UIBatchGeometryRenderable Blueprint.OnUpdateGeometry"), STAT_BatchGeometryRenderable_OnUpdateGeometry, STATGROUP_LGUI);
void UUIBatchGeometryRenderable::OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		if (!IsValid(GeometryHelper))
		{
			GeometryHelper = NewObject<ULGUIGeometryHelper>(this);
		}
		GeometryHelper->UIGeo = &InGeo;
		SCOPE_CYCLE_COUNTER(STAT_BatchGeometryRenderable_OnUpdateGeometry);
		ReceiveOnUpdateGeometry(GeometryHelper, InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
	}
}



void ULGUIGeometryHelper::AddVertexSimple(FVector position, FColor color, FVector2D uv0)
{
#if !UE_BUILD_SHIPPING
	if (position.ContainsNaN()
		|| uv0.ContainsNaN()
		)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::AddVertexFull]Vertex data contains NaN!."));
		return;
	}
#endif
	auto& originVertices = UIGeo->originVertices;
	originVertices.Add(position);
	auto& vertices = UIGeo->vertices;
	FLGUIMeshVertex vert(FVector::ZeroVector);
	vert.Color = color;
	vert.TextureCoordinate[0] = uv0;
	vertices.Add(vert);
}
void ULGUIGeometryHelper::AddVertexFull(FVector position, FColor color, FVector2D uv0, FVector2D uv1, FVector2D uv2, FVector2D uv3, FVector normal, FVector tangent)
{
#if !UE_BUILD_SHIPPING
	if (position.ContainsNaN()
		|| normal.ContainsNaN()
		|| tangent.ContainsNaN()
		|| uv0.ContainsNaN()
		|| uv1.ContainsNaN()
		|| uv2.ContainsNaN()
		|| uv3.ContainsNaN()
		)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::AddVertexFull]Vertex data contains NaN!."));
		return;
	}
#endif
	auto& originVertices = UIGeo->originVertices;
	originVertices.Add(FLGUIOriginVertexData(position, normal, tangent));
	auto& vertices = UIGeo->vertices;
	FLGUIMeshVertex vert(FVector::ZeroVector);
	vert.Color = color;
	vert.TextureCoordinate[0] = uv0;
	vert.TextureCoordinate[1] = uv1;
	vert.TextureCoordinate[2] = uv2;
	vert.TextureCoordinate[3] = uv3;
	vertices.Add(vert);
}
void ULGUIGeometryHelper::AddVertexStruct(FLGUIGeometryVertex vertex)
{
#if !UE_BUILD_SHIPPING
	if (vertex.position.ContainsNaN()
		|| vertex.normal.ContainsNaN()
		|| vertex.tangent.ContainsNaN()
		|| vertex.uv0.ContainsNaN()
		|| vertex.uv1.ContainsNaN()
		|| vertex.uv2.ContainsNaN()
		|| vertex.uv3.ContainsNaN()
		)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::AddVertexStruct]Vertex data contains NaN!."));
		return;
	}
#endif
	auto& originVertices = UIGeo->originVertices;
	originVertices.Add(FLGUIOriginVertexData(vertex.position, vertex.normal, vertex.tangent));
	auto& vertices = UIGeo->vertices;
	FLGUIMeshVertex vert(FVector::ZeroVector);
	vert.Color = vertex.color;
	vert.TextureCoordinate[0] = vertex.uv0;
	vert.TextureCoordinate[1] = vertex.uv1;
	vert.TextureCoordinate[2] = vertex.uv2;
	vert.TextureCoordinate[3] = vertex.uv3;
	vertices.Add(vert);
}
void ULGUIGeometryHelper::AddTriangle(int index0, int index1, int index2)
{
#if !UE_BUILD_SHIPPING
	int vertCount = UIGeo->vertices.Num();
	if (index0 >= vertCount || index1 >= vertCount || index2 >= vertCount)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::AddTriangle]Triangle index reference out of vertex range."));
		return;
	}
#endif
	auto& triangles = UIGeo->triangles;
	triangles.Reserve(triangles.Num() + 3);
	triangles.Add(index0);
	triangles.Add(index1);
	triangles.Add(index2);
}
void ULGUIGeometryHelper::SetGeometry(const TArray<FLGUIGeometryVertex>& InVertices, const TArray<int>& InIndices)
{
	int vertCount = InVertices.Num();
#if !UE_BUILD_SHIPPING
	for (auto& i : InIndices)
	{
		if (i >= vertCount)
		{
			UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::SetGeometry]Triangle index reference out of vertex range."));
			return;
		}
	}
	if ((InIndices.Num() % 3) != 0)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::SetGeometry]Indices count must be multiple of 3."));
		return;
	}
	for (auto& vertex : InVertices)
	{
		if (vertex.position.ContainsNaN()
			|| vertex.normal.ContainsNaN()
			|| vertex.tangent.ContainsNaN()
			|| vertex.uv0.ContainsNaN()
			|| vertex.uv1.ContainsNaN()
			|| vertex.uv2.ContainsNaN()
			|| vertex.uv3.ContainsNaN()
			)
		{
			UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::SetGeometry]Vertex data contains NaN!."));
			break;
		}
	}
#endif
	auto& triangles = UIGeo->triangles;
	triangles.SetNumUninitialized(InIndices.Num());
	for (int i = 0; i < InIndices.Num(); i++)
	{
		triangles[i] = InIndices[i];
	}

	auto& vertices = UIGeo->vertices;
	auto& originVertices = UIGeo->originVertices;
	vertices.SetNumUninitialized(vertCount);
	originVertices.SetNumUninitialized(vertCount);

	for (int i = 0; i < vertCount; i++)
	{
		auto& originVert = InVertices[i];
		originVertices[i] = FLGUIOriginVertexData(originVert.position, originVert.normal, originVert.tangent);
		auto& vert = vertices[i];
		vert.Color = originVert.color;
		vert.TextureCoordinate[0] = originVert.uv0;
		vert.TextureCoordinate[1] = originVert.uv1;
		vert.TextureCoordinate[2] = originVert.uv2;
		vert.TextureCoordinate[3] = originVert.uv3;
	}
}

void ULGUIGeometryHelper::AddVertexTriangleStream(const TArray<FLGUIGeometryVertex>& InVertexTriangleStream)
{
#if !UE_BUILD_SHIPPING
	if ((InVertexTriangleStream.Num() % 3) != 0)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::AddVertexTriangleStream]Indices count must be multiple of 3."));
		return;
	}
	for (auto& vertex : InVertexTriangleStream)
	{
		if (vertex.position.ContainsNaN()
			|| vertex.normal.ContainsNaN()
			|| vertex.tangent.ContainsNaN()
			|| vertex.uv0.ContainsNaN()
			|| vertex.uv1.ContainsNaN()
			|| vertex.uv2.ContainsNaN()
			|| vertex.uv3.ContainsNaN()
			)
		{
			UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::AddVertexTriangleStream]Vertex data contains NaN!."));
			return;
		}
	}
#endif
	auto& triangles = UIGeo->triangles;
	auto& vertices = UIGeo->vertices;
	auto& originVertices = UIGeo->originVertices;
	auto vertCount = vertices.Num();
	triangles.Reserve(InVertexTriangleStream.Num());
	for (int i = 0; i < InVertexTriangleStream.Num(); i++)
	{
		triangles.Add(vertCount + i);
	}

	vertices.Reserve(InVertexTriangleStream.Num());
	originVertices.Reserve(InVertexTriangleStream.Num());

	for (int i = 0; i < InVertexTriangleStream.Num(); i++)
	{
		auto& originVert = InVertexTriangleStream[i];
		originVertices.Add(FLGUIOriginVertexData(originVert.position, originVert.normal, originVert.tangent));
		FLGUIMeshVertex vert(FVector::ZeroVector);
		vert.Color = originVert.color;
		vert.TextureCoordinate[0] = originVert.uv0;
		vert.TextureCoordinate[1] = originVert.uv1;
		vert.TextureCoordinate[2] = originVert.uv2;
		vert.TextureCoordinate[3] = originVert.uv3;
		vertices.Add(vert);
	}
}

void ULGUIGeometryHelper::ClearVerticesAndIndices()
{
	UIGeo->Clear();
}

void ULGUIGeometryHelper::GetVertexTriangleStream(TArray<FLGUIGeometryVertex>& OutVertexTriangleStream)
{
	auto& triangles = UIGeo->triangles;
	auto& vertices = UIGeo->vertices;
	auto& originVertices = UIGeo->originVertices;
	auto vertCount = vertices.Num();
	OutVertexTriangleStream.Reserve(triangles.Num());
	for (int i = 0; i < triangles.Num(); i++)
	{
		FLGUIGeometryVertex vertex;
		auto vertIndex = triangles[i];
		auto& vert = vertices[vertIndex];
		vertex.uv0 = vert.TextureCoordinate[0];
		vertex.uv1 = vert.TextureCoordinate[1];
		vertex.uv2 = vert.TextureCoordinate[2];
		vertex.uv3 = vert.TextureCoordinate[3];
		vertex.color = vert.Color;
		auto& originVert = originVertices[vertIndex];
		vertex.position = originVert.Position;
		vertex.normal = originVert.Normal;
		vertex.tangent = originVert.Tangent;
		OutVertexTriangleStream.Add(vertex);
	}
}
