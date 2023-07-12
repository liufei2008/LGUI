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


FName ULGUICanvasCustomClip_Circle::CenterAndSizeParameterName = FName(TEXT("LGUI_CircleClip_CenterAndSize"));
#include "Materials/MaterialInstanceDynamic.h"
#include "Core/UIDrawcall.h"
ULGUICanvasCustomClip_Circle::ULGUICanvasCustomClip_Circle()
{
	replaceMaterialMap.Add(LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/LGUI_NoClip")), LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/CustomClip_Circle/LGUICustomClip_Circle")));
	replaceMaterialMap.Add(LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/LGUI_SDF_Font_NoClip")), LoadObject<UMaterialInterface>(NULL, TEXT("/LGUI/Materials/CustomClip_Circle/LGUICustomClip_Circle_SDFFont")));
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
