#include "request_handler.h"

std::set<Bus> GetBusesOnRoute(const transport_catalogue::TransportCatalogue& catalogue){
    std::set<Bus> buses;
    for(const Bus& bus : catalogue.GetAllBuses()){
        if(!bus.stops.empty()){
            buses.insert(bus);
        }
    }
    return buses;
}

std::vector<Coordinates> GetStopsOnRouteCoordinates(const std::set<Bus>& buses){
    std::vector<Coordinates> coordinates;
    for (const Bus &bus: buses) {
        for (const auto &stop: bus.stops) {
            coordinates.push_back(stop->coordinates);
        }
    }
    return coordinates;
}