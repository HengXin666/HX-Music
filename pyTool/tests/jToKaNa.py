import pykakasi

def isInAtoZ(c: str) -> bool:
    if (c[0] >= 'A' and c[0] <= 'Z')\
    or (c[0] >= 'a' and c[0] <= 'z'):
        return True
    return False

kks = pykakasi.kakasi()
text = "かな漢字交じり文Main You And Me"
result = kks.convert(text)

for item in result:
    if (isInAtoZ(item['orig'])):
        print("todo")
    else:
        print("{}[{}] ".format(item['orig'], item['hira'].capitalize()), end='')
print()