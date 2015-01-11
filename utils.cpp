#include "utils.h"

namespace Mcucpp
{
	uint32_t Populate(uint8_t x)
	{
		uint32_t result = 0;
		while(x--)
		{
			result = (result << 1) | 1;
		}
		return result;
	}
}
