# ParkMassAI

ParkMassAI is an Unreal Engine plugin that provides a Mass-based pedestrian wander setup and a lightweight pedestrian alert event system.

The plugin includes:

- Mass pedestrian actor synchronization from Mass entity transforms.
- Wander pedestrian assets for ZoneGraph / MassSpawner workflows.
- A Mass-specific Manny animation setup.
- Alert-capable pedestrian actors with overhead alert UI and custom depth support.
- Event points that can spawn alert pedestrians at named locations.
- A global alert subsystem and Blueprint function library so external systems can trigger alerts without referencing a Director actor.

## Requirements

This plugin was developed in Unreal Engine 5.7.

Enable the Unreal plugins used by the included assets:

- MassGameplay
- MassEntity
- MassActors
- MassSpawner
- MassMovement
- MassCrowd
- MassLOD
- MassRepresentation
- MassAIBehavior
- MassZoneGraphNavigation
- StateTree
- ZoneGraph
- UMG
- Control Rig, if you use the Manny animation blueprint path that references it

The `.uplugin` already enables `MassGameplay`; depending on the target project, you may still need to enable the related Mass, StateTree, and ZoneGraph plugins in the editor.

## Important Content Dependency

Some animation assets in this plugin reference Unreal's Manny template content under `/Game/Characters/Mannequins/...`, including Manny skeleton, mesh, and locomotion animations.

If you migrate this plugin into a clean project, make sure that project also contains the Manny mannequin assets, or migrate those assets into the plugin with Unreal Editor's AssetTools / Content Browser migration workflow.

Do not move `.uasset` files with the Windows file explorer if you need references to update.

## Installation

1. Copy the `ParkMassAI` folder into your project's `Plugins/` directory.
2. Open the project in Unreal Editor.
3. Enable `ParkMassAI` and the required Mass / StateTree / ZoneGraph plugins.
4. Let Unreal rebuild the plugin module.
5. Place the included MassSpawner / EventPoint assets in a level, or use the Blueprint function library from your own gameplay logic.

## Key Runtime Classes

### Pedestrian Actor

`AParkMassPedestrianCharacter` supports:

- `SetAlertState(bool bNewAlert)`
- Alert text display
- Alert widget visibility
- Custom depth stencil setup for alert highlighting

The included `BP_WanderPersonActor` inherits from this class.

### Actor Sync

`UParkMassActorSyncProcessor` synchronizes Mass entity movement to the represented Actor.

It only processes entities with `FParkMassActorSyncTag`, which is added through `UParkMassActorSyncTrait`.

The processor also writes Mass velocity into `CharacterMovementComponent->Velocity` so Animation Blueprints can read Ground Speed.

### Alert Subsystem

`UParkMassPedestrianAlertSubsystem` is a `UWorldSubsystem` and stores the single global alert source mode.

Public functions:

- `SetExternalAlertMode(bool bEnabled)`
- `SetAlertSourceMode(EPedestrianAlertSourceMode NewMode)`
- `GetAlertSourceMode()`
- `IsRandomAlertAllowed()`
- `IsExternalAlertAllowed()`
- `TriggerPointAlert(FName PointName, const FText& AlertText, float Duration, int32 SpawnCount, bool bDestroyAfterAlert)`
- `TriggerActorAlert(AActor* TargetActor, const FText& AlertText, float Duration)`
- `TriggerRandomPedestrianAlert(const FText& AlertText, float Duration)`
- `ClearAllPedestrianAlerts()`

### Global Blueprint Nodes

`UParkMassPedestrianAlertBlueprintLibrary` exposes global Blueprint nodes:

- `ParkMass_SetExternalAlertMode`
- `ParkMass_SetPedestrianAlertSourceMode`
- `ParkMass_GetPedestrianAlertSourceMode`
- `ParkMass_TriggerPointAlert`
- `ParkMass_TriggerActorAlert`
- `ParkMass_TriggerRandomPedestrianAlert`
- `ParkMass_ClearAllPedestrianAlerts`

These nodes use a World Context Object and do not require a `BP_PedestrianEventDirector` reference.

## Alert Source Modes

`EPedestrianAlertSourceMode`:

- `DemoRandom`: Director random demo alerts are allowed. External alerts are also allowed.
- `ExternalOnly`: External point / actor alerts are allowed. Random alerts are blocked.
- `Hybrid`: Random and external alerts are both allowed.
- `Disabled`: All alert triggers are blocked.

## Point Alert Workflow

1. Place `BP_PedestrianEventPoint` in the level.
2. Set its `PointName`. This name must be unique in the level.
3. Call:

```text
ParkMass_TriggerPointAlert(
    PointName,
    AlertText,
    Duration,
    SpawnCount,
    bDestroyAfterAlert
)
```

The subsystem finds the matching EventPoint by `PointName`.

If no point is found, the call returns `false`.

If multiple points have the same `PointName`, the call returns `false` and logs the duplicate actors and locations.

## Director Role

`BP_PedestrianEventDirector` is now only a demo random-alert controller.

External systems should use the global Blueprint nodes instead of holding a Director reference.

Director wrapper functions are kept for compatibility and forward to the subsystem.

## Repository Notes

Commit these directories and files:

- `ParkMassAI.uplugin`
- `Source/`
- `Content/`
- `Resources/`
- `Config/`, if present

Do not commit generated files:

- `Binaries/`
- `Intermediate/`
- `Saved/`
- `DerivedDataCache/`
- `.vs/`
