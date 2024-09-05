// transport_router.h

#pragma once

#include "transport_catalogue.h"
#include "router.h"

#include <optional>
#include <string>
#include <vector>

using namespace graph;

namespace transport {
    namespace catalogue {

        struct RoutingSettings {
            int bus_velocity = 0;
            int bus_wait_time = 0;
        };

        struct RouteItem {
            std::string type;
            std::string name;
            int span_count = 0; // может быть пустым если запрос - Route
            double time = 0;
        };


        class TransportRouter {
        public:
            TransportRouter(const RoutingSettings& settings, const TransportCatalogue& catalogue);
            void BuildGraph(const TransportCatalogue& catalogue);

            std::optional<std::tuple<double, std::vector<RouteItem>>> GetRoute(const std::string_view from, const std::string_view to) const;

       

            // std::optional<std::tuple<double, std::vector<RouteItem>>> GetRoute(const TransportCatalogue& catalogue, const std::string_view from, const std::string_view to) const;

        private:
            RoutingSettings settings_;
            
            std::unordered_map<std::string_view, VertexId> stop_to_vertex_id;
            std::unordered_map<VertexId, std::string_view> vertex_id_to_stop;

            std::optional<DirectedWeightedGraph<double>> graph_;
            std::optional<Router<double>> router_;
            std::unordered_map<std::string_view, BusRoute*> buses_;
            std::unordered_map<std::string_view, Stop*> stops_;
        };

    } // namespace catalogue
} // namespace transport
