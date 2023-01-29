#pragma once

#include "geo.h"
#include "json.h"
#include "domain.h"

#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>


namespace transport_catalogue{
class TransportCatalogue{
public:

    class StopHasher{
    public:
        size_t operator()(const std::string_view stopname) const {
            return hash_sv(stopname);
        }
    private:
        std::hash<std::string_view> hash_sv;
    };

    class BusHasher{
    public:
        size_t operator()(const std::string_view busname) const {
            return hash_sv(busname);
        }
    private:
        std::hash<std::string_view> hash_sv;
    };

    class PairOfStopsHasher{
    public:
        size_t operator()(const std::pair<const Stop*, const Stop*>& pair_of_stops) const {
            return ptr_hasher(pair_of_stops.first) + 37 * ptr_hasher(pair_of_stops.second);
        }
    private:
        std::hash<const void*> ptr_hasher;
    };

    struct BusInfo{
        std::string busname;
        int stops_on_route;
        int unique_stops;
        double route_length;
        double geo_length;
        bool is_roundtrip;
    };

    const std::deque<Stop>& GetAllStops() const{
        return stops_;
    }

    const std::deque<Bus>& GetAllBuses() const{
        return buses_;
    }

    bool IsExistingStop(std::string_view stopname) const;
    void AddStop(std::string_view stopname, const Coordinates coordinates);
    const Stop* FindStop(std::string_view stopname) const;
    void AddBusRouteLength(std::string_view busname, const std::vector<const Stop*>& stops_to_bus);
    void AddBus(std::string_view busname, const std::vector<json::Node>& stops, bool is_roundtrip);
    const Bus* FindBus(std::string_view busname) const;
    const BusInfo GetBusInfo(std::string_view busname) const;
    const std::set<std::string_view> GetStopInfo(std::string_view stopname) const;
    void AddDistances(std::string_view stopname, const json::Dict& distances);
    int GetDistance(std::string_view stopname1, std::string_view stopname2) const;

private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, const Stop*, StopHasher> stopname_to_stop_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Bus*, BusHasher> busname_to_bus_;
    std::unordered_map<std::string_view, std::set<std::string_view>, StopHasher> buses_to_stop_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, PairOfStopsHasher> distances_;
    std::unordered_map<std::string_view, std::pair<double, double>, BusHasher> busname_to_routelength_;
};
}