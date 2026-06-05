# CLAWD MOCHI STANDALONE

ESP32-C3 Super Mini + ST7789 240x240 显示屏。无需 WiFi，按键切换 + 自动轮播表情。

## 接线

| 引脚 | GPIO | 说明 |
|------|------|------|
| SDA  | 10   | 硬件 SPI MOSI |
| SCL  | 8    | 硬件 SPI SCK |
| RST  | 2    | 复位 |
| DC   | 1    | 数据/命令 |
| CS   | 4    | 片选 |
| BL   | 3    | 背光 |
| BTN  | 0    | 短按切表情，长按 0.8s 开关背光 |
| VCC  | -    | 3V3 |
| GND  | -    | GND |

GPIO 0 是 ESP32-C3 的 BOOT 按钮，大多数开发板自带。

## 表情列表（20 个）

自动轮播 10 秒一切，按键手动切换。每个表情包含眼睛 + 嘴巴 + 特效。

| # | 名称 | 眼睛 | 嘴巴 | 特效 |
|---|------|------|------|------|
| 1 | **Normal** ▬ ▬ | 填充矩形 | 直线 ─── | 每 5 秒眨眼一次，偶尔左右看 |
| 2 | **Squish** > < | V 形挤压 | 锯齿 >_< | 快速开合动画 |
| 3 | **Happy** ^_^ | 向上弧线 | 弧形微笑 | 腮红，眯眼笑动画 |
| 4 | **Surprised** O_O | 大圆形 | O 型嘴 + 高光 | 放大缩小闪烁 |
| 5 | **Heart** ♥_♥ | 红心 | 弧形微笑 | 心跳脉冲，腮红 |
| 6 | **Sleepy** -_- | 横线→慢慢闭合 | 下垂嘴 ⌢ | "z Z" 文字，惊醒循环 |
| 7 | **Wink** ;_) | 一只眨眼 | 歪嘴坏笑 | 俏皮舌头 |
| 8 | **Angry** ಠ_ಠ | 压扁矩形 + 白瞳孔 | 倒 V 型 + 牙齿 | 红色怒眉，瞳孔转动 |
| 9 | **Cry** ;_; | 下弯弧线（倒 U） | 波浪颤抖 | 蓝色泪滴 + 泪痕 |
| 10 | **Sweat** 😅 | 两条直线 >_> | 不对称歪嘴 | 大/小汗滴 + 脸红 |
| 11 | **Drool** 😋 | 迷离半闭 | 傻笑张嘴 | 口水丝 + 水滴 + 脸红 |
| 12 | **Blush** 害羞 | 大眼 + 瞳孔偏移 | 小小 ω 嘴 | 大面积扩散脸红 |
| 13 | **Jealous** 😤 | 矩形眼 + 侧目瞳孔 | 倒 V 撇嘴 | 绿色背景 + 红色不对称眉毛 |
| 14 | **Roll Eyes** 🙄 | 白色眼白 + 小瞳孔 | 无奈直线 | 瞳孔翻到最上面 |
| 15 | **Explode** 🤯 | 巨大圆形震惊眼 | 波浪 O 型嘴 | 橙色放射爆炸线 |
| 16 | **Devilish** 😈 | 邪恶眯眼 + 红瞳孔 | 不对称奸笑 + 尖牙 | 紫色背景 + 恶魔角 |
| 17 | **Sick** 🤮 | 绿色晕眩眼 | 张嘴呕吐 + 舌头 | 绿色背景 + 呕吐物 |
| 18 | **Pixel Pet** 🐾 | 行走 Mochi 宠物 | 微笑 | 腿部动画 + 随机跳跃/待机 |
| 19 | **Thinking** 🤔 | 半闭向上看 | 微撇嘴 | 额头纹 + "..." 弹跳动画 + 腮红 |

## 启动流程

1. 显示 "Clawd Mochi" 文字 1.2 秒
2. Anthropic Logo 逐线绘制动画（162 个三角形）
3. 进入 Normal 表情，开始自动轮播

## 按键逻辑

| 操作 | 行为 |
|------|------|
| 短按（>50ms） | 切换到下一个表情 |
| 长按（>800ms） | 切换背光开关 |
| 休眠中短按 | 唤醒 + 显示当前表情 |
| 休眠时长按 | 进入休眠 |

## Claude Code 情绪显示屏

通过 USB 串口连接电脑，Mochi 可实时反映 Claude Code 的工作状态。

