#include <iostream>
#include <variant>
#include <vector>
#include <map>
#include <optional>
#include <string>
#include <fstream>
#include <sstream>

namespace json
{
    struct Node;
    using Null = std::monostate; // 使用variant时表示为一个空的状态
    // std::variant<int, std::string, std::monostate> var;
    // // var 初始化为 std::monostate，表示空状态
    // std::cout << "Index of currently active type: " << var.index() << std::endl;  // 输出 2，表示当前活跃的类型是 std::monostate

    using Bool = bool;
    using Int = int64_t;
    using Float = double;
    using String = std::string;
    using Array = std::vector<Node>;
    using Object = std::map<std::string, Node>;
    using Value = std::variant<Null, Bool, Int, Float, String, Array, Object>;

    // 这种设计允许 Node 对象的 value 成员根据需要存储不同类型的数据，可以是基本数据类型（Bool、Int、Float、String）或复杂数据类型（Array、Object），并提供一种统一的访问方式，即使用 operator[] 进行属性或成员的访问。
    struct Node
    {
        Value value;
        // 构造函数
        Node() : value(Null{}) {}
        Node(Value _value) : value(_value) {}

        auto& operator[](const std::string &key)
        {
            // 重载了 operator[] 的成员函数，用于从一个类（或结构体）中获取键为 std::string 类型的成员（或属性）。该代码的实现假设 value 是一个 std::variant，可以包含不同类型的值，其中之一是 Object 类型Object=std::map<std::string,Node>;。
            // std::get_if 函数的作用是检查 value 是否包含 Object 类型的值，并且返回一个指向该值的指针（如果包含），或者返回 nullptr（如果不包含或者 value 当前存储的不是 Object 类型的值）。
            if (auto object = std::get_if<Object>(&value))
            {
                return (*object)[key];
            }
            throw std::runtime_error("not an object");
        }

        auto operator[](size_t index)
        {
            if (auto array = std::get_if<Array>(&value))
            {
                return array->at(index); // vector类型
            }
            throw std::runtime_error("not an array");
        }

        void push(const Node &rhs)
        {
            if (auto array = std::get_if<Array>(&value))
            {
                array->push_back(rhs);
            }
        }
    };

    struct JsonParser
    {
        std::string_view json_str;
        size_t pos = 0;
        void parse_whitespace();

        // std::optional<Value>，表示可能返回一个 Value 类型的值，也可能不返回任何值（即空值）。
        auto parse_null() -> std::optional<Value>;
        auto parse_true() -> std::optional<Value>;
        auto parse_false() -> std::optional<Value>;
        auto parse_number() -> std::optional<Value>;
        auto parse_string() -> std::optional<Value>;
        auto parse_array() -> std::optional<Value>;
        auto parse_object() -> std::optional<Value>;
        auto parse_value() -> std::optional<Value>;
        auto parse() -> std::optional<Node>;
    };

    auto parser(std::string_view json_str) -> std::optional<Node>
    {
        JsonParser p{json_str};
        return p.parse();
    }

    class JsonGenerator
    {
    public:
        static auto generate(const Node &node) -> std::string;
        static auto generate_string(const String &str) -> std::string;
        static auto generate_array(const Array &array) -> std::string;
        static auto generate_object(const Object &object) -> std::string;
    };

    inline auto generate(const Node &node) -> std::string
    {
        return JsonGenerator::generate(node);
    }

    auto operator<<(std::ostream &out, const Node &t) -> std::ostream &
    {
        out << JsonGenerator::generate(t);
        return out;
    }

}
