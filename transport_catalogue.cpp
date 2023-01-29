#include "transport_catalogue.h"

#include <iostream>
#include <unordered_set>

namespace transport_catalogue{

void TransportCatalogue::AddStop(std::string_view stopname, const Coordinates coordinates) {
    using namespace std::string_literals;
    Stop stop{std::string(stopname), coordinates};
    stops_.push_back(stop);
    stopname_to_stop_[stops_.back().stopname] = &stops_.back();
    buses_to_stop_[stops_.back().stopname];
}

const Stop* TransportCatalogue::FindStop(std::string_view stopname) const {
    if(!stopname_to_stop_.count(stopname)){
        return {};
    }
    return stopname_to_stop_.at(stopname);
}

void TransportCatalogue::AddBus(std::string_view busname, const std::vector<json::Node>& stops, bool is_roundtrip) {
    Bus bus{std::string(busname), {}, is_roundtrip};
    bus.is_roundtrip = is_roundtrip;
    bus.stops.reserve(stops.size());

    buses_.push_back(bus);
    auto added_bus = &buses_.back();
    busname_to_bus_[buses_.back().busname] = added_bus;

    if(!stops.empty()) {

        for (const json::Node &stop: stops) {
            auto stop_to_add = this->FindStop(stop.AsString());
            (*added_bus).stops.push_back(stop_to_add);
            buses_to_stop_[stop_to_add->stopname].insert(added_bus->busname);
        }
        AddBusRouteLength(buses_.back().busname, buses_.back().stops);
    }
}

const Bus *TransportCatalogue::FindBus(const std::string_view busname) const {
    if(!busname_to_bus_.count(busname)){
        return {};
    }
    return busname_to_bus_.at(busname);
}

const TransportCatalogue::BusInfo TransportCatalogue::GetBusInfo(const std::string_view busname) const{
    using namespace std::string_literals;
    if(!busname_to_bus_.count(busname)){
        return {};
    }

    BusInfo bus_info{};
    bus_info.busname = std::string(busname);

    const Bus* bus = FindBus(busname);
    auto stops_to_bus = bus->stops;
    bool is_roundtrip = bus->is_roundtrip;

    bus_info.stops_on_route = is_roundtrip ? static_cast<int>(stops_to_bus.size())
                                            : static_cast<int>(stops_to_bus.size() * 2 - 1);

    std::unordered_set<const Stop*> unique_stops;
    unique_stops.insert(stops_to_bus.begin(), stops_to_bus.end());
    bus_info.unique_stops = static_cast<int>(unique_stops.size());
    bus_info.route_length = busname_to_routelength_.at(busname).first;
    bus_info.geo_length = busname_to_routelength_.at(busname).second;
    bus_info.is_roundtrip = is_roundtrip;

    return bus_info;
}

const std::set<std::string_view> TransportCatalogue::GetStopInfo(std::string_view stopname) const {
    return buses_to_stop_.at(stopname);
}

bool TransportCatalogue::IsExistingStop(std::string_view stopname) const{
    return stopname_to_stop_.count(stopname);
}

void TransportCatalogue::AddDistances(std::string_view stopname, const json::Dict& distances) {
    const Stop* stop_from = FindStop(stopname);
    for(const auto& dist_info : distances){
        const Stop* stop_to = FindStop(dist_info.first);
        distances_[{stop_from, stop_to}] =  dist_info.second.AsInt();
        if(!distances_.count({stop_to, stop_from})){
            distances_[{stop_to, stop_from}] =  dist_info.second.AsInt();
        }
    }
}

int TransportCatalogue::GetDistance(std::string_view stopname1, std::string_view stopname2) const{
    const Stop* stop_from = FindStop(stopname1);
    const Stop* stop_to = FindStop(stopname2);
    if(!distances_.count({stop_from, stop_to}))
        return 0;
    return distances_.at({stop_from,stop_to});
}

void TransportCatalogue::AddBusRouteLength(std::string_view busname, const std::vector<const Stop*>& stops_to_bus) {

    for(auto to = stops_to_bus.begin(), from = to++; to != stops_to_bus.end(); ++to, ++from){
        double length = GetDistance((*from)->stopname, (*to)->stopname);
        double geo_length = ComputeDistance((*from)->coordinates, (*to)->coordinates);
        busname_to_routelength_[busname].first += length;
        busname_to_routelength_[busname].second += geo_length;
    }

    if(!FindBus(busname)->is_roundtrip) {
        for (auto to = stops_to_bus.rbegin(), from = to++; to != stops_to_bus.rend(); ++to, ++from) {
            double length = GetDistance((*from)->stopname, (*to)->stopname);
            double geo_length = ComputeDistance((*from)->coordinates, (*to)->coordinates);
            busname_to_routelength_[busname].first += length;
            busname_to_routelength_[busname].second += geo_length;
        }
    }
}
}