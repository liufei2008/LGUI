// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "GeometryModifier/UIGeometryModifierBase.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "Core/LGUIIndexBuffer.h"

UUIGeometryModifierBase::UUIGeometryModifierBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIGeometryModifierBase::BeginPlay()
{
	Super::BeginPlay();
	
}
void UUIGeometryModifierBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
}
UUIBatchGeometryRenderable* UUIGeometryModifierBase::GetRenderableUIItem()
{
	if(!renderableUIItem.IsValid())
	{
		if (auto actor = GetOwner())
		{
			TInlineComponentArray<class UUIBatchGeometryRenderable*> components;
			actor->GetComponents(components);
			if (components.Num() > 1)
			{
				for (auto comp : components)
				{
					if (comp->GetFName() == componentName)
					{
						renderableUIItem = (UUIBatchGeometryRenderable*)comp;
						break;
					}
				}
				if (!renderableUIItem.IsValid())
				{
					UE_LOG(LGUI, Warning, TEXT("[UUIGeometryModifierBase::GetRenderableUIItem]Cannot find component of name:%s, will use first one."), *(componentName.ToString()));
					renderableUIItem = (UUIBatchGeometryRenderable*)components[0];
				}
			}
			else if(components.Num() == 1)
			{
				renderableUIItem = (UUIBatchGeometryRenderable*)components[0];
			}
		}
	}
	return renderableUIItem.Get();
}
#if WITH_EDITOR
void UUIGeometryModifierBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (GetOwner())
	{
		if (auto rootComp = GetOwner()->GetRootComponent())
		{
			if (auto uiRenderable = Cast<UUIItem>(rootComp))
			{
				uiRenderable->EditorForceUpdate();
			}
		}
	}
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIGeometryModifierBase, componentName))
		{
			//remove from old
			RemoveFromUIBatchGeometry();
			//add to new
			AddToUIBatchGeometry();
		}
	}
}
#endif

void UUIGeometryModifierBase::OnRegister()
{
	Super::OnRegister();
	AddToUIBatchGeometry();
}
void UUIGeometryModifierBase::OnUnregister()
{
	Super::OnUnregister();
	RemoveFromUIBatchGeometry();
}

void UUIGeometryModifierBase::RemoveFromUIBatchGeometry()
{
	if (renderableUIItem.IsValid())
	{
		renderableUIItem->RemoveGeometryModifier(this);
		renderableUIItem = nullptr;
	}
}
void UUIGeometryModifierBase::AddToUIBatchGeometry()
{
	if (GetRenderableUIItem() != nullptr)
	{
		renderableUIItem->AddGeometryModifier(this);
	}
}

void UUIGeometryModifierBase::SetEnable(bool value)
{ 
	if (bEnable != value)
	{
		bEnable = value;
		if (GetRenderableUIItem() != nullptr)
		{
			renderableUIItem->MarkTriangleDirty();
		}
	}
}

void UUIGeometryModifierBase::SetExecuteOrder()
{
	if (renderableUIItem.IsValid())
	{
		renderableUIItem->SortGeometryModifier();
	}
}

void UUIGeometryModifierBase::ModifyUIGeometry(
	TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged,
	bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged, bool InTransformChanged
)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		if (GeometryModifierHelper == nullptr)
		{
			GeometryModifierHelper = NewObject<ULGUIGeometryModifierHelper>(this);
		}

		GeometryModifierHelper->BeginModify(InGeometry, InOutOriginVerticesCount, InOutOriginTriangleIndicesCount);
		ReceiveModifyUIGeometry(GeometryModifierHelper, InUVChanged, InColorChanged, InVertexPositionChanged, InTransformChanged);
		GeometryModifierHelper->EndModify(InGeometry, InOutOriginVerticesCount, InOutOriginTriangleIndicesCount, OutTriangleChanged);
	}
}


