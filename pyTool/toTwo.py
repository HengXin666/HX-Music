from pathlib import Path
from src.utils.callLua import callApplyKaraokeTemplateLua, callSetKaraokeStyleLua

from main import KaRaOKAss

assFile = Path("6.ass")
outputFile = Path("6_ok.ass")

lua = KaRaOKAss()

# lua.doJapanesePhonetics("/home/loli/Loli/code/HX-Music/pyTool/6.ass")

# callSetKaraokeStyleLua(assFile, outputFile, 'orig')

# 应用卡拉ok模板
callApplyKaraokeTemplateLua(assFile, outputFile)