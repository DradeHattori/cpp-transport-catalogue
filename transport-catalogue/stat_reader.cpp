#include "stat_reader.h"

#include <iomanip>

namespace transport {
namespace stat {

void ParseAndPrintStat(const transport::catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
    if (request.substr(0, 4) == "Bus ") {
        auto bus_name = std::string(request.substr(4));
        const auto* bus_info = transport_catalogue.FindBus(bus_name);
        if (bus_info) {
            auto [stop_count, unique_stop_count, route_length] = transport_catalogue.GetBusInfo(bus_name);
            output << "Bus " << bus_name << ": " << stop_count << " stops on route, "
                   << unique_stop_count << " unique stops, "
                   << std::setprecision(6) << route_length << " route length\n";
        } else {
            output << "Bus " << bus_name << ": not found\n";
        }
    } else if (request.substr(0, 5) == "Stop ") {
        auto stop_name = std::string(request.substr(5));
        const auto* stop_info = transport_catalogue.FindStop(stop_name);
        if (stop_info) {
            const auto* buses = transport_catalogue.GetBusesForStop(stop_name);
            if (!buses || buses->empty()) {
                output << "Stop " << stop_name << ": no buses\n";
            } else {
                output << "Stop " << stop_name << ": buses";
                for (const auto& bus : *buses) {
                    output << ' ' << bus;
                }
                output << '\n';
            }
        } else {
            output << "Stop " << stop_name << ": not found\n";
        }
    }
}

} // namespace stat
} // namespace transport
