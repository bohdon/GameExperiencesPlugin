// Copyright Bohdon Sayre, All Rights Reserved.


#include "AsyncAction_GameExperienceReady.h"

#include "GameExperienceComponent.h"


UAsyncAction_GameExperienceReady* UAsyncAction_GameExperienceReady::WaitForGameExperienceReady(UObject* WorldContextObject,
                                                                                               TEnumAsByte<EGameExperienceLoadEventPriority> Priority)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		UAsyncAction_GameExperienceReady* Action = NewObject<UAsyncAction_GameExperienceReady>();
		Action->GameWorld = World;
		Action->Priority = Priority;
		Action->RegisterWithGameInstance(World);
		return Action;
	}
	return nullptr;
}

void UAsyncAction_GameExperienceReady::Activate()
{
	UWorld* World = GameWorld.Get();
	if (!World)
	{
		// can never finish without a world
		SetReadyToDestroy();
		return;
	}

	if (AGameStateBase* GameState = World->GetGameState())
	{
		OnGameStateAvailable(GameState);
	}
	else
	{
		World->GameStateSetEvent.AddUObject(this, &ThisClass::OnGameStateAvailable);
	}
}

void UAsyncAction_GameExperienceReady::OnGameStateAvailable(AGameStateBase* GameState)
{
	if (UWorld* World = GameWorld.Get())
	{
		World->GameStateSetEvent.RemoveAll(this);
	}

	UGameExperienceComponent* ExperienceComponent = GameState->FindComponentByClass<UGameExperienceComponent>();
	if (!ExperienceComponent)
	{
		// experience component is not available
		SetReadyToDestroy();
		return;
	}

	ExperienceComponent->CallOrRegisterOnExperienceLoaded(
		FOnGameExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded),
		Priority);
}

void UAsyncAction_GameExperienceReady::OnExperienceLoaded(const UGameExperienceDef* CurrentExperience)
{
	OnReady.Broadcast();

	SetReadyToDestroy();
}
