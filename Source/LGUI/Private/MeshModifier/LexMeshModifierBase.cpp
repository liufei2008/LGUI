// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUI/Public/MeshModifier/LexMeshModifierBase.h"
#include "LGUI.h"
#include "Core/Components/LexVisualBatchMesh.h"
#include "Core/Components/LexWidget.h"

ULexMeshModifierBase::ULexMeshModifierBase()
{
	
}

ULexVisualBatchMesh* ULexMeshModifierBase::GetVisualBatchMesh()const
{
	if (!CacheVisualBatchMesh.IsValid())
	{
		if (auto Widget = GetWidget())
		{
			CacheVisualBatchMesh = Cast<ULexVisualBatchMesh>(Widget->GetVisual());
		}
	}
	return CacheVisualBatchMesh.Get();
}
#if WITH_EDITOR
void ULexMeshModifierBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (GetVisualBatchMesh())
	{
		CacheVisualBatchMesh->MarkVerticesDirty(true, true, true, true);
	}
}
#endif

void ULexMeshModifierBase::OnRegister()
{
	Super::OnRegister();
	if (auto Widget = GetWidget())
	{
		if (!ComponentsChangedDelegateHandle.IsValid())
		{
			ComponentsChangedDelegateHandle = Widget->GetComponentsChangedEvent().AddWeakLambda(this,
				[this](ELexWidgetComponentsChangedType ChangedType)
			{
				if (ChangedType == ELexWidgetComponentsChangedType::Reorder)
				{
					if (GetVisualBatchMesh() != nullptr)
					{
						CacheVisualBatchMesh->MarkMeshModifierOrderChanged();
					}
				}
			});
		}
	}
	if (GetVisualBatchMesh() != nullptr)
	{
		CacheVisualBatchMesh->AddMeshModifier(this);
	}
}

void ULexMeshModifierBase::OnUnregister()
{
	Super::OnUnregister();
	if (auto Widget = GetWidget())
	{
		if (ComponentsChangedDelegateHandle.IsValid())
		{
			Widget->GetComponentsChangedEvent().Remove(ComponentsChangedDelegateHandle);
			ComponentsChangedDelegateHandle.Reset();
		}
	}
	if (CacheVisualBatchMesh.IsValid())
	{
		CacheVisualBatchMesh->RemoveMeshModifier(this);
	}
}

void ULexMeshModifierBase::SetEnable(bool Value)
{ 
	if (bEnable != Value)
	{
		bEnable = Value;
		if (GetVisualBatchMesh() != nullptr)
		{
			CacheVisualBatchMesh->MarkVerticesDirty(true, true, true, true);
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("UIGeometryModifierBase_Blueprint.ModifyUIGeometry"), STAT_UIGeometryModifierBase_ModifyUIGeometry, STATGROUP_LGUI);
void ULexMeshModifierBase::ModifyUIGeometry(
	FLexUIGeometry& InGeometry, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		if (GeometryModifierHelper == nullptr)
		{
			GeometryModifierHelper = NewObject<ULexVisualBatchMeshModifierHelper>(this);
		}
		GeometryModifierHelper->UIGeo = &InGeometry;
		SCOPE_CYCLE_COUNTER(STAT_UIGeometryModifierBase_ModifyUIGeometry);
		ReceiveModifyUIGeometry(GeometryModifierHelper);
	}
}



float ULexVisualBatchMeshModifierHelper::UITextHelperFunction_GetCharHorizontalPositionRatio01(ULexText* InUIText, int InCharIndex)const
{
	if (InUIText == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_GetCharHorizontalPositionRatio01]InUIText not valid!"));
		return 0;
	}
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_GetCharHorizontalPositionRatio01]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return 0;
	}
#endif
	auto& originVertices = UIGeo->OriginVertices;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	auto Widget = InUIText->GetWidget();
	float leftPos = Widget->GetLocalSpaceLeft();
	float charPivotPos = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPos += originVertices[vertIndex].Position.Y;
	}
	charPivotPos /= charPropertyItem.VertCount;
	return (charPivotPos - leftPos) / Widget->GetWidth();
}

void ULexVisualBatchMeshModifierHelper::UITextHelperFunction_GetCharGeometry_AbsolutePosition(ULexText* InUIText, int InCharIndex, FVector& OutPosition)const
{
	if (InUIText == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_GetCharGeometry_AbsolutePosition]InUIText not valid!"));
		return;
	}
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Transform]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return;
	}
#endif
	auto& originVertices = UIGeo->OriginVertices;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	float charPivotPosH = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPosH += originVertices[vertIndex].Position.Y;
	}
	charPivotPosH /= charPropertyItem.VertCount;
	OutPosition = FVector(0, charPivotPosH, 0);
}

void ULexVisualBatchMeshModifierHelper::UITextHelperFunction_ModifyCharGeometry_Transform(ULexText* InUIText, int InCharIndex
	, ELexUIMeshModifierHelper_TextPositionType InPositionType
	, const FVector& InPosition
	, const FRotator& InRotator
	, const FVector& InScale
)
{
	if (InUIText == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Transform]InUIText not valid!"));
		return;
	}
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Transform]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return;
	}
