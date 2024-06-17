#pragma once

#include "geo.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <vector>
#include <string_view>
#include <optional> 

namespace transport {
namespace catalogue {

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
    };

    struct BusRoute {
        std::string name;
        std::deque<std::string> stops;
        bool is_circular;
    };

    struct BusInfo {
        int stop_count;
        int unique_stop_count;
        double route_length;
    };

    class TransportCatalogue {
    public:
        void AddStop(const std::string& name, geo::Coordinates coordinates);
        void AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_circular);

        std::string_view GetStopNameView(const std::string& name) const;
        const Stop* FindStop(std::string_view name) const;
        const BusRoute* FindBus(std::string_view name) const;
        std::optional<BusInfo> GetBusInfo(std::string_view name) const;
        const std::unordered_set<std::string_view>* GetBusesForStop(std::string_view stop_name) const;

    private:
        std::unordered_map<std::string, Stop> stops_;
        std::unordered_map<std::string, BusRoute> buses_;
        std::unordered_map<std::string, std::unordered_set<std::string>> stop_to_buses_;

        std::deque<Stop> stop_objects_;
        std::deque<BusRoute> bus_objects_;
    };

} // namespace catalogue
} // namespace transport
