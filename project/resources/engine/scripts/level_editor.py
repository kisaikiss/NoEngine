import bpy

# ブレンダーに登録するアドオン情報
bl_info = {
"name": "Level_Editor",
"author": "Masato Kobayashi",
"version": (1, 0),
"blender": (3, 3, 1),
"location": "",
"description": "Level_Editor",
"warning": "",
"wiki_url": "",
"tracker_url": "",
"category": "Object"
}
#アドオン有効化時コールバック
def register(): print("レベルエディタが有効化されました。")
#アドオン無効化時コールバック
def unregister(): print("レベルエディタが無効化されました。")

if __name__ == "__main__":
    register()