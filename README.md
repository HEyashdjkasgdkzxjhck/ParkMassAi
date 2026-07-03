# ParkMassAI README — Beginner-Friendly Usage Guide

> Scope: This guide covers the `ParkMassAI` plugin, including pedestrian Mass Wander, ZoneGraph setup and baking, random alert demos, fixed-point alert spawning, external `PointName` alert triggering, CustomStencil highlight/post-process setup, and all exposed Blueprint functions.  
> This document assumes the plugin is intended to be self-contained. If your plugin still references `/Game/Characters/Mannequins` or `/Game/Variant_Combat`, migrate those dependencies into the plugin first.

---

## Table of Contents

1. What this plugin does
2. Required plugins in a new project
3. Project settings
4. Content Browser settings
5. Minimum working level setup
6. Creating and baking ZoneGraph paths
7. Spawning normal pedestrians
8. Using `BP_PedestrianEventPoint` for fixed-area alerts
9. Using `BP_PedestrianEventDirector` for random demo alerts
10. Exposed global Blueprint functions
11. `AlertSourceMode` explained
12. Post-process highlight / outline setup
13. Troubleshooting
14. Migration and source-control notes
15. Recommended test flow
16. Minimal Blueprint examples
17. Recommended production usage
18. One-sentence summary

---

# 1. What this plugin does

`ParkMassAI` is a UE Mass / ZoneGraph based pedestrian plugin.

It supports:

- Spawning pedestrian Mass agents on ZoneGraph lanes.
- Automatic pedestrian wandering.
- Normal pedestrian crowds.
- Random high-risk pedestrian alert demos.
- Fixed-area alert spawning through `BP_PedestrianEventPoint`.
- External alert triggering through `PointName`.
- Alert labels above pedestrians.
- CustomDepth / CustomStencil based post-process highlighting.
- Alert recovery after a duration.
- Optional cleanup of pedestrians spawned by an alert point.
- Global Blueprint functions, so external systems do not need a hard reference to `BP_PedestrianEventDirector`.

---

# 2. Required plugins in a new project

Before using `ParkMassAI` in a new project, open:

```text
Edit
→ Plugins
```

Enable the following plugins. The exact names may vary slightly depending on your UE version:

```text
ParkMassAI

Mass Entity
Mass Gameplay
Mass AI
Mass Crowd
Mass Actors

ZoneGraph
Mass ZoneGraph Navigation

StateTree
StructUtils
```

Restart the editor if prompted.

---

# 3. Project settings

## 3.1 Enable Custom Depth-Stencil

Alert highlighting / outlining depends on `CustomDepth` and `CustomStencil`.

Go to:

```text
Edit
→ Project Settings
→ Rendering
→ Postprocessing
→ Custom Depth-Stencil Pass
```

Set it to:

```text
Enabled with Stencil
```

You can verify it at runtime with the console command:

```text
r.CustomDepth
```

Expected value:

```text
3
```

If it is not `3`, CustomStencil may not work correctly.

---

## 3.2 Recommended Stencil value

For alert pedestrians, the recommended stencil value is:

```text
CustomDepth Stencil Value = 255
```

In the post-process material, use a threshold such as:

```text
CustomStencil > 0.5
```

This is more stable than using `Stencil = 1` with a tiny threshold such as `0.001`.

---

# 4. Content Browser settings

Plugin content may be hidden by default.

In the Content Browser settings, enable:

```text
Show Plugin Content
Show Engine Content, if needed
```

Then you should be able to see:

```text
/ParkMassAI/...
```

Common assets include:

```text
/ParkMassAI/Blueprint/Character/BP_WanderPersonActor
/ParkMassAI/Pedestrian/Events/BP_PedestrianEventPoint
/ParkMassAI/Pedestrian/Events/BP_PedestrianEventDirector
/ParkMassAI/Mass/DA_Mass_Pedestrian_Wander
/ParkMassAI/Mass/MassSpawner_Wander
/ParkMassAI/Characters/AnimBP/ABP_Manny_Mass
/ParkMassAI/Characters/BlendSpaces/BS_Manny_Mass_Walk
```