void ULGUIGeometryModifierHelper::BeginModify(TSharedPtr<UIGeometry> InGeometry, int32 InOriginVerticesCount, int32 InOriginTriangleIndicesCount)
{
	auto& vertices = InGeometry->vertices;
	auto& originPositions = InGeometry->originPositions;
	auto& originNormals = InGeometry->originNormals;
	auto& originTangents = InGeometry->originTangents;

	int vertCount = InOriginVerticesCount;
	cacheVertices.SetNum(vertCount);
	if (originNormals.Num() < vertCount)
	{
		originNormals.SetNumZeroed(vertCount);
	}
	if (originTangents.Num() < vertCount)
	{
		originTangents.SetNumZeroed(vertCount);
	}
	for (int i = 0; i < vertCount; i++)
	{
		auto& vert = cacheVertices[i];
		const auto& originVert = vertices[i];
		vert.position = originPositions[i];
		vert.color = originVert.Color;
		vert.uv0 = originVert.TextureCoordinate[0];
		vert.uv1 = originVert.TextureCoordinate[1];
		vert.uv2 = originVert.TextureCoordinate[2];
		vert.uv3 = originVert.TextureCoordinate[3];
		vert.normal = originNormals[i];
		vert.tangent = originTangents[i];
	}
	auto& triangles = InGeometry->triangles;
	int triangleIndicesCount = InOriginTriangleIndicesCount;
	cacheTriangleIndices.SetNum(triangleIndicesCount);
	for (int i = 0; i < triangleIndicesCount; i++)
	{
		cacheTriangleIndices[i] = triangles[i];
	}
}
void ULGUIGeometryModifierHelper::EndModify(TSharedPtr<UIGeometry> InGeometry, int32& OutOriginVerticesCount, int32& OutOriginTriangleIndicesCount, bool& OutTriangleChanged)
{
	auto& vertices = InGeometry->vertices;
	auto& originPositions = InGeometry->originPositions;
	auto& originNormals = InGeometry->originNormals;
	auto& originTangents = InGeometry->originTangents;

#if !UE_BUILD_SHIPPING
	for (auto& i : cacheTriangleIndices)
	{
		if (i >= vertices.Num())
		{
			UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::EndModify]Triangle index reference out of vertex range."));
			return;
		}
		if (i >= MAX_TRIANGLE_COUNT)
		{
			UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::EndModify]Triangle index out of precision range."));
			return;
		}
	}
	if ((cacheTriangleIndices.Num() % 3) != 0)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::EndModify]Indices count must be multiple of 3."));
		return;
	}
	for (auto& vertex : cacheVertices)
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
			UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::EndUpdateVertices]Vertex position contains NaN!."));
			return;
		}
	}
#endif
	if (cacheVertices.Num() > vertices.Num())
	{
		vertices.SetNumUninitialized(cacheVertices.Num());
	}
	if (cacheVertices.Num() > originPositions.Num())
	{
		originPositions.SetNumUninitialized(cacheVertices.Num());
	}
	if (cacheVertices.Num() > originNormals.Num())
	{
		originNormals.SetNumUninitialized(cacheVertices.Num());
	}
	if (cacheVertices.Num() > originTangents.Num())
	{
		originTangents.SetNumUninitialized(cacheVertices.Num());
	}
	auto& triangles = InGeometry->triangles;
	if (cacheTriangleIndices.Num() > triangles.Num())
	{
		triangles.SetNumUninitialized(cacheTriangleIndices.Num());
		OutTriangleChanged = true;
	}
	auto vertCount = cacheVertices.Num();
	for (int i = 0; i < vertCount; i++)
	{
		const auto& vert = cacheVertices[i];
		auto& originVert = vertices[i];
		originPositions[i] = vert.position;
		originVert.Color = vert.color;
		originVert.TextureCoordinate[0] = vert.uv0;
		originVert.TextureCoordinate[1] = vert.uv1;
		originVert.TextureCoordinate[2] = vert.uv2;
		originVert.TextureCoordinate[3] = vert.uv3;
		originNormals[i] = vert.normal;
		originTangents[i] = vert.tangent;
	}
	for (int i = 0; i < cacheTriangleIndices.Num(); i++)
	{
		triangles[i] = cacheTriangleIndices[i];
	}

	OutOriginVerticesCount = cacheVertices.Num();
	OutOriginTriangleIndicesCount = cacheTriangleIndices.Num();
}



float ULGUIGeometryModifierHelper::UITextHelperFunction_GetCharHorizontalPositionRatio01(UUIText* InUIText, int InCharIndex)const
{
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_GetCharHorizontalPositionRatio01]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return 0;
	}
