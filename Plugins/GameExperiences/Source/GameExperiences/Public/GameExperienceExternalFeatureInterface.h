// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameExperienceExternalFeatureInterface.generated.h"


UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class UGameExperienceExternalFeatureInterface : public UInterface
{
	GENERATED_BODY()
};


/**
 * Interface for an additional feature to load with the game experience.
 */
class GAMEEXPERIENCES_API IGameExperienceExternalFeatureInterface
{
	GENERATED_BODY()

public:
	/** Return the name of this feature for debugging. */
	virtual FString GetFeatureName() const = 0;

	/** Load this feature and trigger the callback when finished. */
	virtual void LoadFeature(const FSimpleDelegate& CompleteDelegate) = 0;
};
