import json
import shlex
import platform
import subprocess
from pathlib import Path

from .basePath import BasePath

def callLua(
    assPath: Path, 
    outPath: Path, 
    luaScript: str,
    *luaArgs: str
) -> bool:
    """
    跨平台应用卡拉OK模板
    Windows 下直接调用本地 Aegisub CLI
    Linux 下使用 Wine 调用 Windows 版 Aegisub CLI
    MacOs 不支持
    """

    aegisubCliWinExe = BasePath.relativePath("./bin/aegisub-cli.exe")  # Windows CLI
    aegisubCliLinuxExe = BasePath.relativePath("./bin/aegisub-cli")    # Linux 本地 CLI, 如果有

    system = platform.system().lower()
    if system == "windows":
        command = [
            str(aegisubCliWinExe)
        ]
    elif system == "linux":
        command = [
            str(aegisubCliLinuxExe)
        ]
    else:
        print(f"不支持的操作系统: {system}")
        return False
    
    command.extend([
        "--automation",
        luaScript,
        str(assPath),
        str(outPath),
    ])

    command.extend(luaArgs)

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
    
def callApplyKaraokeTemplateLua(inAssPath: Path, outAssPath: Path) -> None:
    callLua(inAssPath, outAssPath,
            "kara-templater.lua",
            "Apply karaoke template")

def callSetKaraokeStyleLua(
    inAssPath: Path,
    outAssPath: Path,
    styleName: str,
    advanceTime: int = 100,
    sepThreshold: int = 2000
) -> None:
    callLua(inAssPath, outAssPath,
            "set-karaoke-style.lua",
            'Set Karaoke Style CLI')