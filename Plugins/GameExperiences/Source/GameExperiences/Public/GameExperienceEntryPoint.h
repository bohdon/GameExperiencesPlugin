// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "GameExperienceEntryPoint.generated.h"

class UCommonSession_HostSessionRequest;
class UGameExperienceEntryPoint;


/**
 * UI data for displaying an entry point to the user.
 * Subclass this in your project to add additional properties.
 */
UCLASS(DefaultToInstanced, EditInlineNew)
class UGameExperienceEntryPointUIData : public UObject
{
	GENERATED_BODY()

public:
	/** The name of the experience or group of experiences. 'Arcade', 'Campaign', 'Elimination' */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIData")
	FText Title;

	/** A description of the experience. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIData")
	FText Description;

	/** Sort order for presenting this experience in a list. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIData")
	int32 SortOrder;

	/** Sort an array of entry point data assets by their SortOrder defined in UIData. */
	UFUNCTION(BlueprintCallable)
	static TArray<UGameExperienceEntryPoint*> SortGameExperienceEntryPoints(const TArray<UGameExperienceEntryPoint*>& EntryPoints);
};


/**
 * A data asset that provides all the info necessary to launch an experience.
 * These can be referenced from a game's frontend UI in order to launch various game modes.
 */
UCLASS(Blueprintable)
class GAMEEXPERIENCES_API UGameExperienceEntryPoint : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Tags that defined the traits of this entry point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience")
	FGameplayTagContainer OwnedTags;

	/** The level to open to begin this experience. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowedTypes = "Map"), Category = "Experience")
	FPrimaryAssetId Level;

	/** The experience to use. Optional depending on whether the Level or game mode defines a default experience. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowedTypes = "GameExperienceDef"), Category = "Experience")
	FPrimaryAssetId GameExperience;

	/** Additional arguments passed as URL options to the game. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience")
	TMap<FString, FString> ExtraArgs;

	/** Max number of players allowed in the session. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience)
	int32 MaxPlayerCount = 16;

	/** UI data used to display this entry point to the user in menus, etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "UIData")
	TObjectPtr<UGameExperienceEntryPointUIData> UIData;

	/** Create a request for hosting a session to launch this experience online. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	UCommonSession_HostSessionRequest* CreateOnlineHostSessionRequest() const;

	/** Create a request for hosting a session to launch this experience offline. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	UCommonSession_HostSessionRequest* CreateOfflineHostSessionRequest() const;
};
