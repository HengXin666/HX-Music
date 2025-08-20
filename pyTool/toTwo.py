from pathlib import Path
from src.utils.callLua import callApplyKaraokeTemplateLua, callSetKaraokeStyleLua

assFile = Path("output.ass")
outputFile = Path("output_ok.ass")

# 转双行 k 歌曲
callSetKaraokeStyleLua(assFile, assFile, "orig")

# 应用卡拉ok模板
callApplyKaraokeTemplateLua(assFile, outputFile)