Your actual paths may differ slightly depending on how the plugin content is organized.

---

# 5. Minimum working level setup

If pedestrians are not spawning in a new project, test the plugin with this minimal setup first.

## Step 1: Enable plugins and restart

Make sure these are enabled:

```text
ParkMassAI
Mass-related plugins
ZoneGraph
StateTree
```

---

## Step 2: Create a ZoneGraph path

The level must contain a usable ZoneGraph path.

The plugin does not automatically create pedestrian lanes for a new map.

---

## Step 3: Place `BP_PedestrianEventPoint`

Drag this asset into the level:

```text
BP_PedestrianEventPoint
```

Set:

```text
PointName = Gate_A
EntityConfig = DA_Mass_Pedestrian_Wander
SpawnCount = 1
SpawnRadius = 50
ClaimRadius = 300
bSpawnAsAlert = true
AlertDuration = 8
bDestroyAfterAlert = false
bAutoSpawnOnBeginPlay = false
bLoop = false
```

Important: `PointName` must be unique within the level.

---

## Step 4: Test with the global Blueprint function

In the Level Blueprint:

```text
Event BeginPlay
→ Delay 1.0
→ ParkMass_SetExternalAlertMode(true)
→ ParkMass_TriggerPointAlert
```

Use these parameters:

```text
PointName = Gate_A
AlertText = Abnormal Behavior
Duration = 8.0
SpawnCount = 1
bDestroyAfterAlert = false
```

If this works, the following parts are valid:

```text
Plugin code
Global Blueprint API
EventPoint
Mass spawning chain
```

If it does not work, check the troubleshooting section.

---

# 6. Creating and baking ZoneGraph paths

## 6.1 Why ZoneGraph is required

`ParkMassAI` pedestrians do not behave like simple `SpawnActor` characters.

The system depends on:

```text
Mass Entity
+
ZoneGraph lanes
+
StateTree / Movement
+
Representation Actor
```

If a new map has no ZoneGraph, you may see:

```text
No pedestrian spawns
Mass entities spawn but no visible actors appear
Pedestrians appear but do not move
Pathfinding fails
```

---

## 6.2 Create a ZoneGraph / ZoneShape

Typical process:

```text
Place Actors
→ Search for ZoneShape or ZoneGraph
→ Drag it into the level
→ Edit the path points
→ Set the Lane Profile / Tags
```

Start with a simple straight or slightly curved path. Do not begin with a complex road network.

For first tests:

```text
Keep the EventPoint very close to the ZoneGraph lane
Set SpawnRadius to 50
Set ClaimRadius to 300
```

---

## 6.3 Lane Profile must match

`DA_Mass_Pedestrian_Wander` must be able to find a compatible ZoneGraph lane.

Check:

```text
ZoneShape / ZoneGraph Lane Profile
Tags required by DA_Mass_Pedestrian_Wander
StateTree / Movement requirements
```

If the Lane Profile does not match, the Mass agent may not find a valid lane.

---

## 6.4 Bake / Build ZoneGraph

After creating or editing a ZoneShape, make sure the ZoneGraph data is updated.

Common workflow:

```text
1. Save the level
2. Build the level
3. If available, run Build ZoneGraph / Rebuild ZoneGraph
4. Enter PIE again
```

If you cannot find a dedicated ZoneGraph build command, run:

```text
Build All Levels
```

Then save the map.

---

## 6.5 Use ZoneGraph debug view

If pedestrians do not move, check the ZoneGraph debug visualization.

Verify:

```text
The level actually has lanes
The lanes are connected
The EventPoint is near the lane
The LaneProfile is correct
The ZoneGraph data has been rebuilt
```

If no lanes are visible in the debug view, the ZoneGraph has not been generated correctly.

---

# 7. Spawning normal pedestrians

For normal pedestrian crowds, place:

```text
MassSpawner_Wander
```

Check:

```text
EntityConfig = DA_Mass_Pedestrian_Wander
SpawnCount > 0
Spawn On BeginPlay / Auto Spawn = true
```

