// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameExperienceComponent.h"

#include "GameExperienceActionSet.h"
#include "GameExperiencesModule.h"
#include "GameExperienceWorldSettings.h"
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeaturesSubsystemSettings.h"
#include "TimerManager.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Net/UnrealNetwork.h"


TAutoConsoleVariable CVarGameExperienceDebugLoadDelay(
	TEXT("experience.debug.LoadDelay"),
	0.f,
	TEXT("Delays the load completion of experiences (in addition to LoadDelayRandom) for debugging."));

TAutoConsoleVariable CVarGameExperienceDebugLoadRandomDelay(
	TEXT("experience.debug.LoadDelayRandom"),
	0.f,
	TEXT("Delays the load completion of experiences between 0..RandomDelay (in addition to LoadDelay) for debugging."));


/** Return the total delay to apply to experience loading for debugging. */
float GetGameExperienceDebugLoadDelay()
{
	const float Delay = CVarGameExperienceDebugLoadDelay.GetValueOnAnyThread();
	const float RandomDelay = FMath::FRand() * CVarGameExperienceDebugLoadRandomDelay.GetValueOnAnyThread();
	return FMath::Max(0.0f, Delay + RandomDelay);
}


UGameExperienceComponent::UGameExperienceComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UGameExperienceComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Experience);
}

FPrimaryAssetId UGameExperienceComponent::GetDesiredGameExperience(FString& OutDebugSource) const
{
	// check various sources in order of priority until one of them provides a game experience
	// 1. Game Mode Options String
	// 2. Command Line
	// 3. World Settings (via IGameExperienceProviderInterface)
	// 4. Game Mode (via IGameExperienceProviderInterface)

	FPrimaryAssetId ExperienceId;

	const AGameModeBase* GameMode = GetGameMode<AGameModeBase>();
	check(GameMode);

	// check game mode options
	if (!ExperienceId.IsValid() && UGameplayStatics::HasOption(GameMode->OptionsString, TEXT("Experience")))
	{
		const FString ExperienceFromOptions = UGameplayStatics::ParseOption(GameMode->OptionsString, TEXT("Experience"));
		ExperienceId = ParseExperiencePrimaryAssetIdFromString(ExperienceFromOptions);
		OutDebugSource = TEXT("OptionsString");
	}

	// check command line
	if (!ExperienceId.IsValid())
	{
		FString ExperienceFromCommandLine;
		if (FParse::Value(FCommandLine::Get(), TEXT("Experience="), ExperienceFromCommandLine))
		{
			ExperienceId = ParseExperiencePrimaryAssetIdFromString(ExperienceFromCommandLine);
			OutDebugSource = TEXT("CommandLine");
		}
	}

	// check world settings
	if (!ExperienceId.IsValid())
	{
		if (const IGameExperienceProviderInterface* WorldSettingsExpProvider = Cast<IGameExperienceProviderInterface>(GameMode->GetWorldSettings()))
		{
			ExperienceId = WorldSettingsExpProvider->GetDesiredGameExperience(OutDebugSource);
		}
	}

	// check game mode
	if (!ExperienceId.IsValid())
	{
		if (const IGameExperienceProviderInterface* GameModeExpProvider = Cast<IGameExperienceProviderInterface>(GameMode))
		{
			ExperienceId = GameModeExpProvider->GetDesiredGameExperience(OutDebugSource);
		}
	}

	// TODO: get fallback default from a project setting?

	return ExperienceId;
}

void UGameExperienceComponent::AutoResolveExperience()
{
	FString DebugExperienceSource;
	const FPrimaryAssetId ExperienceId = GetDesiredGameExperience(DebugExperienceSource);

	if (!ExperienceId.IsValid())
	{
		// no experience was found
		UE_LOG(LogGameExperience, Warning, TEXT("No desired GameExperience found"));
		return;
	}

	UE_LOG(LogGameExperience, Log, TEXT("Using GameExperience '%s' from %s"), *ExperienceId.ToString(), *DebugExperienceSource);

	SetExperience(ExperienceId);
}

void UGameExperienceComponent::SetExperience(const FPrimaryAssetId& ExperienceId)
{
	const UAssetManager& AssetManager = UAssetManager::Get();
	const FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(ExperienceId);
	const TSubclassOf<UGameExperienceDef> ExperienceClass = Cast<UClass>(AssetPath.TryLoad());

	Experience = ExperienceClass->GetDefaultObject<UGameExperienceDef>();
	check(Experience);

	LoadExperience();
}

