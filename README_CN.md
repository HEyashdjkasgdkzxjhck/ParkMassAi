# ParkMassAI 中文使用说明（小白版）

> 适用范围：`ParkMassAI` 插件的 Mass 行人、ZoneGraph 路径、随机告警、定点告警、PointName 外部触发接口、CustomStencil 高亮/描边。  
> 目标读者：第一次在新项目里使用这个插件的人。  
> 重要提醒：**插件自包含 ≠ 新关卡里自动生成人物。** 新项目仍然需要启用 Mass/ZoneGraph 相关插件、搭建 ZoneGraph 路径、放置 EventPoint 或 MassSpawner。

---

## 目录

1. 插件能做什么
2. 新项目必须启用的插件
3. 项目设置
4. 内容浏览器设置
5. 最小可运行流程
6. ZoneGraph 路径创建与烘焙
7. 普通行人生成
8. BP_PedestrianEventPoint 定点告警
9. BP_PedestrianEventDirector 随机演示告警
10. 全局蓝图函数说明
11. AlertSourceMode 模式说明
12. 后处理高亮/描边设置
13. 常见问题排查
14. 迁移与提交注意事项
15. 推荐测试流程

---

# 1. 插件能做什么

`ParkMassAI` 是一个基于 UE Mass / ZoneGraph 的行人插件，主要功能包括：

- 在 ZoneGraph 路径上生成行人。
- 行人自动 Wander 行走。
- 支持普通行人。
- 支持随机高危人物告警。
- 支持在指定区域生成告警人物。
- 支持通过 `PointName` 外部触发指定区域告警。
- 支持告警人物头顶标签。
- 支持 CustomDepth / CustomStencil 后处理高亮或描边。
- 支持告警结束后恢复普通状态，或自动删除该 EventPoint 生成的人。
- 支持全局蓝图函数调用，不需要强绑定 `BP_PedestrianEventDirector`。

---

# 2. 新项目必须启用的插件

进入：

```text
Edit / 编辑 → Plugins / 插件
```

建议确认以下插件已启用。不同 UE 版本名字可能略有差异，以项目实际显示为准：

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

启用后如果提示重启编辑器，请重启。

---

# 3. 项目设置

## 3.1 开启 Custom Depth-Stencil

告警人物高亮/描边依赖 CustomDepth / CustomStencil。

进入：

```text
Edit / 编辑
→ Project Settings / 项目设置
→ Rendering / 渲染
→ Postprocessing / 后处理
→ Custom Depth-Stencil Pass
```

设置为：

```text
Enabled with Stencil / 启用模板
```

运行时可以在控制台输入：

```text
r.CustomDepth
```

期望值：

```text
3
```

如果不是 3，CustomStencil 可能不会正常工作。

## 3.2 推荐 Stencil 值

告警人物建议使用：

```text
CustomDepth Stencil Value = 255
```

后处理材质中建议判断：

```text
CustomStencil > 0.5
```

这样比 `Stencil = 1`、阈值 `0.001` 更稳定。

---

# 4. 内容浏览器设置

插件内容默认可能不显示。请在 Content Browser 右上角设置里勾选：

```text
Show Plugin Content / 显示插件内容
Show Engine Content / 显示引擎内容（必要时）
```

然后可以看到类似路径：

```text
/ParkMassAI/...
```

常用资产大致包括：

```text
/ParkMassAI/.../BP_WanderPersonActor
/ParkMassAI/.../BP_PedestrianEventPoint
/ParkMassAI/.../BP_PedestrianEventDirector
/ParkMassAI/.../DA_Mass_Pedestrian_Wander
/ParkMassAI/.../MassSpawner_Wander
/ParkMassAI/.../ABP_Manny_Mass
/ParkMassAI/.../BS_Manny_Mass_Walk
```

实际路径以你的插件资产为准。

---

# 5. 最小可运行流程

如果你在新项目里发现“不生成人物”，请先按这个最小流程验证。

## 第一步：启用插件并重启

确认：

```text
ParkMassAI
Mass 相关插件
ZoneGraph
StateTree
```

均已启用。

## 第二步：创建 ZoneGraph 路径

场景里必须有 ZoneGraph 路径。  
只启用插件不会自动生成人物。

## 第三步：放置 EventPoint

拖入：

```text
BP_PedestrianEventPoint
```

设置：

