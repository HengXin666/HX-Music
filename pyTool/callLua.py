import subprocess
from pathlib import Path
import shlex

def applyKaraokeTemplateWithWine(
    assPath: Path, 
    outPath: Path, 
    luaScript: Path, 
    aegisubCliExe: str
) -> bool:
    """
    使用 Wine 调用 Windows 版 Aegisub CLI + Lua 脚本处理卡拉OK模板

    assPath: 输入 ASS 文件
    outPath: 输出 ASS 文件
    luaScript: Lua 脚本路径
    aegisubCliExe: Windows 版 Aegisub CLI 可执行文件路径 (.exe)
    mkvFile: 对应的 MKV 视频文件（Lua 脚本可能需要）
    wineCmd: Wine 可执行命令，一般为 'wine'
    """

    command = [
        str(aegisubCliExe),
        "--automation",
        str(luaScript),
        str(assPath), str(outPath),
        "Apply karaoke template"
    ]

    print("执行命令:", " ".join(shlex.quote(c) for c in command))

    try:
        result = subprocess.run(
            command,
            capture_output=True,
            text=True,
            check=True
        )
        print(result.stdout)
        return outPath.exists()
    except subprocess.CalledProcessError as e:
        print("执行失败:", e.stderr)
        return False


# 示例使用
assFile = Path("output.ass")
outputFile = Path("output_ok.ass")
luaFile = Path("kara-templater.lua")
aegisubCliExe = "bin/aegisub-cli"

success = applyKaraokeTemplateWithWine(assFile, outputFile, luaFile, aegisubCliExe)
if success:
    print("卡拉OK模板应用成功:", outputFile)
else:
    print("卡拉OK模板应用失败")
