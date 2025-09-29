from pathlib import Path
from src.utils.callLua import callApplyKaraokeTemplateLua, callSetKaraokeStyleLua

assFile = Path("6.ass")
outputFile = Path("6_ok.ass")

# 应用卡拉ok模板
callApplyKaraokeTemplateLua(assFile, outputFile)