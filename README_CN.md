# ParkMassAI 中文说明

ParkMassAI 是一个 Unreal Engine 插件，用于提供基于 Mass 的行人 Wander 移动系统，以及轻量级的行人告警事件工具。

当前插件包含：

- 基于 Mass Entity Transform 的 Actor 同步。
- ZoneGraph / MassSpawner 行人 Wander 示例资产。
- 面向 Mass 行人的 Manny 动画蓝图与 BlendSpace 适配。
- 支持告警状态的行人 Actor，包括头顶告警 Widget 和 Custom Depth 高亮支持。
- 可放置在关卡中的行人事件点，用于按 PointName 定点生成告警行人。
- 全局告警 Subsystem 和 Blueprint Function Library，外部系统不需要引用 Director Actor 就能触发告警。

## 适用版本

本插件在 Unreal Engine 5.7 下开发和验证。

## 需要启用的 Unreal 插件

建议在目标项目中启用以下插件：

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
- Control Rig，如果继续使用当前 Manny 动画蓝图相关资源

`ParkMassAI.uplugin` 当前已经声明启用 `MassGameplay`。在干净项目中，如果打开资产或编译时提示缺少模块，请确认 Mass、StateTree、ZoneGraph 相关插件已经启用。

## 重要资源依赖

当前插件不是完全自包含。

部分动画资源仍引用 Unreal Manny 模板资源：

- `/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple`
- `/Game/Characters/Mannequins/Meshes/SK_Mannequin`
- `/Game/Characters/Mannequins/Anims/Unarmed/...`
- `/Game/Characters/Mannequins/Rigs/CR_Mannequin_FootIK`

如果迁移到一个全新的项目，需要保证目标项目里也有这些 Manny 模板资源。

如果后续要让插件完全自包含，请通过 Unreal Editor 的 AssetTools / Content Browser 迁移功能把 Manny 相关资源迁入插件。不要用 Windows 文件管理器直接移动 `.uasset`，否则引用不会自动更新。

## 安装方式

1. 将整个 `ParkMassAI` 文件夹复制到目标项目的 `Plugins/` 目录。
2. 打开 Unreal Editor。
3. 启用 `ParkMassAI` 插件，以及所需的 Mass / StateTree / ZoneGraph 插件。
4. 让 Unreal 自动重新编译插件模块。
5. 在关卡中放置插件内的 MassSpawner / EventPoint 资产，或在自己的蓝图中调用全局告警节点。

## 核心类说明

### AParkMassPedestrianCharacter

行人角色基类。

主要能力：

- `SetAlertState(bool bNewAlert)`
- 设置告警文本。
- 显示 / 隐藏头顶告警 Widget。
- 开启 / 关闭 Mesh 的 Custom Depth Stencil。

插件内的 `BP_WanderPersonActor` 继承自该类。

### UParkMassActorSyncProcessor

用于把 Mass Entity 的 Transform 同步到表现 Actor。

它只处理带有 `FParkMassActorSyncTag` 的 Entity。该 Tag 通过 `UParkMassActorSyncTrait` 添加。

同步逻辑包括：

- 将 Mass Transform 同步到 Actor。
- 根据 Mass Velocity 调整 Actor 朝向。
- 将 Mass Velocity 写入 `CharacterMovementComponent->Velocity`，让动画蓝图可以读取 Ground Speed 并播放走路动画。

### UParkMassPedestrianAlertSubsystem

这是一个 `UWorldSubsystem`，是告警系统的全局状态和执行入口。

主要函数：

- `SetExternalAlertMode(bool bEnabled)`
- `SetAlertSourceMode(EPedestrianAlertSourceMode NewMode)`
- `GetAlertSourceMode()`
- `IsRandomAlertAllowed()`
- `IsExternalAlertAllowed()`
- `TriggerPointAlert(FName PointName, const FText& AlertText, float Duration, int32 SpawnCount, bool bDestroyAfterAlert)`
- `TriggerActorAlert(AActor* TargetActor, const FText& AlertText, float Duration)`
- `TriggerRandomPedestrianAlert(const FText& AlertText, float Duration)`
- `ClearAllPedestrianAlerts()`

### UParkMassPedestrianAlertBlueprintLibrary

这是全局蓝图函数库。任意蓝图都可以直接搜索并调用，不需要先拿到 `BP_PedestrianEventDirector`。

暴露的蓝图节点：

- `ParkMass_SetExternalAlertMode`
- `ParkMass_SetPedestrianAlertSourceMode`
- `ParkMass_GetPedestrianAlertSourceMode`
- `ParkMass_TriggerPointAlert`
- `ParkMass_TriggerActorAlert`
- `ParkMass_TriggerRandomPedestrianAlert`
- `ParkMass_ClearAllPedestrianAlerts`

这些函数内部通过 `WorldContextObject` 获取当前 World，再获取 `UParkMassPedestrianAlertSubsystem`。

## 告警来源模式

`EPedestrianAlertSourceMode` 有四种模式：

- `DemoRandom`：允许 Director 随机演示告警，也允许外部触发告警。
- `ExternalOnly`：只允许外部触发，禁用随机告警。
- `Hybrid`：随机告警和外部告警都允许，适合调试。
- `Disabled`：禁用所有告警触发。

默认模式是 `DemoRandom`。

## 定点告警流程

1. 在关卡中放置 `BP_PedestrianEventPoint`。
2. 设置它的 `PointName`。
3. 确保同一个关卡里 `PointName` 唯一。
4. 在任意蓝图中调用：

```text
ParkMass_TriggerPointAlert(
    PointName,
    AlertText,
    Duration,
    SpawnCount,
    bDestroyAfterAlert
)
```

Subsystem 会根据 `PointName` 查找唯一匹配的 EventPoint。

如果找不到对应点位，返回 `false`。

如果发现多个 EventPoint 使用同一个 `PointName`，返回 `false`，并在日志中打印重复的 Actor 名称和位置。系统不会默认使用第一个点位。

## BP_PedestrianEventDirector 的职责

`BP_PedestrianEventDirector` 现在只负责 DemoRandom 随机演示逻辑。

外部系统不应该依赖 Director 实例。

旧的 Director 函数仍然保留为兼容包装，但内部只转发到 `UParkMassPedestrianAlertSubsystem`。

推荐以后统一使用 `UParkMassPedestrianAlertBlueprintLibrary` 暴露的全局蓝图节点。

## 推荐提交内容

应该提交：

- `ParkMassAI.uplugin`
- `Source/`
- `Content/`
- `Resources/`
- `Config/`，如果存在
- `README.md`
- `README_CN.md`
- `.gitignore`

不应该提交：

- `Binaries/`
- `Intermediate/`
- `Saved/`
- `DerivedDataCache/`
- `.vs/`
- 编译缓存
- 临时日志

## 当前限制

1. 插件内容仍依赖部分 `/Game/Characters/Mannequins/...` Manny 模板资源。
2. 当前没有接 WebSocket。
3. 当前没有接 GameplayMessageRouter。
4. 当前没有做告警列表、告警历史、点击定位、聚焦人物等 UI 系统。

这些能力可以在后续基于现有 `UParkMassPedestrianAlertSubsystem` 和全局蓝图节点继续扩展。
