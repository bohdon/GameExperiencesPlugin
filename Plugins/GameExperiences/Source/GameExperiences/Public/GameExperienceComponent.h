// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameExperienceDef.h"
#include "GameExperienceProviderInterface.h"
#include "GameFeaturePluginOperationResult.h"
#include "Components/GameStateComponent.h"
#include "GameExperienceComponent.generated.h"

class UGameExperienceDef;


enum class EGameExperienceLoadState
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	ExecutingActions,
	DebugDelay,
	Loaded,
	Deactivating
};


/** The priority of callbacks for OnGameExperienceLoaded events. */
UENUM(BlueprintType)
enum class EGameExperienceLoadEventPriority
{
	// Called 2nd, after High priority
	Normal,
	// Called 1st, before other priorities
	High,
	// Called 3rd, after High and Normal priority
	Low
};


DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameExperienceLoaded, const UGameExperienceDef* /*Experience*/);


/**
 * A game state component that manages loading and unloading game experiences.
 */
UCLASS(Meta = (BlueprintSpawnableComponent))
class GAMEEXPERIENCES_API UGameExperienceComponent : public UGameStateComponent,
                                                     public IGameExperienceProviderInterface
{
	GENERATED_BODY()

public:
	UGameExperienceComponent(const FObjectInitializer& ObjectInitializer);

	// IGameExperienceProviderInterface
	virtual FPrimaryAssetId GetDesiredGameExperience(FString& OutDebugSource) const override;

	/** Set the current experience automatically from the world settings, game mode or other sources. */
	virtual void AutoResolveExperience();

	/** Set the current experience and start loading. The experience cannot be changed once set. */
	void SetExperience(const FPrimaryAssetId& ExperienceId);

	/** Return the current experience. */
	const UGameExperienceDef* GetExperience() const { return Experience; }

	/** Return the current experience, cast to a type. */
	template <class T>
	const T* GetExperience() const
	{
		return Cast<T>(GetExperience());
	}

	/** Return true if the experience is fully loaded. */
	bool IsExperienceLoaded() const;

	/**
	 * Register a delegate to be called when the experience is loaded,
	 * or call the delegate immediately if the experience is already loaded.
	 */
	void CallOrRegisterOnExperienceLoaded(FOnGameExperienceLoaded::FDelegate&& Delegate,
	                                      EGameExperienceLoadEventPriority Priority = EGameExperienceLoadEventPriority::Normal);

protected:
	/** The current experience. */
	UPROPERTY(ReplicatedUsing = OnRep_Experience)
	TObjectPtr<UGameExperienceDef> Experience;

	/** The current loading state of the experience. */
	EGameExperienceLoadState LoadState = EGameExperienceLoadState::Unloaded;

	/** List of game feature plugins that were enabled by the experience. */
	TArray<FString> GameFeaturePluginURLs;

	int32 NumFeaturePluginsLoading = 0;
	int32 NumExpectedPausers = 0;
	int32 NumPausers = 0;


	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnRep_Experience();

	/** Start loading the experience, beginning with assets. */
	virtual void LoadExperience();

	/**
	 * Called when the assets for the experience have been loaded.
	 * Starts loading the needed game features plugins.
	 */
	void OnExperienceAssetsLoaded();

	/** Load the game feature plugins needed by the experience. */
	void LoadGameFeaturePlugins();

	/**
	 * Called once any game feature plugin has been loaded.
	 * Once all game feature plugins are loaded, OnExperienceLoaded will be called.
	 */
	void OnGameFeaturePluginLoaded(const UE::GameFeatures::FResult& Result);

	/** Called when the experience has been fully loaded. */
	virtual void OnExperienceLoaded();

	/** Activate the experience actions. */
	virtual void ExecuteActions();

	/** Deactivate the experience. */
	virtual void DeactivateExperience();

	/** Called when an action has been deactivated during experience deactivate. */
	void OnActionDeactivationCompleted();

	/** Called once all actions have been deactivated during experience deactivate. */
	void OnAllActionsDeactivated();

	/** Called when the experience has been fully loaded, before other events. */
	FOnGameExperienceLoaded OnExperienceLoadedEvent_HighPriority;

	/** Called when the experience has been fully loaded. */
	FOnGameExperienceLoaded OnExperienceLoadedEvent;

	/** Called when the experience has been fully loaded, after other events. */
	FOnGameExperienceLoaded OnExperienceLoadedEvent_LowPriority;

public:
	/**
	 * Parse string representation of an experience and return the primary asset id.
	 * Supports both "GameExperienceDef.MyExperience" and "MyExperience".
	 */
	static FPrimaryAssetId ParseExperiencePrimaryAssetIdFromString(const FString& ExperienceIdString);
};
