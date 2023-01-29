#include "map_renderer.h"

svg::Color MakeColorFromJsonNode(const json::Node& color_node){
    if(color_node.IsString()){
        return color_node.AsString();
    }
    if(color_node.IsArray()){
        if(color_node.AsArray().size() == 3){
            return svg::Rgb{
                    color_node.AsArray()[0].AsInt(),
                    color_node.AsArray()[1].AsInt(),
                    color_node.AsArray()[2].AsInt()};
        }
        if(color_node.AsArray().size() == 4){
            return svg::Rgba{
                    color_node.AsArray()[0].AsInt(),
                    color_node.AsArray()[1].AsInt(),
                    color_node.AsArray()[2].AsInt(),
                    color_node.AsArray()[3].AsDouble()};
        }
    }
    return svg::Color{};
}

PolylineSettings::PolylineSettings(const json::Dict& settings){
    
    std::vector<svg::Color> color_palette;
    for(const json::Node& color_node : settings.at("color_palette").AsArray()){
        color_palette.emplace_back(MakeColorFromJsonNode(color_node));
    }

    width_ = settings.at("width").AsDouble();
    height_ = settings.at("height").AsDouble();
    padding_ = settings.at("padding").AsDouble();
    line_width_ = settings.at("line_width").AsDouble();
    color_palette_ = move(color_palette);
}

