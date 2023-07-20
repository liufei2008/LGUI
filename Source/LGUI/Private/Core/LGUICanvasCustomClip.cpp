// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUICanvasCustomClip.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/ActorComponent/UIItem.h"
#include "Materials/MaterialInterface.h"
#include "LTweenManager.h"
#include "LTweener.h"

ULGUICanvasCustomClip::ULGUICanvasCustomClip()
{
	bCanExecuteBlueprintEvent = GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native);
}
UMaterialInterface* ULGUICanvasCustomClip::GetReplaceMaterial(UMaterialInterface* InMaterial)
{
	if (auto FindMatPtr = replaceMaterialMap.Find(InMaterial))
	{
		if (FindMatPtr != nullptr && *FindMatPtr != nullptr)
		{
			return *FindMatPtr;
		}
	}
	return InMaterial;
}
void ULGUICanvasCustomClip::ApplyMaterialParameter(UMaterialInstanceDynamic* InMaterial, ULGUICanvas* InCanvas, UUIItem* InUIItem)
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveApplyMaterialParameter(InMaterial, InCanvas, InUIItem);
	}
}
bool ULGUICanvasCustomClip::CheckPointVisible(const FVector& InWorldPoint, ULGUICanvas* InCanvas, UUIItem* InUIItem)
{
	if (bCanExecuteBlueprintEvent)
	{
		return ReceiveCheckPointVisible(InWorldPoint, InCanvas, InUIItem);
	}
	return false;
}
bool ULGUICanvasCustomClip::MaterialContainsClipParameter(UMaterialInterface* InMaterial)
{
	if (bCanExecuteBlueprintEvent)
	{
		return ReceiveMaterialContainsClipParameter(InMaterial);
	}
	return false;
}


#include "Materials/MaterialInstanceDynamic.h"
#include "Core/UIDrawcall.h"

FName ULGUICanvasCustomClip_Circle::CenterAndSizeParameterName = FName(TEXT("LGUI_CircleClip_CenterAndSize"));
ULGUICanvasCustomClip_Circle::ULGUICanvasCustomClip_Circle()
{
	replaceMaterialMap.Add(LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/LGUI_NoClip")), LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/CustomClip_Circle/LGUICustomClip_Circle")));
	replaceMaterialMap.Add(LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/LGUI_SDF_Font_NoClip")), LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/CustomClip_Circle/LGUICustomClip_Circle_SDFFont")));
	replaceMaterialMap.Add(LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/LGUI_ProceduralRect_NoClip")), LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/CustomClip_Circle/LGUICustomClip_Circle_ProceduralRect")));
}
void ULGUICanvasCustomClip_Circle::ApplyMaterialParameter(UMaterialInstanceDynamic* InMaterial, class ULGUICanvas* InCanvas, class UUIItem* InUIItem)
{
	auto LocalSpaceCenter = InUIItem->GetLocalSpaceCenter();
	auto CenterAndSize = FLinearColor(LocalSpaceCenter.X, LocalSpaceCenter.Y, InUIItem->GetWidth() * sizeMultiply, InUIItem->GetHeight() * sizeMultiply);
	InMaterial->SetVectorParameterValue(CenterAndSizeParameterName, CenterAndSize);
}
bool ULGUICanvasCustomClip_Circle::CheckPointVisible(const FVector& InWorldPoint, class ULGUICanvas* InCanvas, class UUIItem* InUIItem)
{
	auto LocalPoint = InUIItem->GetComponentTransform().InverseTransformPosition(InWorldPoint);
	auto LocalPoint2D = FVector2D(LocalPoint.Y, LocalPoint.Z);
	auto LocalSpaceCenter2D = InUIItem->GetLocalSpaceCenter();
	auto d = ((LocalPoint2D - LocalSpaceCenter2D) / FVector2D(InUIItem->GetWidth() * sizeMultiply, InUIItem->GetHeight() * sizeMultiply) * 2).Size();
	return d <= 1;
}
bool ULGUICanvasCustomClip_Circle::MaterialContainsClipParameter(UMaterialInterface* InMaterial)
{
	static TArray<FMaterialParameterInfo> ParameterInfos;
	static TArray<FGuid> ParameterIds;
	InMaterial->GetAllVectorParameterInfo(ParameterInfos, ParameterIds);
	auto FoundIndex = ParameterInfos.IndexOfByPredicate([](const FMaterialParameterInfo& Item)
		{
			return Item.Name == CenterAndSizeParameterName;
		});
	return FoundIndex != INDEX_NONE;
}
void ULGUICanvasCustomClip_Circle::SetSizeMultiply(float value)
{
	if (sizeMultiply != value)
	{
		sizeMultiply = value;
		if (auto Canvas = this->GetTypedOuter<ULGUICanvas>())
		{
			if (auto UIItem = Canvas->GetUIItem())
			{
				auto LocalSpaceCenter = UIItem->GetLocalSpaceCenter();
				auto CenterAndSize = FLinearColor(LocalSpaceCenter.X, LocalSpaceCenter.Y, UIItem->GetWidth() * sizeMultiply, UIItem->GetHeight() * sizeMultiply);
				auto& DrawcallList = Canvas->GetUIDrawcallList();
				for (auto& Item : DrawcallList)
				{
					if (Item->bMaterialContainsLGUIParameter)
					{
						((UMaterialInstanceDynamic*)Item->RenderMaterial.Get())->SetVectorParameterValue(CenterAndSizeParameterName, CenterAndSize);
					}
				}
			}
		}
	}
}