If `MassSpawner_Wander` is not placed in the level, normal pedestrians will not spawn automatically.

---

# 8. Using `BP_PedestrianEventPoint` for fixed-area alerts

`BP_PedestrianEventPoint` is a fixed-area alert spawn point.

## 8.1 Key parameters

```text
PointName
```

A unique alert point name.

External systems and Blueprint functions use this name to trigger the point.

Examples:

```text
Gate_A
Restricted_Area_Entrance
Cafeteria_Door
Dormitory_Entrance
Commercial_Building_Lobby
```

`PointName` must be unique in the current level.

---

```text
EntityConfig
```

The Mass EntityConfig used to spawn the pedestrian.

Usually:

```text
DA_Mass_Pedestrian_Wander
```

---

```text
SpawnCount
```

Number of pedestrians to spawn.

For testing, use:

```text
1
```

---

```text
SpawnRadius
```

Spawn radius around the EventPoint.

Recommended for new-project testing:

```text
50 - 100
```

If the radius is too large, the spawn location may be too far from the ZoneGraph.

---

```text
ClaimRadius
```

The radius used by the EventPoint to claim newly spawned representation actors.

Recommended:

```text
300
```

This prevents the EventPoint from accidentally claiming unrelated pedestrians.

---

```text
bSpawnAsAlert
```

Whether the spawned pedestrian should immediately enter alert state.

---

```text
AlertText
```

The text displayed above the alert pedestrian.

Examples:

```text
Abnormal Behavior
High-risk Person
Suspicious Stay
```

---

```text
AlertDuration
```

Alert duration in seconds.

---

```text
bDestroyAfterAlert
```

Whether to remove the pedestrian spawned by this EventPoint after the alert ends.

```text
true  = remove the spawned pedestrian after the alert
false = keep the pedestrian and only clear the alert state
```

---

```text
bAutoSpawnOnBeginPlay
```

Whether to automatically spawn on BeginPlay.

If you are triggering alerts through `ParkMass_TriggerPointAlert`, set this to:

```text
false
```

Otherwise, the first trigger may create two pedestrians:

```text
BeginPlay auto spawn
+
TriggerPointAlert spawn
=
Two pedestrians
```

---

```text
bLoop
```

Whether the EventPoint should repeatedly spawn.

For external trigger usage, set:

```text
false
```

---

## 8.2 Recommended configuration for external triggering

```text
PointName = Gate_A
EntityConfig = DA_Mass_Pedestrian_Wander
SpawnCount = 1
SpawnRadius = 50
ClaimRadius = 300
bSpawnAsAlert = true
AlertText = Abnormal Behavior
AlertDuration = 8
bDestroyAfterAlert = true
bAutoSpawnOnBeginPlay = false
bLoop = false
```

---

# 9. Using `BP_PedestrianEventDirector` for random demo alerts

`BP_PedestrianEventDirector` is now mainly a DemoRandom driver.

It is used for:

```text
Randomly selecting an existing pedestrian and turning it into an alert pedestrian
```

It is not the recommended external API entry point.

For external triggering, use the global Blueprint function:

```text
ParkMass_TriggerPointAlert
```

---

## 9.1 When to place the Director

Place:

```text
BP_PedestrianEventDirector
```

only if you want random high-risk pedestrian demo behavior.

The alert source mode should be:

```text
DemoRandom
```

---

## 9.2 Random alerts are disabled in ExternalOnly

After calling:

```text
ParkMass_SetExternalAlertMode(true)
```

the mode becomes:

```text
ExternalOnly
```

The Director will no longer generate random alerts.

---

# 10. Exposed global Blueprint functions

These functions come from:

```text
UParkMassPedestrianAlertBlueprintLibrary
```

They can be called from any Blueprint.

You do not need to get a reference to `BP_PedestrianEventDirector`.

---

## 10.1 `ParkMass_SetExternalAlertMode`

Purpose:

```text
Enable or disable external alert mode
```

Parameter:

