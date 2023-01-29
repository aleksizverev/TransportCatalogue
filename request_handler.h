#pragma once

#include "transport_catalogue.h"

#include <set>

std::set<Bus> GetBusesOnRoute(const transport_catalogue::TransportCatalogue& catalogue);
std::vector<Coordinates> GetStopsOnRouteCoordinates(const std::set<Bus>& buses);
