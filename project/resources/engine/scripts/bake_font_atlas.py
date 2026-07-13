"""
bake_font_atlas.py — 使用文字を収集してSDFフォントアトラスを焼く

使い方:
    python bake_font_atlas.py \
        --font Isego.otf \
        --strings "resources/localization/**/*.json" "resources/localization/**/*.csv" \
        --out-dir build/fonts/Isego

内容が変わるたびに手動(またはリリースビルド前のステップ)でこれを実行し、
生成された .png / .font を通常のアセットと同様にコミットする運用を想定。
"""

import argparse
import glob
import string
import freetype
import numpy as np
from scipy.ndimage import distance_transform_edt
from PIL import Image

# 文字列ファイルに一切出てこなくても常に収録しておきたい文字
# (スコア表示など、実行時に組み立てられる数字・記号を取りこぼさないための安全網)
BASELINE_CHARS = set(string.ascii_letters + string.digits + string.punctuation + " ")


def collect_used_chars(patterns: list[str]) -> set[str]:
    """
    指定パターンにマッチする全ファイルをUTF-8テキストとして読み、
    出現した文字を集合として返す。
    JSON/CSVの構造記号({}, ",", :, など)も一緒に拾うが実害はない
    (アトラスに数文字余分に入るだけ)ため、あえて厳密なパーサーは使わない。
    """
    chars: set[str] = set()
    files: list[str] = []
    for pattern in patterns:
        files.extend(glob.glob(pattern, recursive=True))

    if not files:
        print(f"[warn] マッチするファイルが見つかりませんでした: {patterns}")

    for path in files:
        try:
            with open(path, "r", encoding="utf-8") as f:
                content = f.read()
        except UnicodeDecodeError:
            print(f"[warn] UTF-8として読めないためスキップ: {path}")
            continue
        chars.update(content)

    # 制御文字(改行・タブ以外)は除外
    chars = {c for c in chars if c in "\n\t" or (ord(c) >= 0x20 and c != "\x7f")}
    print(f"[info] {len(files)}ファイルから{len(chars)}文字を収集しました")
    return chars


def bake_atlas(font_path: str, charset: set[str], out_atlas: str, out_meta: str,
                atlas_path_in_meta: str, pixel_size: int = 48, padding: int = 6,
                spread: float = 8.0, atlas_width: int = 1024):
    face = freetype.Face(font_path)
    face.set_pixel_sizes(0, pixel_size)

    ordered = sorted(charset, key=ord)
    glyphs = []

    for ch in ordered:
        if ch in "\n\t":
            continue  # 改行・タブはグリフを持たないので描画側でハンドリングする
        try:
            face.load_char(ch, freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_NORMAL)
        except Exception:
            continue

        bmp = face.glyph.bitmap
        w, h = bmp.width, bmp.rows
        bearing_x = face.glyph.bitmap_left
        bearing_y = face.glyph.bitmap_top
        advance = face.glyph.advance.x / 64.0

        if w == 0 or h == 0:
            glyphs.append(dict(ch=ch, sdf=None, w=0, h=0,
                                bearing_x=0.0, bearing_y=0.0, advance=advance))
            continue

        buf = np.array(bmp.buffer, dtype=np.uint8).reshape(h, w)
        mask = (buf > 127).astype(np.uint8)

        padded = np.zeros((h + 2 * padding, w + 2 * padding), dtype=np.uint8)
        padded[padding:padding + h, padding:padding + w] = mask

        dist_in = distance_transform_edt(padded)
        dist_out = distance_transform_edt(1 - padded)
        signed = dist_in - dist_out

        sdf = np.clip(0.5 + signed / (2.0 * spread), 0.0, 1.0)
        sdf_u8 = (sdf * 255.0).astype(np.uint8)

        glyphs.append(dict(
            ch=ch, sdf=sdf_u8, w=sdf_u8.shape[1], h=sdf_u8.shape[0],
            bearing_x=float(bearing_x - padding),
            bearing_y=float(bearing_y + padding),
            advance=advance,
        ))

    visible = [g for g in glyphs if g["sdf"] is not None]
    visible.sort(key=lambda g: g["h"], reverse=True)

    # 十分大きな高さで確保しておき、詰め終わった後に実際に使った分だけ切り出す
    atlas = np.zeros((32768, atlas_width), dtype=np.uint8)
    cursor_x = cursor_y = row_height = 0
    gap = 2

    for g in visible:
        if cursor_x + g["w"] > atlas_width:
            cursor_x = 0
            cursor_y += row_height + gap
            row_height = 0
        atlas[cursor_y:cursor_y + g["h"], cursor_x:cursor_x + g["w"]] = g["sdf"]
        g["atlas_x"], g["atlas_y"] = cursor_x, cursor_y
        cursor_x += g["w"] + gap
        row_height = max(row_height, g["h"])

    atlas_height = max(cursor_y + row_height, 1)
    atlas = atlas[:atlas_height, :]
    Image.fromarray(atlas, mode="L").save(out_atlas)

    line_height = face.size.height / 64.0
    ascent = face.size.ascender / 64.0

    with open(out_meta, "w", encoding="utf-8") as f:
        f.write(f"LINEHEIGHT {line_height:.4f}\n")
        f.write(f"ASCENT {ascent:.4f}\n")
        f.write(f"DISTANCERANGE {spread:.4f}\n")
        f.write(f"ATLAS {atlas_path_in_meta}\n")
        for g in glyphs:
            cp = ord(g["ch"])
            if g["sdf"] is None:
                f.write(f"GLYPH {cp} u=0 v=0 uw=0 vh=0 w=0 h=0 "
                        f"bx={g['bearing_x']:.4f} by={g['bearing_y']:.4f} adv={g['advance']:.4f}\n")
                continue
            u, v = g["atlas_x"] / atlas_width, g["atlas_y"] / atlas_height
            uw, vh = g["w"] / atlas_width, g["h"] / atlas_height
            f.write(f"GLYPH {cp} u={u:.6f} v={v:.6f} uw={uw:.6f} vh={vh:.6f} "
                    f"w={g['w']} h={g['h']} bx={g['bearing_x']:.4f} by={g['bearing_y']:.4f} "
                    f"adv={g['advance']:.4f}\n")

    print(f"[info] atlas: {atlas_width}x{atlas_height}px, {len(visible)}グリフ配置")
    print(f"[info] 出力: {out_atlas}, {out_meta}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--font", required=True, help="元となる.otf/.ttfのパス")
    parser.add_argument("--strings", nargs="+", required=True,
                         help="使用文字を集めるファイルのglobパターン(複数可)")
    parser.add_argument("--out-atlas", required=True)
    parser.add_argument("--out-meta", required=True)
    parser.add_argument("--atlas-path-in-meta", required=True,
                         help=".fontファイルのATLAS行に書き込むランタイム上のパス(.dds想定)")
    parser.add_argument("--atlas-width", type=int, default=1024,
                         help="アトラスの横幅(px)。収録文字数が多い場合は広げる(例: 常用漢字込みなら2048推奨)")
    args = parser.parse_args()

    charset = collect_used_chars(args.strings) | BASELINE_CHARS
    bake_atlas(args.font, charset, args.out_atlas, args.out_meta, args.atlas_path_in_meta,
               atlas_width=args.atlas_width)


if __name__ == "__main__":
    main()
