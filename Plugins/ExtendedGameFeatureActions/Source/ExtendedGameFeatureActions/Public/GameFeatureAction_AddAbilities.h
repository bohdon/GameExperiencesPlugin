// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExtendedAbilitySet.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeatureWorldAction.h"
#include "GameFeatureAction_AddAbilities.generated.h"

class UExtendedAbilitySet;


/**
 * Defines an actor class and the ability set to grant.
 * For use in UGameFeatureAction_AddAbilities.
 */
USTRUCT()
struct FGameFeatureExtendedAbilitySetEntry
{
	GENERATED_BODY()

	/** The base actor class to give abilities to. */
	UPROPERTY(EditAnywhere, Category="Abilities")
	TSoftClassPtr<AActor> ActorClass;

	/** List of ability sets to grant. */
	UPROPERTY(EditAnywhere, Category="Attributes", Meta = (AssetBundles = "Client,Server"))
	TArray<TSoftObjectPtr<const UExtendedAbilitySet>> AbilitySets;
};


/**
 * Grants additional gameplay ability sets to characters via game feature actions.
 */
UCLASS(DisplayName = "Add Abilities")
class EXTENDEDGAMEFEATUREACTIONS_API UGameFeatureAction_AddAbilities : public UGameFeatureWorldAction
{
	GENERATED_BODY()

public:
	/** List of abilities to grant to different actor classes. */
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<FGameFeatureExtendedAbilitySetEntry> Abilities;

protected:
	struct FAbilityContextHandles final : public FContextHandles
	{
		/** Handles tracking which abilities and effects were granted to each actor, for removal later. */
		TMap<AActor*, TArray<FExtendedAbilitySetHandles>> AbilitySetHandles;

		virtual bool IsEmpty() const override
		{
			return FContextHandles::IsEmpty() && AbilitySetHandles.IsEmpty();
		}
	};

	virtual FContextHandles* AllocContextHandles() const override;
	virtual void Reset(FContextHandles& Handles) override;
	virtual void AddToWorld(const FWorldContext& WorldContext, FGameFeatureStateChangeContext ChangeContext) override;

	/**
	 * Called when an actor's state changes for extension.
	 * The index of the FGameFeatureExtendedAbilitySetEntry to add is passed, along with the feature's context.
	 */
	void HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIdx, FGameFeatureStateChangeContext Context);

	/** Add all ability sets in an entry to an actor, and store the handles. */
	void AddAbilitySets(AActor* Actor, const TArray<TSoftObjectPtr<const UExtendedAbilitySet>>& AbilitySets, FAbilityContextHandles& Handles);

	/** Remove all ability sets added for an actor by handles. */
	void RemoveAbilitySets(AActor* Actor, FAbilityContextHandles& Handles);
};
