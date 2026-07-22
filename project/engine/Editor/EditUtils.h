#pragma once
#include "Reflection.h"
#include "engine/Functions/ECS/Registry.h"
#include "engine/Runtime/GpuResource/PixelBuffer/ColorBuffer.h"
#include "engine/Runtime/GpuResource/ReadbackBuffer.h"
#include "engine/Runtime/Command/CommandContext.h"
#include "EditTag.h"

namespace NoEngine {
namespace Editor {

/// <summary>
/// ImGuiウィンドウに2Dカメラで描画したシーンでのマウスポインタのワールド座標を取得します
/// </summary>
/// <param name="registry">ECSレジストリ</param>
/// <returns>マウスポインタのワールド座標</returns>
Math::Vector2 Get2DGameWindowMousePosition(ECS::Registry& registry);
bool IsMouseOverGameWindow();
Math::Vector2 Get2DSceneWindowMousePosition(ECS::Registry& registry);
bool IsMouseOverSceneWindow();


/// <summary>
/// 2Dグリッドを描画する。
/// </summary>
/// <param name="gridSize">グリッドの幅と高さを表す2次元ベクトル。</param>
void DrawGrid2D(Math::Vector2 gridSize);
}

/// <summary>
/// 既存の名前集合と衝突しない一意の名前を生成する。
/// </summary>
/// <param name="used">既に使用されている名前の集合。</param>
/// <param name="base">基となる希望の名前。必要に応じて接尾辞（数値）を付与して一意化される。</param>
/// <returns>used に含まれない一意の名前を返す。</returns>
std::string MakeUniqueName(const std::unordered_set<std::string>& used, const std::string& base);

/// <summary>
/// レジストリから編集タグ名を収集する。指定したエンティティのタグは除外される。
/// </summary>
/// <param name="registry">検索対象の ECS レジストリへの参照。</param>
/// <param name="except">除外するエンティティ。デフォルトは ECS::INVALID_ENTITY（除外なし）。</param>
/// <returns>重複のない編集タグ名の集合（std::unordered_set<std::string>）。</returns>
std::unordered_set<std::string> CollectEditTagNames(ECS::Registry& registry, ECS::Entity except = ECS::INVALID_ENTITY);

ECS::Entity PickObject(CommandContext& ctx, ColorBuffer& idColorBuffer, ReadbackBuffer& readBackBuffer);

void InitGameImGuiWindow(ColorBuffer& mainColor);
void InitSceneImGuiWindow(ColorBuffer& debugColor);
void DrawGameImGuiWindow();
void DrawSceneImGuiWindow(ECS::Registry& registry, CommandContext& ctx, ColorBuffer& worldPositionColorBuffer, ReadbackBuffer& readBackBuffer);
void SetGizmoCallback(std::function<void(const Math::Vector4& imageRect)> cb);

void DrawComponentUI(ECS::Registry& registry, ECS::Entity e);
void DrawFieldUI(ECS::Registry& registry, const FieldInfo& field, void* ptr); 
}