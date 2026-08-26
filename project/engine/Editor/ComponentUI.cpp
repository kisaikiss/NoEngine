#include "ComponentUI.h"
#include "engine/Editor/ComponentRegistry.h"
#include "engine/Functions/Command/EditCommand/ChangeValueCommand.h"
#include "engine/Functions/Command/EditCommand/FunctionCommand.h"
#include "engine/Functions/Command/EditCommand/RemoveComponentCommand.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"
#include "engine/Functions/ECS/System/Editor/DrawManipulatorSystem.h"
#include "engine/Utilities/Conversion/ConvertString.h"
#include "DataDriven/SceneSerializer.h"
#include "EditorCommandOperator.h"
#include "engine/Math/MathInclude.h"


#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace NoEngine {
using namespace ECS;
void DrawComponentUI(Registry& registry, Entity e) {
#ifdef USE_IMGUI
	for (auto& compInfo : ComponentRegistry::GetAll()) {
		if (!registry.Has(compInfo.typeId, e))
			continue;

		if (ImGui::CollapsingHeader(compInfo.name.c_str())) {
			void* compPtr = compInfo.getter(registry, e);

			for (auto& field : compInfo.fields) {
				DrawFieldUI(registry, e, field, compPtr);
			}
			// 削除ボタン
			if (ImGui::SmallButton("Remove")) {
				ImGui::OpenPopup("ConfirmRemove");
			}
			if (ImGui::BeginPopupModal("ConfirmRemove")) {
				ImGui::Text("Remove this component?");
				if (ImGui::Button("Yes")) {
					Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::RemoveComponentCommand>(registry, e, compInfo.typeId));
					registry.RemoveComponent(compInfo.typeId, e);
				}
				ImGui::SameLine();
				if (ImGui::Button("No")) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
	}
#else
	static_cast<void>(registry);
	static_cast<void>(e);
#endif // USE_IMGUI
}

void DrawFieldUI(ECS::Registry& registry, ECS::Entity e, const FieldInfo& field, void* ptr) {
#ifdef USE_IMGUI
	if (!field.attributes.editable) {
		return;
	}
	uint8_t* base = (uint8_t*)ptr;
	void* valuePtr = base + field.offset;
	switch (field.type) {
	case FieldType::Float: {
		float* fPtr = static_cast<float*>(valuePtr);

		// 一時保存用の変数（同時に編集できるUIは1つなのでstaticで使い回せます）
		static float oldFloatValue;

		if (field.attributes.hasRange) {
			ImGui::DragFloat(field.name.c_str(), fPtr, field.attributes.valueSpeed, field.attributes.minValue, field.attributes.maxValue);
		} else {
			ImGui::DragFloat(field.name.c_str(), fPtr, field.attributes.valueSpeed);
		}

		// 編集が開始された瞬間（マウスでクリックした時など）に元の値を保存
		if (ImGui::IsItemActivated()) {
			oldFloatValue = *fPtr;
		}

		// 編集が確定した瞬間（マウスを離した時、Enterを押した時など）にコマンドを発行
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::ChangeValueCommand<float>>(fPtr, oldFloatValue, *fPtr)
			);
		}
		break;
	}
	case FieldType::Float2: {
		Math::Vector2* vPtr = static_cast<Math::Vector2*>(valuePtr);

		static Math::Vector2 oldVector2Value;

		if (field.attributes.hasRange) {
			ImGui::DragFloat2(field.name.c_str(), static_cast<float*>(valuePtr), field.attributes.valueSpeed, field.attributes.minValue, field.attributes.maxValue);
		} else {
			ImGui::DragFloat2(field.name.c_str(), static_cast<float*>(valuePtr), field.attributes.valueSpeed);
		}


		// 編集が開始された瞬間（マウスでクリックした時など）に元の値を保存
		if (ImGui::IsItemActivated()) {
			oldVector2Value = *vPtr;
		}

		// 編集が確定した瞬間（マウスを離した時、Enterを押した時など）にコマンドを発行
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::ChangeValueCommand<Math::Vector2>>(vPtr, oldVector2Value, *vPtr)
			);
		}
		break;
	}
	case FieldType::Float3: {
		Math::Vector3* vPtr = static_cast<Math::Vector3*>(valuePtr);

		static Math::Vector3 oldVectorValue;

		if (field.attributes.hasRange) {
			ImGui::DragFloat3(field.name.c_str(), static_cast<float*>(valuePtr), field.attributes.valueSpeed, field.attributes.minValue, field.attributes.maxValue);
		} else {
			ImGui::DragFloat3(field.name.c_str(), static_cast<float*>(valuePtr), field.attributes.valueSpeed);
		}


		// 編集が開始された瞬間（マウスでクリックした時など）に元の値を保存
		if (ImGui::IsItemActivated()) {
			oldVectorValue = *vPtr;
		}

		// 編集が確定した瞬間（マウスを離した時、Enterを押した時など）にコマンドを発行
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::ChangeValueCommand<Math::Vector3>>(vPtr, oldVectorValue, *vPtr)
			);
		}
		break;
	}
	case FieldType::Float4: {
		Math::Vector4* vPtr = static_cast<Math::Vector4*>(valuePtr);

		static Math::Vector4 oldVectorValue;

		if (field.attributes.hasRange) {
			ImGui::DragFloat4(field.name.c_str(), static_cast<float*>(valuePtr), field.attributes.valueSpeed, field.attributes.minValue, field.attributes.maxValue);
		} else {
			ImGui::DragFloat4(field.name.c_str(), static_cast<float*>(valuePtr), field.attributes.valueSpeed);
		}


		// 編集が開始された瞬間（マウスでクリックした時など）に元の値を保存
		if (ImGui::IsItemActivated()) {
			oldVectorValue = *vPtr;
		}

		// 編集が確定した瞬間（マウスを離した時、Enterを押した時など）にコマンドを発行
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::ChangeValueCommand<Math::Vector4>>(vPtr, oldVectorValue, *vPtr)
			);
		}
		break;
	}
	case FieldType::Color: {
		Math::Vector4* vPtr = static_cast<Math::Vector4*>(valuePtr);

		static Math::Vector4 oldVectorValue;

		ImGui::ColorEdit4(field.name.c_str(), static_cast<float*>(valuePtr));


		// 編集が開始された瞬間（マウスでクリックした時など）に元の値を保存
		if (ImGui::IsItemActivated()) {
			oldVectorValue = *vPtr;
		}

		// 編集が確定した瞬間（マウスを離した時、Enterを押した時など）にコマンドを発行
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::ChangeValueCommand<Math::Vector4>>(vPtr, oldVectorValue, *vPtr)
			);
		}
		break;
	}
	case FieldType::Int: {
		int* iPtr = static_cast<int*>(valuePtr);

		// 一時保存用の変数（同時に編集できるUIは1つなのでstaticで使い回せます）
		static int oldIntValue;

		if (field.attributes.hasRange) {
			ImGui::DragInt(field.name.c_str(), static_cast<int*>(valuePtr), field.attributes.valueSpeed, static_cast<int>(field.attributes.minValue), static_cast<int>(field.attributes.maxValue));
		} else {
			ImGui::DragInt(field.name.c_str(), static_cast<int*>(valuePtr), field.attributes.valueSpeed);
		}

		// 編集が開始された瞬間（マウスでクリックした時など）に元の値を保存
		if (ImGui::IsItemActivated()) {
			oldIntValue = *iPtr;
		}

		// 編集が確定した瞬間（マウスを離した時、Enterを押した時など）にコマンドを発行
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::ChangeValueCommand<int>>(iPtr, oldIntValue, *iPtr)
			);
		}
		break;
	}
	case FieldType::Uint: {
		uint32_t* iPtr = static_cast<uint32_t*>(valuePtr);

		// 一時保存用の変数（同時に編集できるUIは1つなのでstaticで使い回せます）
		static uint32_t oldIntValue;

		ImGui::DragInt(field.name.c_str(), static_cast<int*>(valuePtr), field.attributes.valueSpeed, 0, INT32_MAX);

		// 編集が開始された瞬間（マウスでクリックした時など）に元の値を保存
		if (ImGui::IsItemActivated()) {
			oldIntValue = *iPtr;
		}

		// 編集が確定した瞬間（マウスを離した時、Enterを押した時など）にコマンドを発行
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::ChangeValueCommand<uint32_t>>(iPtr, oldIntValue, *iPtr)
			);
		}
		break;
	}
	case FieldType::Bool: {
		bool* bPtr = static_cast<bool*>(valuePtr);

		// 一時保存用の変数（同時に編集できるUIは1つなのでstaticで使い回せます）
		static bool oldBoolValue;

		ImGui::Checkbox(field.name.c_str(), static_cast<bool*>(valuePtr));

		// 編集が開始された瞬間（マウスでクリックした時など）に元の値を保存
		if (ImGui::IsItemActivated()) {
			oldBoolValue = *bPtr;
		}

		// 編集が確定した瞬間（マウスを離した時、Enterを押した時など）にコマンドを発行
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::ChangeValueCommand<bool>>(bPtr, oldBoolValue, *bPtr)
			);
		}
		break;
	}
	case FieldType::String: {
		std::string* s = static_cast<std::string*>(valuePtr);

		// 編集中の状態を保持するためのstatic変数
		static char editBuf[1024] = "";
		static std::string oldStringValue;
		static void* activePtr = nullptr; // 現在編集中の変数のポインタ

		// このフィールドが現在アクティブ（編集中）でなければ、実際の値(*s)をバッファに同期する
		// （ポインタで判定することで、他のStringフィールド描画時に上書きされるのを防ぎます）
		if (activePtr != s) {
			strncpy_s(editBuf, s->c_str(), sizeof(editBuf));
			editBuf[sizeof(editBuf) - 1] = '\0';
		}

		// Enterを押したタイミングを検知するためのフラグ
		ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

		// InputTextを描画（入力中は editBuf のみが書き換わり、*s は変更されません）
		bool enterPressed = ImGui::InputText(field.name.c_str(), editBuf, sizeof(editBuf), flags);
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
				const char* path = (const char*)payload->Data;
				*s = path;
			}
		}
		// 編集が開始された瞬間（クリックした時など）
		if (ImGui::IsItemActivated()) {
			oldStringValue = *s;
			activePtr = s; // この変数を編集中としてマーク
		}

		// Enterが押された、またはフォーカスが外れて編集が完了した時
		if (enterPressed || ImGui::IsItemDeactivatedAfterEdit()) {
			if (*s != editBuf) {
				// 変更があればコマンドを積んでから、実際の値に反映
				Editor::EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<std::string>>(s, oldStringValue, editBuf)
				);
				*s = editBuf;
			}
		}

		// 編集状態が解除された時（Enter確定後、フォーカス外れ、Escキーでのキャンセル等）
		if (ImGui::IsItemDeactivated()) {
			activePtr = nullptr; // マークを外す
		}

		break;
	}
	case FieldType::WString: {
		std::wstring* s = static_cast<std::wstring*>(valuePtr);

		// 編集中の状態を保持するためのstatic変数
		static char editBuf[1024] = "";
		static std::wstring oldStringValue;
		static void* activePtr = nullptr; // 現在編集中の変数のポインタ

		// このフィールドが現在アクティブ（編集中）でなければ、実際の値(*s)をバッファに同期する
		// （ポインタで判定することで、他のStringフィールド描画時に上書きされるのを防ぎます）
		if (activePtr != s) {
			strncpy_s(editBuf, ConvertString(s->c_str()).c_str(), sizeof(editBuf));
			editBuf[sizeof(editBuf) - 1] = '\0';
		}

		// Enterを押したタイミングを検知するためのフラグ
		ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

		// InputTextを描画（入力中は editBuf のみが書き換わり、*s は変更されません）
		bool enterPressed = ImGui::InputText(field.name.c_str(), editBuf, sizeof(editBuf), flags);

		// 編集が開始された瞬間（クリックした時など）
		if (ImGui::IsItemActivated()) {
			oldStringValue = *s;
			activePtr = s; // この変数を編集中としてマーク
		}

		// Enterが押された、またはフォーカスが外れて編集が完了した時
		if (enterPressed || ImGui::IsItemDeactivatedAfterEdit()) {
			if (ConvertString(*s) != editBuf) {
				// 変更があればコマンドを積んでから、実際の値に反映
				Editor::EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<std::wstring>>(s, oldStringValue, ConvertString(editBuf))
				);
				*s = ConvertString(editBuf);
			}
		}

		// 編集状態が解除された時（Enter確定後、フォーカス外れ、Escキーでのキャンセル等）
		if (ImGui::IsItemDeactivated()) {
			activePtr = nullptr; // マークを外す
		}

		break;
	}
	case FieldType::Enum: {
		std::vector<std::string> names = field.enumOps->names();
		std::vector<const char*> labels;
		labels.reserve(names.size());
		for (auto& n : names) labels.push_back(n.c_str());

		int oldIndex = field.enumOps->getIndex(valuePtr); // 変更前のインデックス
		int currentIndex = oldIndex;

		bool changed = ImGui::Combo(field.name.c_str(), &currentIndex, labels.data(), static_cast<int>(labels.size()));

		if (changed && currentIndex != oldIndex) {
			auto setIndex = field.enumOps->setIndex;

			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::FunctionCommand>(
					[valuePtr, setIndex, currentIndex] { setIndex(valuePtr, currentIndex); },
					[valuePtr, setIndex, oldIndex] { setIndex(valuePtr, oldIndex); },
					field.name + " (Enum change)"
				)
			);

			field.enumOps->setIndex(valuePtr, currentIndex);
		}
		break;
	}
	case FieldType::Struct: {
		if (ImGui::TreeNode(field.name.c_str())) {
			TypeInfo* nested = field.structTypeInfo ? field.structTypeInfo() : nullptr;
			if (nested) {
				for (auto& subField : nested->fields) {
					DrawFieldUI(registry, e, subField, valuePtr); // valuePtr = ネスト構造体の先頭アドレス
				}
			} else {
				ImGui::TextDisabled("(struct type not registered)");
			}
			ImGui::TreePop();
		}
		break;
	}
	case FieldType::Array: {
		if (ImGui::TreeNode(field.name.c_str())) {
			size_t count = field.arrayOps->size ? field.arrayOps->size(valuePtr) : 0;
			int removeIndex = -1;

			for (size_t i = 0; i < count; ++i) {
				ImGui::PushID(static_cast<int>(i));
				void* elemPtr = field.arrayOps->getElement(valuePtr, i);

				if (field.arrayOps->elementType == FieldType::Struct) {
					std::string label = "[" + std::to_string(i) + "]";
					if (ImGui::TreeNode(label.c_str())) {
						TypeInfo* nested = field.arrayOps->elementStructTypeInfo ? field.arrayOps->elementStructTypeInfo() : nullptr;
						if (nested) {
							for (auto& subField : nested->fields) {
								DrawFieldUI(registry, e, subField, elemPtr);
							}
						}
						ImGui::TreePop();
					}
				} else {
					// プリミティブ要素は仮のFieldInfoを組み立てて既存の描画分岐を再利用
					FieldInfo elemField{};
					elemField.name = "[" + std::to_string(i) + "]";
					elemField.offset = 0; // arrayGetElementが要素先頭ポインタを直接返すため0でよい
					elemField.size = field.arrayOps->elementSize;
					elemField.type = field.arrayOps->elementType;
					elemField.attributes = field.attributes;
					DrawFieldUI(registry, e, elemField, elemPtr);
				}

				ImGui::SameLine();
				if (ImGui::SmallButton("x")) {
					removeIndex = static_cast<int>(i);
				}
				ImGui::PopID();
			}

			if (removeIndex >= 0) {
				size_t index = static_cast<size_t>(removeIndex);
				void* elemPtr = field.arrayOps->getElement(valuePtr, index);

				// 削除前の値をJSONスナップショットとして保存（構造体でもプリミティブでも対応）
				nlohmann::json snapshot = Editor::WriteArrayElementToJson(registry, field, elemPtr);

				auto arrayRemove = field.arrayOps->removeElement;
				auto arrayInsert = field.arrayOps->insertElement;

				Editor::EditorCommandOperator::AddCommand(
					std::make_unique<Command::FunctionCommand>(
						[valuePtr, arrayRemove, index] {
							arrayRemove(valuePtr, index);
						},
						[valuePtr, arrayInsert, index, snapshot, &registry, field] {
							arrayInsert(valuePtr, index);
							void* restoredPtr = field.arrayOps->getElement(valuePtr, index);
							Editor::ReadArrayElementFromJson(registry, snapshot, field, restoredPtr);
						},
						field.name + " (Array Remove)"
					)
				);
				arrayRemove(valuePtr, index);
			}

			if (ImGui::SmallButton("+ Add")) {
				auto arrayAdd = field.arrayOps->addElement;
				auto arrayRemove = field.arrayOps->removeElement;
				size_t newIndex = field.arrayOps->size(valuePtr);

				Editor::EditorCommandOperator::AddCommand(
					std::make_unique<Command::FunctionCommand>(
						[valuePtr, arrayAdd] { arrayAdd(valuePtr); },
						[valuePtr, arrayRemove, newIndex] { arrayRemove(valuePtr, newIndex); },
						field.name + " (Array Add)"
					)
				);
				arrayAdd(valuePtr);

				// TransformRoutineComponent(3D) / TransformRoutineComponent2D 共通の keyframes 専用処理:
				// 追加したwaypointのtranslate/rotation/scaleを原点・単位値ではなく
				// 「最初の1個ならエンティティ自身の値」「2個目以降なら直前のkeyframeの値」に揃える。
				// 両コンポーネントとも配列フィールド名が同じ"keyframes"のため、
				// translateフィールドの型(Float3 or Float2)で3D/2Dどちらかを判別する。
				if (field.name == "keyframes" &&
					field.arrayOps->elementType == FieldType::Struct &&
					field.arrayOps->elementStructTypeInfo) {

					void* newElemPtr = field.arrayOps->getElement(valuePtr, newIndex);
					TypeInfo* nested = field.arrayOps->elementStructTypeInfo();
					if (nested) {
						// まずtranslateフィールドを探して3D/2Dを判定する
						const FieldInfo* translateField = nullptr;
						for (auto& subField : nested->fields) {
							if (subField.name == "translate") {
								translateField = &subField;
								break;
							}
						}

						void* prevElemPtr = (newIndex > 0) ? field.arrayOps->getElement(valuePtr, newIndex - 1) : nullptr;

						if (translateField && translateField->type == FieldType::Float3) {
							// --- 3D版: TransformRoutineComponent::keyframes ---
							auto* transform = (prevElemPtr == nullptr)
								? registry.GetComponent<Component::TransformComponent>(e)
								: nullptr;

							for (auto& subField : nested->fields) {
								uint8_t* dst = static_cast<uint8_t*>(newElemPtr) + subField.offset;

								if (subField.name == "translate") {
									auto* newVal = reinterpret_cast<Math::Vector3*>(dst);
									if (prevElemPtr) {
										*newVal = *reinterpret_cast<Math::Vector3*>(static_cast<uint8_t*>(prevElemPtr) + subField.offset);
									} else if (transform) {
										*newVal = transform->translate;
									}
								} else if (subField.name == "rotation") {
									auto* newVal = reinterpret_cast<Math::Quaternion*>(dst);
									if (prevElemPtr) {
										*newVal = *reinterpret_cast<Math::Quaternion*>(static_cast<uint8_t*>(prevElemPtr) + subField.offset);
									} else if (transform) {
										*newVal = transform->rotation;
									}
								} else if (subField.name == "scale") {
									auto* newVal = reinterpret_cast<Math::Vector3*>(dst);
									if (prevElemPtr) {
										*newVal = *reinterpret_cast<Math::Vector3*>(static_cast<uint8_t*>(prevElemPtr) + subField.offset);
									} else if (transform) {
										*newVal = transform->scale;
									}
								}
							}
							ECS::DrawManipulatorSystem::SetSelectWaypointIndex(static_cast<int>(newIndex));
						} else if (translateField && translateField->type == FieldType::Float2) {
							// --- 2D版: TransformRoutineComponent2D::keyframes ---
							auto* transform2d = (prevElemPtr == nullptr)
								? registry.GetComponent<Component::Transform2DComponent>(e)
								: nullptr;

							for (auto& subField : nested->fields) {
								uint8_t* dst = static_cast<uint8_t*>(newElemPtr) + subField.offset;

								if (subField.name == "translate") {
									auto* newVal = reinterpret_cast<Math::Vector2*>(dst);
									if (prevElemPtr) {
										*newVal = *reinterpret_cast<Math::Vector2*>(static_cast<uint8_t*>(prevElemPtr) + subField.offset);
									} else if (transform2d) {
										*newVal = transform2d->translate;
									}
								} else if (subField.name == "rotation") {
									auto* newVal = reinterpret_cast<float*>(dst);
									if (prevElemPtr) {
										*newVal = *reinterpret_cast<float*>(static_cast<uint8_t*>(prevElemPtr) + subField.offset);
									} else if (transform2d) {
										*newVal = transform2d->rotation;
									}
								} else if (subField.name == "scale") {
									auto* newVal = reinterpret_cast<Math::Vector2*>(dst);
									if (prevElemPtr) {
										*newVal = *reinterpret_cast<Math::Vector2*>(static_cast<uint8_t*>(prevElemPtr) + subField.offset);
									} else if (transform2d) {
										*newVal = transform2d->scale;
									}
								}
							}
							ECS::DrawManipulatorSystem::SetSelectWaypointIndex2D(static_cast<int>(newIndex));
						}
					}
				}
			}
			ImGui::TreePop();
		}
		break;
	}
	case FieldType::Entity: {
		ECS::Entity* ePtr = static_cast<ECS::Entity*>(valuePtr);

		std::string label = "(None)";
		if (*ePtr != ECS::INVALID_ENTITY) {
			auto* refTag = registry.GetComponent<Editor::EditTag>(*ePtr);
			label = refTag ? refTag->name : ("(Unknown Entity)");
		}

		// ドロップ先として機能するボタン
		ImGui::Button(label.c_str(), ImVec2(160, 0));

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) {
				ECS::Entity dropped = *(ECS::Entity*)payload->Data;
				ECS::Entity oldValue = *ePtr;

				if (dropped != oldValue) {
					*ePtr = dropped;
					Editor::EditorCommandOperator::AddCommand(
						std::make_unique<Command::ChangeValueCommand<ECS::Entity>>(ePtr, oldValue, dropped)
					);
				}
			}
			ImGui::EndDragDropTarget();
		}

		// 右クリックで参照解除
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Clear")) {
				ECS::Entity oldValue = *ePtr;
				*ePtr = ECS::INVALID_ENTITY;
				Editor::EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<ECS::Entity>>(ePtr, oldValue, ECS::INVALID_ENTITY)
				);
			}
			ImGui::EndPopup();
		}
		ImGui::SameLine();
		ImGui::Text("%s", field.name.c_str());
		break;
	}
	default:
		ImGui::Text(field.name.c_str());
		ImGui::SameLine();
		ImGui::Text(" : Unsupported field type");
		break;
	}

#else
	static_cast<void>(field);
	static_cast<void>(ptr);
	static_cast<void>(registry);
	static_cast<void>(e);
#endif // USE_IMGUI

}
}