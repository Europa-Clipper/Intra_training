#ifndef PIPEREADER_H
#define PIPEREADER_H

#include "ContentReader.h"
#include "../../../utility/include/IFStream.h"
#include <cstdint>


namespace dji{
  namespace gateway{
		class PipeReader : public ContentReader {
			public:

			IFStream* ifst;

			int Next() override;
			int releaseCache();

			std::string getReadCache();

			PipeReader(std::string path, std::int64_t totolSzie, std::int64_t partSize);
			~PipeReader();

			private:

			const std::int64_t totol_size;
			const std::int64_t part_size;
			std::int64_t finished_size{0};
			std::string readcache{};

			bool finished{false};

		};



}  //namenaspcae gateway
}  //namespace dji

#endif