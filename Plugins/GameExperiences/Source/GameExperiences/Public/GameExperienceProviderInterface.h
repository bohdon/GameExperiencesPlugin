// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameExperienceProviderInterface.generated.h"


class UGameExperienceDef;

UINTERFACE(Meta = (CannotImplementInterfaceInBlueprint))
class UGameExperienceProviderInterface : public UInterface
{
	GENERATED_BODY()
};


/**
 * Interface for objects that provide a game experience id.
 */
class GAMEEXPERIENCES_API IGameExperienceProviderInterface
{
	GENERATED_BODY()

public:
	/**
	 * Return the primary asset id of the desired game experience to use.
	 * @param OutDebugSource A debug string representing the source of the game experience that was returned.
	 */
	UFUNCTION(BlueprintCallable)
	virtual FPrimaryAssetId GetDesiredGameExperience(FString& OutDebugSource) const = 0;

	/**
	 * Helper function to retrieve the primary asset id of a game experience.
	 * Logs an error if the asset id could not be retrieved do to asset manager settings.
	 */
	static FPrimaryAssetId GetGameExperiencePrimaryAssetIdFromSoftClass(const TSoftClassPtr<UGameExperienceDef>& Experience, const UObject* ContextObject);
};
