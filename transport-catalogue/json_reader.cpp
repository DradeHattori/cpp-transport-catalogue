// json_reader.cpp

#include "json_reader.h"
#include "json.h"
#include "map_renderer.h"

#include <algorithm>
#include <unordered_map>
#include <sstream>


namespace transport {
    namespace catalogue {

        std::string error_message = "not found";

        void JsonReader::LoadData(const json::Document& doc) {
            const auto& base_requests = doc.GetRoot().AsMap().at("base_requests").AsArray();
            for (const auto& request : base_requests) {
                const auto& request_map = request.AsMap();
                if (request_map.at("type").AsString() == "Stop") {
                    std::string stop_name = request_map.at("name").AsString();
                    double lat = request_map.at("latitude").AsDouble();
                    double lng = request_map.at("longitude").AsDouble();
                    std::unordered_map<std::string, int> distances;
                    for (const auto& [name, distance] : request_map.at("road_distances").AsMap()) {
                        distances[name] = distance.AsInt();
                    }
                    catalogue_.AddStop(std::move(stop_name), { lat, lng }, distances);
                }
                else if (request_map.at("type").AsString() == "Bus") {
                    std::string bus_name = request_map.at("name").AsString();
                    bool is_circular = request_map.at("is_roundtrip").AsBool();

                    std::vector<std::string> stops;
                    for (const auto& stop_name : request_map.at("stops").AsArray()) {
                        stops.push_back(stop_name.AsString());
                    }
                    catalogue_.AddBus(std::move(bus_name), std::move(stops), is_circular);
                }
            }
        }

        void JsonReader::ProcessRequests(const json::Document& doc, std::ostream& output) {
            const auto& stat_requests = doc.GetRoot().AsMap().at("stat_requests").AsArray();
            json::Array responses;
            for (const auto& request : stat_requests) {
                const auto& request_map = request.AsMap();
                int request_id = request_map.at("id").AsInt();
                const std::string_view type = request_map.at("type").AsString();
                if (type == "Bus") {
                    const std::string_view bus_name = request_map.at("name").AsString();
                    std::optional<BusInfo> bus = catalogue_.GetBusInfo(bus_name);
                    if (bus.has_value()) {
                        double route_length = bus->route_length;
                        double curvature = bus->curvature;
                        int stop_count = bus->stop_count;
                        int unique_stop_count = bus->unique_stop_count;
                        responses.push_back(json::Node(json::Dict{
                            {"request_id", request_id},
                            {"curvature", curvature},
                            {"route_length", route_length},
                            {"stop_count", stop_count},
                            {"unique_stop_count", unique_stop_count}
                            }));
                    }
                    else {
                        responses.push_back(json::Node(json::Dict{
                            {"request_id", request_id},
                            {"error_message", error_message}
                            }));
                    }
                }
                else if (type == "Stop") {
                    const std::string& stop_name = request_map.at("name").AsString();
                    const Stop* stop = catalogue_.FindStop(stop_name);
                    if (stop) {
                        auto buses = catalogue_.GetBusesForStop(stop_name);
                        if (buses && !buses->empty()) {
                            std::vector<std::string> sorted_buses(buses->begin(), buses->end());
                            std::sort(sorted_buses.begin(), sorted_buses.end());

                            json::Array buses_array;
                            buses_array.reserve(sorted_buses.size());
                            for (const auto& bus : sorted_buses) {
                                buses_array.push_back(json::Node(bus));
                            }

                            responses.push_back(json::Node(json::Dict{
                                {"request_id", request_id},
                                {"buses", std::move(buses_array)}
                                }));
                        }
                        else {
                            responses.push_back(json::Node(json::Dict{
                                {"request_id", request_id},
                                {"buses", json::Array{}}
                                }));
                        }
                    }
                    else {
                        responses.push_back(json::Node(json::Dict{
                            {"request_id", request_id},
                            {"error_message", error_message}
                            }));
                    }
                }
                else if (type == "Map") {
                    auto render_settings = GetRenderSettings(doc);
                    MapRenderer map_renderer(render_settings);

                    std::ostringstream map_output;
                    map_renderer.RenderMap(catalogue_, map_output);

                    responses.push_back(json::Node(json::Dict{
                        {"request_id", request_id},
                        {"map", map_output.str()}
                        }));
                }
            }
            json::Print(json::Document(json::Node(std::move(responses))), output);
        }

    } // namespace catalogue
}  // namespace transport
