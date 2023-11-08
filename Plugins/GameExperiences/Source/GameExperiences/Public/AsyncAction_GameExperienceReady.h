// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameExperienceComponent.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_GameExperienceReady.generated.h"

class UGameExperienceDef;


/**
 * Waits for the game experience to be ready, then calls OnReady.
 * If the experience is already active, OnReady will be called immediately.
 */
UCLASS()
class GAMEEXPERIENCES_API UAsyncAction_GameExperienceReady : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = true))
	static UAsyncAction_GameExperienceReady* WaitForGameExperienceReady(UObject* WorldContextObject, TEnumAsByte<EGameExperienceLoadEventPriority> Priority);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReadyDynDelegate);

	/** Called when the experience is ready, or immediately if it is already. */
	UPROPERTY(BlueprintAssignable)
	FOnReadyDynDelegate OnReady;

	virtual void Activate() override;

protected:
	/** The world whose experience is loading. */
	TWeakObjectPtr<UWorld> GameWorld;

	/** The priority to use for the experience loaded callback. */
	TEnumAsByte<EGameExperienceLoadEventPriority> Priority;

	void OnGameStateAvailable(AGameStateBase* GameState);
	void OnExperienceLoaded(const UGameExperienceDef* CurrentExperience);
};
