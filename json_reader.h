#pragma once

#include <string>
#include <vector>

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"


namespace transport_catalogue{
namespace input{

class JSONReader{

public:

    JSONReader(transport_catalogue::TransportCatalogue& catalogue);

    void AddStopToDb(const json::Dict& data);
    void AddBusToDb(const json::Dict& data);
    void AddDistancesToDb(const json::Dict& data);
    void PrintBusInfo(const json::Dict& query, json::Array& response);
    void PrintStopInfo(const json::Dict& query, json::Array& response);
    json::Document LoadJSON(const std::string &s);
    void CreateDb(json::Dict& requests);
    json::Dict ReadInput(std::istream& input);
    void PrintResponse(MapRenderer& map_renderer, json::Dict& root, std::ostream& output);
    void PrintMapInfo(const json::Dict& query, const std::string& map_data, json::Array& response);

private:
    transport_catalogue::TransportCatalogue& catalogue_;
};

}
}