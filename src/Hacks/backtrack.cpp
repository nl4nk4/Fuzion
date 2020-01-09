#include "backtrack.h"
#include "../interfaces.h"

#include <iomanip>
#include "../Utils/draw.h"
#include "../Utils/xorstring.h"
#include "../settings.h"

int Backtrack::latest_tick;
Backtrack::data Backtrack::headPositions[64][12];
bool Settings::Backtrack::enabled = false;

void Backtrack::FrameStageNotify(ClientFrameStage_t stage)
{
    if (!Settings::Backtrack::enabled)
        return;

	if (!engine->IsInGame())
		return;

	if (stage != ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_END)
		return;

    if (!globalVars)
        return;

    latest_tick = globalVars->tickcount;
}

inline Vector btAngleQAngles(QAngle angle1, QAngle angle2)
{
    Vector vector1 = Vector(angle1.x,angle1.y,angle1.z);
    Vector vector2 = Vector(angle2.x,angle2.y,angle2.z);
    
    Vector angle = vector1 + (vector2 * 2.f);

    auto sy = sin(angle.y / 180.f * static_cast<float>(M_PI));
    auto cy = cos(angle.y / 180.f * static_cast<float>(M_PI));
    
    auto sp = sin(angle.x / 180.f * static_cast<float>(M_PI));
    auto cp = cos(angle.x / 180.f* static_cast<float>(M_PI));
    
    return Vector(cp * cy, cp * sy, -sp);
}

inline float distanceToPoint(Vector point, Vector lineOrigin, Vector dir)
{
    Vector pointDir = point - lineOrigin;
    
    float tempOffset = pointDir.Dot(dir) / (dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    if (tempOffset < 0.000001f)
        return MAXFLOAT;
    
    Vector perpendicularPoint = lineOrigin + (dir * tempOffset);
    
    return (point - perpendicularPoint).Length();
}

bool IsTickValid(int tick)
{
    int delta = Backtrack::latest_tick - tick;
    float deltaTime = delta * globalVars->interval_per_tick;
    return (fabs(deltaTime) <= 0.2f);
}

void Backtrack::CreateMove(CUserCmd* cmd)
{
    if (!Settings::Backtrack::enabled)
        return;

    if (!engine->IsInGame())
		return;

    C_BasePlayer* pLocal = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
    if (!pLocal) {
        return;
    }

    int bestTargetIndex = -1;
    float bestFov = MAXFLOAT;
    if (!pLocal->GetAlive())
        return;
        
    for (int i = 0; i < engine->GetMaxClients(); i++)
    {
        C_BasePlayer* entity = (C_BasePlayer*)entityList->GetClientEntity(i);
            
        if (!entity || !pLocal)
            continue;
            
        if (entity == pLocal)
            continue;
            
        if (entity->GetDormant())
             continue;
            
        if (entity->GetTeam() == pLocal->GetTeam())
            continue;
        
        if (entity->GetAlive())
        {

            float simtime = entity->GetSimulationTime();
            Vector hitboxPos = entity->GetBonePosition(8);
            headPositions[i][cmd->command_number % 13] = data{ simtime, hitboxPos };

            QAngle viewAngles = cmd->viewangles;
            QAngle aimPunchAngle = *pLocal->GetAimPunchAngle();

            Vector ViewDir    = btAngleQAngles(viewAngles, aimPunchAngle);
            float FOVDistance = distanceToPoint(hitboxPos, pLocal->GetEyePosition(), ViewDir);
            
            if (bestFov > FOVDistance)
            {
                bestFov = FOVDistance;
                bestTargetIndex = i;
            }
        }
    }
    float bestTargetSimTime = 0.0;
    if (bestTargetIndex != -1)
    {
        float tempFloat = MAXFLOAT;

        QAngle viewAngles = cmd->viewangles;
        QAngle aimPunchAngle = *pLocal->GetAimPunchAngle();

        Vector ViewDir    = btAngleQAngles(viewAngles, aimPunchAngle);
        for (int t = 0; t < 12; ++t)
        {
            float tempFOVDistance = distanceToPoint(headPositions[bestTargetIndex][t].hitboxPos, pLocal->GetEyePosition(), ViewDir);
            if (tempFloat > tempFOVDistance && headPositions[bestTargetIndex][t].simtime > pLocal->GetSimulationTime() - 1)
            {
                tempFloat = tempFOVDistance;
                bestTargetSimTime = headPositions[bestTargetIndex][t].simtime;
            }
        }
        if (cmd->buttons & IN_ATTACK)
        {
            cmd->tick_count = (int)(0.5f + ((float)(bestTargetSimTime) / globalVars->interval_per_tick));
        }
    }
}