### 串口命令协议

每行一条命令，ESP32 回复 `OK`。

| 命令 | 示例 | 说明 |
|------|------|------|
| `EXPR:<名>` | `EXPR:happy` | 切换到指定表情 |
| `TEXT:<内容>` | `TEXT:正在编译...` | 底部状态文字叠加 |
| `MODE:claude` | `MODE:claude` | 开启 Claude 模式（禁自动轮播） |
| `MODE:normal` | `MODE:normal` | 恢复自动轮播 |
| `PING` | `PING` | 连接测试 |
| `CLEAR` | `CLEAR` | 清除底部状态文字 |
| `STATUS` | `STATUS` | 查询当前状态 |

### PC 端桥接脚本

```bash
python mochi_bridge.py expr happy       # 切换到 Happy
python mochi_bridge.py text "编译中..."  # 显示状态文字
python mochi_bridge.py mode claude      # 开启 Claude 模式
python mochi_bridge.py animate explode,500,happy,500,heart  # 动画序列
python mochi_bridge.py ping             # 测试连接
```

依赖: `pip install pyserial`

### Claude Code 集成建议

在 `CLAUDE.md` 中添加声明，让 Claude Code 在关键节点主动调用脚本：

> 每次工具调用后，根据上下文通过 `python mochi_bridge.py expr <表情>` 更新 Mochi 表情：
> 开始思考 → `thinking`，写代码 → `happy`，运行命令 → `sweat`，
> 出错 → `explode`，修复成功 → `heart`，等待输入 → `sleepy`

或通过 `settings.json` 的 `postToolCall` hook 自动触发。

## 代码结构

| 文件 | 行 | 说明 |
|------|-----|------|
| `clawd_mochi_standalone.ino` | ~1440 | 主程序，全部在一份文件中 |
| `mochi_bridge.py` | ~140 | Python 串口桥接脚本 |

### 功能分区

- **Pins / Display / Colours** — 引脚定义、屏幕参数、颜色常量
- **Logo Data** — Anthropic Logo 的 162 个三角形坐标（PROGMEM 存储）
- **Helpers** — 背光控制、颜色转换、`drawTeardrop()` 水滴辅助函数
- **Logo** — Logo 填充绘制 + 逐线 reveal 动画
- **Eye Helpers** — 眼睛位置计算（`eyeLX`, `eyeRX`, `eyeY`, `eyeCY`）
- **Expressions** — 18 个表情的绘制函数（含 6 个新夸张表情）
- **Mochi Pet** — 像素宠物行走、跳跃、待机动画
- **Thinking** — 思考表情 + "..." 弹跳动画
- **Serial Handler** — 串口命令解析和 Claude 模式控制
- **Animation Updates** — 每个表情 + 宠物的动画循环更新
- **View Management** — `enterView()`, `nextView()` 视图切换
- **Button** — 按键防抖、短按/长按检测
- **Setup / Loop** — 初始化 + 主循环

### 动画机制

每个表情的 `updateXxx()` 函数通过 `lastAnimMs` 控制动画帧率，在 loop() 中被反复调用。主循环每帧检测：
1. 按键事件
2. 串口命令（Claude 模式时禁用自动轮播）
3. 自动轮播计时（10 秒，claudeMode=false）
4. 当前表情的动画更新

## 技术栈

| 组件 | 说明 |
|------|------|
| MCU | ESP32-C3 Super Mini |
| 屏幕 | ST7789 240x240 SPI |
| 驱动 | Adafruit GFX + ST7789 |
| 通信 | 硬件 SPI（40MHz） |
| 存储 | PROGMEM 存储 Logo 数据 |
| 按钮 | GPIO 0（内置 BOOT 按钮） |

## 修改记录

| 日期 | 变更 |
|------|------|
| - | 初始版本：11 个视图（含 Claude Code 和通知栏） |
| 2026-06-04 | 移除 Claude Code 和通知系统，为所有表情添加嘴巴 |
| 2026-06-04 | 新增 4 个表情（Cry, Sweat, Drool, Blush），减慢眨眼速度 |
| 2026-06-05 | 移除 Ambient 粒子效果 |
| 2026-06-05 | 新增 5 个夸张表情（Jealous, Roll Eyes, Explode, Devilish, Sick）+ Mochi 像素宠物 |
| 2026-06-05 | 新增 Thinking 表情 + 串口命令协议 + Python 桥接脚本，支持 Claude Code 情绪显示屏 |