```text
PointName = 禁区入口
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

## 第四步：用全局蓝图函数测试

在 Level Blueprint 中：

```text
Event BeginPlay
→ Delay 1.0
→ ParkMass_SetExternalAlertMode(true)
→ ParkMass_TriggerPointAlert
```

参数：

```text
PointName = 禁区入口
AlertText = 异常行为
Duration = 8.0
SpawnCount = 1
bDestroyAfterAlert = false
```

如果这一步能生成告警人物，说明全局接口、EventPoint、Mass 生成链路基本正常。

---

# 6. ZoneGraph 路径创建与烘焙

## 6.1 为什么必须有 ZoneGraph

ParkMassAI 的 Wander 行人依赖：

```text
Mass Entity
+ ZoneGraph 路径
+ StateTree / Movement
+ Representation Actor
```

新地图里没有 ZoneGraph 时，可能出现：

```text
无法生成
生成 Entity 但没有表现 Actor
人物出现但不走
找不到路径
```

## 6.2 创建 ZoneGraph / ZoneShape

常见流程：

```text
Place Actors / 放置 Actor
→ 搜索 ZoneShape 或 ZoneGraph
→ 拖到场景
→ 编辑路径点
→ 设置 Lane Profile / Tags
```

建议先做一条简单直线或折线路径，不要一开始做复杂路网。

测试时：

```text
EventPoint 放在 ZoneGraph 线旁边
SpawnRadius 先设 50
ClaimRadius 设 300
```

## 6.3 Lane Profile 要匹配

检查：

```text
ZoneShape / ZoneGraph 的 Lane Profile
DA_Mass_Pedestrian_Wander 使用的 ZoneGraph 标签 / Lane 要求
StateTree / Movement 是否需要特定 ZoneGraph Tag
```

如果 LaneProfile 不匹配，Mass Entity 可能找不到可走 Lane。

## 6.4 烘焙 / Build ZoneGraph

创建或修改 ZoneShape 后，需要让 ZoneGraph 数据更新。

常见做法：

```text
1. 保存关卡
2. 执行 Build / 构建当前关卡
3. 如果菜单里有 Build ZoneGraph / Rebuild ZoneGraph，执行它
4. 重新进入 PIE
```

如果找不到单独的 ZoneGraph Build 菜单，可以先执行：

```text
Build All Levels / 构建全部关卡
```

然后保存地图。

## 6.5 用 ZoneGraph Debug 检查

如果人物不走，优先打开 ZoneGraph 调试视图。

重点看：

```text
场景里是否真的有 Lane
Lane 是否连续
EventPoint 是否离 Lane 太远
LaneProfile 是否正确
ZoneGraph 数据是否更新
```

如果调试视图里看不到任何 Lane，说明 ZoneGraph 没有正确生成或没有 Build。

---

# 7. 普通行人生成

普通行人建议用：

```text
MassSpawner_Wander
```

放到场景后检查：

```text
EntityConfig = DA_Mass_Pedestrian_Wander
SpawnCount > 0
Spawn On BeginPlay / Auto Spawn = true
```

如果场景里没有 `MassSpawner_Wander`，普通行人不会自动出现。

---

# 8. BP_PedestrianEventPoint 定点告警

`BP_PedestrianEventPoint` 是“指定区域刷告警人物”的点。

## 8.1 核心参数

### PointName

唯一告警点名称。以后后台或蓝图接口会通过这个名字触发告警。

例如：

```text
禁区入口
商业楼大厅
食堂门口
宿舍楼门口
```

同一关卡中 `PointName` 必须唯一。

### EntityConfig

生成 Mass 行人使用的 EntityConfig。通常设置为：

```text
DA_Mass_Pedestrian_Wander
```

### SpawnCount

本次生成数量。测试时建议：

```text
1
```

### SpawnRadius

生成半径。新项目测试建议：

```text
50 - 100
```

半径太大可能生成到 ZoneGraph 外面。

### ClaimRadius

EventPoint 认领新生成 Actor 的范围。建议：

```text
300
```

意思是 EventPoint 只认领自己附近的新人物，避免误把别的行人变成告警。

### bSpawnAsAlert

是否生成后直接进入告警状态。

### AlertText

头顶显示文本，例如：

```text
异常行为
高危人员
人员异常停留
```

### AlertDuration

告警持续时间，单位秒。

### bDestroyAfterAlert

告警结束后是否删除这个 EventPoint 生成的人。

```text
true  = 告警结束后清理人物
false = 告警结束后人物恢复普通状态并继续存在
```

### bAutoSpawnOnBeginPlay

是否开局自动生成。使用外部接口 `ParkMass_TriggerPointAlert` 时建议关闭：

```text
false
```

否则第一次可能出现：

```text
BeginPlay 自动生成一次
+ TriggerPointAlert 又生成一次
= 生成两个人
```

### bLoop

是否循环生成。使用外部接口时建议：

```text
false
```

## 8.2 推荐配置：外部接口触发

```text
PointName = 禁区入口
EntityConfig = DA_Mass_Pedestrian_Wander
SpawnCount = 1
SpawnRadius = 50
ClaimRadius = 300
bSpawnAsAlert = true
AlertText = 异常行为
AlertDuration = 8
bDestroyAfterAlert = true
bAutoSpawnOnBeginPlay = false
bLoop = false
```

---

# 9. BP_PedestrianEventDirector 随机演示告警

`BP_PedestrianEventDirector` 现在只建议作为 DemoRandom 演示器使用。

作用：

```text
在没有后台接口时，随机挑一个已有行人进入告警
```

它不是正式外部接口入口。正式外部接口请使用：

```text
ParkMass_TriggerPointAlert
```

如果只想用 PointName 外部触发，可以不放 Director。

调用：

```text
ParkMass_SetExternalAlertMode(true)
```

之后：

```text
AlertSourceMode = ExternalOnly
```

此时 Director 不会再随机触发告警。

---

# 10. 全局蓝图函数说明

这些函数来自 `UParkMassPedestrianAlertBlueprintLibrary`。

特点：

```text
任意蓝图都可以直接调用
不需要获取 BP_PedestrianEventDirector
不需要手动找 Subsystem
```

## 10.1 ParkMass_SetExternalAlertMode

用途：切换是否进入外部告警模式。

参数：

```text
bEnabled : bool
```

逻辑：

```text
true:
    AlertSourceMode = ExternalOnly
    随机告警禁用

