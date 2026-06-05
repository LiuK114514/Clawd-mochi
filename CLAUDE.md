# Mochi 情绪显示屏

项目路径：`D:\Clawd mochi\clawd_mochi_standalone\`
桥接脚本：`python "D:\Clawd mochi\clawd_mochi_standalone\mochi_bridge.py"`

每次工具调用后，根据上下文更新 Mochi 表情：

- 开始思考/分析 → `expr thinking` + `text "思考中..."`
- 写代码/编译/构建 → `expr happy` + `text "写代码中..."`
- 搜索/查阅文档 → `expr thinking` + `text "查资料中..."`
- 运行命令 → `expr sweat` + `text "执行中..."`
- 出错 → `expr explode` + `text "出错了!"`
- 修复 bug 成功 → `expr heart` + `text "修好了 ♥"`
- 完成任务 → `expr happy` + `clear`
- 等待用户输入 → `expr sleepy` + `text "等你中..."`
- 测试运行 → `expr sweat`
- 处理文件/IO → `expr squish`
- 长时间任务 → `expr pet`

## 快速示例

```bash
python "D:\Clawd mochi\clawd_mochi_standalone\mochi_bridge.py" expr thinking
python "D:\Clawd mochi\clawd_mochi_standalone\mochi_bridge.py" text "帮你解决问题..."
```