void MapRenderer::DrawBusRoutePolyline(svg::Document& doc,
                          const std::vector<std::pair<std::string, svg::Point>>& bus_stops_to_coords,
                          bool is_roundtrip,
                          int color_number){

    svg::Polyline polyline;
    for(const auto& stop_coords : bus_stops_to_coords){
        polyline.AddPoint(stop_coords.second);
    }
    if(!is_roundtrip){
        for(auto r_it = bus_stops_to_coords.rbegin()+1; r_it != bus_stops_to_coords.rend(); ++r_it){
            polyline.AddPoint((*r_it).second);
        }
    }

    doc.Add(polyline.SetStrokeWidth(route_polyline_settings_.line_width_).SetFillColor("none").SetStrokeColor(route_polyline_settings_.color_palette_[color_number])
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
}


void MapRenderer::DrawBusname(svg::Document& doc,
                svg::Point busname_pos,
                 const Bus& bus,
                 int color_number,
                 std::vector<svg::Color>& color_palette){

    svg::Text routetitle_underlayer_text;
    svg::Point buslabel_offset{render_settings_.at("bus_label_offset").AsArray()[0].AsDouble(),render_settings_.at("bus_label_offset").AsArray()[1].AsDouble()};
    routetitle_underlayer_text.SetPosition(busname_pos).SetOffset(buslabel_offset).SetFontFamily(busname_settings_.font_family_).SetFontWeight(busname_settings_.font_weight_)
            .SetFontSize(busname_settings_.font_size_).SetData(bus.busname);

    doc.Add(routetitle_underlayer_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeWidth(busname_settings_.stroke_width_).SetFillColor(busname_settings_.fill_).SetStrokeColor(busname_settings_.stroke_));

    svg::Text bustitle_text;
    bustitle_text.SetPosition(busname_pos).SetOffset(buslabel_offset).SetFontFamily(busname_settings_.font_family_).SetFontWeight(busname_settings_.font_weight_)
            .SetFontSize(busname_settings_.font_size_).SetData(bus.busname);

    doc.Add(bustitle_text.SetFillColor(color_palette[color_number]));
}

void MapRenderer::DrawStopCircle(svg::Document& doc, const std::map<std::string, svg::Point>& stops_to_coords){
    for(const auto& p : stops_to_coords){
        svg::Circle circle;
        circle.SetCenter(p.second).SetFillColor("white").SetRadius(render_settings_.at("stop_radius").AsDouble());
        doc.Add(circle);
    }
}

void MapRenderer::DrawStopname(svg::Document& doc,
                       const std::map<std::string, svg::Point>& stops_to_coords){

    for(const auto& p : stops_to_coords){
        svg::Text stopname_underlayer_text;
        svg::Point stoplabel_offset{render_settings_.at("stop_label_offset").AsArray()[0].AsDouble(),render_settings_.at("stop_label_offset").AsArray()[1].AsDouble()};

        stopname_underlayer_text.SetPosition(p.second).SetOffset(stoplabel_offset).SetFontFamily(stopname_settings_.font_family_).SetFontSize(stopname_settings_.font_size_).SetData(p.first);

        doc.Add(stopname_underlayer_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeWidth(stopname_settings_.stroke_width_).SetFillColor(stopname_settings_.fill_).SetStrokeColor(stopname_settings_.stroke_));

        svg::Text stoptitle_text;
        stoptitle_text.SetPosition(p.second).SetOffset(stoplabel_offset).SetFontFamily(stopname_settings_.font_family_).SetFontSize(stopname_settings_.font_size_).SetData(p.first);
        doc.Add(stoptitle_text.SetFillColor("black"));
    }
}

svg::Document MapRenderer::DrawMap(const std::set<Bus>& buses){

    std::map<std::string, svg::Point> stops_to_coords;
    std::map<Bus, std::vector<std::pair<std::string, svg::Point>>> screen_crds_to_buses;

    screen_crds_to_buses = ProjectSphericalCoordsOnScreen(buses, stops_to_coords);

    svg::Document doc;

    int color_num_at_palette = 0;
    for (const auto &p: screen_crds_to_buses) {

        DrawBusRoutePolyline(doc, p.second, p.first.is_roundtrip, color_num_at_palette);

        if (screen_crds_to_buses.size() > route_polyline_settings_.color_palette_.size()) {
            route_polyline_settings_.color_palette_.push_back(route_polyline_settings_.color_palette_[color_num_at_palette]);
        }
        ++color_num_at_palette;
    }

    color_num_at_palette = 0;

    for (const auto &p: screen_crds_to_buses) {

        //BusnameUnderlayerSettings bnus(p.second[0].second, "bus", settings);
        DrawBusname(doc, p.second[0].second, p.first, color_num_at_palette, route_polyline_settings_.color_palette_);

        if ((!p.first.is_roundtrip && (p.second[0].first != p.second.back().first))) {
            //BusnameUnderlayerSettings additional_bnus(p.second.back().second, "bus", settings);
            DrawBusname(doc, p.second.back().second, p.first, color_num_at_palette, route_polyline_settings_.color_palette_);
        }
        ++color_num_at_palette;
    }

    DrawStopCircle(doc, stops_to_coords);
    DrawStopname(doc,stops_to_coords);

    return doc;
}

std::map<Bus, std::vector<std::pair<std::string, svg::Point>>> MapRenderer::ProjectSphericalCoordsOnScreen(
        std::set<Bus> buses,
        std::map<std::string, svg::Point>& stops_to_coords){

   std::vector<Coordinates> coordinates = GetStopsOnRouteCoordinates(buses);

    const SphereProjector proj{
            coordinates.begin(), coordinates.end(), route_polyline_settings_.width_, route_polyline_settings_.height_, route_polyline_settings_.padding_
    };

    std::map<Bus, std::vector<std::pair<std::string, svg::Point>>> screen_crds_to_buses;
    for (const Bus& bus : buses) {
        std::vector<std::pair<std::string, svg::Point>> coord_vector;
        for (const auto &stop : bus.stops) {
            svg::Point transformed_coords = proj(stop->coordinates);
            coord_vector.push_back({stop->stopname, transformed_coords});
            stops_to_coords[stop->stopname] = transformed_coords;
        }
        screen_crds_to_buses[bus] = coord_vector;
    }
    return screen_crds_to_buses;
}

MapRenderer::MapRenderer(const json::Dict& rs,
                        PolylineSettings& ps, 
                        StopnameUnderlayerSettings& ss, 
                        BusnameUnderlayerSettings& bs): 

                        render_settings_(rs), 
                        route_polyline_settings_(ps), 
                        stopname_settings_(ss),
                        busname_settings_(bs){}
