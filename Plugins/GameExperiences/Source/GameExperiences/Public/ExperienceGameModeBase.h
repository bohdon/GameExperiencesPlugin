// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameExperienceProviderInterface.h"
#include "ModularGameMode.h"

#include "ExperienceGameModeBase.generated.h"

class UGameExperienceComponent;


/**
 * Base class for a game mode that utilizes experiences.
 * Games can subclass this, or implement similar features in a custom game mode.
 */
UCLASS()
class GAMEEXPERIENCES_API AExperienceGameModeBase : public AModularGameModeBase,
                                                    public IGameExperienceProviderInterface
{
	GENERATED_BODY()

public:
	AExperienceGameModeBase(const FObjectInitializer& ObjectInitializer);

	/** The default experience to use in this game mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Meta = (AllowAbstract = false), Category = "Experience")
	TSoftClassPtr<UGameExperienceDef> DefaultGameExperience;

	/** Should all players be restarted as soon as the experience loads? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Experience")
	bool bRestartPlayersOnExperienceLoad;

	// IGameExperienceProviderInterface
	virtual FPrimaryAssetId GetDesiredGameExperience(FString& OutDebugSource) const override;

	virtual void InitGameState() override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	/** Resolve and set the current game experience, then start loading. */
	virtual void InitGameExperience();

protected:
	/**
	 * Return the experience component to use.
	 * Default implementation retrieves it from the GameState.
	 */
	virtual UGameExperienceComponent* GetExperienceComponent() const;

	/** Return true if the current game experience is fully loaded. */
	bool IsExperienceLoaded() const;

	virtual void OnExperienceLoaded(const UGameExperienceDef* Experience);
};
