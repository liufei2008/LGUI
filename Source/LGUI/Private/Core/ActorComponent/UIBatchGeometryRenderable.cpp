// Copyright 2019-Present LexLiu. All Rights Reserved.

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

void UUIBatchGeometryRenderable::OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache)
{
    Super::OnAnchorChange(InPivotChange, InWidthChange, InHeightChange, InDiscardCache);
	if (InPivotChange || InWidthChange || InHeightChange)
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
		//when use pixel-perfect, the pixel-perfect calculation will take consider transform matrix, so we need to recalculate geometry if pixel-perfect & bTransformChanged
		bool pixelPerfect = this->GetShouldAffectByPixelPerfect() && this->GetRenderCanvas()->GetActualPixelPerfect();
		bool pixelPerfectAffectTransform = pixelPerfect && bTransformChanged;
		if (bTriangleChanged || bLocalVertexPositionChanged || pixelPerfectAffectTransform || bColorChanged || bUVChanged)
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
			OnUpdateGeometry(*(geometry.Get()), bTriangleChanged, bLocalVertexPositionChanged || pixelPerfectAffectTransform, bUVChanged, bColorChanged);
			ApplyGeometryModifier(bTriangleChanged, bUVChanged, bColorChanged, bLocalVertexPositionChanged);
			drawcall->bNeedToUpdateVertex = true;
			if (bLocalVertexPositionChanged || pixelPerfectAffectTransform)//pixelPerfect is affected by transform, and can affect localVertex calculation
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
		auto errorMsg = FText::Format(NSLOCTEXT("UIBatchGeometryRenderable", "TooManyTrianglesInSingleDdrawcall", "{0} Too many vertex ({1}) in single UI element: {2}")
			, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__))
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
	switch (RaycastType)
	{
	default:
	case EUIRenderableRaycastType::Rect:
		return Super::LineTraceUI(OutHit, Start, End);
		break;
	case EUIRenderableRaycastType::Geometry:
		return LineTraceUIGeometry(geometry.Get(), OutHit, Start, End);
		break;
	case EUIRenderableRaycastType::VisiblePixel:
		return LineTraceVisiblePixel(VisiblePixelThreadhold, OutHit, Start, End);
		break;
	case EUIRenderableRaycastType::Custom:
		return LineTraceUICustom(OutHit, Start, End);
		break;
	}
}
bool UUIBatchGeometryRenderable::LineTraceVisiblePixel(float InAlphaThreshold, FHitResult& OutHit, const FVector& Start, const FVector& End)
{
	const auto InverseTf = GetComponentTransform().Inverse();
	const auto LocalSpaceRayOrigin = InverseTf.TransformPosition(Start);
	const auto LocalSpaceRayEnd = InverseTf.TransformPosition(End);

	//DrawDebugLine(this->GetWorld(), Start, End, FColor::Red, false, 5.0f);//just for test
	//check Line-Plane intersection first, then check Line-Triangle
	//start and end point must be different side of X plane
	if (FMath::Sign(LocalSpaceRayOrigin.X) != FMath::Sign(LocalSpaceRayEnd.X))
	{
		//triangle hit test
		//triangle hit test
		auto& originVertices = geometry->originVertices;
		auto& vertices = geometry->vertices;
		auto& triangleIndices = geometry->triangles;
		const int triangleCount = triangleIndices.Num() / 3;
		int index = 0;
		for (int i = 0; i < triangleCount; i++)
		{
			auto vertIndex0 = triangleIndices[index++];
			auto vertIndex1 = triangleIndices[index++];
			auto vertIndex2 = triangleIndices[index++];
			auto point0 = (FVector)(originVertices[vertIndex0].Position);
			auto point1 = (FVector)(originVertices[vertIndex1].Position);
			auto point2 = (FVector)(originVertices[vertIndex2].Position);
			FVector OutHitPoint, OutHitNormal;
			if (FMath::SegmentTriangleIntersection(LocalSpaceRayOrigin, LocalSpaceRayEnd, point0, point1, point2, OutHitPoint, OutHitNormal))
			{
				OutHit.TraceStart = Start;
				OutHit.TraceEnd = End;
				OutHit.Component = (UPrimitiveComponent*)this;//acturally this convert is incorrect, but I need this pointer
				OutHit.Location = GetComponentTransform().TransformPosition(OutHitPoint);
				OutHit.Normal = GetComponentTransform().TransformVector(OutHitNormal);
				OutHit.Normal.Normalize();
				OutHit.Distance = FVector::Distance(Start, OutHit.Location);
				OutHit.ImpactPoint = OutHit.Location;
				OutHit.ImpactNormal = OutHit.Normal;

				auto baryCentric = FMath::ComputeBaryCentric2D(OutHitPoint, point0, point1, point2);
				auto& uv0 = vertices[vertIndex0].TextureCoordinate[0];
				auto& uv1 = vertices[vertIndex1].TextureCoordinate[0];
				auto& uv2 = vertices[vertIndex2].TextureCoordinate[0];
				auto uv = FVector2D(baryCentric.X * uv0 + baryCentric.Y * uv1 + baryCentric.Z * uv2);
				//get pixel
				FColor Pixel;
				if (ReadPixelFromMainTexture(uv, Pixel))
				{
					auto AlphaValue = Pixel.A;
					auto AlphaValue01 = LGUIUtils::Color255To1_Table[AlphaValue];
					if (AlphaValue01 > InAlphaThreshold)
					{
						return true;
					}
					else
					{
						return false;
					}
				}
				else
				{
					return true;
				}
			}
		}
	}
	return false;
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
		UE_LOG(LGUI, Error, TEXT("[%s].%s Vertex data contains NaN!."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}
#endif
	auto& originVertices = UIGeo->originVertices;
	originVertices.Add(FVector3f(position));
	auto& vertices = UIGeo->vertices;
	FLGUIMeshVertex vert(FVector3f::ZeroVector);
	vert.Color = color;
	vert.TextureCoordinate[0] = FVector2f(uv0);
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
		UE_LOG(LGUI, Error, TEXT("[%s].%d Vertex data contains NaN!."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}
#endif
	auto& originVertices = UIGeo->originVertices;
	originVertices.Add(FLGUIOriginVertexData((FVector3f)position, (FVector3f)normal, (FVector3f)tangent));
	auto& vertices = UIGeo->vertices;
	FLGUIMeshVertex vert(FVector3f::ZeroVector);
	vert.Color = color;
	vert.TextureCoordinate[0] = FVector2f(uv0);
	vert.TextureCoordinate[1] = FVector2f(uv1);
	vert.TextureCoordinate[2] = FVector2f(uv2);
	vert.TextureCoordinate[3] = FVector2f(uv3);
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
		UE_LOG(LGUI, Error, TEXT("[%s].%d Vertex data contains NaN!."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return;
	}
#endif
	auto& originVertices = UIGeo->originVertices;
	originVertices.Add(FLGUIOriginVertexData((FVector3f)vertex.position, (FVector3f)vertex.normal, (FVector3f)vertex.tangent));
	auto& vertices = UIGeo->vertices;
	FLGUIMeshVertex vert(FVector3f::ZeroVector);
	vert.Color = vertex.color;
	vert.TextureCoordinate[0] = FVector2f(vertex.uv0);
	vert.TextureCoordinate[1] = FVector2f(vertex.uv1);
	vert.TextureCoordinate[2] = FVector2f(vertex.uv2);
	vert.TextureCoordinate[3] = FVector2f(vertex.uv3);
	vertices.Add(vert);
}
void ULGUIGeometryHelper::AddTriangle(int index0, int index1, int index2)
{
#if !UE_BUILD_SHIPPING
	int vertCount = UIGeo->vertices.Num();
	if (index0 >= vertCount || index1 >= vertCount || index2 >= vertCount)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Triangle index reference out of vertex range."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
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
			UE_LOG(LGUI, Error, TEXT("[%s].%d Triangle index reference out of vertex range."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return;
		}
	}
	if ((InIndices.Num() % 3) != 0)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Indices count must be multiple of 3."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
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
			UE_LOG(LGUI, Error, TEXT("[%s].%d Vertex data contains NaN!."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
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
		originVertices[i] = FLGUIOriginVertexData((FVector3f)originVert.position, (FVector3f)originVert.normal, (FVector3f)originVert.tangent);
		auto& vert = vertices[i];
		vert.Color = originVert.color;
		vert.TextureCoordinate[0] = FVector2f(originVert.uv0);
		vert.TextureCoordinate[1] = FVector2f(originVert.uv1);
		vert.TextureCoordinate[2] = FVector2f(originVert.uv2);
		vert.TextureCoordinate[3] = FVector2f(originVert.uv3);
	}
}

void ULGUIGeometryHelper::AddVertexTriangleStream(const TArray<FLGUIGeometryVertex>& InVertexTriangleStream)
{
#if !UE_BUILD_SHIPPING
	if ((InVertexTriangleStream.Num() % 3) != 0)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Indices count must be multiple of 3."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
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
			UE_LOG(LGUI, Error, TEXT("[%s].%d Vertex data contains NaN!."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
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
		originVertices.Add(FLGUIOriginVertexData((FVector3f)originVert.position, (FVector3f)originVert.normal, (FVector3f)originVert.tangent));
		FLGUIMeshVertex vert(FVector3f::ZeroVector);
		vert.Color = originVert.color;
		vert.TextureCoordinate[0] = FVector2f(originVert.uv0);
		vert.TextureCoordinate[1] = FVector2f(originVert.uv1);
		vert.TextureCoordinate[2] = FVector2f(originVert.uv2);
		vert.TextureCoordinate[3] = FVector2f(originVert.uv3);
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
		vertex.uv0 = FVector2D(vert.TextureCoordinate[0]);
		vertex.uv1 = FVector2D(vert.TextureCoordinate[1]);
		vertex.uv2 = FVector2D(vert.TextureCoordinate[2]);
		vertex.uv3 = FVector2D(vert.TextureCoordinate[3]);
		vertex.color = vert.Color;
		auto& originVert = originVertices[vertIndex];
		vertex.position = (FVector)originVert.Position;
		vertex.normal = (FVector)originVert.Normal;
		vertex.tangent = (FVector)originVert.Tangent;
		OutVertexTriangleStream.Add(vertex);
	}
}