```text
bEnabled : bool
```

Behavior:

```text
true:
    AlertSourceMode = ExternalOnly
    Random demo alerts are disabled

false:
    AlertSourceMode = DemoRandom
    Random demo alerts are allowed again
```

Common usage:

```text
Call true before using a backend / WebSocket / external alert source
Call false for standalone demo mode
```

---

## 10.2 `ParkMass_SetPedestrianAlertSourceMode`

Purpose:

```text
Directly set the alert source mode
```

Parameter:

```text
NewMode : EPedestrianAlertSourceMode
```

Available values:

```text
DemoRandom
ExternalOnly
Hybrid
Disabled
```

---

## 10.3 `ParkMass_GetPedestrianAlertSourceMode`

Purpose:

```text
Get the current alert source mode
```

Return:

```text
EPedestrianAlertSourceMode
```

Useful for debugging whether the system is in `ExternalOnly` or `Disabled`.

---

## 10.4 `ParkMass_TriggerPointAlert`

The most important external API.

Purpose:

```text
Spawn an alert pedestrian at a fixed EventPoint by PointName
```

Parameters:

```text
PointName : Name
AlertText : Text
Duration : float
SpawnCount : int
bDestroyAfterAlert : bool
```

Return:

```text
bool
```

Example:

```text
ParkMass_TriggerPointAlert(
    PointName = Gate_A,
    AlertText = Abnormal Behavior,
    Duration = 8.0,
    SpawnCount = 1,
    bDestroyAfterAlert = true
)
```

Behavior:

```text
1. Finds BP_PedestrianEventPoint by PointName
2. If PointName is not found: returns false
3. If PointName is duplicated: returns false
4. If exactly one point is found: spawns alert pedestrian through that EventPoint
```

Important:

```text
PointName must be unique.
PointName is the area key / alert point key.
Do not duplicate it.
```

---

## 10.5 `ParkMass_TriggerActorAlert`

Purpose:

```text
Turn a specific existing pedestrian actor into an alert pedestrian
```

Parameters:

```text
TargetActor : Actor
AlertText : Text
Duration : float
```

Return:

```text
bool
```

Use this when you already know the specific pedestrian actor.

---

## 10.6 `ParkMass_TriggerRandomPedestrianAlert`

Purpose:

```text
Randomly select an existing pedestrian and put it into alert state
```

Parameters:

```text
AlertText : Text
Duration : float
```

Return:

```text
bool
```

Mode rules:

```text
DemoRandom: allowed
Hybrid: allowed
ExternalOnly: rejected
Disabled: rejected
```

---

## 10.7 `ParkMass_ClearAllPedestrianAlerts`

Purpose:

```text
Clear all active pedestrian alert states
```

Return:

```text
int
```

The return value is the number of alert pedestrians cleared.

Note: this clears alert state. It does not necessarily delete pedestrians.

---

# 11. `AlertSourceMode` explained

Enum:

```text
EPedestrianAlertSourceMode
```

---

## DemoRandom

Random demo mode.

```text
Random alerts are allowed
External TriggerPointAlert is also allowed
```

Use for:

```text
Standalone demo
Development testing
```

---

## ExternalOnly

External API mode.

```text
Random alerts are disabled
PointName external triggering is allowed
Actor external triggering is allowed
```

Use for:

```text
WebSocket integration
GameplayMessageRouter integration
Backend event integration
Production demo
```

Recommended production call:

```text
ParkMass_SetExternalAlertMode(true)
```

---

## Hybrid

Mixed mode.

```text
Random alerts are allowed
External alerts are also allowed
```

Use only for debugging.

Not recommended for formal demos because it can be hard to tell whether an alert came from the backend or the random demo driver.

---

## Disabled

Everything is disabled.

```text
Random alerts are disabled
PointName triggering is disabled
Actor triggering is disabled
```

Use for:

```text
Temporarily disabling the alert system
Troubleshooting
```

---

# 12. Post-process highlight / outline setup

## 12.1 Add a PostProcessVolume

The level needs:

