#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "json_builder.h"
#include "svg.h"

namespace transport {
    namespace catalogue {

        struct RenderSettings {
            double width = 0.0;
            double height = 0.0;
            double padding = 0.0;
            double stop_radius = 0.0;
            double line_width = 0.0;
            int bus_label_font_size = 0;
            std::pair<double, double> bus_label_offset;
            int stop_label_font_size = 0;
            std::pair<double, double> stop_label_offset;
            svg::Color underlayer_color;
            double underlayer_width = 0;
            std::vector<svg::Color> color_palette = {};
        };

        svg::Color ParseColor(const json::Node& color_node);
        RenderSettings GetRenderSettings(const json::Document& doc);

        class JsonReader {
        public:
            JsonReader(TransportCatalogue& tc) : catalogue_(tc) {}
            void LoadData(const json::Document& doc);
            void ProcessRequests(const json::Document& doc, std::ostream& output);

        private:
            void ProcessBusRequest(const json::Dict& request_map, int request_id, json::Array& responses);
            void ProcessStopRequest(const json::Dict& request_map, int request_id, json::Array& responses);
            void ProcessMapRequest(int request_id, const json::Document& doc, json::Array& responses);

            TransportCatalogue& catalogue_;
            const std::string error_message = "not found";
        };

    } // namespace catalogue
}  // namespace transport
