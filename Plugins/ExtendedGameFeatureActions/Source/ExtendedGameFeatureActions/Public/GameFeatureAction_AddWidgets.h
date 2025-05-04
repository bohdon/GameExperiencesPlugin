// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureWorldAction.h"
#include "GameplayTagContainer.h"
#include "UIExtensionSystem.h"
#include "GameFeatureAction_AddWidgets.generated.h"

class AHUD;
class UCommonActivatableWidget;


USTRUCT(BlueprintType)
struct FGameFeatureLayoutWidgetEntry
{
	GENERATED_BODY()

	/** The layout widget to add. */
	UPROPERTY(EditAnywhere, Meta = (AssetBundles = "Client"), Category = "UI")
	TSoftClassPtr<UCommonActivatableWidget> WidgetClass;

	/** The layer where the widget should be added. */
	UPROPERTY(EditAnywhere, Category = "UI")
	FGameplayTag Layer;
};


USTRUCT(BlueprintType)
struct FGameFeatureExtensionWidgetEntry
{
	GENERATED_BODY()

	/** The widget to add. */
	UPROPERTY(EditAnywhere, Meta = (AssetBundles = "Client"), Category = "UI")
	TSoftClassPtr<UUserWidget> WidgetClass;

	/** The extension point where the widget should be added. */
	UPROPERTY(EditAnywhere, Category = "UI")
	FGameplayTag ExtensionPoint;
};


/**
 * Adds UI widgets using the CommonGame plugin's PrimaryGameLayout and/or ui extension points.
 */
UCLASS(DisplayName = "Add Widgets")
class EXTENDEDGAMEFEATUREACTIONS_API UGameFeatureAction_AddWidgets : public UGameFeatureWorldAction
{
	GENERATED_BODY()

public:
	UGameFeatureAction_AddWidgets();

	/** The HUD class to extend. */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSoftClassPtr<AHUD> HUDClass;

	/** List of widgets to add to PrimaryGameLayout layers. */
	UPROPERTY(EditAnywhere, Meta = (TitleProperty = "{Layer} -> {WidgetClass}"), Category = "UI")
	TArray<FGameFeatureLayoutWidgetEntry> Layouts;

	/** List of widgets to add to ui extension points. */
	UPROPERTY(EditAnywhere, Meta = (TitleProperty = "{ExtensionPoint} -> {WidgetClass}"), Category = "UI")
	TArray<FGameFeatureExtensionWidgetEntry> Widgets;

protected:
	struct FActorHandlesData
	{
		/** Layout widget instances that were added for this actor. */
		TArray<TWeakObjectPtr<UCommonActivatableWidget>> LayoutsAdded;
		/** UI extension handles for widgets added. */
		TArray<FUIExtensionHandle> ExtensionHandles;
	};

	struct FWidgetContextHandles : public FContextHandles
	{
		/** Per-actor data about which added widgets. */
		TMap<FObjectKey, FActorHandlesData> ActorData;

		virtual bool IsEmpty() const override
		{
			return FContextHandles::IsEmpty() && ActorData.IsEmpty();
		}
	};

	virtual FContextHandles* AllocContextHandles() const override;
	virtual void Reset(FContextHandles& Handles) override;
	virtual void AddToWorld(const FWorldContext& WorldContext, FGameFeatureStateChangeContext ChangeContext) override;

	void HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);

	void AddWidgets(AActor* Actor, FWidgetContextHandles& Handles);

	void RemoveWidgets(AActor* Actor, FWidgetContextHandles& Handles);
};
