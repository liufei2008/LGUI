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
UUIBatchGeometryRenderable* UUIGeometryModifierBase::GetUIRenderable()
{
	if(!UIRenderable.IsValid())
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
						UIRenderable = (UUIBatchGeometryRenderable*)comp;
						break;
					}
				}
				if (!UIRenderable.IsValid())
				{
					UE_LOG(LGUI, Warning, TEXT("[UUIGeometryModifierBase::GetUIRenderable]Cannot find component of name:%s, will use first one."), *(componentName.ToString()));
					UIRenderable = (UUIBatchGeometryRenderable*)components[0];
				}
			}
			else if(components.Num() == 1)
			{
				UIRenderable = (UUIBatchGeometryRenderable*)components[0];
			}
		}
	}
	return UIRenderable.Get();
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
			UIRenderable->MarkTriangleDirty();
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
		ReceiveModifyUIGeometry(GeometryModifierHelper, InTriangleChanged, InUVChanged, InColorChanged, InVertexPositionChanged);
	}
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
	auto& originPositions = UIGeo->originPositions;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	float leftPos = InUIText->GetLocalSpaceLeft();
	float charPivotPos = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPos += originPositions[vertIndex].Y;
	}
	charPivotPos /= charPropertyItem.VertCount;
	return (charPivotPos - leftPos) / InUIText->GetWidth();
}

void ULGUIGeometryModifierHelper::UITextHelperFunction_GetCharGeometry_AbsolutePosition(UUIText* InUIText, int InCharIndex, FVector& OutPosition)const
{
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Transform]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return;
	}
#endif
	auto& originPositions = UIGeo->originPositions;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	float charPivotPosH = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPosH += originPositions[vertIndex].Y;
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
	auto& CharPropertyArray = InUIText->GetCharPropertyArray();
#if !UE_BUILD_SHIPPING
	if (InCharIndex < 0 || InCharIndex >= CharPropertyArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIGeometryModifierHelper::UITextHelperFunction_ModifyCharGeometry_Transform]InCharIndex out of range, InCharIndex: %d, ArrayNum: %d"), InCharIndex, CharPropertyArray.Num());
		return;
	}
#endif
	auto& originPositions = UIGeo->originPositions;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	float charPivotPosH = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPosH += originPositions[vertIndex].Y;
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
			auto& pos = originPositions[vertIndex];
			pos += InPosition;
		}
	}
	break;
	case ELGUIGeometryModifierHelper_UITextModifyPositionType::Absolute:
	{
		auto charPivotOffset = charPivotPos - InPosition;
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originPositions[vertIndex];
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
			auto& pos = originPositions[vertIndex];
			auto vector = pos - InPosition;
			pos = InPosition + calcRotationMatrix.TransformPosition(vector);
		}
	}

	if (InScale != FVector::OneVector)
	{
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originPositions[vertIndex];
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
	auto& originPositions = UIGeo->originPositions;
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
			auto& pos = originPositions[vertIndex];
			pos += InPosition;
		}
	}
	break;
	case ELGUIGeometryModifierHelper_UITextModifyPositionType::Absolute:
	{
		auto charCenterPos = FVector::ZeroVector;
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			charCenterPos += originPositions[vertIndex];
		}
		charCenterPos /= charPropertyItem.VertCount;
		auto centerOffset = charCenterPos - InPosition;
		for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
		{
			auto& pos = originPositions[vertIndex];
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
	auto& originPositions = UIGeo->originPositions;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;
	float charPivotPos = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPos += originPositions[vertIndex].Y;
	}
	charPivotPos /= charPropertyItem.VertCount;

	auto calcRotationMatrix = FRotationMatrix(InRotator);
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		auto& pos = originPositions[vertIndex];
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
	auto& originPositions = UIGeo->originPositions;
	auto& charPropertyItem = CharPropertyArray[InCharIndex];
	int startVertIndex = charPropertyItem.StartVertIndex;
	int endVertIndex = charPropertyItem.StartVertIndex + charPropertyItem.VertCount;

	float charPivotPosH = 0;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		charPivotPosH += originPositions[vertIndex].Y;
	}
	charPivotPosH /= charPropertyItem.VertCount;
	auto charPivotPos = FVector(0, charPivotPosH, 0);

	auto calcScale = InScale;
	for (int vertIndex = startVertIndex; vertIndex < endVertIndex; vertIndex++)
	{
		auto& pos = originPositions[vertIndex];
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
