// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameFeatureAction_AddWidgets.h"

#include "CommonActivatableWidget.h"
#include "CommonUIExtensions.h"
#include "UIExtensionSystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/HUD.h"


UGameFeatureAction_AddWidgets::UGameFeatureAction_AddWidgets()
	: HUDClass(AHUD::StaticClass())
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
		for (TWeakObjectPtr<UCommonActivatableWidget>& Layout : Elem.Value.LayoutsAdded)
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

	// register for HUD actors
	TSoftClassPtr<AActor> ActorClass = !HUDClass.IsNull() ? HUDClass : AHUD::StaticClass();

	TSharedPtr<FComponentRequestHandle> RequestHandle = ComponentManager->AddExtensionHandler(
		ActorClass, UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtension, ChangeContext));

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

void UGameFeatureAction_AddWidgets::AddWidgets(AActor* Actor, FWidgetContextHandles& Handles)
{
	AHUD* HUD = CastChecked<AHUD>(Actor);

	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(HUD->GetOwningPlayerController()->Player);
	if (!LocalPlayer)
	{
		// only operate on local players
		return;
	}

	FActorHandlesData& ActorData = Handles.ActorData.FindOrAdd(Actor);

	// add primary game layout widgets
	for (const FGameFeatureLayoutWidgetEntry& Entry : Layouts)
	{
		if (TSubclassOf<UCommonActivatableWidget> WidgetClass = Entry.WidgetClass.Get())
		{
			UCommonActivatableWidget* Layout = UCommonUIExtensions::PushContentToLayer_ForPlayer(LocalPlayer, Entry.Layer, WidgetClass);
			ActorData.LayoutsAdded.Add(Layout);
		}
	}

	// add all extension point widgets
	UUIExtensionSubsystem* ExtensionSubsystem = HUD->GetWorld()->GetSubsystem<UUIExtensionSubsystem>();
	for (const FGameFeatureExtensionWidgetEntry& Entry : Widgets)
	{
		FUIExtensionHandle ExtensionHandle = ExtensionSubsystem->RegisterExtensionAsWidgetForContext(Entry.ExtensionPoint, LocalPlayer, Entry.WidgetClass.Get(), -1);
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

	for (const TWeakObjectPtr<UCommonActivatableWidget>& Layout : ActorData->LayoutsAdded)
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
