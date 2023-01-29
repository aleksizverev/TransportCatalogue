#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <variant>

namespace svg {

    struct Rgb{
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b);
        Rgb(int r, int g, int b);
        uint8_t red = 0u, green = 0u, blue = 0u;
    };

    struct Rgba{
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b, double op);
        Rgba(int r, int g, int b, double op);
        uint8_t red = 0u, green = 0u, blue = 0u;
        double opacity= 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{"none"};

    struct ColorPrinter {
        std::ostream& out;

        void operator()(std::monostate) const;
        void operator()(std::string color) const;
        void operator()(Rgb color) const;
        void operator()(Rgba color) const;
    };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    struct Point {
        Point() = default;
        Point(double x, double y)
                : x(x)
                , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct RenderContext {
        RenderContext(std::ostream& out)
                : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
                : out(out)
                , indent_step(indent_step)
                , indent(indent) {
        }

        RenderContext Indented() const {
            return {out, indent_step, indent + indent_step};
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class ObjectContainer{
    public:
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

        template <typename Obj>
        void Add(Obj obj) {
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }
        virtual ~ObjectContainer() = default;
    };



    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join);
    std::ostream& operator<<(std::ostream& stream, const StrokeLineCap& line_cap);
    std::ostream& operator<<(std::ostream& out, const svg::Color& color);

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width){
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap){
            line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join){
            line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if(stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if(line_cap_){
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if(line_join_){
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& obj_container) const = 0;
        virtual ~Drawable() = default;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    class Polyline : public Object, public PathProps<Polyline> {
    public:
        Polyline& AddPoint(Point point);
    private:
        void RenderObject(const RenderContext& context) const override;
        std::vector<Point> peaks_;
    };

    class Text : public Object, public PathProps<Text>{
    public:
        Text& SetPosition(Point pos);
        Text& SetOffset(Point offset);
        Text& SetFontSize(uint32_t size);
        Text& SetFontFamily(std::string font_family);
        Text& SetFontWeight(std::string font_weight);
        Text& SetData(std::string data);
    private:
        std::string EncodeString(const std::string& str_to_encode) const {
            using namespace std;
            std::string encoded_str;
            std::map<char, std::string> special_symbols = {
                    {'"' , "&quot;"s},
                    {'\'', "&apos;"s},
                    {'<', "&lt;"s},
                    {'>', "&gt;"s},
                    {'&', "&amp;"s}
            };

            for(size_t i = 0; i < str_to_encode.size(); ++i){
                if(special_symbols.count(str_to_encode[i])){
                    encoded_str += special_symbols[str_to_encode[i]];
                }else{
                    encoded_str += str_to_encode[i];
                }
            }
            return encoded_str;
        }

        void RenderObject(const RenderContext& context) const override;

        Point pos_, offset_;
        uint32_t size_ = 1;
        std::string font_family_, font_weight_, data_;
    };

    class Document : public ObjectContainer{
    public:
        void AddPtr(std::unique_ptr<Object>&& obj) override;
        void Render(std::ostream& out) const;
    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };
}