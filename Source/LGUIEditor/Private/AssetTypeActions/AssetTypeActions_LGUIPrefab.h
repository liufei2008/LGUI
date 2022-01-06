// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/IToolkitHost.h"
#include "AssetTypeActions_Base.h"

class ULGUIPrefab;

class FAssetTypeActions_LGUIPrefab : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_LGUIPrefab(EAssetTypeCategories::Type InAssetType);

	// FAssetTypeActions_Base overrides
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override { return FColor::White; }
	virtual UClass* GetSupportedClass() const override;
	virtual bool HasActions ( const TArray<UObject*>& InObjects ) const override { return true; }
	virtual uint32 GetCategories() override;
	virtual bool CanFilter() override;
	virtual void OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>() ) override;
private:

	EAssetTypeCategories::Type AssetType;
};
