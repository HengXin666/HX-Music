import include
from pathlib import Path

from src.lddcCmd.localMatcAss import localMatcAss

if __name__ == "__main__":
    localMatcAss(Path("/mnt/anime/音乐/eufonius - 比翼の羽根 (比翼的羽根).mp3"), Path("./xxx.ass"))