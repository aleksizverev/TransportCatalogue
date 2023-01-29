#pragma once

#include "geo.h"
#include "svg.h"
#include "json.h"
#include "domain.h"
#include "request_handler.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <set>
#include <deque>

inline const double EPSILON = 1e-6;
inline auto buses_cmp = [](const Bus& l, const Bus& r){return l.busname < r.busname;};

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
            : padding_(padding) //
    {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (std::abs(max_lon - min_lon_) >= 1e-6) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (std::abs(max_lon - min_lon_) >= 1e-6) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

    svg::Point operator()(Coordinates coords) const {
        return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

svg::Color MakeColorFromJsonNode(const json::Node& color_node);

struct PolylineSettings{

    PolylineSettings(const json::Dict& settings);

    double width_, height_, padding_, line_width_;
    std::vector<svg::Color>color_palette_;

};

struct LabelSettings{
    LabelSettings(std::string label_settings_type, const json::Dict& settings){

        std::string label_font_size;
        if(label_settings_type == "bus"){
            label_font_size = "bus_label_font_size";
        }
        if(label_settings_type == "stop"){
            label_font_size = "stop_label_font_size";
        }

        font_size_ = settings.at(label_font_size).AsDouble();
    }

    double font_size_;
    std::string font_family_ = "Verdana";
};

struct BusnameUnderlayerSettings : LabelSettings{

    BusnameUnderlayerSettings(std::string label_settings_type, const json::Dict& settings): LabelSettings(label_settings_type ,settings){
        auto color_settings = settings.at("underlayer_color");
        fill_ = MakeColorFromJsonNode(color_settings);
        stroke_ = MakeColorFromJsonNode(color_settings);
        stroke_width_ = settings.at("underlayer_width").AsDouble();
    }

    svg::Color fill_, stroke_;
    double stroke_width_;
    std::string font_weight_ = "bold";
};

struct StopnameUnderlayerSettings : LabelSettings{

    StopnameUnderlayerSettings(std::string label_settings_type, const json::Dict& settings): LabelSettings(label_settings_type, settings){
        auto color_settings = settings.at("underlayer_color");
        fill_ = MakeColorFromJsonNode(color_settings);
        stroke_ = MakeColorFromJsonNode(color_settings);
        stroke_width_ = settings.at("underlayer_width").AsDouble();
    }

    svg::Color fill_, stroke_;
    double stroke_width_;
};


class MapRenderer{

public:

    MapRenderer(const json::Dict& rs, 
                PolylineSettings& ps, 
                StopnameUnderlayerSettings& ss, 
                BusnameUnderlayerSettings& bs);

    void DrawBusRoutePolyline(svg::Document& doc,
                  const std::vector<std::pair<std::string, svg::Point>>& bus_stops_to_coords,
                  bool is_roundtrip,
                  int color_number);

    void DrawBusname(svg::Document& doc,
                    svg::Point busname_pos,
                    const Bus& bus,
                    int color_number,
                    std::vector<svg::Color>& color_palette);

    void DrawStopCircle(svg::Document& doc,
                        const std::map<std::string, svg::Point>& stops_to_coords);

    void DrawStopname(svg::Document& doc,
                    const std::map<std::string, svg::Point>& stops_to_coords);

    svg::Document DrawMap(const std::set<Bus>& buses);

    std::map<Bus, std::vector<std::pair<std::string, svg::Point>>> ProjectSphericalCoordsOnScreen(
            std::set<Bus> buses,
            std::map<std::string, svg::Point>& stops_to_coords);

private:

    const json::Dict& render_settings_;
    PolylineSettings& route_polyline_settings_;
    StopnameUnderlayerSettings& stopname_settings_;
    BusnameUnderlayerSettings& busname_settings_;
};





