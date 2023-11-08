// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeaturesProjectPolicies.h"
#include "ExtendedGameFeaturesProjectPolicies.generated.h"


/**
 * Adds support for AddGameplayCuePaths game feature actions.
 */
UCLASS()
class EXTENDEDGAMEFEATUREACTIONS_API UExtendedGameFeaturesProjectPolicies : public UDefaultGameFeaturesProjectPolicies
{
	GENERATED_BODY()

public:
	virtual void InitGameFeatureManager() override;
	virtual void ShutdownGameFeatureManager() override;

protected:
	/** Game feature observer instances. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UObject>> Observers;
};
