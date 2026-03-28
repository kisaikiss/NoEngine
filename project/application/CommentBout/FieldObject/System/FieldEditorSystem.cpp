#include "stdafx.h"
#include "FieldEditorSystem.h"
#include "application/CommentBout/FieldObject/Editor/FieldObjectEditor.h"

void FieldEditorSystem::Update(No::Registry& registry, float /*deltaTime*/)
{
	if (editor_) {
		editor_->Update(registry);
	}
}
