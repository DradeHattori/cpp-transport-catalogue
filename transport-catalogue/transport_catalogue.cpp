#include "transport_catalogue.h"
#include "geo.h"

#include <algorithm>
#include <vector>

namespace transport {
    namespace catalogue {

        void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates coordinates, std::unordered_map<std::string, int>& distances) {
            auto it = stops_.find(name);
            if (it != stops_.end()) {
                it->second->coordinates = coordinates;
            }
            else {
                stop_objects_.push_back({ name, coordinates });
                Stop* stop_ptr = &stop_objects_.back();
                stops_[stop_ptr->name] = stop_ptr;
            }

            for (const auto& [neighbor_name, distance] : distances) {
                AddDistance(name, neighbor_name, distance);
            }
        }

        void TransportCatalogue::AddDistance(const std::string& stop_name, const std::string& other_stop_name, int distance) {
            Stop* stop = nullptr;
            if (stops_.count(stop_name)) {
                stop = stops_.at(stop_name);
            }
            else {
                stop_objects_.push_back({ stop_name, {0, 0} });
                stop = &stop_objects_.back();
                stops_[stop->name] = stop;
            }

            Stop* other_stop = nullptr;
            if (stops_.count(other_stop_name)) {
                other_stop = stops_.at(other_stop_name);
            }
            else {
                stop_objects_.push_back({ other_stop_name, {0, 0} });
                other_stop = &stop_objects_.back();
                stops_[other_stop->name] = other_stop;
            }

            stop_distances_[{stop, other_stop}] = distance;
            if (stop_distances_.find({ other_stop, stop }) == stop_distances_.end()) {
                stop_distances_[{other_stop, stop}] = distance;
            }
        }

        void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_circular) {
            bus_objects_.push_back({ name, stops, is_circular });
            BusRoute* bus_ptr = &bus_objects_.back();
            buses_[bus_ptr->name] = bus_ptr;

            for (const auto& stop : bus_ptr->stops) {
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
            double geo_length = 0.0;

            for (size_t i = 1; i < bus.stops.size(); ++i) {
                const Stop* from = stops_.at(bus.stops[i - 1]);
                const Stop* to = stops_.at(bus.stops[i]);
                if (stop_distances_.count({ from, to })) {
                    route_length += stop_distances_.at({ from, to });
                }
                else {
                    route_length += 0;
                }

                geo_length += geo::ComputeDistance(from->coordinates, to->coordinates);
            }

            double curvature = route_length / geo_length;
            return BusInfo{ stop_count, unique_stop_count, route_length, curvature };
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
