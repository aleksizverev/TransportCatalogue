#include "json_reader.h"

#include <iostream>
#include <sstream>

using namespace std::string_literals;

namespace transport_catalogue{
namespace input{

JSONReader::JSONReader(transport_catalogue::TransportCatalogue& catalogue): catalogue_(catalogue){}

void JSONReader::AddStopToDb(const json::Dict& data){
    if(data.at("type") == "Stop"s){
        catalogue_.AddStop(
                data.at("name").AsString(),
                {data.at("latitude").AsDouble(),data.at("longitude").AsDouble()});
    }
}

void JSONReader::AddBusToDb(const json::Dict& data){
    if(data.at("type") == "Bus"s){
        catalogue_.AddBus(data.at("name").AsString(), data.at("stops").AsArray(), data.at("is_roundtrip").AsBool());
    }
}

void JSONReader::AddDistancesToDb(const json::Dict& data){
    if(data.at("type") == "Stop"s){
        catalogue_.AddDistances(
                data.at("name").AsString(), data.at("road_distances").AsMap());    // передаем словарь в функцию
    }
}

json::Document JSONReader::LoadJSON(const std::string& s) {
    std::istringstream strm(s);
    return json::Load(strm);
}

void JSONReader::PrintBusInfo(const json::Dict& query, json::Array& response){
    using namespace std::string_literals;

    std::string busname = query.at("name").AsString();
    auto bus_info = catalogue_.GetBusInfo(busname);

    json::Dict bus_response;

    if(bus_info.busname.empty()) {
        bus_response["error_message"] = "not found"s;
    }else {
        bus_response["route_length"] = bus_info.route_length;
        bus_response["stop_count"] = bus_info.stops_on_route;
        bus_response["unique_stop_count"] = bus_info.unique_stops;
        bus_response["curvature"] = bus_info.route_length / bus_info.geo_length;
    }
    bus_response["request_id"] = query.at("id").AsInt();
    response.push_back(bus_response);
}

void JSONReader::PrintStopInfo(const json::Dict& query, json::Array& response){
    using namespace std::string_literals;

    std::string stopname = query.at("name").AsString();
    json::Dict stop_response;
    json::Array stops;

    if(!catalogue_.IsExistingStop(stopname)){
        stop_response["error_message"] = "not found"s;
    }else {
        auto stop_info = catalogue_.GetStopInfo(stopname);
        for (const auto &bus: stop_info) {
            stops.push_back(std::string(bus));
        }
        stop_response["buses"] = stops;
    }

    stop_response["request_id"] = query.at("id").AsInt();
    response.push_back(stop_response);
}

void JSONReader::PrintMapInfo(const json::Dict& query, const std::string& map_data, json::Array& response){

    json::Dict map_response;
    map_response["map"] = map_data;
    map_response["request_id"] = query.at("id").AsInt();

    response.push_back(map_response);
}

void JSONReader::CreateDb(json::Dict& requests){
    auto base_requests = requests.at("base_requests").AsArray();

    for(const auto& r : base_requests){
        if(r.AsMap().at("type") == "Stop"s){
            AddStopToDb(r.AsMap());
        }
    }
    for(const auto& r : base_requests){
        if(r.AsMap().at("type") == "Stop"s){
            AddDistancesToDb(r.AsMap());
        }
    }
    for(const auto& r : base_requests){
        if(r.AsMap().at("type") == "Bus"s){
            AddBusToDb(r.AsMap());
        }
    }
}

json::Dict JSONReader::ReadInput(std::istream& input){
    std::string str, queries;
    while(getline(input, str)){
        queries += str;
    }
    auto root = LoadJSON(queries).GetRoot().AsMap();
    return root;
}

void JSONReader::PrintResponse(MapRenderer& map_renderer, json::Dict& requests, std::ostream& output){
    json::Array response_array;
    auto stat_requests = requests.at("stat_requests").AsArray();
    for (const auto &request: stat_requests) {
        if (request.AsMap().at("type") == "Stop"s) {
            PrintStopInfo(request.AsMap(), response_array);
        }
        if (request.AsMap().at("type") == "Bus"s) {
            PrintBusInfo(request.AsMap(), response_array);
        }
        if (request.AsMap().at("type") == "Map"s) {
            svg::Document doc = map_renderer.DrawMap( GetBusesOnRoute(catalogue_));
            std::stringstream ss;
            doc.Render(ss);
            PrintMapInfo(request.AsMap(), ss.str(), response_array);
        }
    }
    json::Node response(response_array);
    json::PrintNode(response, output);
}

}
}
