// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFeatureWorldAction.generated.h"


/**
 * Base class for game feature actions that operate on a specific world context.
 *
 * Subclasses will likely want to track specific modifications made by the feature (for removal later).
 * In those cases, subclass the FGameFeatureWorldActionContextHandles struct and override
 * UGameFeatureWorldAction::AllocContextHandles. This way feature-specific info can be stored
 * in the pre-existing context handle structs.
 */
UCLASS(Abstract)
class EXTENDEDGAMEFEATUREACTIONS_API UGameFeatureWorldAction : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

protected:
	/** Base class for handles that are stored per-context for a GameFeatureWorldAction. */
	struct FContextHandles
	{
		FContextHandles()
		{
		}

		virtual ~FContextHandles() = default;

		/** Delegate handle for when a game instance starts, e.g. for PIE. */
		FDelegateHandle GameInstanceStartHandle;

		/** Delegate handles for actor extension events. */
		TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequestHandles;

		virtual bool IsEmpty() const
		{
			return ComponentRequestHandles.IsEmpty();
		}
	};

	/** Map of feature-specific handles by game feature state change context. */
	TMap<FGameFeatureStateChangeContext, TSharedPtr<FContextHandles>> ContextHandles;

	FContextHandles* FindContextHandles(const FGameFeatureStateChangeContext& Context) const;

	template <typename T>
	T* FindContextHandles(const FGameFeatureStateChangeContext& Context) const
	{
		return static_cast<T*>(FindContextHandles(Context));
	}

	FContextHandles& FindOrAddContextHandles(const FGameFeatureStateChangeContext& Context);

	template <typename T>
	T& FindOrAddContextHandles(const FGameFeatureStateChangeContext& Context)
	{
		return static_cast<T&>(FindOrAddContextHandles(Context));
	}

	/**
	 * Allocate a new FGameFeatureWorldActionContextHandles object.
	 * Override in subclasses to use a custom context handles struct.
	 */
	virtual FContextHandles* AllocContextHandles() const;

	/** Reset this feature for a specific context, clearing delegates and removing abilities as necessary. */
	virtual void Reset(FContextHandles& Handles);

	/** Called when a game instance is started with the context of the relevant game feature. */
	void OnStartGameInstance(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext);

	/** Apply this game feature to a world, registering extension delegates as needed. */
	virtual void AddToWorld(const FWorldContext& WorldContext, FGameFeatureStateChangeContext ChangeContext) PURE_VIRTUAL(UGameFeatureWorldAction::AddToWorld,);
};