bool UGameExperienceComponent::IsExperienceLoaded() const
{
	return Experience && LoadState == EGameExperienceLoadState::Loaded;
}

void UGameExperienceComponent::CallOrRegisterOnExperienceLoaded(FOnGameExperienceLoaded::FDelegate&& Delegate, EGameExperienceLoadEventPriority Priority)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(Experience);
	}
	else
	{
		switch (Priority)
		{
		default:
		case EGameExperienceLoadEventPriority::Normal:
			OnExperienceLoadedEvent.Add(MoveTemp(Delegate));
			break;
		case EGameExperienceLoadEventPriority::High:
			OnExperienceLoadedEvent_HighPriority.Add(MoveTemp(Delegate));
			break;
		case EGameExperienceLoadEventPriority::Low:
			OnExperienceLoadedEvent_LowPriority.Add(MoveTemp(Delegate));
			break;
		}
	}
}

void UGameExperienceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	DeactivateExperience();
}

void UGameExperienceComponent::OnRep_Experience()
{
	LoadExperience();
}

void UGameExperienceComponent::LoadExperience()
{
	check(LoadState == EGameExperienceLoadState::Unloaded);
	check(Experience);

	LoadState = EGameExperienceLoadState::Loading;

	// determine whether to load client/server bundles
	TArray<FName> BundlesToLoad;
	const ENetMode OwnerNetMode = GetOwner()->GetNetMode();
	if (GIsEditor || OwnerNetMode != NM_DedicatedServer)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateClient);
	}
	if (GIsEditor || OwnerNetMode != NM_Client)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateServer);
	}

	// gather list of data assets for which to load bundles
	TSet<FPrimaryAssetId> BundleAssetList;
	BundleAssetList.Add(Experience->GetPrimaryAssetId());
	for (const UGameExperienceActionSet* ActionSet : Experience->ActionSets)
	{
		if (ActionSet)
		{
			BundleAssetList.Add(ActionSet->GetPrimaryAssetId());
		}
	}

	// start bundle load
	UAssetManager& AssetManager = UAssetManager::Get();

	const FStreamableDelegate BundleLoadDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnExperienceAssetsLoaded);

	const TSharedPtr<FStreamableHandle> LoadHandle = AssetManager.ChangeBundleStateForPrimaryAssets(
		BundleAssetList.Array(), BundlesToLoad, {}, false, BundleLoadDelegate, FStreamableManager::AsyncLoadHighPriority);

	if (LoadHandle.IsValid() && !LoadHandle->HasLoadCompleted())
	{
		// ensure delegate is called even when load is canceled
		LoadHandle->BindCancelDelegate(FStreamableDelegate::CreateLambda([BundleLoadDelegate]
		{
			BundleLoadDelegate.ExecuteIfBound();
		}));
	}
}

void UGameExperienceComponent::OnExperienceAssetsLoaded()
{
	check(LoadState == EGameExperienceLoadState::Loading);

	LoadGameFeaturePlugins();
}

void UGameExperienceComponent::LoadGameFeaturePlugins()
{
	check(Experience);

	GameFeaturePluginURLs.Reset();

	for (const UGameExperienceActionSet* ActionSet : Experience->ActionSets)
	{
		for (const FString& PluginName : ActionSet->GameFeatures)
		{
			FString PluginURL;
			if (UGameFeaturesSubsystem::Get().GetPluginURLByName(PluginName, PluginURL))
			{
				GameFeaturePluginURLs.AddUnique(PluginURL);
			}
			else
			{
				ensureMsgf(false, TEXT("UGameExperienceComponent::OnExperienceAssetsLoaded: Failed to find game feature plugin %s for experience %s"),
				           *PluginName, *Experience->GetName());
			}
		}
	}

	NumFeaturePluginsLoading = GameFeaturePluginURLs.Num();
	if (NumFeaturePluginsLoading > 0)
	{
		LoadState = EGameExperienceLoadState::LoadingGameFeatures;

		for (FString& PluginURL : GameFeaturePluginURLs)
		{
			UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin(
				PluginURL, FGameFeaturePluginLoadComplete::CreateUObject(this, &ThisClass::OnGameFeaturePluginLoaded));
		}
	}
	else
	{
		OnExperienceLoaded();
	}
}

void UGameExperienceComponent::OnGameFeaturePluginLoaded(const UE::GameFeatures::FResult& Result)
{
	--NumFeaturePluginsLoading;

	// continue once all plugins are loaded
	if (NumFeaturePluginsLoading == 0)
	{
		OnExperienceLoaded();
	}
}

