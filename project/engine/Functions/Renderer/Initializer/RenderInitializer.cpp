#include "stdafx.h"
#include "RenderInitializer.h"
#include "MeshInitialzer.h"
#include "ParticleInitialzer.h"
#include "SpriteInitializer.h"
#include "PreRenderInitialzer.h"
#include "PrimitiveInitializer.h"

namespace NoEngine {

void RenderInitializer::Initialize(RenderContext& renderContext) {
	if (renderContext.IsInitialized()) return;
	MeshInitialzer::Initialize(renderContext);
	SpriteInitializer::Initialize(renderContext);
	ParticleInitialzer::Initialize(renderContext);
	PreRenderInitialzer::Initialize(renderContext);
	PrimitiveInitializer::Initialize(renderContext);
}
}