false:
    AlertSourceMode = DemoRandom
    随机演示恢复
```

## 10.2 ParkMass_SetPedestrianAlertSourceMode

用途：直接设置告警来源模式。

参数：

```text
NewMode : EPedestrianAlertSourceMode
```

可选值：

```text
DemoRandom
ExternalOnly
Hybrid
Disabled
```

## 10.3 ParkMass_GetPedestrianAlertSourceMode

用途：获取当前告警来源模式。

返回：

```text
EPedestrianAlertSourceMode
```

## 10.4 ParkMass_TriggerPointAlert

最重要的外部接口。

用途：根据 PointName 在指定区域生成告警人物。

参数：

```text
PointName : Name
AlertText : Text
Duration : float
SpawnCount : int
bDestroyAfterAlert : bool
```

返回：

```text
bool
```

示例：

```text
ParkMass_TriggerPointAlert(
    PointName = 禁区入口,
    AlertText = 异常行为,
    Duration = 8.0,
    SpawnCount = 1,
    bDestroyAfterAlert = true
)
```

执行逻辑：

```text
1. 根据 PointName 查找场景中的 BP_PedestrianEventPoint
2. PointName 找不到：返回 false
3. PointName 重复：返回 false
4. 找到唯一点：调用该点生成告警人物
```

注意：

```text
PointName 必须唯一
PointName 就是区域名 / 告警点名
不要重复
```

## 10.5 ParkMass_TriggerActorAlert

用途：让指定已有行人进入告警。

参数：

```text
TargetActor : Actor
AlertText : Text
Duration : float
```

返回：

```text
bool
```

## 10.6 ParkMass_TriggerRandomPedestrianAlert

用途：随机挑一个已有行人进入告警。

参数：

```text
AlertText : Text
Duration : float
```

返回：

```text
bool
```

模式限制：

```text
DemoRandom：允许
Hybrid：允许
ExternalOnly：拒绝
Disabled：拒绝
```

## 10.7 ParkMass_ClearAllPedestrianAlerts

用途：清除当前所有行人告警状态。

返回：

```text
int
```

返回值表示清理了多少个告警人物。它只是取消告警状态，不一定删除人物。

---

# 11. AlertSourceMode 模式说明

枚举：

```text
EPedestrianAlertSourceMode
```

## DemoRandom

演示随机模式。

```text
随机告警可用
外部 TriggerPointAlert 也可手动调用
```

适合无后台演示和开发测试。

## ExternalOnly

外部接口模式。

```text
随机告警禁用
PointName 外部触发可用
Actor 外部触发可用
```

适合接 WebSocket、GameplayMessageRouter、后台事件、正式演示。

推荐正式使用：

```text
ParkMass_SetExternalAlertMode(true)
```

## Hybrid

混合模式。

```text
随机告警可用
外部接口也可用
```

只建议调试时使用，正式演示不建议用，因为容易分不清告警来源。

## Disabled

全部禁用。

```text
随机告警禁用
PointName 触发禁用
Actor 触发禁用
```

适合临时关闭告警系统或排查问题。

---

# 12. 后处理高亮 / 描边设置

## 12.1 必须放 PostProcessVolume

场景中需要有：

```text
PostProcessVolume
```

建议设置：

```text
Infinite Extent / Unbound = true
```

然后在：

```text
Post Process Materials
```

添加插件里的告警后处理材质，例如：

```text
M_PP_PedestrianAlertHighlight
```

实际名字以你的插件资产为准。

## 12.2 如果整个屏幕都被描边 / 变色

通常是 Mask 全屏为 1。

检查顺序：

```text
1. Project Settings 里 Custom Depth-Stencil Pass 是否 Enabled with Stencil
2. r.CustomDepth 是否为 3
3. Buffer Visualization → Custom Stencil
4. 是否只有告警人物有 Stencil
5. 是否地面、天空、建筑也开启了 Render CustomDepth
6. 后处理材质是否用 CustomStencil > 0.5 判断
```

正常情况：

```text
只有告警人物在 Custom Stencil 里是白色 / 有值
其他区域应该是黑色
```

---

# 13. 常见问题排查

## 13.1 不生成人物

检查：

```text
Mass 相关插件是否启用
ZoneGraph 是否存在
ZoneGraph 是否 Build
BP_PedestrianEventPoint 是否放在 ZoneGraph 附近
EntityConfig 是否是 DA_Mass_Pedestrian_Wander
SpawnCount 是否大于 0
PointName 是否匹配
AlertSourceMode 是否 Disabled
```

## 13.2 ParkMass_TriggerPointAlert 返回 false

常见原因：

```text
PointName 找不到
PointName 重复
AlertSourceMode = Disabled
WorldContextObject 无效
Subsystem 获取失败
```

解决：

```text
确认场景里有 BP_PedestrianEventPoint
确认 PointName 完全一致
确认 PointName 唯一
确认不是 Disabled 模式
```

## 13.3 返回 true，但没看到人物

检查：

```text
EventPoint EntityConfig 是否为空
DA_Mass_Pedestrian_Wander 是否有效
EventPoint 是否离 ZoneGraph 太远
SpawnRadius 是否太大
Mass Representation Actor 是否有效
BP_WanderPersonActor Mesh / AnimClass 是否有效
```

## 13.4 人物出来但不走

检查：

```text
ZoneGraph 是否 Build
LaneProfile 是否匹配
StateTree 是否有效
Mass Movement 是否正常
EventPoint 是否在 Lane 附近
```

## 13.5 第一次生成出现两个人

最常见原因：

```text
bAutoSpawnOnBeginPlay = true
+
你又调用了 ParkMass_TriggerPointAlert
```

解决：

```text
使用外部接口触发时：
bAutoSpawnOnBeginPlay = false
bLoop = false
SpawnCount = 1
```

另一个原因：

```text
DemoRandom 没关
+
PointName 外部触发
```

解决：

```text
ParkMass_SetExternalAlertMode(true)
```

## 13.6 随机告警不出现

检查：

```text
场景里是否放了 BP_PedestrianEventDirector
AlertSourceMode 是否是 DemoRandom 或 Hybrid
是否有可用普通行人
是否所有行人都已经处于告警状态
```

如果模式是 ExternalOnly 或 Disabled，随机告警不会出现，这是正常的。

## 13.7 告警人物没有高亮

检查：

```text
Custom Depth-Stencil Pass 是否 Enabled with Stencil
r.CustomDepth 是否为 3
PostProcessVolume 是否 Unbound
后处理材质是否加入 Post Process Materials
告警人物 Mesh 是否 Render CustomDepth = true
Stencil 值是否为 255
```

---

# 14. 迁移与提交注意事项

提交插件时建议包含：

```text
Plugins/ParkMassAI/ParkMassAI.uplugin
Plugins/ParkMassAI/Source/
Plugins/ParkMassAI/Content/
Plugins/ParkMassAI/Resources/
```

不要提交：

```text
Plugins/ParkMassAI/Binaries/
Plugins/ParkMassAI/Intermediate/
Plugins/ParkMassAI/Saved/
Plugins/ParkMassAI/DerivedDataCache/
```

推荐 `.gitignore`：

```gitignore
Plugins/ParkMassAI/Binaries/
Plugins/ParkMassAI/Intermediate/
Plugins/ParkMassAI/Saved/
Plugins/ParkMassAI/DerivedDataCache/
```

如果插件要完全自包含，需要确保插件资产不再引用：

```text
/Game/Characters/Mannequins
/Game/Variant_Combat
```

可以用 Reference Viewer 或 Python 依赖扫描确认。

---

# 15. 推荐测试流程

新项目第一次测试建议按这个顺序：

```text
1. 启用 ParkMassAI 和 Mass / ZoneGraph / StateTree 相关插件
2. 重启编辑器
3. 显示插件内容
4. 新建空关卡
5. 创建一条 ZoneGraph 路径
6. Build / Rebuild ZoneGraph
7. 放 BP_PedestrianEventPoint
8. 设置 PointName = 禁区入口
9. EntityConfig = DA_Mass_Pedestrian_Wander
10. SpawnCount = 1
11. SpawnRadius = 50
12. ClaimRadius = 300
13. bAutoSpawnOnBeginPlay = false
14. bLoop = false
15. 放 PostProcessVolume，并设置 Unbound
16. 添加告警后处理材质
17. Level Blueprint 调用 ParkMass_SetExternalAlertMode(true)
18. Delay 1 秒
19. 调用 ParkMass_TriggerPointAlert("禁区入口", "异常行为", 8.0, 1, false)
```

预期结果：

```text
指定 EventPoint 附近生成 1 个告警人物
人物头顶显示异常行为
人物身体高亮 / 描边
人物可以正常移动
8 秒后告警恢复
```

---

# 16. 最小蓝图调用示例

## 外部接口触发指定区域告警

```text
Event BeginPlay
→ Delay 1.0
→ ParkMass_SetExternalAlertMode(true)
→ ParkMass_TriggerPointAlert
    PointName = 禁区入口
    AlertText = 异常行为
    Duration = 8.0
    SpawnCount = 1
    bDestroyAfterAlert = false
