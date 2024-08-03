#include "json_reader.h"
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"

#include <algorithm>
#include <unordered_map>
#include <sstream>

namespace transport {
    namespace catalogue {

        void JsonReader::LoadData(const json::Document& doc) {
            const auto& base_requests = doc.GetRoot().AsDict().at("base_requests").AsArray();
            for (const auto& request : base_requests) {
                const auto& request_map = request.AsDict();
                const auto& Request = request_map.at("type").AsString();
                if (Request == "Stop") {
                    std::string stop_name = request_map.at("name").AsString();
                    double lat = request_map.at("latitude").AsDouble();
                    double lng = request_map.at("longitude").AsDouble();
                    std::unordered_map<std::string, int> distances;
                    for (const auto& [name, distance] : request_map.at("road_distances").AsDict()) {
                        distances[name] = distance.AsInt();
                    }
                    catalogue_.AddStop(std::move(stop_name), { lat, lng }, distances);
                }
                else if (Request == "Bus") {
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
            const auto& stat_requests = doc.GetRoot().AsDict().at("stat_requests").AsArray();
            json::Array responses;
            for (const auto& request : stat_requests) {
                const auto& request_map = request.AsDict();
                int request_id = request_map.at("id").AsInt();
                const std::string_view type = request_map.at("type").AsString();
                if (type == "Bus") {
                    ProcessBusRequest(request_map, request_id, responses);
                }
                else if (type == "Stop") {
                    ProcessStopRequest(request_map, request_id, responses);
                }
                else if (type == "Map") {
                    ProcessMapRequest(request_id, doc, responses);
                }
            }
            json::Print(json::Document(json::Node(std::move(responses))), output);
        }

        void JsonReader::ProcessBusRequest(const json::Dict& request_map, int request_id, json::Array& responses) {
            const std::string_view bus_name = request_map.at("name").AsString();
            std::optional<BusInfo> bus = catalogue_.GetBusInfo(bus_name);
            json::Builder builder;

            if (bus.has_value()) {
                builder.StartDict()
                    .Key("request_id").Value(request_id)
                    .Key("curvature").Value(bus->curvature)
                    .Key("route_length").Value(bus->route_length)
                    .Key("stop_count").Value(bus->stop_count)
                    .Key("unique_stop_count").Value(bus->unique_stop_count)
                    .EndDict();
            }
            else {
                builder.StartDict()
                    .Key("request_id").Value(request_id)
                    .Key("error_message").Value(error_message)
                    .EndDict();
            }

            responses.push_back(builder.Build());
        }

        void JsonReader::ProcessStopRequest(const json::Dict& request_map, int request_id, json::Array& responses) {
            const std::string& stop_name = request_map.at("name").AsString();
            const Stop* stop = catalogue_.FindStop(stop_name);
            json::Builder builder;

            if (stop) {
                auto buses = catalogue_.GetBusesForStop(stop_name);
                builder.StartDict()
                    .Key("request_id").Value(request_id);

                if (buses && !buses->empty()) {
                    std::vector<std::string> sorted_buses(buses->begin(), buses->end());
                    std::sort(sorted_buses.begin(), sorted_buses.end());

                    auto buses_array = builder.Key("buses").StartArray();
                    for (const auto& bus : sorted_buses) {
                        buses_array.Value(bus);
                    }
                    buses_array.EndArray();
                }
                else {
                    builder.Key("buses").Value(json::Array{});
                }

                builder.EndDict();
            }
            else {
                builder.StartDict()
                    .Key("request_id").Value(request_id)
                    .Key("error_message").Value(error_message)
                    .EndDict();
            }

            responses.push_back(builder.Build());
        }

        void JsonReader::ProcessMapRequest(int request_id, const json::Document& doc, json::Array& responses) {
            auto render_settings = GetRenderSettings(doc);
            MapRenderer map_renderer(render_settings);

            std::ostringstream map_output;
            map_renderer.RenderMap(catalogue_, map_output);

            json::Builder builder;
            builder.StartDict()
                .Key("request_id").Value(request_id)
                .Key("map").Value(map_output.str())
                .EndDict();

            responses.push_back(builder.Build());
        }

        svg::Color ParseColor(const json::Node& color_node) {
            if (color_node.IsArray() && color_node.AsArray().size() == 4) {
                return svg::Color{
                    svg::Rgba{
                        static_cast<uint8_t>(color_node.AsArray().at(0).AsInt()),
                        static_cast<uint8_t>(color_node.AsArray().at(1).AsInt()),
                        static_cast<uint8_t>(color_node.AsArray().at(2).AsInt()),
                        color_node.AsArray().at(3).AsDouble()
                    }
                };
            }
            else if (color_node.IsArray() && color_node.AsArray().size() == 3) {
                return svg::Color{
                    svg::Rgb{
                        static_cast<uint8_t>(color_node.AsArray().at(0).AsInt()),
                        static_cast<uint8_t>(color_node.AsArray().at(1).AsInt()),
                        static_cast<uint8_t>(color_node.AsArray().at(2).AsInt()),
                    }
                };
            }
            else {
                return color_node.AsString();
            }
        }

        RenderSettings GetRenderSettings(const json::Document& doc) {
            const auto& render_settings = doc.GetRoot().AsDict().at("render_settings").AsDict();
            RenderSettings settings;

            settings.width = render_settings.at("width").AsDouble();
            settings.height = render_settings.at("height").AsDouble();
            settings.padding = render_settings.at("padding").AsDouble();
            settings.stop_radius = render_settings.at("stop_radius").AsDouble();
            settings.line_width = render_settings.at("line_width").AsDouble();
            settings.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
            settings.bus_label_offset = {
                render_settings.at("bus_label_offset").AsArray().at(0).AsDouble(),
                render_settings.at("bus_label_offset").AsArray().at(1).AsDouble()
            };

            settings.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
            settings.stop_label_offset = {
                render_settings.at("stop_label_offset").AsArray().at(0).AsDouble(),
                render_settings.at("stop_label_offset").AsArray().at(1).AsDouble()
            };

            settings.underlayer_color = ParseColor(render_settings.at("underlayer_color"));
            settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();

            for (const auto& color_node : render_settings.at("color_palette").AsArray()) {
                settings.color_palette.push_back(ParseColor(color_node));
            }

            return settings;
        }

    } // namespace catalogue
}  // namespace transport
