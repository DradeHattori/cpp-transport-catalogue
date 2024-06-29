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
            std::vector<std::string> stops;
            bool is_circular;
        };

        struct BusInfo {
            int stop_count;
            int unique_stop_count;
            double route_length;
            double curvature;
        };

        class TransportCatalogue {
        public:
            void AddStop(const std::string& name, geo::Coordinates coordinates, std::unordered_map<std::string, int>& distances);
            void AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_circular);
            void AddDistance(const std::string& stop_name, const std::string& other_stop_name, int distance);

            std::string_view GetStopNameView(const std::string& name) const;
            const Stop* FindStop(std::string_view name) const;
            const BusRoute* FindBus(std::string_view name) const;
            std::optional<BusInfo> GetBusInfo(std::string_view name) const;
            const std::unordered_set<std::string_view>* GetBusesForStop(std::string_view stop_name) const;

        private:

            struct StopPairHasher {
                std::size_t operator()(const std::pair<const Stop*, const Stop*>& StopsPair) const {
                    std::size_t left = std::hash<const void*>()(static_cast<const void*>(StopsPair.first));
                    std::size_t right = std::hash<const void*>()(static_cast<const void*>(StopsPair.second));
                    return left ^ (right << 1); // Комбинирование хешей
                }
            };

            std::unordered_map<std::string_view, Stop*> stops_;
            std::unordered_map<std::string_view, BusRoute*> buses_;
            std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_to_buses_;
            std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopPairHasher> stop_distances_;

            std::deque<Stop> stop_objects_;
            std::deque<BusRoute> bus_objects_;
        };

    } // namespace catalogue
} // namespace transport
