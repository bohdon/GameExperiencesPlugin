// Copyright Bohdon Sayre, All Rights Reserved.


#include "ExtendedGameFeaturesProjectPolicies.h"

#include "GameFeatureAction_AddGameplayCuePaths.h"


void UExtendedGameFeaturesProjectPolicies::InitGameFeatureManager()
{
	Observers.Add(NewObject<UGameFeature_AddGameplayCuePaths>());

	UGameFeaturesSubsystem& FeaturesSubsystem = UGameFeaturesSubsystem::Get();
	for (UObject* Observer : Observers)
	{
		FeaturesSubsystem.AddObserver(Observer);
	}

	Super::InitGameFeatureManager();
}

void UExtendedGameFeaturesProjectPolicies::ShutdownGameFeatureManager()
{
	Super::ShutdownGameFeatureManager();

	UGameFeaturesSubsystem& FeaturesSubsystem = UGameFeaturesSubsystem::Get();
	for (UObject* Observer : Observers)
	{
		FeaturesSubsystem.RemoveObserver(Observer);
	}

	Observers.Empty();
}
