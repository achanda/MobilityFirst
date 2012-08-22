#ifndef __DATA_H_
#define __DATA_H_

#include "ns3/object.h"

namespace ns3 
{
	namespace mf 
	{
		class Data : public Object
		{
			Time mTime;
			Ipv4Address mSrc;
			Ipv4Address mDest;
			
		public:
			
		}
	}
}

#endif