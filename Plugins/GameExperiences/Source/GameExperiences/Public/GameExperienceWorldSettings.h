// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameExperienceProviderInterface.h"
#include "GameFramework/WorldSettings.h"
#include "GameExperienceWorldSettings.generated.h"

class UGameExperienceDef;


/**
 * World Settings that allows specifying a default game experience.
 */
UCLASS()
class GAMEEXPERIENCES_API AGameExperienceWorldSettings : public AWorldSettings,
                                                         public IGameExperienceProviderInterface
{
	GENERATED_BODY()

public:
	AGameExperienceWorldSettings(const FObjectInitializer& ObjectInitializer);

protected:
	/** The experience to use for this level if not overridden elsewhere. */
	UPROPERTY(EditDefaultsOnly, Meta = (AllowAbstract = false), Category = "GameMode")
	TSoftClassPtr<UGameExperienceDef> DefaultGameExperience;

public:
	// IGameExperienceProviderInterface
	virtual FPrimaryAssetId GetDesiredGameExperience(FString& OutDebugSource) const override;
};
