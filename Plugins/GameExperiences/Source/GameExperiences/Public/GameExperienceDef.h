// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameExperienceDef.generated.h"

class UGameExperienceActionSet;


/**
 * The definition of a gameplay experience.
 * Declares which game feature plugins and actions should be activated during this experience.
 */
UCLASS(BlueprintType, Const)
class GAMEEXPERIENCES_API UGameExperienceDef : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UGameExperienceDef();

	/** The actions and game features to activate when this experience is active. */
	UPROPERTY(EditAnywhere, Category = "Experience")
	TArray<TObjectPtr<UGameExperienceActionSet>> ActionSets;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