void UGameExperienceComponent::OnExperienceLoaded()
{
	check(LoadState == EGameExperienceLoadState::Loading ||
		LoadState == EGameExperienceLoadState::LoadingGameFeatures ||
		LoadState == EGameExperienceLoadState::DebugDelay);

	if (LoadState != EGameExperienceLoadState::DebugDelay)
	{
		const float Delay = GetGameExperienceDebugLoadDelay();
		if (Delay > 0.f)
		{
			LoadState = EGameExperienceLoadState::DebugDelay;

			// schedule calling this function again after the delay
			FTimerHandle DebugDelayHandle;
			GetWorldTimerManager().SetTimer(DebugDelayHandle, this, &ThisClass::OnExperienceLoaded, Delay, /*InbLoop*/ false);
			return;
		}
	}

	ExecuteActions();

	LoadState = EGameExperienceLoadState::Loaded;

	// broadcast events
	OnExperienceLoadedEvent_HighPriority.Broadcast(Experience);
	OnExperienceLoadedEvent_HighPriority.Clear();

	OnExperienceLoadedEvent.Broadcast(Experience);
	OnExperienceLoadedEvent.Clear();

	OnExperienceLoadedEvent_LowPriority.Broadcast(Experience);
	OnExperienceLoadedEvent_LowPriority.Clear();

	UE_LOG(LogGameExperience, Verbose, TEXT("GameExperience load completed: %s"), *GetNameSafe(Experience));
}

void UGameExperienceComponent::ExecuteActions()
{
	LoadState = EGameExperienceLoadState::ExecutingActions;

	FGameFeatureActivatingContext Context;
	if (const FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(GetWorld()))
	{
		Context.SetRequiredWorldContextHandle(WorldContext->ContextHandle);
	}

	for (const UGameExperienceActionSet* ActionSet : Experience->ActionSets)
	{
		if (!ActionSet)
		{
			continue;
		}
		for (UGameFeatureAction* Action : ActionSet->Actions)
		{
			if (!Action)
			{
				continue;
			}

			Action->OnGameFeatureRegistering();
			Action->OnGameFeatureLoading();
			Action->OnGameFeatureActivating(Context);
		}
	}
}

void UGameExperienceComponent::DeactivateExperience()
{
	for (const FString& PluginURL : GameFeaturePluginURLs)
	{
		UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin(PluginURL);
	}

	if (LoadState == EGameExperienceLoadState::Loaded)
	{
		LoadState = EGameExperienceLoadState::Deactivating;

		NumExpectedPausers = INDEX_NONE;
		NumPausers = 0;

		// setup a callback for deactivate complete
		FGameFeatureDeactivatingContext Context(TEXT(""), [this](FStringView InPauserTag)
		{
			OnActionDeactivationCompleted();
		});

		if (const FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(GetWorld()))
		{
			Context.SetRequiredWorldContextHandle(WorldContext->ContextHandle);
		}

		// deactivate all the actions
		for (const UGameExperienceActionSet* ActionSet : Experience->ActionSets)
		{
			if (!ActionSet)
			{
				continue;
			}

			for (UGameFeatureAction* Action : ActionSet->Actions)
			{
				if (!Action)
				{
					continue;
				}

				Action->OnGameFeatureDeactivating(Context);
				Action->OnGameFeatureUnregistering();
			}
		}

		NumExpectedPausers = Context.GetNumPausers();

		if (NumExpectedPausers == NumPausers)
		{
			OnAllActionsDeactivated();
		}
	}
}

void UGameExperienceComponent::OnActionDeactivationCompleted()
{
	++NumPausers;

	if (NumPausers == NumExpectedPausers)
	{
		OnAllActionsDeactivated();
	}
}

void UGameExperienceComponent::OnAllActionsDeactivated()
{
	// TODO: actually unload, don't just deactivate
	LoadState = EGameExperienceLoadState::Unloaded;
	Experience = nullptr;

	NumExpectedPausers = 0;
	NumPausers = 0;
	GameFeaturePluginURLs.Reset();
}

FPrimaryAssetId UGameExperienceComponent::ParseExperiencePrimaryAssetIdFromString(const FString& ExperienceIdString)
{
	FPrimaryAssetId Result = FPrimaryAssetId::ParseTypeAndName(ExperienceIdString);
	if (!Result.PrimaryAssetType.IsValid())
	{
		Result = FPrimaryAssetId(FPrimaryAssetType(UGameExperienceDef::StaticClass()->GetFName()), FName(*ExperienceIdString));
	}
	return Result;
}
