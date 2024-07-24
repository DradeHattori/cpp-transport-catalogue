//map_renderer.cpp
#include "map_renderer.h"
#include <set>

namespace transport {
    namespace catalogue {

        inline const double EPSILON = 1e-6;
        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }

        class SphereProjector {
        public:
            // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
            template <typename PointInputIt>
            SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                double max_width, double max_height, double padding)
                : padding_(padding) //
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

        RenderSettings GetRenderSettings(const json::Document& doc) {
            const auto& render_settings = doc.GetRoot().AsMap().at("render_settings").AsMap();
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

            auto underlayer_color = render_settings.at("underlayer_color");
            if (underlayer_color.IsArray() && underlayer_color.AsArray().size() == 4) {
                settings.underlayer_color = svg::Color{
                svg::Rgba{
                    static_cast<uint8_t>(render_settings.at("underlayer_color").AsArray().at(0).AsInt()),
                    static_cast<uint8_t>(render_settings.at("underlayer_color").AsArray().at(1).AsInt()),
                    static_cast<uint8_t>(render_settings.at("underlayer_color").AsArray().at(2).AsInt()),
                    render_settings.at("underlayer_color").AsArray().at(3).AsDouble()
                }
                };
            }
            else if (underlayer_color.IsArray() && underlayer_color.AsArray().size() == 3) {
                settings.underlayer_color = svg::Color{
                svg::Rgb{
                    static_cast<uint8_t>(render_settings.at("underlayer_color").AsArray().at(0).AsInt()),
                    static_cast<uint8_t>(render_settings.at("underlayer_color").AsArray().at(1).AsInt()),
                    static_cast<uint8_t>(render_settings.at("underlayer_color").AsArray().at(2).AsInt()),
                }
                };
            }
            else {
                settings.underlayer_color = underlayer_color.AsString();
            }

            settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();

            for (const auto& color : render_settings.at("color_palette").AsArray()) {
                if (color.IsArray() && color.AsArray().size() == 3) {
                    svg::Rgb rgb{
                          static_cast<uint8_t>(color.AsArray().at(0).AsInt()),
                          static_cast<uint8_t>(color.AsArray().at(1).AsInt()),
                          static_cast<uint8_t>(color.AsArray().at(2).AsInt()),
                    };
                    settings.color_palette.emplace_back(rgb);
                }
                else if (color.IsArray() && color.AsArray().size() == 4) {
                    svg::Rgba rgba{
                        static_cast<uint8_t>(color.AsArray().at(0).AsInt()),
                        static_cast<uint8_t>(color.AsArray().at(1).AsInt()),
                        static_cast<uint8_t>(color.AsArray().at(2).AsInt()),
                        color.AsArray().at(3).AsDouble()
                    };
                    settings.color_palette.emplace_back(rgba);
                }
                else {
                    std::string color_str = color.AsString();
                    settings.color_palette.emplace_back(svg::Color{ std::move(color_str) });
                }
            }


            return settings;
        }



        MapRenderer::MapRenderer(RenderSettings settings) : settings_(std::move(settings)) {}


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

            // нарисовать линии маршрутов
            size_t color_index = 0;
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


            // нарисовать названия маршрутов
            color_index = 0;
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

            doc.Render(output);
        }


    } // namespace catalogue
} // namespace transport
