#include "Game/GameCommon.hpp"
#include "Engine/Network/NetworkSystem.hpp"

int GetIntSign(int value)
{
	if (value > 0)
	{
		return 1;
	}
	else if (value < 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

bool IsPlayingLocally()
{
	return g_theNetwork->GetState() == NetState::IDLE;
}
