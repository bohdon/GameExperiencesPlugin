// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ModularHUD.generated.h"


/**
 * HUD class that supports extension by game feature plugins.
 */
UCLASS(Blueprintable)
class EXTENDEDGAMEFEATUREACTIONS_API AModularHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void PreInitializeComponents() override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
