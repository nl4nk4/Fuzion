#pragma once

#include "../SDK/IInputSystem.h"
#include "../SDK/definitions.h"

namespace Backtrack
{
    struct lbyRecords
    {
        int tick_count;
        float lby;
        Vector headPosition;
    };

    struct data
    {
        float simtime;
        Vector hitboxPos;
        Vector origin;
        QAngle angs;
    };
    
    extern int latest_tick;
    extern data headPositions[64][12];
	//Hooks
	void CreateMove(CUserCmd* cmd);
    void FrameStageNotify(ClientFrameStage_t stage);
}