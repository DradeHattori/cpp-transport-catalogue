// map_renderer.h
#pragma once

#include "svg.h"
#include "geo.h"
#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <set>



namespace transport {
    namespace catalogue {

        class SphereProjector {
        public:
            template <typename PointInputIt>
            SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding);

            // Проецирует широту и долготу в координаты внутри SVG-изображения
            svg::Point operator()(geo::Coordinates coords) const {
                return {
                    (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_
                };
            }

        private:
            double padding_;
            double min_lon_ = 0;
            double max_lat_ = 0;
            double zoom_coeff_ = 0;
        };

        class MapRenderer {
        public:
            MapRenderer(RenderSettings settings);

            
           
            void RenderMap(
                TransportCatalogue& transport_catalogue,
                std::ostream& output) const;

        private:
            void DrawRouteLines(svg::Document& doc, const std::map<std::string_view, BusRoute*>& buses, const std::map<std::string_view, Stop*>& stops, const SphereProjector& projector, size_t& color_index) const;
            void DrawRouteNames(svg::Document& doc, const std::map<std::string_view, BusRoute*>& buses, const std::map<std::string_view, Stop*>& stops, const SphereProjector& projector, size_t& color_index) const;
            void DrawStops(svg::Document& doc, const std::set<std::string_view>& stops_set, const std::map<std::string_view, Stop*>& stops, const SphereProjector& projector) const;
             RenderSettings settings_;
        };

    } // namespace catalogue
} // namespace transport
