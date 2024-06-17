#pragma once

#include "geo.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <tuple>
#include <optional>
#include <string_view>

namespace transport {
    namespace catalogue {

        struct Stop {
            std::string name;
            geo::Coordinates coordinates;
        };

        struct BusRoute {
            std::string name;
            std::vector<std::string> stops;
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
            std::optional<Stop> FindStop(std::string_view name) const;
            std::optional<BusRoute> FindBus(std::string_view name) const;
            std::optional<BusInfo> GetBusInfo(std::string_view name) const;
            const std::set<std::string>* GetBusesForStop(std::string_view stop_name) const;

        private:
            std::unordered_map<std::string, Stop> stops_;
            std::unordered_map<std::string, BusRoute> buses_;
            std::unordered_map<std::string, std::set<std::string>> stop_to_buses_;
        };

    } // namespace catalogue
} // namespace transport
