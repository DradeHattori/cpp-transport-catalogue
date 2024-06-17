#include "transport_catalogue.h"
#include "geo.h"

namespace transport {
    namespace catalogue {

        void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates coordinates) {
            stops_[name] = { name, coordinates };
        }

        void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_circular) {
            buses_[name] = { name, stops, is_circular };
            for (const auto& stop : stops) {
                stop_to_buses_[stop].insert(name);
            }
        }

        std::optional<Stop> TransportCatalogue::FindStop(std::string_view name) const {
            auto it = stops_.find(std::string(name));
            if (it != stops_.end()) {
                return it->second;
            }
            else {
                return std::nullopt;
            }
        }

        std::optional<BusRoute> TransportCatalogue::FindBus(std::string_view name) const {
            auto it = buses_.find(std::string(name));
            if (it != buses_.end()) {
                return it->second;
            }
            else {
                return std::nullopt;
            }
        }

        std::optional<BusInfo> TransportCatalogue::GetBusInfo(std::string_view name) const {
            auto bus_opt = FindBus(name);
            if (!bus_opt) {
                return std::nullopt;
            }

            const auto& bus = *bus_opt;
            int stop_count = static_cast<int>(bus.stops.size());
            std::unordered_set<std::string> unique_stops(bus.stops.begin(), bus.stops.end());
            int unique_stop_count = static_cast<int>(unique_stops.size());
            double route_length = 0.0;

            for (size_t i = 1; i < bus.stops.size(); ++i) {
                auto from_opt = FindStop(bus.stops[i - 1]);
                auto to_opt = FindStop(bus.stops[i]);
                if (from_opt && to_opt) {
                    route_length += geo::ComputeDistance(from_opt->coordinates, to_opt->coordinates);
                }
            }

            return BusInfo{ stop_count, unique_stop_count, route_length };
        }

        const std::set<std::string>* TransportCatalogue::GetBusesForStop(std::string_view stop_name) const {
            auto it = stop_to_buses_.find(std::string(stop_name));
            return it != stop_to_buses_.end() ? &it->second : nullptr;
        }

    } // namespace catalogue
} // namespace transport
