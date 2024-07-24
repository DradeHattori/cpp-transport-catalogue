// map_renderer.h
#pragma once

#include "svg.h"
#include "geo.h"
#include "json.h"
#include "transport_catalogue.h"

#include <vector>
#include <map>
#include <string>
#include <algorithm>

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

        RenderSettings GetRenderSettings(const json::Document& doc);

        class MapRenderer {
        public:
            MapRenderer(RenderSettings settings);

            void RenderMap(
                TransportCatalogue& transport_catalogue,
                std::ostream& output) const;

        private:
            RenderSettings settings_;
        };

    } // namespace catalogue
} // namespace transport