#endif
	auto& originVertices = UIGeo->OriginVertices;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	float charPivotPosH = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPosH += originVertices[vertIndex].Position.Y;
	}
	charPivotPosH /= charPropertyItem.VertCount;
	auto charPivotPos = FVector3f(0, charPivotPosH, 0);
	switch (InPositionType)
	{
	default:
	case ELexUIMeshModifierHelper_TextPositionType::Relative:
	{
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			pos += (FVector3f)InPosition;
		}
	}
	break;
	case ELexUIMeshModifierHelper_TextPositionType::Absolute:
	{
		auto charPivotOffset = charPivotPos - (FVector3f)InPosition;
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			pos -= charPivotOffset;
		}
	}
	break;
	}

	if (InRotator != FRotator::ZeroRotator)
	{
		auto calcRotationMatrix = FRotationMatrix44f((FRotator3f)InRotator);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			auto vector = pos - (FVector3f)InPosition;
			pos = (FVector3f)InPosition + calcRotationMatrix.TransformPosition(vector);
		}
	}

	if (InScale != FVector::OneVector)
	{
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			auto vector = pos - (FVector3f)InPosition;
			pos = (FVector3f)InPosition + vector * (FVector3f)InScale;
		}
	}
}
void ULexVisualBatchMeshModifierHelper::UITextHelperFunction_ModifyCharGeometry_Position(ULexText* InUIText, int InCharIndex, const FVector& InPosition, ELexUIMeshModifierHelper_TextPositionType InPositionType)
{
	if (InUIText == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Position]InUIText not valid!"));
		return;
	}
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Position]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return;
	}
#endif
	auto& originVertices = UIGeo->OriginVertices;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	switch (InPositionType)
	{
	default:
	case ELexUIMeshModifierHelper_TextPositionType::Relative:
	{
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			pos += (FVector3f)InPosition;
		}
	}
	break;
	case ELexUIMeshModifierHelper_TextPositionType::Absolute:
	{
		auto charCenterPos = FVector3f::ZeroVector;
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			charCenterPos += originVertices[vertIndex].Position;
		}
		charCenterPos /= charPropertyItem.VertCount;
		auto centerOffset = charCenterPos - (FVector3f)InPosition;
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			pos -= centerOffset;
		}
	}
	break;
	}
}
void ULexVisualBatchMeshModifierHelper::UITextHelperFunction_ModifyCharGeometry_Rotate(ULexText* InUIText, int InCharIndex, const FRotator& InRotator)
{
	if (InUIText == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Rotate]InUIText not valid!"));
		return;
	}
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Rotate]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return;
	}
#endif
	auto& originVertices = UIGeo->OriginVertices;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
	float charPivotPos = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPos += originVertices[vertIndex].Position.Y;
	}
	charPivotPos /= charPropertyItem.VertCount;

	auto calcRotationMatrix = FRotationMatrix44f((FRotator3f)InRotator);
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		auto& pos = originVertices[vertIndex].Position;
		auto vector = pos - FVector3f(0, charPivotPos, 0);
		pos = FVector3f(0, charPivotPos, 0) + calcRotationMatrix.TransformPosition(vector);
	}
}
void ULexVisualBatchMeshModifierHelper::UITextHelperFunction_ModifyCharGeometry_Scale(ULexText* InUIText, int InCharIndex, const FVector& InScale)
{
	if (InUIText == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Scale]InUIText not valid!"));
		return;
	}
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Scale]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return;
	}
#endif
	auto& originVertices = UIGeo->OriginVertices;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	float charPivotPosH = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPosH += originVertices[vertIndex].Position.Y;
	}
	charPivotPosH /= charPropertyItem.VertCount;
	auto charPivotPos = FVector3f(0, charPivotPosH, 0);

	auto calcScale = (FVector3f)InScale;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		auto& pos = originVertices[vertIndex].Position;
		auto vector = pos - charPivotPos;
		pos = charPivotPos + vector * calcScale;
	}
}
void ULexVisualBatchMeshModifierHelper::UITextHelperFunction_ModifyCharGeometry_Color(ULexText* InUIText, int InCharIndex, const FColor& InColor)
{
	if (InUIText == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Color]InUIText not valid!"));
		return;
	}
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Color]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return;
	}
#endif
	auto& vertices = UIGeo->Vertices;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		auto& color = vertices[vertIndex].Color;
		color = InColor;
	}
}
void ULexVisualBatchMeshModifierHelper::UITextHelperFunction_ModifyCharGeometry_Alpha(ULexText* InUIText, int InCharIndex, const float& InAlpha)
{
	if (InUIText == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Alpha]InUIText not valid!"));
		return;
	}
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Alpha]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return;
	}
#endif
	auto& vertices = UIGeo->Vertices;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		auto& color = vertices[vertIndex].Color;
		color.A = InAlpha;
	}
}
