#pragma once

#include "geo.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <tuple>

namespace transport {
namespace catalogue {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct BusRoute {
    std::string name;
    std::vector<std::string> stops;
    bool is_circular;
};

class TransportCatalogue {
public:
    void AddStop(const std::string& name, geo::Coordinates coordinates);
    void AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_circular);
    const Stop* FindStop(const std::string& name) const;
    const BusRoute* FindBus(const std::string& name) const;
    std::tuple<int, int, double> GetBusInfo(const std::string& name) const;
    const std::set<std::string>* GetBusesForStop(const std::string& stop_name) const;

private:
    std::unordered_map<std::string, Stop> stops_;
    std::unordered_map<std::string, BusRoute> buses_;
    std::unordered_map<std::string, std::set<std::string>> stop_to_buses_;
};

} // namespace catalogue
} // namespace transport
