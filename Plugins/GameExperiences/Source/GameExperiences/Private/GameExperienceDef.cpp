// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameExperienceDef.h"

#include "Misc/PackageName.h"
#include "UObject/Package.h"


UGameExperienceDef::UGameExperienceDef()
{
}

FPrimaryAssetId UGameExperienceDef::GetPrimaryAssetId() const
{
	const UClass* PrimaryAssetClass = UGameExperienceDef::StaticClass();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		if (GetClass()->HasAnyClassFlags(CLASS_Native | CLASS_Intrinsic))
		{
			// this is a native class CDO
			return FPrimaryAssetId();
		}

		return FPrimaryAssetId(PrimaryAssetClass->GetFName(), FPackageName::GetShortFName(GetOutermost()->GetName()));
	}

	// Data assets use Class and ShortName by default, there's no inheritance so class works fine
	return FPrimaryAssetId(PrimaryAssetClass->GetFName(), GetFName());
}
