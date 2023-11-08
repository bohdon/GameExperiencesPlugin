// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameExperienceProviderInterface.h"

#include "GameExperiencesModule.h"
#include "Engine/AssetManager.h"


FPrimaryAssetId IGameExperienceProviderInterface::GetGameExperiencePrimaryAssetIdFromSoftClass(const TSoftClassPtr<UGameExperienceDef>& Experience,
                                                                                               const UObject* ContextObject)
{
	if (Experience.IsNull())
	{
		return FPrimaryAssetId();
	}

	const FPrimaryAssetId Result = UAssetManager::Get().GetPrimaryAssetIdForPath(Experience.ToSoftObjectPath());
	if (!Result.IsValid())
	{
		UE_LOG(LogGameExperience, Error, TEXT("%s requested experience %s, but a primary asset id could not be resolved. "
			       "Check Primary Asset Types in project settings or the game feature plugin."),
		       *GetPathNameSafe(ContextObject), *Experience.ToString());
	}
	return Result;
}
