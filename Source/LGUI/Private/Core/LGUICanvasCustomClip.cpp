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


#include "Materials/MaterialInstanceDynamic.h"
ULGUICanvasCustomClip_Circle::ULGUICanvasCustomClip_Circle()
{
	replaceMaterialMap.Add(LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/LGUI_NoClip")), LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/CustomClip_Circle/LGUICustomClip_Circle")));
	replaceMaterialMap.Add(LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/LGUI_SDF_Font_NoClip")), LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/CustomClip_Circle/LGUICustomClip_Circle_SDFFont")));
}
void ULGUICanvasCustomClip_Circle::ApplyMaterialParameter(UMaterialInstanceDynamic* InMaterial, class ULGUICanvas* InCanvas, class UUIItem* InUIItem)
{
	auto LocalSpaceCenter = InUIItem->GetLocalSpaceCenter();
	cacheParam_CenterAndSize = FLinearColor(LocalSpaceCenter.X, LocalSpaceCenter.Y, InUIItem->GetWidth() * sizeMultiply, InUIItem->GetHeight() * sizeMultiply);
	cacheMaterial = InMaterial;
	cacheMaterial->SetVectorParameterValue(FName(TEXT("CenterAndSize")), cacheParam_CenterAndSize);
}
bool ULGUICanvasCustomClip_Circle::CheckPointVisible(const FVector& InWorldPoint, class ULGUICanvas* InCanvas, class UUIItem* InUIItem)
{
	auto LocalPoint = InUIItem->GetComponentTransform().InverseTransformPosition(InWorldPoint);
	auto LocalPoint2D = FVector2D(LocalPoint.Y, LocalPoint.Z);
	auto LocalSpaceCenter2D = InUIItem->GetLocalSpaceCenter();
	auto d = ((LocalPoint2D - LocalSpaceCenter2D) / FVector2D(InUIItem->GetWidth() * sizeMultiply, InUIItem->GetHeight() * sizeMultiply) * 2).Size();
	return d <= 1;
}
void ULGUICanvasCustomClip_Circle::SetSizeMultiply(float value)
{
	if (sizeMultiply != value)
	{
		sizeMultiply = value;
		if (cacheMaterial.IsValid())
		{
			cacheMaterial->SetVectorParameterValue(FName(TEXT("CenterAndSize")), cacheParam_CenterAndSize);
		}
	}
}
