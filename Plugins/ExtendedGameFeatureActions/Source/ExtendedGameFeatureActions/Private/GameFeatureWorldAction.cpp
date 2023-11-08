// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameFeatureWorldAction.h"

#include "GameFeaturesSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureWorldAction)


void UGameFeatureWorldAction::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FContextHandles& Handles = FindOrAddContextHandles(Context);

	// listen for new game instances starting
	Handles.GameInstanceStartHandle = FWorldDelegates::OnStartGameInstance.AddUObject(
		this, &ThisClass::OnStartGameInstance, FGameFeatureStateChangeContext(Context));

	if (!ensureAlways(Handles.IsEmpty()))
	{
		Reset(Handles);
	}

	// add to any matching worlds that have already been initialized
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (Context.ShouldApplyToWorldContext(WorldContext))
		{
			AddToWorld(WorldContext, Context);
		}
	}
}

void UGameFeatureWorldAction::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	FContextHandles& Handles = FindOrAddContextHandles(Context);

	FWorldDelegates::OnStartGameInstance.Remove(Handles.GameInstanceStartHandle);

	Reset(Handles);
}

void UGameFeatureWorldAction::OnStartGameInstance(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext)
{
	if (const FWorldContext* WorldContext = GameInstance->GetWorldContext())
	{
		if (ChangeContext.ShouldApplyToWorldContext(*WorldContext))
		{
			AddToWorld(*WorldContext, ChangeContext);
		}
	}
}

UGameFeatureWorldAction::FContextHandles* UGameFeatureWorldAction::FindContextHandles(const FGameFeatureStateChangeContext& Context) const
{
	const TSharedPtr<FContextHandles> HandlesPtr = ContextHandles.FindRef(Context);
	if (HandlesPtr.IsValid())
	{
		return HandlesPtr.Get();
	}
	return nullptr;
}

UGameFeatureWorldAction::FContextHandles& UGameFeatureWorldAction::FindOrAddContextHandles(const FGameFeatureStateChangeContext& Context)
{
	TSharedPtr<FContextHandles>& HandlePtr = ContextHandles.FindOrAdd(Context);
	if (!HandlePtr.IsValid())
	{
		HandlePtr = TSharedPtr<FContextHandles>(AllocContextHandles());
	}
	return *HandlePtr;
}

UGameFeatureWorldAction::FContextHandles* UGameFeatureWorldAction::AllocContextHandles() const
{
	return new FContextHandles();
}

void UGameFeatureWorldAction::Reset(FContextHandles& Handles)
{
	// release extension request to unregister delegates.
	// note that component requests are not ever used in this class, but are
	// included as base functionality since they are so common in subclasses.
	Handles.ComponentRequestHandles.Empty();
}
