#pragma once
#include "engine/Runtime/GpuResource/PixelBuffer/ColorBuffer.h"

namespace NoEngine {
class RenderResourceManager {
public:
    /// <summary>
    /// 指定した名前、サイズ、フォーマットで ColorBuffer を作成する。
    /// </summary>
    /// <param name="name">バッファの識別用の名前（デバッグやトレース向け）。</param>
    /// <param name="width">バッファの幅（ピクセル）。</param>
    /// <param name="height">バッファの高さ（ピクセル）。</param>
    /// <param name="format">ピクセルフォーマットを示す DXGI_FORMAT 値。</param>
    /// <returns>作成された ColorBuffer へのポインタ。作成に失敗した場合は nullptr を返す可能性がある。</returns>
    ColorBuffer* Create(const std::string& name, uint32_t width, uint32_t height, DXGI_FORMAT format);
    ColorBuffer* Create(const std::string& name, float width, float height, DXGI_FORMAT format);

    /// <summary>
    /// 名前に対応する ColorBuffer を取得する。
    /// </summary>
    /// <param name="name">取得する ColorBuffer の名前。</param>
    /// <returns>指定した名前の ColorBuffer へのポインタ。見つからない場合は nullptr を返す。</returns>
    ColorBuffer* GetColorBuffer(const std::string& name);

    /// <summary>
    /// 保持する ColorBuffer をクリアして全要素を削除する関数。
    /// </summary>
    void DestroyAll() {
        colorBuffers_.clear();
    }
private:
	std::unordered_map<std::string, ColorBuffer> colorBuffers_;
};
}

