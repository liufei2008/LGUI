// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "GeometryModifier/UIGeometryModifierBase.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "Core/LGUIIndexBuffer.h"

UUIGeometryModifierBase::UUIGeometryModifierBase()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UUIGeometryModifierBase::BeginPlay()
{
	Super::BeginPlay();
	
}
void UUIGeometryModifierBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
}
UUIBatchGeometryRenderable* UUIGeometryModifierBase::GetUIRenderable()const
{
	if(!UIRenderable.IsValid())
	{
		if (auto Actor = GetOwner())
		{
			if (auto RootComp = Actor->GetRootComponent())
			{
				if (auto UIRenderableComp = Cast<UUIBatchGeometryRenderable>(RootComp))
				{
					UIRenderable = UIRenderableComp;
				}
			}
		}
	}
	return UIRenderable.Get();
}
#if WITH_EDITOR
void UUIGeometryModifierBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (GetUIRenderable())
	{
		UIRenderable->EditorForceUpdate();
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
	if (UIRenderable.IsValid())
	{
		UIRenderable->RemoveGeometryModifier(this);
		UIRenderable = nullptr;
	}
}
void UUIGeometryModifierBase::AddToUIBatchGeometry()
{
	if (GetUIRenderable() != nullptr)
	{
		UIRenderable->AddGeometryModifier(this);
	}
}

void UUIGeometryModifierBase::SetEnable(bool value)
{ 
	if (bEnable != value)
	{
		bEnable = value;
		if (GetUIRenderable() != nullptr)
		{
			UIRenderable->MarkVerticesDirty(true, true, true, true);
		}
	}
}

void UUIGeometryModifierBase::SetExecuteOrder()
{
	if (UIRenderable.IsValid())
	{
		UIRenderable->SortGeometryModifier();
	}
}

DECLARE_CYCLE_STAT(TEXT("UIGeometryModifierBase_Blueprint.ModifyUIGeometry"), STAT_UIGeometryModifierBase_ModifyUIGeometry, STATGROUP_LGUI);
void UUIGeometryModifierBase::ModifyUIGeometry(
	UIGeometry& InGeometry, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		if (GeometryModifierHelper == nullptr)
		{
			GeometryModifierHelper = NewObject<ULGUIGeometryModifierHelper>(this);
		}
		GeometryModifierHelper->UIGeo = &InGeometry;
		SCOPE_CYCLE_COUNTER(STAT_UIGeometryModifierBase_ModifyUIGeometry);
		ReceiveModifyUIGeometry(GeometryModifierHelper);
	}
}



float ULGUIGeometryModifierHelper::UITextHelperFunction_GetCharHorizontalPositionRatio01(UUIText* InUIText, int InCharIndex)const
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
	auto& originVertices = UIGeo->originVertices;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	float leftPos = InUIText->GetLocalSpaceLeft();
	float charPivotPos = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPos += originVertices[vertIndex].Position.Y;
	}
	charPivotPos /= charPropertyItem.VertCount;
	return (charPivotPos - leftPos) / InUIText->GetWidth();
}

void ULGUIGeometryModifierHelper::UITextHelperFunction_GetCharGeometry_AbsolutePosition(UUIText* InUIText, int InCharIndex, FVector& OutPosition)const
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
	auto& originVertices = UIGeo->originVertices;
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

void ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Transform(UUIText* InUIText, int InCharIndex
	, ELGUIGeometryModifierHelper_UITextModifyPositionType InPositionType
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
	auto& originVertices = UIGeo->originVertices;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	float charPivotPosH = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPosH += originVertices[vertIndex].Position.Y;
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
			auto& pos = originVertices[vertIndex].Position;
			pos += InPosition;
		}
	}
	break;
	case ELGUIGeometryModifierHelper_UITextModifyPositionType::Absolute:
	{
		auto charPivotOffset = charPivotPos - InPosition;
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
		auto calcRotationMatrix = FRotationMatrix(InRotator);
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			auto vector = pos - InPosition;
			pos = InPosition + calcRotationMatrix.TransformPosition(vector);
		}
	}

	if (InScale != FVector::OneVector)
	{
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			auto vector = pos - InPosition;
			pos = InPosition + vector * InScale;
		}
	}
}
void ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Position(UUIText* InUIText, int InCharIndex, const FVector& InPosition, ELGUIGeometryModifierHelper_UITextModifyPositionType InPositionType)
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
	auto& originVertices = UIGeo->originVertices;
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
			auto& pos = originVertices[vertIndex].Position;
			pos += InPosition;
		}
	}
	break;
	case ELGUIGeometryModifierHelper_UITextModifyPositionType::Absolute:
	{
		auto charCenterPos = FVector::ZeroVector;
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			charCenterPos += originVertices[vertIndex].Position;
		}
		charCenterPos /= charPropertyItem.VertCount;
		auto centerOffset = charCenterPos - InPosition;
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originVertices[vertIndex].Position;
			pos -= centerOffset;
		}
	}
	break;
	}
}
void ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Rotate(UUIText* InUIText, int InCharIndex, const FRotator& InRotator)
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
	auto& originVertices = UIGeo->originVertices;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
	float charPivotPos = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPos += originVertices[vertIndex].Position.Y;
	}
	charPivotPos /= charPropertyItem.VertCount;

	auto calcRotationMatrix = FRotationMatrix(InRotator);
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		auto& pos = originVertices[vertIndex].Position;
		auto vector = pos - FVector(0, charPivotPos, 0);
		pos = FVector(0, charPivotPos, 0) + calcRotationMatrix.TransformPosition(vector);
	}
}
void ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Scale(UUIText* InUIText, int InCharIndex, const FVector& InScale)
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
	auto& originVertices = UIGeo->originVertices;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	float charPivotPosH = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPosH += originVertices[vertIndex].Position.Y;
	}
	charPivotPosH /= charPropertyItem.VertCount;
	auto charPivotPos = FVector(0, charPivotPosH, 0);

	auto calcScale = InScale;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		auto& pos = originVertices[vertIndex].Position;
		auto vector = pos - charPivotPos;
		pos = charPivotPos + vector * calcScale;
	}
}
void ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Color(UUIText* InUIText, int InCharIndex, const FColor& InColor)
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
	auto& vertices = UIGeo->vertices;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		auto& color = vertices[vertIndex].Color;
		color = InColor;
	}
}
void ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Alpha(UUIText* InUIText, int InCharIndex, const float& InAlpha)
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
	auto& vertices = UIGeo->vertices;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		auto& color = vertices[vertIndex].Color;
		color.A = InAlpha;
	}
}
