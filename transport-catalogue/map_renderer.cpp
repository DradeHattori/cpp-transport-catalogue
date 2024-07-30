//map_renderer.cpp
#include "map_renderer.h"


namespace transport {
    namespace catalogue {

        inline const double EPSILON = 1e-6;
        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }

       
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding)
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }


        MapRenderer::MapRenderer(RenderSettings settings) : settings_(std::move(settings)) {}

        void MapRenderer::DrawRouteLines(svg::Document& doc, const std::map<std::string_view, BusRoute*>& buses, const std::map<std::string_view, Stop*>& stops, const SphereProjector& projector, size_t& color_index) const {
            for (const auto& [_, bus] : buses) {
                svg::Polyline polyline;
                polyline.SetStrokeColor(settings_.color_palette[color_index])
                    .SetFillColor(svg::NoneColor)
                    .SetStrokeWidth(settings_.line_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                const auto bus_stops = bus->stops;

                for (const auto& stop_name : bus_stops) {
                    const auto& stop = stops.find(stop_name);
                    polyline.AddPoint(projector(stop->second->coordinates));
                }

                if (!bus->is_circular) {
                    std::vector<std::string> bus_stops_reversed = { bus_stops.rbegin() + 1, bus_stops.rend() };
                    for (const auto& stop_name : bus_stops_reversed) {
                        const auto& stop = stops.find(stop_name);
                        polyline.AddPoint(projector(stop->second->coordinates));
                    }
                }
                doc.Add(std::move(polyline));
                color_index = (color_index + 1) % settings_.color_palette.size();
            }
        }

        void MapRenderer::DrawRouteNames(svg::Document& doc, const std::map<std::string_view, BusRoute*>& buses, const std::map<std::string_view, Stop*>& stops, const SphereProjector& projector, size_t& color_index) const {

            for (const auto& [_, bus] : buses) {
                const auto& bus_stops = bus->stops;
                const auto& start_stop = stops.find(bus_stops.front());
                const auto& end_stop = stops.find(bus_stops.back());

                svg::Text text;
                text.SetFillColor(settings_.color_palette.at(color_index))
                    .SetPosition(projector(start_stop->second->coordinates))
                    .SetOffset({ settings_.bus_label_offset.first, settings_.bus_label_offset.second })
                    .SetFontSize(settings_.bus_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetData(std::string(bus->name));

                svg::Text underlayer = text;
                underlayer.SetFillColor(settings_.underlayer_color)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                doc.Add(underlayer);
                doc.Add(text);

                if (!bus->is_circular && bus_stops.front() != bus_stops.back()) {
                    svg::Text end_text = text;
                    end_text.SetPosition(projector(end_stop->second->coordinates));
                    svg::Text end_underlayer = end_text;
                    end_underlayer.SetFillColor(settings_.underlayer_color)
                        .SetStrokeColor(settings_.underlayer_color)
                        .SetStrokeWidth(settings_.underlayer_width)
                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                    doc.Add(end_underlayer);
                    doc.Add(end_text);
                }
                color_index = (color_index + 1) % settings_.color_palette.size();
            }
        }

        void MapRenderer::DrawStops(svg::Document& doc, const std::set<std::string_view>& stops_set, const std::map<std::string_view, Stop*>& stops, const SphereProjector& projector) const {
            // нарисовать кружочки
            for (const auto& stop : stops_set) {
                const auto stop_point = stops.find(stop);
                svg::Circle circle;
                circle.SetCenter(projector(stop_point->second->coordinates))
                    .SetRadius(settings_.stop_radius)
                    .SetFillColor("white");
                doc.Add(circle);
            }
            // вывести названия остановок
            for (const auto& stop : stops_set) {
                const auto stop_point = stops.find(stop);
                svg::Text stop_name;
                stop_name.SetPosition(projector(stop_point->second->coordinates))
                    .SetOffset({ settings_.stop_label_offset.first, settings_.stop_label_offset.second })
                    .SetFontSize(settings_.stop_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetData(std::string(stop_point->second->name))
                    .SetFillColor("black");

                svg::Text underlayer = stop_name;
                underlayer.SetFillColor(settings_.underlayer_color)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                doc.Add(underlayer);
                doc.Add(stop_name);
            }

        }

        void MapRenderer::RenderMap(TransportCatalogue& transport_catalogue,
            std::ostream& output) const {

            const auto allbuses = transport_catalogue.GetAllBuses();
            const std::map<std::string_view, BusRoute*>& buses = { allbuses.begin(), allbuses.end() };

            const auto allstops = transport_catalogue.GetAllStops();
            const std::map<std::string_view, Stop*> stops = { allstops.begin(), allstops.end() };

            svg::Document doc;

            std::vector<geo::Coordinates> coords;

            std::set<std::string_view> stops_set;


            // задать масштаб карты и добавить в set все используемые остановки
            for (const auto& bus : buses) {
                for (const auto& stop_name : bus.second->stops) {
                    coords.push_back(transport_catalogue.FindStop(stop_name)->coordinates);
                    stops_set.insert(transport_catalogue.FindStop(stop_name)->name);
                }
            }

            SphereProjector projector(coords.begin(), coords.end(), settings_.width, settings_.height, settings_.padding);

            size_t color_index = 0;

            DrawRouteLines(doc, buses, stops, projector, color_index); color_index = 0;
            DrawRouteNames(doc, buses, stops, projector, color_index);
            DrawStops(doc, stops_set, stops, projector);
            

            doc.Render(output);
        }


    } // namespace catalogue
} // namespace transport
