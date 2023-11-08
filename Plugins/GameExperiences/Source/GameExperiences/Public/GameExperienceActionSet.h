// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameExperienceActionSet.generated.h"

class UGameFeatureAction;


/**
 * A reusable group of game feature actions.
 */
UCLASS(BlueprintType)
class GAMEEXPERIENCES_API UGameExperienceActionSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UGameExperienceActionSet();

	/** Game feature plugins to activate. */
	UPROPERTY(EditAnywhere, Category = "Features")
	TArray<FString> GameFeatures;

	/** Actions to perform. */
	UPROPERTY(EditAnywhere, Instanced, Category = "Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
