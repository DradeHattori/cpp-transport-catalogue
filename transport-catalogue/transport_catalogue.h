//// transport_catalogue.h

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
        struct StopPairHasher {
            std::size_t operator()(const std::pair<const Stop*, const Stop*>& StopsPair) const {
                std::size_t left = std::hash<const void*>()(static_cast<const void*>(StopsPair.first));
                std::size_t right = std::hash<const void*>()(static_cast<const void*>(StopsPair.second));
                return left ^ (right << 1); // Комбинирование хешей
            }
        };
        class TransportCatalogue {
        public:
            void AddStop(const std::string_view name, geo::Coordinates coordinates, std::unordered_map<std::string, int>& distances);
            void AddBus(const std::string_view name, const std::vector<std::string>& stops, bool is_circular);
            void AddDistance(const std::string_view stop_name, const std::string_view other_stop_name, int distance);

            const Stop* FindStop(std::string_view name) const;
            const BusRoute* FindBus(std::string_view name) const;
            std::optional<BusInfo> GetBusInfo(std::string_view name) const;
            const std::unordered_set<std::string_view>* GetBusesForStop(std::string_view stop_name) const;

            const std::unordered_map<std::string_view, Stop*>& GetAllStops() const;
            const std::unordered_map<std::string_view, BusRoute*>& GetAllBuses() const;

            std::optional <double>  GetDistance(std::string_view from, std::string_view to) const;

        private:
            std::unordered_map<std::string_view, Stop*> stops_;
            std::unordered_map<std::string_view, BusRoute*> buses_;
            std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_to_buses_;
            std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopPairHasher> stop_distances_;

            std::deque<Stop> stop_objects_;
            std::deque<BusRoute> bus_objects_;
        };

    } // namespace catalogue
} // namespace transport