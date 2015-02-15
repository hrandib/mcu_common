// timer IRQ priority must be lower than EXTI IRQ priority.
#pragma once
#include "gpio.h"
#include "delay.h"
#include "timers.h"
#include "utils.h"
#include <stdint.h>
#include <limits>

bool _fail_ = false;

namespace Mcucpp
{
namespace Sensors
{
	template<typename Timer, typename Pin>
	class Am2302
	{
	private:
		static enum State
		{
			Start,
			GetResponse,
			Reading,
			Timeout
		} state;
		enum {
			PollPeriod = 2500,	//ms
			TimerStep = 4,		//us
			ResponseTimeMin = 40 / TimerStep,		//Real sensor timings not conform to datasheet
			ResponseTimeMax = 100 / TimerStep,
			Threshold = 50 / TimerStep,
		};  
		static uint8_t bitcount_, csum_;
		static uint32_t value_;
		FORCEINLINE
		static void DetectResponse()
		{
			static uint8_t stagecount;
			Led1::Set();
			uint8_t rtime = Timer::ReadCounter();
			Timer::Clear();
			switch (stagecount)
			{
				case 0: case 1: ++stagecount;
					break;
				case 2: case 3: if (rtime >= ResponseTimeMin && rtime <= ResponseTimeMax)
				{
					if (Pin::IsSet()) ++stagecount;
					else if (stagecount == 3)
					{
						stagecount = 0;
						bitcount_ = value_ = 0;
						state = Reading;
						Led1::Clear();
					}
				}
				else stagecount = 0;
					break;
			}
		}
		FORCEINLINE
		static void ReadingProcess()
		{
			Led2::Set();
			uint8_t rtime = Timer::ReadCounter();
			Timer::Clear();
 			if (!Pin::IsSet())
			{
				if(bitcount_ < 32)
				{
					value_ <<= 1;
					if(rtime > Threshold) value_ |= 0x01;
				}
				else
				{
					csum_ <<= 1;
					if(rtime > Threshold) csum_ |= 0x01;
				}
				bitcount_++;
			}
			if (bitcount_ >= 40)
			{
				bitcount_ = 0;
				uint8_t sum = (value_ & 0xFF) + (value_ >> 8 & 0xFF) + (value_ >> 16 & 0xFF) + (value_ >> 24 & 0xFF);
				if (sum != csum_)
				{
					_fail_ = true;
					value_ = 0;
				}
				else Led2::Clear();
				state = Timeout;
			}
		}
	public:
		static bool IsFail()
		{
			if(_fail_)
			{
				_fail_ = false;
				return true;
			}
			else return false;
		}
		FORCEINLINE
		static void Init()
		{
			using namespace Gpio;
			Pin::Exti::EnableIRQ(Trigger::BothEdges);
			Pin::template SetConfig<Input, PullUp>();
			using namespace Timers;
			Timer:: template Init<UpCount, (F_CPU * TimerStep) / 1000000UL, 512>(); //step 4us, cycle 2ms
			Timer::EnableIRQ(UpdateIRQ);
			Timer::Enable();
			Pin::Clear();
			Pin::template SetConfig<Gpio::OutputFast, Gpio::OpenDrain>();
			state = Start;
		}
		FORCEINLINE
		static uint32_t GetValues()
		{
			return value_;
		}
		FORCEINLINE
		static void DeInit()
		{
			state = Start;
			Timer::Disable();
			Timer::DisableIRQ(Timers::UpdateIRQ);
			Pin::template SetConfig<Gpio::Input, Gpio::PullUp>();
		}
		FORCEINLINE
		static void ExtiIRQ()
		{
			Pin::Exti::ClearPending();
			switch (state)
			{
			case GetResponse: DetectResponse();
				break;
			case Reading: ReadingProcess();
				break;
			default:
				break;
			}
		}
		FORCEINLINE
		static void TimerIRQ()
		{
			using namespace Timers;
			Timer::ClearEvent(UpdateEv);
			static uint16_t timeout;
			if (state == Start)
			{
				state = GetResponse;
				Pin::template SetConfig<Gpio::Input, Gpio::PullUp>();
			}
			else if (timeout++ == PollPeriod / 2)
			{
				if(state != Timeout) _fail_ = true;
				Pin::template SetConfig<Gpio::OutputFast, Gpio::OpenDrain>();
				timeout = 0;
				state = Start;
 			}
 		}
	};

	template<typename Timer, typename Pin>
	uint8_t Am2302<Timer, Pin>::bitcount_;
	template<typename Timer, typename Pin>
	uint8_t Am2302<Timer, Pin>::csum_;
	template<typename Timer, typename Pin>
	uint32_t Am2302<Timer, Pin>::value_;
	template<typename Timer, typename Pin>
	typename Am2302<Timer, Pin>::State Am2302<Timer, Pin>::state;

}//AM2302
}//Mcucpp