```

## 清除所有告警

```text
Keyboard C
→ ParkMass_ClearAllPedestrianAlerts
```

## 临时关闭所有告警触发

```text
ParkMass_SetPedestrianAlertSourceMode(Disabled)
```

## 恢复随机演示模式

```text
ParkMass_SetExternalAlertMode(false)
```

---

# 17. 推荐正式使用方式

正式项目 / 后台接入时推荐：

```text
1. 开局调用 ParkMass_SetExternalAlertMode(true)
2. 后台收到告警消息
3. 解析 pointName / alertText / duration / spawnCount / destroyAfterAlert
4. 调用 ParkMass_TriggerPointAlert
```

后台消息可以理解为：

```json
{
  "pointName": "禁区入口",
  "alertText": "异常行为",
  "duration": 8,
  "spawnCount": 1,
  "destroyAfterAlert": true
}
```

UE 侧对应调用：

```text
ParkMass_TriggerPointAlert(
    PointName = 禁区入口,
    AlertText = 异常行为,
    Duration = 8.0,
    SpawnCount = 1,
    bDestroyAfterAlert = true
)
```

---

# 18. 一句话总结

`ParkMassAI` 的使用核心是：

```text
ZoneGraph 提供路
MassSpawner / EventPoint 负责生成
BP_WanderPersonActor 负责表现
SetAlertState 负责告警状态
PostProcess 负责高亮
ParkMass_TriggerPointAlert 负责外部按 PointName 触发告警
```

如果新项目不生成人物，优先检查：

```text
ZoneGraph 是否存在并 Build
EventPoint 是否在 ZoneGraph 附近
EntityConfig 是否正确
PointName 是否匹配
AlertSourceMode 是否 Disabled
Mass 相关插件是否启用
```

