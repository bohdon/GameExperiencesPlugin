// Copyright Bohdon Sayre, All Rights Reserved.


#include "ExperienceGameModeBase.h"

#include "GameExperienceComponent.h"
#include "GameExperiencesModule.h"
#include "TimerManager.h"


AExperienceGameModeBase::AExperienceGameModeBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bRestartPlayersOnExperienceLoad(true)
{
}

FPrimaryAssetId AExperienceGameModeBase::GetDesiredGameExperience(FString& OutDebugSource) const
{
	const FPrimaryAssetId Result = GetGameExperiencePrimaryAssetIdFromSoftClass(DefaultGameExperience, this);
	OutDebugSource = TEXT("GameMode");
	return Result;
}

UGameExperienceComponent* AExperienceGameModeBase::GetExperienceComponent() const
{
	return GameState->FindComponentByClass<UGameExperienceComponent>();
}

bool AExperienceGameModeBase::IsExperienceLoaded() const
{
	if (const UGameExperienceComponent* ExperienceComponent = GetExperienceComponent())
	{
		return ExperienceComponent->IsExperienceLoaded();
	}
	return false;
}

void AExperienceGameModeBase::InitGameState()
{
	Super::InitGameState();

	UGameExperienceComponent* ExperienceComponent = GetExperienceComponent();
	if (!ExperienceComponent)
	{
		UE_LOG(LogGameExperience, Error, TEXT("Could not find a UGameExperienceComponent. Make sure it exists on the GameState."));
		return;
	}

	// listen for experience ready
	ExperienceComponent->CallOrRegisterOnExperienceLoaded(
		FOnGameExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded),
		EGameExperienceLoadEventPriority::Normal);
}

void AExperienceGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::InitGameExperience);
}

bool AExperienceGameModeBase::PlayerCanRestart_Implementation(APlayerController* Player)
{
	if (!Player || !IsExperienceLoaded() || !IsPlayerExperienceLoaded(Player))
	{
		return false;
	}

	return Super::PlayerCanRestart_Implementation(Player);
}

void AExperienceGameModeBase::InitGameExperience()
{
	if (UGameExperienceComponent* ExperienceComponent = GetExperienceComponent())
	{
		ExperienceComponent->AutoResolveExperience();
	}
}

bool AExperienceGameModeBase::IsPlayerExperienceLoaded(APlayerController* Player) const
{
	// implement in subclass if you want to block players
	// from restarting until their client experience has finished loading
	return true;
}

void AExperienceGameModeBase::OnExperienceLoaded(const UGameExperienceDef* Experience)
{
	if (!bRestartPlayersOnExperienceLoad)
	{
		return;
	}

	// restart players
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Cast<APlayerController>(*Iterator);
		if (PC && PC->GetPawn() == nullptr)
		{
			if (PlayerCanRestart(PC))
			{
				RestartPlayer(PC);
			}
		}
	}
}
