#include "transport_catalogue.h"
#include "geo.h"

#include <algorithm>
#include <vector>

namespace transport {
    namespace catalogue {

        void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates coordinates) {
            stop_objects_.push_back({ name, coordinates });
            Stop* stop_ptr = &stop_objects_.back();
            stops_[stop_ptr->name] = stop_ptr;
        }

        void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_circular) {
            bus_objects_.push_back({ name, std::deque<std::string>(stops.begin(), stops.end()), is_circular });
            BusRoute* bus_ptr = &bus_objects_.back();
            buses_[bus_ptr->name] = bus_ptr;
            for (const auto& stop : stops) {
                stop_to_buses_[stop].insert(bus_ptr->name);
            }
        }

        std::string_view TransportCatalogue::GetStopNameView(const std::string& name) const {
            auto it = stops_.find(name);
            if (it != stops_.end()) {
                return it->second->name;
            }
            else {
                return {};
            }
        }

        const Stop* TransportCatalogue::FindStop(std::string_view name) const {
            auto it = stops_.find(name);
            if (it != stops_.end()) {
                return it->second;
            }
            else {
                return nullptr;
            }
        }

        const BusRoute* TransportCatalogue::FindBus(std::string_view name) const {
            auto it = buses_.find(name);
            if (it != buses_.end()) {
                return it->second;
            }
            else {
                return nullptr;
            }
        }

        std::optional<BusInfo> TransportCatalogue::GetBusInfo(std::string_view name) const {
            auto bus_it = buses_.find(name);
            if (bus_it == buses_.end()) {
                return std::nullopt;
            }

            const auto& bus = *bus_it->second;
            int stop_count = static_cast<int>(bus.stops.size());
            std::unordered_set<std::string_view> unique_stops(bus.stops.begin(), bus.stops.end());
            int unique_stop_count = static_cast<int>(unique_stops.size());
            double route_length = 0.0;

            auto prev_stop_it = stops_.find(bus.stops.front());
            for (const auto& stop_name : bus.stops) {
                auto stop_it = stops_.find(stop_name);
                if (stop_it != stops_.end() && prev_stop_it != stops_.end()) {
                    route_length += geo::ComputeDistance(prev_stop_it->second->coordinates, stop_it->second->coordinates);
                    prev_stop_it = stop_it;
                }
            }

            return BusInfo{ stop_count, unique_stop_count, route_length };
        }

        const std::unordered_set<std::string_view>* TransportCatalogue::GetBusesForStop(std::string_view stop_name) const {
            auto it = stop_to_buses_.find(stop_name);
            if (it != stop_to_buses_.end()) {
                return &it->second;
            }
            else {
                return nullptr;
            }
        }

    } // namespace catalogue
} // namespace transport
