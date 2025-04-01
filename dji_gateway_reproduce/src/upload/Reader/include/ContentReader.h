#include <codecvt>
#include <cstdint>
#include <iostream>



namespace dji{

  namespace gateway {
		
		class ContentReader{
		public:
			virtual int Next() = 0;
	};
    
}//namespace gateway
} //namespace dji