FName ULGUICanvasCustomClip_RoundedRect::CenterAndSizeParameterName = FName(TEXT("LGUI_RoundedRectClip_CenterAndSize"));
FName ULGUICanvasCustomClip_RoundedRect::CornerRadiusParameterName = FName(TEXT("LGUI_RoundedRectClip_CornerRadius"));
ULGUICanvasCustomClip_RoundedRect::ULGUICanvasCustomClip_RoundedRect()
{
	replaceMaterialMap.Add(LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/LGUI_NoClip")), LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/CustomClip_RoundedRect/LGUICustomClip_RoundedRect")));
	replaceMaterialMap.Add(LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/LGUI_SDF_Font_NoClip")), LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/CustomClip_RoundedRect/LGUICustomClip_RoundedRect_SDFFont")));
	replaceMaterialMap.Add(LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/LGUI_ProceduralRect_NoClip")), LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/CustomClip_RoundedRect/LGUICustomClip_RoundedRect_ProceduralRect")));
}
void ULGUICanvasCustomClip_RoundedRect::ApplyMaterialParameter(UMaterialInstanceDynamic* InMaterial, class ULGUICanvas* InCanvas, class UUIItem* InUIItem)
{
	auto LocalSpaceCenter = InUIItem->GetLocalSpaceCenter();
	auto CenterAndSize = FLinearColor(LocalSpaceCenter.X, LocalSpaceCenter.Y, InUIItem->GetWidth(), InUIItem->GetHeight());
	auto TempCornerRadius = GetCornerRadiusWithUnitMode(InUIItem->GetWidth(), InUIItem->GetHeight(), 0.5f);
	ApplyMaterialParameter(InMaterial, CenterAndSize, TempCornerRadius);
}
void ULGUICanvasCustomClip_RoundedRect::ApplyMaterialParameter(UMaterialInstanceDynamic* InMaterial, const FLinearColor& InCenterAndSize, const FLinearColor& InConerRadius)
{
	InMaterial->SetVectorParameterValue(CenterAndSizeParameterName, InCenterAndSize);
	InMaterial->SetVectorParameterValue(CornerRadiusParameterName, InConerRadius);
}
FVector4f ULGUICanvasCustomClip_RoundedRect::GetCornerRadiusWithUnitMode(float RectWidth, float RectHeight, float AdditionalScale)
{
	auto Result = CornerRadiusUnitMode == ELGUICanvasCustomClip_RoundedRect_UnitMode::Value ? CornerRadius : (CornerRadius * 0.01f * (RectWidth < RectHeight ? RectWidth : RectHeight) * AdditionalScale);
	Result = FVector4f(Result.Y, Result.X, Result.W, Result.Z);//change the order so it is the same as UIProceduralRect
	return Result;
}

