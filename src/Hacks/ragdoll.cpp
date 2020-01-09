#include "ragdoll.h"

#include "../interfaces.h"
#include "../settings.h"

bool Settings::InvertGravity::enabled = false;

void RagdollGravity::FrameStageNotify(ClientFrameStage_t stage)
{
	if (!Settings::InvertGravity::enabled)
		return;

	if (!engine->IsInGame())
		return;

	static ConVar* invertedragdoll = cvar->FindVar("cl_ragdoll_gravity");
	invertedragdoll->flags &= ~FCVAR_CHEAT;
	invertedragdoll->SetValue(-600);
}
