#pragma once

#include "transport_catalogue.h"
#include "geo.h"

#include <string>
#include <string_view>
#include <vector>

namespace transport {
namespace input {

struct CommandDescription {
    explicit operator bool() const {
        return !command.empty();
    }
    bool operator!() const {
        return !operator bool();
    }

    std::string command;
    std::string id;
    std::string description;
};

class InputReader {
public:
    void ParseLine(std::string_view line);
    void ApplyCommands(transport::catalogue::TransportCatalogue& catalogue) const;

private:
    std::vector<CommandDescription> commands_;
};

namespace detail {
    transport::geo::Coordinates ParseCoordinates(std::string_view str);
    std::string_view Trim(std::string_view string);
    std::vector<std::string_view> Split(std::string_view string, char delim);
    std::vector<std::string_view> ParseRoute(std::string_view route);
    CommandDescription ParseCommandDescription(std::string_view line);
}

} // namespace input
} // namespace transport