```text
PostProcessVolume
```

Recommended setting:

```text
Infinite Extent / Unbound = true
```

Then add the plugin alert post-process material under:

```text
Post Process Materials
```

Example:

```text
M_PP_PedestrianAlertHighlight
```

The actual asset name may differ in your plugin.

---

## 12.2 If the whole screen is outlined or tinted

Usually the mask is becoming full-screen `1`.

Check:

```text
1. Is Custom Depth-Stencil Pass set to Enabled with Stencil?
2. Is r.CustomDepth equal to 3?
3. Check Buffer Visualization → Custom Stencil.
4. Is only the alert pedestrian visible in the stencil buffer?
5. Are the floor, sky, buildings, water, or glass also using Render CustomDepth?
6. Does the post-process material use CustomStencil > 0.5?
```

Expected result:

```text
Only the alert pedestrian has a stencil value.
Everything else should be black in Custom Stencil view.
```

If the whole screen has a stencil value, a large object is probably rendering to CustomDepth.

---

# 13. Troubleshooting

## 13.1 Pedestrians do not spawn

Check:

```text
Are Mass-related plugins enabled?
Does the level have a ZoneGraph path?
Has the ZoneGraph been built?
Is BP_PedestrianEventPoint close to the ZoneGraph lane?
Is EntityConfig set to DA_Mass_Pedestrian_Wander?
Is SpawnCount greater than 0?
Does PointName match?
Is AlertSourceMode set to Disabled?
```

---

## 13.2 `ParkMass_TriggerPointAlert` returns false

Common causes:

```text
PointName not found
PointName duplicated
AlertSourceMode = Disabled
Invalid WorldContextObject
Subsystem could not be found
```

Fix:

```text
Make sure BP_PedestrianEventPoint exists in the level
Make sure PointName matches exactly
Make sure PointName is unique
Make sure the mode is not Disabled
```

---

## 13.3 Function returns true, but no pedestrian is visible

Check:

```text
Is EventPoint EntityConfig valid?
Is DA_Mass_Pedestrian_Wander valid?
Is EventPoint too far from ZoneGraph?
Is SpawnRadius too large?
Is Mass Representation Actor valid?
Are BP_WanderPersonActor Mesh and AnimClass valid?
```

---

## 13.4 Pedestrian appears but does not move

Check:

```text
Has ZoneGraph been built?
Does the LaneProfile match?
Is the StateTree valid?
Is Mass Movement working?
Is EventPoint close to the lane?
```

---

## 13.5 First spawn creates two pedestrians

Most common cause:

```text
bAutoSpawnOnBeginPlay = true
+
ParkMass_TriggerPointAlert is also called
```

Fix:

```text
When using external triggering:
bAutoSpawnOnBeginPlay = false
bLoop = false
SpawnCount = 1
```

Another possible cause:

```text
DemoRandom is still enabled
+
PointName external trigger is also being called
```

Fix:

```text
ParkMass_SetExternalAlertMode(true)
```

---

## 13.6 Random alert does not appear

Check:

```text
Is BP_PedestrianEventDirector placed in the level?
Is AlertSourceMode set to DemoRandom or Hybrid?
Are there available normal pedestrians?
Are all pedestrians already in alert state?
```

If the mode is:

```text
ExternalOnly
Disabled
```

random alerts will not appear. This is expected.

---

## 13.7 Alert pedestrian has no highlight

Check:

```text
Is Custom Depth-Stencil Pass set to Enabled with Stencil?
Is r.CustomDepth equal to 3?
Is PostProcessVolume Unbound?
Is the post-process material added to Post Process Materials?
Is alert pedestrian Mesh using Render CustomDepth = true?
Is the Stencil value 255?
```

---

# 14. Migration and source-control notes

Recommended files to commit:

```text
Plugins/ParkMassAI/ParkMassAI.uplugin
Plugins/ParkMassAI/Source/
Plugins/ParkMassAI/Content/
Plugins/ParkMassAI/Resources/
```

Do not commit:

