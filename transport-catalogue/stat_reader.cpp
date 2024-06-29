#include "stat_reader.h"
#include "geo.h"
#include <iomanip>
#include <algorithm>

namespace transport {
    namespace stat {

        void PrintBusInfo(const transport::catalogue::TransportCatalogue& transport_catalogue, std::string_view bus_name, std::ostream& output) {
            auto bus_info_opt = transport_catalogue.GetBusInfo(bus_name);
            if (bus_info_opt) {
                const auto& bus_info = *bus_info_opt;
                output << "Bus " << bus_name << ": " << bus_info.stop_count << " stops on route, "
                    << bus_info.unique_stop_count << " unique stops, "
                    << bus_info.route_length << " route length, "
                    << std::setprecision(6) << bus_info.curvature << " curvature\n";
            }
            else {
                output << "Bus " << bus_name << ": not found\n";
            }
        }

        void PrintStopInfo(const transport::catalogue::TransportCatalogue& transport_catalogue, std::string_view stop_name, std::ostream& output) {
            auto stop_info_opt = transport_catalogue.FindStop(stop_name);
            if (stop_info_opt) {
                auto buses = transport_catalogue.GetBusesForStop(stop_name);
                if (!buses || buses->empty()) {
                    output << "Stop " << stop_name << ": no buses\n";
                }
                else {
                    std::vector<std::string_view> sorted_buses(buses->begin(), buses->end());
                    std::sort(sorted_buses.begin(), sorted_buses.end());
                    output << "Stop " << stop_name << ": buses";
                    for (const auto& bus : sorted_buses) {
                        output << ' ' << bus;
                    }
                    output << '\n';
                }
            }
            else {
                output << "Stop " << stop_name << ": not found\n";
            }
        }


        void ParseAndPrintStat(const transport::catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
            if (request.substr(0, 4) == "Bus ") {
                PrintBusInfo(transport_catalogue, request.substr(4), output);
            }
            else if (request.substr(0, 5) == "Stop ") {
                PrintStopInfo(transport_catalogue, request.substr(5), output);
            }
        }

    } // namespace stat
} // namespace transport

