// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameFeatureAction_AddGameplayCuePaths.h"

#include "AbilitySystemGlobals.h"
#include "GameFeatureData.h"
#include "GameFeaturesSubsystem.h"
#include "GameplayCueManager.h"
#include "GameplayCueSet.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif


#define LOCTEXT_NAMESPACE "UGameFeatureAction_AddGameplayCuePaths"


// UGameFeatureAction_AddGameplayCuePaths
// --------------------------------------

UGameFeatureAction_AddGameplayCuePaths::UGameFeatureAction_AddGameplayCuePaths()
{
	GameplayCuePaths.Add(FDirectoryPath{TEXT("/GameplayCues")});
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddGameplayCuePaths::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	for (const FDirectoryPath& GameplayCuePath : GameplayCuePaths)
	{
		if (GameplayCuePath.Path.IsEmpty())
		{
			Context.AddError(FText::Format(LOCTEXT("InvalidCuePathError", "'{0}' is not a valid path"), FText::FromString(GameplayCuePath.Path)));
			Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
		}
	}

	return CombineDataValidationResults(Result, EDataValidationResult::Valid);
}
#endif // WITH_EDITOR


// UGameFeature_AddGameplayCuePaths
// --------------------------------

void UGameFeature_AddGameplayCuePaths::OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UGameFeature_AddGameplayCuePaths::OnGameFeatureRegistering);

	UGameplayCueManager* GameplayCueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager();
	if (!GameplayCueManager)
	{
		return;
	}

	// register paths for each AddGameplayCuePaths action
	for (UGameFeatureAction* Action : GameFeatureData->GetActions())
	{
		const UGameFeatureAction_AddGameplayCuePaths* AddCuePathsAction = Cast<UGameFeatureAction_AddGameplayCuePaths>(Action);
		if (!AddCuePathsAction || AddCuePathsAction->GameplayCuePaths.IsEmpty())
		{
			continue;
		}

		// add all paths from the action
		for (const FDirectoryPath& PathToAdd : AddCuePathsAction->GameplayCuePaths)
		{
			FString AdjustedPath = FixGameplayCuePath(PathToAdd, PluginName);
			GameplayCueManager->AddGameplayCueNotifyPath(AdjustedPath);
		}

		// rebuild cue library
		GameplayCueManager->InitializeRuntimeObjectLibrary();
	}
}

void UGameFeature_AddGameplayCuePaths::OnGameFeatureUnregistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UGameFeature_AddGameplayCuePaths::OnGameFeatureUnregistering);

	UGameplayCueManager* GameplayCueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager();
	if (!GameplayCueManager)
	{
		return;
	}

	for (UGameFeatureAction* Action : GameFeatureData->GetActions())
	{
		const UGameFeatureAction_AddGameplayCuePaths* AddCuePathsAction = Cast<UGameFeatureAction_AddGameplayCuePaths>(Action);
		if (!AddCuePathsAction || AddCuePathsAction->GameplayCuePaths.IsEmpty())
		{
			continue;
		}

		// remove all paths from the action
		for (const FDirectoryPath& PathToAdd : AddCuePathsAction->GameplayCuePaths)
		{
			FString AdjustedPath = FixGameplayCuePath(PathToAdd, PluginName);
			GameplayCueManager->RemoveGameplayCueNotifyPath(AdjustedPath, /*bShouldRescanCueAssets*/ false);
		}

		// rebuild cue library
		GameplayCueManager->InitializeRuntimeObjectLibrary();
	}
}

FString UGameFeature_AddGameplayCuePaths::FixGameplayCuePath(const FDirectoryPath& RelativeCuePath, const FString& PluginName)
{
	const FString PluginRootPath = TEXT("/") + PluginName;
	FString AdjustedPath = RelativeCuePath.Path;
	UGameFeaturesSubsystem::FixPluginPackagePath(AdjustedPath, PluginRootPath, /*bMakeRelativeToPluginRoot*/ false);
	return AdjustedPath;
}

#undef LOCTEXT_NAMESPACE
