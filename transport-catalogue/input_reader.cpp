#include "input_reader.h"


namespace transport {
    namespace input {
        namespace detail {

            geo::Coordinates ParseCoordinates(std::string_view str) {
                auto not_space = str.find_first_not_of(' ');
                auto comma = str.find(',');
                if (comma == str.npos) {
                    return { std::nan(""), std::nan("") };
                }
                auto not_space2 = str.find_first_not_of(' ', comma + 1);
                double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
                double lng = std::stod(std::string(str.substr(not_space2)));
                return { lat, lng };
            }

            std::unordered_map<std::string, int> ParseDistances(std::string_view str) {
                std::unordered_map<std::string, int> distances;

                // Нахождение начала части строки, содержащей расстояния
                auto dist_start = str.find(',') + 1;
                str.remove_prefix(dist_start);

                dist_start = str.find(',') + 1;
                str.remove_prefix(dist_start);

                // stringstream для обработки частей строки
                std::string distances_part(str);
                std::stringstream ss(distances_part);
                std::string segment;

                while (std::getline(ss, segment, ',')) {
                    // Удаление пробелов в начале строки
                    segment.erase(0, segment.find_first_not_of(' '));

                    // Находим позицию "to"
                    auto to_pos = segment.find("to ");
                    if (to_pos != std::string::npos) {
                        // Извлекаем имя остановки
                        std::string stop_name = segment.substr(to_pos + 3);

                        // Извлекаем расстояние
                        auto dist_pos = segment.find("m");
                        if (dist_pos != std::string::npos) {
                            int distance = std::stoi(segment.substr(0, dist_pos));

                            // Удаляем пробелы в конце имени остановки
                            stop_name.erase(stop_name.find_last_not_of(' ') + 1);

                            // Добавляем данные в unordered_map
                            distances[stop_name] = distance;
                        }
                    }
                }

                return distances;
            }


            std::string_view Trim(std::string_view string) {
                const auto start = string.find_first_not_of(' ');
                if (start == string.npos) {
                    return {};
                }
                return string.substr(start, string.find_last_not_of(' ') + 1 - start);
            }

            std::vector<std::string_view> Split(std::string_view string, char delim) {
                std::vector<std::string_view> result;
                size_t pos = 0;
                while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
                    auto delim_pos = string.find(delim, pos);
                    if (delim_pos == string.npos) {
                        delim_pos = string.size();
                    }
                    if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                        result.push_back(substr);
                    }
                    pos = delim_pos + 1;
                }
                return result;
            }

            std::vector<std::string_view> ParseRoute(std::string_view route) {
                if (route.find('>') != route.npos) {
                    return Split(route, '>');
                }
                auto stops = Split(route, '-');
                std::vector<std::string_view> results(stops.begin(), stops.end());
                results.insert(results.end(), std::next(stops.rbegin()), stops.rend());
                return results;
            }

            CommandDescription ParseCommandDescription(std::string_view line) {
                auto colon_pos = line.find(':');
                if (colon_pos == line.npos) {
                    return {};
                }
                auto space_pos = line.find(' ');
                if (space_pos >= colon_pos) {
                    return {};
                }
                auto not_space = line.find_first_not_of(' ', space_pos);
                if (not_space >= colon_pos) {
                    return {};
                }
                return { std::string(line.substr(0, space_pos)),
                         std::string(line.substr(not_space, colon_pos - not_space)),
                         std::string(line.substr(colon_pos + 1)) };
            }

        } // namespace detail

        void InputReader::ParseLine(std::string_view line) {
            auto command_description = detail::ParseCommandDescription(line);
            if (command_description) {
                commands_.push_back(std::move(command_description));
            }
        }

        void InputReader::ApplyCommands(transport::catalogue::TransportCatalogue& catalogue) const {
            for (const auto& command : commands_) {
                if (command.command == "Stop") {
                    auto coordinates = detail::ParseCoordinates(command.description);
                    auto distances = detail::ParseDistances(command.description);
                    catalogue.AddStop(command.id, coordinates, distances);


                }
                else if (command.command == "Bus") {
                    auto stops = detail::ParseRoute(command.description);
                    std::vector<std::string> stop_names(stops.begin(), stops.end());
                    bool is_circular = command.description.find('>') != command.description.npos;
                    catalogue.AddBus(command.id, stop_names, is_circular);
                }
            }
        }

    } // namespace input
} // namespace transport
