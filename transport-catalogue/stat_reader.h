#pragma once

#include "transport_catalogue.h"

#include <iosfwd>
#include <string_view>

namespace transport {
	namespace stat {

		void ParseAndPrintStat(const transport::catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output);

	} // namespace stat
} // namespace transport
