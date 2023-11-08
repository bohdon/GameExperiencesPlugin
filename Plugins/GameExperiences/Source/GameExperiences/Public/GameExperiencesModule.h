// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

GAMEEXPERIENCES_API DECLARE_LOG_CATEGORY_EXTERN(LogGameExperience, Log, All);


class FGameExperiencesModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