```text
Plugins/ParkMassAI/Binaries/
Plugins/ParkMassAI/Intermediate/
Plugins/ParkMassAI/Saved/
Plugins/ParkMassAI/DerivedDataCache/
```

Recommended `.gitignore`:

```gitignore
Plugins/ParkMassAI/Binaries/
Plugins/ParkMassAI/Intermediate/
Plugins/ParkMassAI/Saved/
Plugins/ParkMassAI/DerivedDataCache/
```

For a fully self-contained plugin, make sure plugin assets do not reference:

```text
/Game/Characters/Mannequins
/Game/Variant_Combat
```

Use Reference Viewer or an Asset Registry / Python dependency scan to verify.

---

# 15. Recommended test flow

For first-time testing in a new project:

```text
1. Enable ParkMassAI and Mass / ZoneGraph / StateTree related plugins
2. Restart the editor
3. Show Plugin Content
4. Create an empty level
5. Create a simple ZoneGraph path
6. Build / Rebuild ZoneGraph
7. Place BP_PedestrianEventPoint
8. Set PointName = Gate_A
9. Set EntityConfig = DA_Mass_Pedestrian_Wander
10. Set SpawnCount = 1
11. Set SpawnRadius = 50
12. Set ClaimRadius = 300
13. Set bAutoSpawnOnBeginPlay = false
14. Set bLoop = false
15. Place a PostProcessVolume and set it to Unbound
16. Add the alert post-process material
17. In Level Blueprint, call ParkMass_SetExternalAlertMode(true)
18. Delay 1 second
19. Call ParkMass_TriggerPointAlert("Gate_A", "Abnormal Behavior", 8.0, 1, false)
```

Expected result:

```text
One alert pedestrian spawns near the specified EventPoint
The label appears above the pedestrian
The pedestrian is highlighted / outlined
The pedestrian can move normally
The alert clears after 8 seconds
```

---

# 16. Minimal Blueprint examples

## External trigger for a fixed area alert

```text
Event BeginPlay
→ Delay 1.0
→ ParkMass_SetExternalAlertMode(true)
→ ParkMass_TriggerPointAlert
    PointName = Gate_A
    AlertText = Abnormal Behavior
    Duration = 8.0
    SpawnCount = 1
    bDestroyAfterAlert = false
```

---

## Clear all alerts

```text
Keyboard C
→ ParkMass_ClearAllPedestrianAlerts
```

---

## Temporarily disable all alert triggers

```text
ParkMass_SetPedestrianAlertSourceMode(Disabled)
```

---

## Restore random demo mode

```text
ParkMass_SetExternalAlertMode(false)
```

---

# 17. Recommended production usage

For backend / external alert integration:

```text
1. On startup, call ParkMass_SetExternalAlertMode(true)
2. Receive an alert message from backend
3. Parse pointName / alertText / duration / spawnCount / destroyAfterAlert
4. Call ParkMass_TriggerPointAlert
```

Example backend-style message:

```json
{
  "pointName": "Gate_A",
  "alertText": "Abnormal Behavior",
  "duration": 8,
  "spawnCount": 1,
  "destroyAfterAlert": true
}
```

Equivalent UE call:

```text
ParkMass_TriggerPointAlert(
    PointName = Gate_A,
    AlertText = Abnormal Behavior,
    Duration = 8.0,
    SpawnCount = 1,
    bDestroyAfterAlert = true
)
```

---

# 18. One-sentence summary

The core logic of `ParkMassAI` is:

```text
ZoneGraph provides the walking lanes
MassSpawner / EventPoint spawns pedestrians
BP_WanderPersonActor represents the pedestrian
SetAlertState controls alert visuals
PostProcess provides highlight / outline
ParkMass_TriggerPointAlert triggers fixed-area alerts by PointName
```

If pedestrians do not spawn in a new project, check these first:

```text
Does ZoneGraph exist and has it been built?
Is EventPoint near the ZoneGraph lane?
Is EntityConfig valid?
Does PointName match?
Is AlertSourceMode set to Disabled?
Are Mass-related plugins enabled?
```

