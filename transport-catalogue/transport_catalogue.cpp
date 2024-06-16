#include "transport_catalogue.h"

namespace transport {
namespace catalogue {

void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates coordinates) {
    stops_[name] = {name, coordinates};
}

void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_circular) {
    buses_[name] = {name, stops, is_circular};
    for (const auto& stop : stops) {
        stop_to_buses_[stop].insert(name);
    }
}

const Stop* TransportCatalogue::FindStop(const std::string& name) const {
    auto it = stops_.find(name);
    return it != stops_.end() ? &it->second : nullptr;
}

const BusRoute* TransportCatalogue::FindBus(const std::string& name) const {
    auto it = buses_.find(name);
    return it != buses_.end() ? &it->second : nullptr;
}

std::tuple<int, int, double> TransportCatalogue::GetBusInfo(const std::string& name) const {
    const auto* bus = FindBus(name);
    if (!bus) {
        return {0, 0, 0.0};
    }
    int stop_count = static_cast<int>(bus->stops.size());
    std::unordered_set<std::string> unique_stops(bus->stops.begin(), bus->stops.end());
    int unique_stop_count = static_cast<int>(unique_stops.size());
    double route_length = 0.0;
    for (size_t i = 1; i < bus->stops.size(); ++i) {
        const auto* from = FindStop(bus->stops[i - 1]);
        const auto* to = FindStop(bus->stops[i]);
        if (from && to) {
            route_length += geo::ComputeDistance(from->coordinates, to->coordinates);
        }
    }
    return {stop_count, unique_stop_count, route_length};
}

const std::set<std::string>* TransportCatalogue::GetBusesForStop(const std::string& stop_name) const {
    auto it = stop_to_buses_.find(stop_name);
    return it != stop_to_buses_.end() ? &it->second : nullptr;
}

} // namespace catalogue
} // namespace transport