#endif
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	float leftPos = InUIText->GetLocalSpaceLeft();
	float charPivotPos = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPos += cacheVertices[vertIndex].position.Y;
	}
	charPivotPos /= charPropertyItem.VertCount;
	return (charPivotPos - leftPos) / InUIText->GetWidth();
}
void ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Transform(UUIText* InUIText, int InCharIndex, const FVector& InPosition, ELGUIGeometryModifierHelper_UITextModifyPositionType InPositionType, const FRotator& InRotator, const FVector& InScale)
{
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Transform]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return;
	}
#endif
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	float charPivotPosH = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPosH += cacheVertices[vertIndex].position.Y;
	}
	charPivotPosH /= charPropertyItem.VertCount;
	auto charPivotPos = FVector(0, charPivotPosH, 0);
	switch (InPositionType)
	{
	default:
	case ELGUIGeometryModifierHelper_UITextModifyPositionType::Relative:
	{
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = cacheVertices[vertIndex].position;
			pos += InPosition;
		}
	}
	break;
	case ELGUIGeometryModifierHelper_UITextModifyPositionType::Absolute:
	{
		auto charPivotOffset = charPivotPos - InPosition;
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = cacheVertices[vertIndex].position;
			pos -= charPivotOffset;
		}
	}
	break;
	}

	if (InRotator != FRotator::ZeroRotator)
	{
		auto calcRotationMatrix = FRotationMatrix(InRotator);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = cacheVertices[vertIndex].position;
			auto vector = pos - InPosition;
			pos = InPosition + calcRotationMatrix.TransformPosition(vector);
		}
	}

	if (InScale != FVector::OneVector)
	{
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = cacheVertices[vertIndex].position;
			auto vector = pos - charPivotPos;
			pos = charPivotPos + vector * InScale;
		}
	}
}
void ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Position(UUIText* InUIText, int InCharIndex, const FVector& InPosition, ELGUIGeometryModifierHelper_UITextModifyPositionType InPositionType)
{
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Position]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return;
	}
#endif
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	switch (InPositionType)
	{
	default:
	case ELGUIGeometryModifierHelper_UITextModifyPositionType::Relative:
	{
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = cacheVertices[vertIndex].position;
			pos += InPosition;
		}
	}
	break;
	case ELGUIGeometryModifierHelper_UITextModifyPositionType::Absolute:
	{
		auto charCenterPos = FVector::ZeroVector;
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			charCenterPos += cacheVertices[vertIndex].position;
		}
		charCenterPos /= charPropertyItem.VertCount;
		auto centerOffset = charCenterPos - InPosition;
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = cacheVertices[vertIndex].position;
			pos -= centerOffset;
		}
	}
	break;
	}
}
void ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Rotate(UUIText* InUIText, int InCharIndex, const FRotator& InRotator)
{
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Rotate]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return;
	}
#endif
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
	float charPivotPos = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPos += cacheVertices[vertIndex].position.Y;
	}
	charPivotPos /= charPropertyItem.VertCount;

	auto calcRotationMatrix = FRotationMatrix(InRotator);
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		auto& pos = cacheVertices[vertIndex].position;
		auto vector = pos - FVector(0, charPivotPos, 0);
		pos = FVector(0, charPivotPos, 0) + calcRotationMatrix.TransformPosition(vector);
	}
}
void ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Scale(UUIText* InUIText, int InCharIndex, const FVector& InScale)
{
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Scale]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return;
	}
#endif
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	float charPivotPosH = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPosH += cacheVertices[vertIndex].position.Y;
	}
	charPivotPosH /= charPropertyItem.VertCount;
	auto charPivotPos = FVector(0, charPivotPosH, 0);

	auto calcScale = InScale;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		auto& pos = cacheVertices[vertIndex].position;
		auto vector = pos - charPivotPos;
		pos = charPivotPos + vector * calcScale;
	}
}
void ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Color(UUIText* InUIText, int InCharIndex, const FColor& InColor)
{
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Color]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return;
	}
#endif
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		auto& color = cacheVertices[vertIndex].color;
		color = InColor;
	}
}
void ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Alpha(UUIText* InUIText, int InCharIndex, const float& InAlpha)
{
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Alpha]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return;
	}
#endif
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		auto& color = cacheVertices[vertIndex].color;
		color.A = InAlpha;
	}
}
