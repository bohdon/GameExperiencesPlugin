# Game Experiences Plugin

An Unreal plugin for defining modular extensions to game modes that leverage the GameFeatures plugin. Based on Lyra experiences.

# Extended Game Feature Actions Plugin

An accompanying plugin that provides additional game feature actions for:

- Adding gameplay abilities and gameplay cue paths.
- Adding widgets to the scren using Primary Game Layout from Lyra's CommonGame plugin.

### Features

- `UGameFeatureAction_AddWidgets` - Add a widget or screen layout using the PrimaryGameLayout and/or UI extension points.
- `UGameFeatureAction_AddAbilities` - Add gameplay abilities to an actor using extended ability sets.
- `UGameFeatureAction_AddGameplayCuePaths` - Add additional gameplay cue paths to search.

### Plugin Dependencies

This plugin is intended to be setup alongside a number of other third-party plugins to get going
quickly. Specific game feature actions and dependencies can be removed easily if you need
only some of them.

TODO: add defines that can compile out unwanted feature actions and dependencies?

- GameFeatures
- ModularGameplay
- CommonUI
- GameplayAbilities
- [ExtendedGameplayAbilities](https://github.com/bohdon/ExtendedGameplayAbilitiesPlugin)
- [GameItems](https://github.com/bohdon/GameItemsPlugin)
- [CommonGame](https://github.com/EpicGames/UnrealEngine/tree/ue5-main/Samples/Games/Lyra/Plugins/CommonGame) from Lyra \*
- [UIExtension](https://github.com/EpicGames/UnrealEngine/tree/ue5-main/Samples/Games/Lyra/Plugins/UIExtension) from Lyra \*

_\* These links currently point the ue5-main branch. Switch to a release branch, or use the markeplace to download Lyra for your specific engine release version._
