from pathlib import Path
from src.utils.callLua import callApplyKaraokeTemplateLua, callSetKaraokeStyleLua

assFile = Path("output.ass")
tmpFile = Path("output_tmp.ass")
outputFile = Path("output_ok.ass")

# 转双行 k 歌曲
callSetKaraokeStyleLua(assFile, tmpFile, "orig")

# 应用卡拉ok模板
callApplyKaraokeTemplateLua(tmpFile, outputFile)