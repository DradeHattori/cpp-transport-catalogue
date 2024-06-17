#include "stat_reader.h"
#include "geo.h"
#include <iomanip>

namespace transport {
    namespace stat {

        void ParseAndPrintStat(const transport::catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
            if (request.substr(0, 4) == "Bus ") {
                auto bus_name = request.substr(4);
                auto bus_info_opt = transport_catalogue.GetBusInfo(bus_name);
                if (bus_info_opt) {
                    const auto& bus_info = *bus_info_opt;
                    output << "Bus " << bus_name << ": " << bus_info.stop_count << " stops on route, "
                        << bus_info.unique_stop_count << " unique stops, "
                        << std::setprecision(6) << bus_info.route_length << " route length\n";
                }
                else {
                    output << "Bus " << bus_name << ": not found\n";
                }
            }
            else if (request.substr(0, 5) == "Stop ") {
                auto stop_name = request.substr(5);
                auto stop_info_opt = transport_catalogue.FindStop(stop_name);
                if (stop_info_opt) {
                    auto buses = transport_catalogue.GetBusesForStop(stop_name);
                    if (!buses || buses->empty()) {
                        output << "Stop " << stop_name << ": no buses\n";
                    }
                    else {
                        output << "Stop " << stop_name << ": buses";
                        for (const auto& bus : *buses) {
                            output << ' ' << bus;
                        }
                        output << '\n';
                    }
                }
                else {
                    output << "Stop " << stop_name << ": not found\n";
                }
            }
        }

    } // namespace stat
} // namespace transport
