# Mochi 情绪显示屏

项目路径：`D:\Clawd mochi\clawd_mochi_standalone\`
快捷命令（PATH 已安装）：`mochi`

## 表情映射

每次操作后根据上下文更新 Mochi 表情：

| 状态 | 命令 |
|------|------|
| 开始思考/分析 | `mochi expr thinking && mochi text "思考中..."` |
| 写代码/构建 | `mochi expr happy && mochi text "写代码中..."` |
| 查资料/搜索 | `mochi expr thinking && mochi text "查资料中..."` |
| 运行命令 | `mochi expr sweat && mochi text "执行中..."` |
| 出错 | `mochi expr explode && mochi text "出错了!"` |
| 修复成功 | `mochi expr heart && mochi text "修好了 ♥"` |
| 完成任务 | `mochi expr happy && mochi clear` |
| 等待用户输入 | `mochi expr sleepy && mochi text "等你中..."` |
| 测试运行 | `mochi expr sweat` |
| 长时间任务 | `mochi expr pet` |

## 示例

```bash
mochi expr thinking
mochi text "帮你解决问题..."
mochi animate explode,500,heart,500,thinking,500
mochi ping
```