bool ULGUICanvasCustomClip_RoundedRect::CheckPointVisible(const FVector& InWorldPoint, class ULGUICanvas* InCanvas, class UUIItem* InUIItem)
{
	auto LocalPoint = InUIItem->GetComponentTransform().InverseTransformPosition(InWorldPoint);
	if (LocalPoint.Y > InUIItem->GetLocalSpaceLeft() && LocalPoint.Y < InUIItem->GetLocalSpaceRight() && LocalPoint.Z > InUIItem->GetLocalSpaceBottom() && LocalPoint.Z < InUIItem->GetLocalSpaceTop())
	{
		auto InLocalHitPoint = FVector2D(LocalPoint.Y, LocalPoint.Z);
		auto TempCornerRadius = GetCornerRadiusWithUnitMode(InUIItem->GetWidth(), InUIItem->GetHeight(), 0.5f);
		TempCornerRadius = FVector4f(TempCornerRadius.Y, TempCornerRadius.X, TempCornerRadius.W, TempCornerRadius.Z);//flip vertical
		auto HalfWidth = InUIItem->GetWidth() * 0.5f;
		auto HalfHeight = InUIItem->GetHeight() * 0.5f;
		auto MinSize = FMath::Min(HalfWidth, HalfHeight);
		TempCornerRadius.X = FMath::Min(TempCornerRadius.X, MinSize);
		TempCornerRadius.Y = FMath::Min(TempCornerRadius.Y, MinSize);
		TempCornerRadius.Z = FMath::Min(TempCornerRadius.Z, MinSize);
		TempCornerRadius.W = FMath::Min(TempCornerRadius.W, MinSize);
		if (InLocalHitPoint.X > 0 && InLocalHitPoint.Y < 0)//right bottom area of rect
		{
			auto Radius = TempCornerRadius.X;
			auto CenterPos = FVector2D(InUIItem->GetLocalSpaceRight() - Radius, InUIItem->GetLocalSpaceBottom() + Radius);
			if (InLocalHitPoint.X > CenterPos.X && InLocalHitPoint.Y < CenterPos.Y)
			{
				if (FVector2D::DistSquared(InLocalHitPoint, CenterPos) > Radius * Radius)
				{
					return false;
				}
			}
			return true;
		}
		else if (InLocalHitPoint.X > 0 && InLocalHitPoint.Y > 0)//right top area of rect
		{
			auto Radius = TempCornerRadius.Y;
			auto CenterPos = FVector2D(InUIItem->GetLocalSpaceRight() - Radius, InUIItem->GetLocalSpaceTop() - Radius);
			if (InLocalHitPoint.X > CenterPos.X && InLocalHitPoint.Y > CenterPos.Y)
			{
				if (FVector2D::DistSquared(InLocalHitPoint, CenterPos) > Radius * Radius)
				{
					return false;
				}
			}
			return true;
		}
		else if (InLocalHitPoint.X < 0 && InLocalHitPoint.Y > 0)//left top area of rect
		{
			auto Radius = TempCornerRadius.Z;
			auto CenterPos = FVector2D(InUIItem->GetLocalSpaceLeft() + Radius, InUIItem->GetLocalSpaceTop() - Radius);
			if (InLocalHitPoint.X < CenterPos.X && InLocalHitPoint.Y > CenterPos.Y)
			{
				if (FVector2D::DistSquared(InLocalHitPoint, CenterPos) > Radius * Radius)
				{
					return false;
				}
			}
			return true;
		}
		else if (InLocalHitPoint.X < 0 && InLocalHitPoint.Y < 0)//left bottom area of rect
		{
			auto Radius = TempCornerRadius.W;
			auto CenterPos = FVector2D(InUIItem->GetLocalSpaceLeft() + Radius, InUIItem->GetLocalSpaceBottom() + Radius);
			if (InLocalHitPoint.X < CenterPos.X && InLocalHitPoint.Y < CenterPos.Y)
			{
				if (FVector2D::DistSquared(InLocalHitPoint, CenterPos) > Radius * Radius)
				{
					return false;
				}
			}
			return true;
		}
		return true;
	}
	return false;
}
bool ULGUICanvasCustomClip_RoundedRect::MaterialContainsClipParameter(UMaterialInterface* InMaterial)
{
	static TArray<FMaterialParameterInfo> ParameterInfos;
	static TArray<FGuid> ParameterIds;
	InMaterial->GetAllVectorParameterInfo(ParameterInfos, ParameterIds);
	auto FoundIndex = ParameterInfos.IndexOfByPredicate([](const FMaterialParameterInfo& Item)
		{
			return
				Item.Name == CenterAndSizeParameterName
				|| Item.Name == CornerRadiusParameterName
				;
		});
	return FoundIndex != INDEX_NONE;
}

