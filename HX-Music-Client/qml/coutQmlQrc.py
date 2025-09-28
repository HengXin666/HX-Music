import os

def generate_qresource_xml(qml_root: str = "./") -> str:
    lines = ['<RCC>\n    <qresource prefix="/">']

    for root, _, files in os.walk(qml_root):
        for file in files:
            if file.endswith(".qml"):
                fullPath = os.path.join(root, file).replace("\\", "/")
                alias = file  # 只取文件名
                lines.append(f'        <file alias="{alias}">{fullPath.replace("./", "")}</file>')

    lines.append('    </qresource>\n</RCC>')

    return "\n".join(lines)

if __name__ == "__main__":
    data = generate_qresource_xml()
    print(data)