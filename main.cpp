#include <iostream>

#include "json_reader.h"
#include "transport_catalogue.h"


int main() {
    transport_catalogue::TransportCatalogue catalogue;
    transport_catalogue::input::JSONReader json_reader(catalogue);

    json::Dict input_info = json_reader.ReadInput(std::cin);
    json_reader.CreateDb(input_info);

    json::Dict render_settings = input_info.at("render_settings").AsMap();

    PolylineSettings ps(render_settings);
    StopnameUnderlayerSettings ss("stop", render_settings);
    BusnameUnderlayerSettings bs("bus", render_settings);
    MapRenderer map_renderer(render_settings, ps, ss, bs);

    json_reader.PrintResponse(map_renderer, input_info, std::cout);
}