void ULGUICanvasCustomClip_RoundedRect::ApplyMaterialParameterOnCanvas()
{
	if (auto Canvas = this->GetTypedOuter<ULGUICanvas>())
	{
		if (auto UIItem = Canvas->GetUIItem())
		{
			auto LocalSpaceCenter = UIItem->GetLocalSpaceCenter();
			auto CenterAndSize = FLinearColor(LocalSpaceCenter.X, LocalSpaceCenter.Y, UIItem->GetWidth(), UIItem->GetHeight());
			auto TempCornerRadius = GetCornerRadiusWithUnitMode(UIItem->GetWidth(), UIItem->GetHeight(), 1.0f);
			auto& DrawcallList = Canvas->GetUIDrawcallList();
			for (auto& Item : DrawcallList)
			{
				if (Item->bMaterialContainsLGUIParameter)
				{
					ApplyMaterialParameter((UMaterialInstanceDynamic*)Item->RenderMaterial.Get(), CenterAndSize, TempCornerRadius);
				}
			}
		}
	}
}

#if WITH_EDITOR
void ULGUICanvasCustomClip_RoundedRect::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ULGUICanvasCustomClip_RoundedRect, CornerRadiusUnitMode))
		{
			if (auto Canvas = this->GetTypedOuter<ULGUICanvas>())
			{
				if (auto UIItem = Canvas->GetUIItem())
				{
					this->OnCornerRadiusUnitModeChanged(UIItem->GetWidth(), UIItem->GetHeight());
				}
			}
		}
		else if (PropertyName == TEXT("X"))
		{
			if (bUniformSetCornerRadius)
			{
				CornerRadius.Y = CornerRadius.Z = CornerRadius.W = CornerRadius.X;
			}
		}
	}
}
bool ULGUICanvasCustomClip_RoundedRect::CanEditChange(const FProperty* InProperty)const
{
	if (
		InProperty->GetFName() == TEXT("Y")
		|| InProperty->GetFName() == TEXT("Z")
		|| InProperty->GetFName() == TEXT("W")
		)
	{
		if (bUniformSetCornerRadius)
		{
			return false;
		}
	}
	return Super::CanEditChange(InProperty);
}
#endif

void ULGUICanvasCustomClip_RoundedRect::OnCornerRadiusUnitModeChanged(float width, float height)
{
	if (CornerRadiusUnitMode == ELGUICanvasCustomClip_RoundedRect_UnitMode::Value)//from percentage to value
	{
		CornerRadius = CornerRadius * 0.01f * (width < height ? width : height) * 0.5f;
	}
	else//from value to percentage
	{
		CornerRadius = CornerRadius * 100.0f / (width < height ? width : height) * 2.0f;
	}
}

void ULGUICanvasCustomClip_RoundedRect::SetCornerRadius(const FVector4& value)
{
	if (CornerRadius != (FVector4f)value)
	{
		CornerRadius = (FVector4f)value;
		ApplyMaterialParameterOnCanvas();
	}
}
void ULGUICanvasCustomClip_RoundedRect::SetCornerRadiusUnitMode(ELGUICanvasCustomClip_RoundedRect_UnitMode value)
{
	if (CornerRadiusUnitMode != value)
	{
		CornerRadiusUnitMode = value;
		ApplyMaterialParameterOnCanvas();
	}
}
