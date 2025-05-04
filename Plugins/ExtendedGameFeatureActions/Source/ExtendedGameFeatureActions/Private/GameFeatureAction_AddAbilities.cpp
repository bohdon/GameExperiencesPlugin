// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameFeatureAction_AddAbilities.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ExtendedAbilitySet.h"
#include "Engine/GameInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddAbilities)


UGameFeatureWorldAction::FContextHandles* UGameFeatureAction_AddAbilities::AllocContextHandles() const
{
	return new FAbilityContextHandles();
}

void UGameFeatureAction_AddAbilities::Reset(FContextHandles& Handles)
{
	Super::Reset(Handles);

	FAbilityContextHandles& AbilityHandles = static_cast<FAbilityContextHandles&>(Handles);

	// remove all abilities
	while (!AbilityHandles.AbilitySetHandles.IsEmpty())
	{
		const auto ActorSetHandlesIt = AbilityHandles.AbilitySetHandles.CreateIterator();
		RemoveAbilitySets(ActorSetHandlesIt->Key, AbilityHandles);
	}
}

void UGameFeatureAction_AddAbilities::AddToWorld(const FWorldContext& WorldContext, FGameFeatureStateChangeContext ChangeContext)
{
	const UWorld* World = WorldContext.World();
	const UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	if (!GameInstance || !World || !World->IsGameWorld())
	{
		return;
	}

	UGameFrameworkComponentManager* ComponentManager = GameInstance->GetSubsystem<UGameFrameworkComponentManager>();
	if (!ComponentManager)
	{
		return;
	}

	FAbilityContextHandles& Handles = FindOrAddContextHandles<FAbilityContextHandles>(ChangeContext);

	for (int32 Idx = 0; Idx < Abilities.Num(); ++Idx)
	{
		const FGameFeatureExtendedAbilitySetEntry& Entry = Abilities[Idx];
		if (Entry.ActorClass.IsNull())
		{
			continue;
		}

		// register an extension handler for all actors by this class
		const UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate =
			UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtension, Idx, ChangeContext);

		TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentManager->AddExtensionHandler(Entry.ActorClass, AddAbilitiesDelegate);

		Handles.ComponentRequestHandles.Add(ExtensionRequestHandle);
	}
}

void UGameFeatureAction_AddAbilities::HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIdx, FGameFeatureStateChangeContext Context)
{
	FAbilityContextHandles* Handles = FindContextHandles<FAbilityContextHandles>(Context);
	if (!Handles || !Abilities.IsValidIndex(EntryIdx))
	{
		return;
	}

	const FGameFeatureExtendedAbilitySetEntry& Entry = Abilities[EntryIdx];

	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded ||
		EventName == UGameFrameworkComponentManager::NAME_ReceiverAdded)
	{
		AddAbilitySets(Actor, Entry.AbilitySets, *Handles);
	}
	else if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved ||
		EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
	{
		RemoveAbilitySets(Actor, *Handles);
	}
}

void UGameFeatureAction_AddAbilities::AddAbilitySets(AActor* Actor, const TArray<TSoftObjectPtr<const UExtendedAbilitySet>>& AbilitySets,
                                                     FAbilityContextHandles& Handles)
{
	if (!Actor->HasAuthority())
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
	{
		TArray<FExtendedAbilitySetHandles>& ActorAbilitySetHandles = Handles.AbilitySetHandles.FindOrAdd(Actor);

		for (const TSoftObjectPtr<const UExtendedAbilitySet>& AbilitySetPtr : AbilitySets)
		{
			if (const UExtendedAbilitySet* AbilitySet = AbilitySetPtr.Get())
			{
				ActorAbilitySetHandles.Emplace(AbilitySet->GiveToAbilitySystem(AbilitySystem, this));
			}
		}
	}
}

void UGameFeatureAction_AddAbilities::RemoveAbilitySets(AActor* Actor, FAbilityContextHandles& Handles)
{
	TArray<FExtendedAbilitySetHandles>* ActorAbilitySetHandles = Handles.AbilitySetHandles.Find(Actor);
	if (!ActorAbilitySetHandles)
	{
		// no record of extending this actor
		return;
	}

	if (UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
	{
		// remove the granted ability sets
		for (FExtendedAbilitySetHandles& AbilitySetHandles : *ActorAbilitySetHandles)
		{
			if (AbilitySetHandles.AbilitySet)
			{
				AbilitySetHandles.AbilitySet->RemoveFromAbilitySystem(AbilitySystem, AbilitySetHandles);
			}
		}
	}

	Handles.AbilitySetHandles.Remove(Actor);
}
