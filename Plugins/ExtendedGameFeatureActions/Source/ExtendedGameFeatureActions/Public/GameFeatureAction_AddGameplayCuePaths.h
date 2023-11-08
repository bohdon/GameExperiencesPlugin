// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction.h"
#include "GameFeatureStateChangeObserver.h"
#include "GameFeatureAction_AddGameplayCuePaths.generated.h"


/**
 * Registers a new path for gameplay cue notify actors.
 */
UCLASS(DisplayName = "Add Gameplay Cue Paths")
class EXTENDEDGAMEFEATUREACTIONS_API UGameFeatureAction_AddGameplayCuePaths : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	UGameFeatureAction_AddGameplayCuePaths();

	/** The gameplay cue paths to add. */
	UPROPERTY(EditAnywhere, Meta = (RelativeToGameContentDir, LongPackageName), Category = "GameplayCues")
	TArray<FDirectoryPath> GameplayCuePaths;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};


/**
 * A game feature observer used in the game features project policy to
 * add and remove gameplay cue paths as game features register/unregister.
 */
UCLASS()
class UGameFeature_AddGameplayCuePaths : public UObject,
                                         public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()

public:
	// IGameFeatureStateChangeObserver
	virtual void OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL) override;
	virtual void OnGameFeatureUnregistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL) override;

	FString FixGameplayCuePath(const FDirectoryPath& RelativeCuePath, const FString& PluginName);
};
