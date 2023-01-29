#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);
        context.out << std::endl;
    }

    Circle& Circle::SetCenter(Point center)  {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)  {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Polyline &Polyline::AddPoint(Point point) {
        peaks_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const {
        auto& out = context.out;
        out << "<polyline"s;
        RenderAttrs(context.out);
        out << " points=\""sv;
        if(!peaks_.empty()) {
            for (size_t i = 0; i < peaks_.size() - 1; ++i) {
                out << peaks_[i].x << ',' << peaks_[i].y << ' ';
            }
            out << peaks_[peaks_.size() - 1].x << ',' << peaks_[peaks_.size() - 1].y;
        }
        out << "\"/>";
    }

    void Document::AddPtr(std::unique_ptr<Object> &&obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream &out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;
        for(const auto& obj : objects_) {
            obj->Render(out);
        }
        out << "</svg>";
    }

    Text &Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text &Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text &Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text &Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text &Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);;
        return *this;
    }

    Text &Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext &context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_;
        if(!font_family_.empty())
            out << "\" "sv << "font-family=\""sv << font_family_ << "\""sv;
        if(!font_weight_.empty())
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        out << ">"sv << EncodeString(data_) << "</text>"sv;
    }

    std::ostream& operator<<(std::ostream& stream, const StrokeLineCap& line_cap){
        switch (line_cap)
        {
            case StrokeLineCap::BUTT :
                stream << "butt"sv;
                break;

            case StrokeLineCap::ROUND :
                stream << "round"sv;
                break;

            case StrokeLineCap::SQUARE :
                stream << "square"sv;
                break;

            default:
                stream << "butt"sv;
                break;
        }

        return stream;
    }

    std::ostream& operator<<(std::ostream& stream, const StrokeLineJoin& line_join) {
        switch (line_join) {
            case StrokeLineJoin::ARCS :
                stream << "arcs"sv;
                break;

            case StrokeLineJoin::BEVEL :
                stream << "bevel"sv;
                break;

            case StrokeLineJoin::MITER :
                stream << "miter"sv;
                break;

            case StrokeLineJoin::MITER_CLIP :
                stream << "miter-clip"sv;
                break;

            case StrokeLineJoin::ROUND :
                stream << "round"sv;
                break;

            default:
                stream << "miter"sv;
                break;
        }
        return stream;
    }

    void ColorPrinter::operator()(std::monostate) const {
        out << "none"sv;
    }

    void ColorPrinter::operator()(std::string color) const {
        out << color;
    }

    void ColorPrinter::operator()(Rgb color) const {
        out << "rgb("sv << static_cast<uint16_t>(color.red) << ","sv
            << static_cast<uint16_t>(color.green) << ","sv
            << static_cast<uint16_t>(color.blue) << ")"sv;
    }

    void ColorPrinter::operator()(Rgba color) const {
        out << "rgba("sv << static_cast<uint16_t>(color.red) << ","sv
            << static_cast<uint16_t>(color.green) << ","sv
            << static_cast<uint16_t>(color.blue) << ","sv
            << color.opacity <<")"sv;
    }

    std::ostream& operator<<(std::ostream& out, const svg::Color& color) {
        std::visit(svg::ColorPrinter{out}, color);
        return out;
    }

    Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b): red(r), green(g), blue(b){}
    Rgb::Rgb(int r, int g, int b) {
        red = static_cast<uint8_t>(r);
        green = static_cast<uint8_t>(g);
        blue = static_cast<uint8_t>(b);
    }

    Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double op): red(r), green(g), blue(b), opacity(op) {}
    Rgba::Rgba(int r, int g, int b, double op) {
        red = static_cast<uint8_t>(r);
        green = static_cast<uint8_t>(g);
        blue = static_cast<uint8_t>(b);
        opacity = op;
    }
}