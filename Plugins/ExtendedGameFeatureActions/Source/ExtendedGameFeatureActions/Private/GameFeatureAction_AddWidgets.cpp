// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameFeatureAction_AddWidgets.h"

#include "CommonActivatableWidget.h"
#include "CommonUIExtensions.h"
#include "UIExtensionSystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerState.h"


UGameFeatureAction_AddWidgets::UGameFeatureAction_AddWidgets()
	: ActorClass(AHUD::StaticClass())
{
}

UGameFeatureWorldAction::FContextHandles* UGameFeatureAction_AddWidgets::AllocContextHandles() const
{
	return new FWidgetContextHandles();
}

void UGameFeatureAction_AddWidgets::Reset(FContextHandles& Handles)
{
	Super::Reset(Handles);

	FWidgetContextHandles& WidgetHandles = static_cast<FWidgetContextHandles&>(Handles);

	for (auto& Elem : WidgetHandles.ActorData)
	{
		// deactivate layouts
		for (TWeakObjectPtr<UCommonActivatableWidget>& Layout : Elem.Value.Layouts)
		{
			if (Layout.IsValid())
			{
				Layout->DeactivateWidget();
			}
		}
		// unregister ui extension points
		for (FUIExtensionHandle& ExtensionHandle : Elem.Value.ExtensionHandles)
		{
			ExtensionHandle.Unregister();
		}
	}

	WidgetHandles.ActorData.Empty();
}

void UGameFeatureAction_AddWidgets::AddToWorld(const FWorldContext& WorldContext, FGameFeatureStateChangeContext ChangeContext)
{
	const UWorld* World = WorldContext.World();
	const UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	if (!GameInstance || !World || !World->IsGameWorld())
	{
		return;
	}

	UGameFrameworkComponentManager* ComponentManager = GameInstance->GetSubsystem<UGameFrameworkComponentManager>();
	if (!ComponentManager)
	{
		return;
	}

	FWidgetContextHandles& Handles = FindOrAddContextHandles<FWidgetContextHandles>(ChangeContext);

	// listen for actor registration
	const TSoftClassPtr<AActor> ActorClassPtr = !ActorClass.IsNull() ? ActorClass : AHUD::StaticClass();

	const TSharedPtr<FComponentRequestHandle> RequestHandle = ComponentManager->AddExtensionHandler(
		ActorClassPtr, UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtension, ChangeContext));

	Handles.ComponentRequestHandles.Add(RequestHandle);
}

void UGameFeatureAction_AddWidgets::HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	FWidgetContextHandles& Handles = FindOrAddContextHandles<FWidgetContextHandles>(ChangeContext);

	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded ||
		EventName == UGameFrameworkComponentManager::NAME_ReceiverAdded)
	{
		AddWidgets(Actor, Handles);
	}
	else if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved ||
		EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
	{
		RemoveWidgets(Actor, Handles);
	}
}

ULocalPlayer* UGameFeatureAction_AddWidgets::GetLocalPlayerFromActor(AActor* Actor) const
{
	if (const AHUD* HUD = Cast<AHUD>(Actor))
	{
		return HUD->GetOwningPlayerController() ? HUD->GetOwningPlayerController()->GetLocalPlayer() : nullptr;
	}
	if (const APlayerController* Player = Cast<APlayerController>(Actor))
	{
		return Player->GetLocalPlayer();
	}
	if (const APlayerState* PlayerState = Cast<APlayerState>(Actor))
	{
		// note that in networked games, Owner (the player controller)
		// will likely not be replicated yet. Use a custom component extension event
		// and override HandleActorExtension to listen for it (instead of ReceiverAdded) if needed.
		if (PlayerState->GetPlayerController())
		{
			return PlayerState->GetPlayerController()->GetLocalPlayer();
		}
		else
		{
			UE_CLOG(!PlayerState->GetOwner(), LogGameFeatures, Warning,
				TEXT("[%hs] PlayerState has no Owner, use a custom extension event for networked games (see comment)."), __FUNCTION__);
		}
		return nullptr;
	}
	UE_LOG(LogGameFeatures, Warning, TEXT("[%hs] Unsupported ActorClass: %s"),
		__FUNCTION__, *Actor->GetClass()->GetName());
	return nullptr;
}

void UGameFeatureAction_AddWidgets::AddWidgets(AActor* Actor, FWidgetContextHandles& Handles)
{
	ULocalPlayer* LocalPlayer = GetLocalPlayerFromActor(Actor);
	if (!LocalPlayer)
	{
		// only operate on local players
		return;
	}

	FActorHandlesData& ActorData = Handles.ActorData.FindOrAdd(Actor);

	// add primary game layout widgets
	for (const FGameFeatureLayoutWidgetEntry& Entry : Layouts)
	{
		if (const TSubclassOf<UCommonActivatableWidget> WidgetClass = Entry.WidgetClass.Get())
		{
			UCommonActivatableWidget* Layout = UCommonUIExtensions::PushContentToLayer_ForPlayer(LocalPlayer, Entry.Layer, WidgetClass);
			ActorData.Layouts.Add(Layout);
		}
	}

	// add all extension point widgets
	UUIExtensionSubsystem* ExtensionSubsystem = Actor->GetWorld()->GetSubsystem<UUIExtensionSubsystem>();
	for (const FGameFeatureExtensionWidgetEntry& Entry : Widgets)
	{
		FUIExtensionHandle ExtensionHandle = ExtensionSubsystem->RegisterExtensionAsWidgetForContext(
			Entry.ExtensionPoint, LocalPlayer, Entry.WidgetClass.Get(), -1);
		ActorData.ExtensionHandles.Add(ExtensionHandle);
	}
}

void UGameFeatureAction_AddWidgets::RemoveWidgets(AActor* Actor, FWidgetContextHandles& Handles)
{
	FActorHandlesData* ActorData = Handles.ActorData.Find(Actor);
	if (!ActorData)
	{
		return;
	}

	for (const TWeakObjectPtr<UCommonActivatableWidget>& Layout : ActorData->Layouts)
	{
		if (Layout.IsValid())
		{
			Layout->DeactivateWidget();
		}
	}
	for (FUIExtensionHandle& ExtensionHandle : ActorData->ExtensionHandles)
	{
		ExtensionHandle.Unregister();
	}

	Handles.ActorData.Remove(Actor);
}
