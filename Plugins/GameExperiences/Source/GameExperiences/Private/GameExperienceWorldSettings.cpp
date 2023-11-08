// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameExperienceWorldSettings.h"


AGameExperienceWorldSettings::AGameExperienceWorldSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FPrimaryAssetId AGameExperienceWorldSettings::GetDesiredGameExperience(FString& OutDebugSource) const
{
	const FPrimaryAssetId Result = GetGameExperiencePrimaryAssetIdFromSoftClass(DefaultGameExperience, this);
	OutDebugSource = TEXT("WorldSettings");
	return Result;
}
