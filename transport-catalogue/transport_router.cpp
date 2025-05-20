// transport_router.cpp

#include "transport_router.h"
#include "log_duration.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <optional>
#include <tuple>
#include <algorithm>
#include <limits>
#include <set>

using namespace graph;

double SPEED_CONVERTION_RATIO = 1000.000 / 60.000;

namespace transport {
    namespace catalogue {

      
        TransportRouter::TransportRouter(const RoutingSettings& settings, const TransportCatalogue& catalogue)
            : settings_(settings), buses_(catalogue.GetAllBuses()), stops_(catalogue.GetAllStops()) {
            LOG_DURATION("Transport Router construction");
            BuildGraph(catalogue);
        }

        DirectedWeightedGraph<double> TransportRouter::BuildGraphFromStops() {
            DirectedWeightedGraph<double> graph(stops_.size());
            VertexId vertex_id = 0;

            for (const auto& [stop_name, stop] : stops_) {
                stop_to_vertex_id[stop_name] = vertex_id;
                vertex_id_to_stop[vertex_id] = stop_name;
                ++vertex_id;
            }
            return graph;
        }

        void TransportRouter::FillGraph(DirectedWeightedGraph<double>& graph, const TransportCatalogue& catalogue) {
            for (const auto& [bus_name, bus] : buses_) {

                const auto& stops_local = bus->stops;
                const size_t stop_count = stops_local.size();

                for (size_t i = 0; i < stop_count; ++i) {
                    VertexId from_vertex = stop_to_vertex_id.at(stops_local[i]);
                    double total_distance = 0;
                    double total_reverse_distance = 0;

                    for (size_t j = i + 1; j < stop_count; ++j) {
                        VertexId to_vertex = stop_to_vertex_id.at(stops_local[j]);

                        auto distance = catalogue.GetDistance(stops_local[j - 1], stops_local[j]);
                        if (distance) {
                            total_distance += distance.value();
                            double travel_time = total_distance / (settings_.bus_velocity * SPEED_CONVERTION_RATIO) + settings_.bus_wait_time;

                            graph.AddEdge({ bus_name, from_vertex, to_vertex, travel_time });
                        }

                        if (!bus->is_circular) {
                            auto reverse_distance = catalogue.GetDistance(stops_local[j], stops_local[j - 1]);
                            if (reverse_distance) {
                                total_reverse_distance += reverse_distance.value();
                                double reverse_travel_time = total_reverse_distance / (settings_.bus_velocity * SPEED_CONVERTION_RATIO) + settings_.bus_wait_time;

                                graph.AddEdge({ bus_name, to_vertex, from_vertex, reverse_travel_time });
                            }

                        }

                    }
                }
            }
            graph_.emplace(graph);
        }

        void TransportRouter::BuildGraph(const TransportCatalogue& catalogue) {
            LOG_DURATION("BuildGraph");
            auto graph = BuildGraphFromStops();
            FillGraph(graph, catalogue);
            router_.emplace(graph_.value());
        }

        std::optional<std::tuple<double, std::vector<RouteItem>>> TransportRouter::GetRoute(
            const std::string_view from, const std::string_view to) const {
            LOG_DURATION("Get Route");

            auto from_stop_it = stop_to_vertex_id.find(from);
            auto to_stop_it = stop_to_vertex_id.find(to);

            if (from_stop_it == stop_to_vertex_id.end() || to_stop_it == stop_to_vertex_id.end() || !graph_.has_value()) {
                return std::nullopt;
            }


            VertexId from_vertex = from_stop_it->second;
            VertexId to_vertex = to_stop_it->second;

            auto route_info = router_.value().BuildRoute(from_vertex, to_vertex);
            if (route_info) {
                double total_time = 0.0;
                std::vector<RouteItem> route_items;

                if (from_stop_it != to_stop_it) {

                    route_items.push_back(RouteItem{
                        .type = "Wait",
                        .name = std::string(vertex_id_to_stop.at(from_vertex)),
                        .time = static_cast<double>(settings_.bus_wait_time)
                        });
                    total_time += settings_.bus_wait_time;

                    std::string current_bus;
                    for (const auto& edge_id : route_info->edges) {


                        const auto& edge = graph_.value().GetEdge(edge_id);

                        double edge_time = edge.weight - settings_.bus_wait_time; 

                        std::string_view bus_name = edge.bus;

                        if ((bus_name != current_bus && !current_bus.empty()) || (buses_.at(bus_name)->is_circular && edge.to == to_vertex && edge.from != from_vertex)) {
                            
                            route_items.push_back(RouteItem{
                                .type = "Wait",
                                .name = std::string(vertex_id_to_stop.at(edge.from)),
                                .time = static_cast<double>(settings_.bus_wait_time)
                                });
                            total_time += settings_.bus_wait_time;
                        }

                        route_items.push_back(RouteItem{
                            .type = "Bus",
                            .name = std::string(bus_name),
                            .span_count = 0,
                            .time = edge_time
                            });
                        total_time += edge_time;

                        current_bus = bus_name;
                    }
                }

                return std::make_tuple(total_time, route_items);
            }
            else {
                return std::nullopt; 
            }
        }

    } // namespace catalogue
} // namespace transport

