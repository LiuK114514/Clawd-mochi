#!/usr/bin/env python3
"""
Mochi Bridge — Claude Code 情绪显示屏 PC 端控制脚本
===================================================
通过串口向 ESP32 Mochi 发送表情切换命令。

用法:
  mochi_bridge.py expr happy         # 切换到 Happy
  mochi_bridge.py text "编译中..."    # 底部状态文字
  mochi_bridge.py mode claude        # 开启 Claude 模式（禁自动轮播）
  mochi_bridge.py mode normal        # 恢复自动轮播
  mochi_bridge.py animate explode,500,happy,500,heart  # 动画序列
  mochi_bridge.py ping               # 测试连接
  mochi_bridge.py clear              # 清除状态文字
  mochi_bridge.py status             # 查询当前状态
  mochi_bridge.py --list             # 列出可用 COM 口

安装依赖:
  pip install pyserial

Claude Code 集成（在 CLAUDE.md 中声明）:
  > 重要事件节点调用 `python mochi_bridge.py expr <表情>` 更新 Mochi 表情
"""
import argparse
import sys
import time
import os

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("需要 pyserial 库，请运行: pip install pyserial", file=sys.stderr)
    sys.exit(1)


EXPR_MAP = {
    "normal": "normal", "squish": "squish", "happy": "happy",
    "surprised": "surprised", "heart": "heart", "sleepy": "sleepy",
    "wink": "wink", "angry": "angry", "cry": "cry", "sweat": "sweat",
    "drool": "drool", "blush": "blush", "jealous": "jealous",
    "roll": "roll", "explode": "explode", "devil": "devil",
    "sick": "sick", "pet": "pet", "thinking": "thinking",
}


def find_port():
    ports = serial.tools.list_ports.comports()
    for p in sorted(ports):
        desc = (p.description + (p.manufacturer or "")).lower()
        # 常见 USB 串口芯片
        if any(kw in desc for kw in ["cp210", "ch340", "ch341", "silicon", "ftdi", "usb串口", "usb串行", "arduino"]):
            return p.device
    # 回退：返回第一个非蓝牙串口
    for p in sorted(ports):
        if "bluetooth" not in p.description.lower():
            return p.device
    return None


def send_cmd(port: str, cmd: str, wait_ms: int = 50) -> str:
    with serial.Serial(port, 115200, timeout=3) as ser:
        ser.write((cmd + "\n").encode())
        time.sleep(wait_ms / 1000)
        resp = ser.readline().decode().strip()
        # 读干净剩余 OK
        extra = ser.readline().decode().strip()
        if extra and not resp:
            resp = extra
        return resp


def cmd_expr(args):
    name = args.name.lower()
    if name not in EXPR_MAP:
        print(f"未知表情: {name}，可选: {', '.join(EXPR_MAP.keys())}")
        sys.exit(1)
    resp = send_cmd(args.port, f"EXPR:{name}")
    print(f"[{name}] {resp}")


def cmd_text(args):
    resp = send_cmd(args.port, f"TEXT:{args.message}")
    print(f"[text] {resp}")


def cmd_mode(args):
    mode = "claude" if args.mode == "claude" else "normal"
    resp = send_cmd(args.port, f"MODE:{mode}")
    print(f"[mode {mode}] {resp}")


def cmd_ping(args):
    resp = send_cmd(args.port, "PING")
    print(f"[ping] {resp}" if resp else "[ping] 无响应")


def cmd_clear(args):
    resp = send_cmd(args.port, "CLEAR")
    print(f"[clear] {resp}")


def cmd_status(args):
    resp = send_cmd(args.port, "STATUS", wait_ms=500)
    print(resp)


def cmd_animate(args):
    steps = [s.strip() for s in args.sequence.split(",")]
    i = 0
    while i < len(steps):
        token = steps[i]
        if token in EXPR_MAP:
            resp = send_cmd(args.port, f"EXPR:{token}")
            print(f"[{token}] {resp}")
        else:
            try:
                delay_ms = int(token)
                time.sleep(delay_ms / 1000)
            except ValueError:
                print(f"跳过未知标记: {token}")
        i += 1


def cmd_list(_args):
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("未检测到 COM 口")
        return
    for p in sorted(ports):
        print(f"{p.device}  —  {p.description}")


def main():
    parser = argparse.ArgumentParser(
        description="Mochi Bridge — 控制 ESP32 Mochi 表情显示屏")
    parser.add_argument("--port", "-p", help="串口号，如 COM3（不指定则自动检测）")
    parser.add_argument("--list", action="store_true", help="列出可用 COM 口")

    sub = parser.add_subparsers(dest="command")

    p_expr = sub.add_parser("expr", help="切换表情")
    p_expr.add_argument("name", choices=list(EXPR_MAP.keys()), help="表情名")
    p_expr.set_defaults(func=cmd_expr)

    p_text = sub.add_parser("text", help="显示底部状态文字")
    p_text.add_argument("message", help="文字内容")
    p_text.set_defaults(func=cmd_text)

    p_mode = sub.add_parser("mode", help="切换模式")
    p_mode.add_argument("mode", choices=["claude", "normal"], help="claude 或 normal")
    p_mode.set_defaults(func=cmd_mode)

    p_ping = sub.add_parser("ping", help="测试连接")
    p_ping.set_defaults(func=cmd_ping)

    p_clear = sub.add_parser("clear", help="清除状态文字")
    p_clear.set_defaults(func=cmd_clear)

    p_status = sub.add_parser("status", help="查询状态")
    p_status.set_defaults(func=cmd_status)

    p_anim = sub.add_parser("animate", help="播放动画序列 (ex: explode,500,happy,500,heart)")
    p_anim.add_argument("sequence", help="ex: explode,500,happy,500,heart")
    p_anim.set_defaults(func=cmd_animate)

    args = parser.parse_args()

    if args.list or not args.command:
        cmd_list(args)
        return

    # 自动检测端口
    if not args.port:
        args.port = find_port()
        if not args.port:
            print("未找到 Mochi 串口，请用 --port 指定或用 --list 查看可用端口", file=sys.stderr)
            sys.exit(1)
        print(f"自动检测到: {args.port}")

    args.func(args)


if __name__ == "__main__":
    main()
