/**
 * @author zyl
 * @brief JSON parsing is the process of converting data in JSON (JavaScript Object Notation) format into data structures or objects that can be understood and processed by a program. In software development, JSON parsing usually occurs when JSON data obtained from external data sources (such as network requests, files, databases) needs to be read, processed, or transformed by an application.
 *
 */

// 以下代码是一个JSON库，包括解析和生成JSON的功能。optional处理非侵入式异常处理机制。通过函数
// parser可以解析一个JSON字符串并返回一个Node对象，通过函数generate可以将一个Node对象转换为
// 字符串形式的JSON。代码中使用了_variant和访问变种中的不同类型的值。最后，通过重载<<操作符，
// 可以方便地将Node对象输出为JSON字符串。
#include <iostream>
#include <variant>
#include <vector>
#include <map>
#include <optional>
#include <string>
#include <fstream>
#include <sstream>

namespace json1
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

        auto &operator[](const std::string &key)
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
                return array->at(index); // array类型
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

        void parse_whitespace()
        {
            while (pos < json_str.size() && std::isspace(json_str[pos]))
            {
                // 判断空格
                ++pos;
            }
        }

        // std::optional<Value>，表示可能返回一个 Value 类型的值，也可能不返回任何值（即空值）。
        auto parse_null() -> std::optional<Value>
        {
            if (json_str.substr(pos, 4) == "null")
            {
                pos += 4;
                return Null{};
            }
            return {};
        }

        auto parse_true() -> std::optional<Value>
        {
            if (json_str.substr(pos, 4) == "true")
            {
                pos += 4;
                return true;
            }
            return {};
        }

        auto parse_false() -> std::optional<Value>
        {
            if (json_str.substr(pos, 5) == "false")
            {
                pos += 5;
                return false;
            }
            return {};
        }

        auto parse_number() -> std::optional<Value>
        {
            // 解析 JSON 字符串中的数字（可能为整数或浮点数）
            size_t endpos = pos;
            while (endpos < json_str.size() && (std::isdigit(json_str[endpos]) || json_str[endpos] == 'e' || json_str[endpos] == '.'))
            {
                // 这是一个 while 循环，用于找到数字的结束位置 endpos
                endpos++;
            }
            std::string number = std::string{json_str.substr(pos, endpos - pos)};
            pos = endpos;
            static auto is_Float = [](std::string &number)
            {
                // 定义了一个静态 lambda 表达式 is_Float，用于判断解析出的数字字符串是否是浮点数。
                // lambda 表达式接受一个 std::string& 类型的参数 number，并根据字符串中是否包含小数点 '.' 或指数符号 'e' 来判断是否为浮点数。
                return number.find('.') != number.npos || number.find('e') != number.npos; // std::string::npos用于表示字符串中的无效位置或失败的查找结果。
            };
            if (is_Float(number))
            {
                // 使用 std::stod 函数将字符串 number 转换为 double 类型（即 Float 类型）。std::stod 是 C++ 标准库中用于将字符串转换为浮点数的函数。
                // 如果 number 无法解析为有效的浮点数格式，std::stod 会抛出 std::invalid_argument 异常或 std::out_of_range 异常，因此这里使用了 try...catch 块捕获任何可能的异常。
                try
                {
                    Float ret = std::stod(number);
                    return ret;
                }
                catch (...)
                {
                    // catch (...) 是 C++ 中异常处理的一种特殊语法，用于捕获任何类型的异常。它是异常处理中的通配符，可以捕获任何可能抛出的异常，无论其类型是什么。
                    return {};
                }
            }
            else
            {
                try
                {
                    Int ret = std::stoi(number);
                    return ret;
                }
                catch (...)
                {
                    return {};
                }
            }
        }

        auto parse_string() -> std::optional<Value>
        {
            pos++; //"
            size_t endpos = pos;
            while (pos < json_str.size() && json_str[endpos] != '"')
            {
                endpos++;
            }
            std::string str = std::string{json_str.substr(pos, endpos - pos)};
            pos = endpos + 1;
            return str;
        }

        auto parse_array() -> std::optional<Value>
        {
            pos++; //[
            Array arr;
            while (pos < json_str.size() && json_str[pos] != ']')
            {
                auto value = parse_value(); // 就是这里递归的，多层嵌套数组
                arr.push_back(value.value());
                parse_whitespace();
                if (pos < json_str.size() && json_str[pos] == ',')
                {
                    pos++; //,
                }
                parse_whitespace();
            }
            pos++; //]
            return arr;
        }

        auto parse_object() -> std::optional<Value>
        {
            pos++; //{ ：将 pos 向前移动一位，跳过当前位置的 { 字符，因为 JSON 对象的开始应该是 {。
            Object obj;
            while (pos < json_str.size() && json_str[pos] != '}')
            {
                auto key = parse_value(); // 解析键值 就是这里递归的
                parse_whitespace();       // 跳过可能的空格

                if (!std::holds_alternative<String>(key.value()))
                {
                    // std::holds_alternative 是 C++ <variant> 头文件中提供的函数模板，用于检查 std::variant 是否包含特定类型的值。它是 std::variant 类型的成员函数，也可以作为全局函数使用。
                    // 检查解析出的键是否是 String 类型。如果不是 String 类型，则返回空的 std::optional<Value>，表示解析失败。
                    return {};
                }

                if (pos < json_str.size() && json_str[pos] == ':')
                {
                    pos++; //,
                }

                parse_whitespace();
                auto val = parse_value(); // 解析valuem，就是这里递归下降的
                obj[std::get<String>(key.value())] = val.value();
                parse_whitespace();
                if (pos < json_str.size() && json_str[pos] == ',')
                {
                    pos++;
                }
                parse_whitespace();
            }
            pos++; //}
            return obj;
        }

        auto parse_value() -> std::optional<Value>
        {
            // parse_value() 函数根据当前 pos 所指向的字符，选择调用上述各个解析函数，以解析并返回对应的 JSON 值。
            parse_whitespace();
            switch (json_str[pos])
            {
            case 'n':
                return parse_null();
            case 't':
                return parse_true();
            case 'f':
                return parse_false();
            case '"':
                return parse_string();
            case '[':
                return parse_array();
            case '{':
                return parse_object();

            default:
                return parse_number();
            }
        }

        auto parse() -> std::optional<Node>
        {
            // parse() 函数是解析器的入口函数，调用 parse_value() 来解析 JSON 字符串的根值，并将解析结果封装在 std::optional<Node> 中返回。如果解析成功，会创建一个 Node 对象来包装解析出的值。
            parse_whitespace();
            auto value = parse_value();
            if (!value)
            {
                return {};
            }
            // 如果解析成功得到了一个有效的值 value，则创建一个 Node 对象，将 *value（value 的值）作为参数传递给 Node 对象的构造函数
            return Node{*value};
            // 为什么是花括号？
            //  在 C++ 中，对于类类型的构造函数调用，使用花括号 {} 和圆括号 () 都可以用来创建对象或调用构造函数，但它们有一些细微的区别。
            //  使用花括号 {} 进行构造：
            //  使用花括号进行构造叫做列表初始化（list initialization），它是 C++11 引入的一种初始化方式，更加严格和类型安全。
            //  当使用花括号 {} 进行构造时，编译器会执行更多的类型检查，确保不发生窄化（narrowing）或其他意外的类型转换。
            //  使用圆括号 () 进行构造：
            //  使用圆括号进行构造是传统的直接初始化方式，它也是 C++ 的标准语法之一。
            //  圆括号构造在某些情况下可能会允许隐式类型转换或窄化，因此在某些情况下可能会导致不符合预期的行为。
        }
    };

    auto parser(std::string_view json_str) -> std::optional<Node>
    {
        // parser() 函数是对外提供的接口函数，用于创建 JsonParser 对象并调用其 parse() 方法来解析 JSON 字符串，并返回解析结果。

        // 在 JsonParser 结构体中没有显式定义构造函数，那么编译器会自动生成默认构造函数。默认构造函数会按照成员变量的类型进行默认初始化，对于基本数据类型，会执行默认的零初始化或者不进行初始化（取决于类型），对于类类型或结构体类型，会调用默认构造函数进行初始化（如果有的话）。所以这里是能构造JsonParser对象的
        JsonParser p{json_str};
        return p.parse();
    }

    class JsonGenerator
    {
    public:
        static auto generate(const Node &node) -> std::string
        {
            // generate 函数是通过调用 std::visit 来处理 Node 对象中的值，并根据值的类型生成相应的 JSON 字符串表示。

            // std::visit 是 C++17 引入的 std::variant 的访问器，用于根据 node.value 的类型执行不同的操作。
            // 它接收一个 lambda 函数和一个 std::variant 类型的值 node.value，根据 node.value 的实际类型执行不同的逻辑。

            // if constexpr 是 C++17 中引入的一种编译期条件语句，它可以根据模板参数或常量表达式在编译期进行条件判断，并选择性地编译执行其中的代码块。
            return std::visit(
                [](auto &&arg) -> std::string //`&&`: 表示引用折叠，根据参数 `arg` 的实际类型来决定是左值引用还是右值引用。
                {
                    using T = std::decay_t<decltype(arg)>; // 用于获取 arg 的实际类型 T，并去除可能的引用和修饰符。
                    if constexpr (std::is_same_v<T, Null>)
                    {
                        return "null";
                    }
                    else if constexpr (std::is_same_v<T, Bool>)
                    {
                        return arg ? "true" : "false";
                    }
                    else if constexpr (std::is_same_v<T, Int>)
                    {
                        return std::to_string(arg);
                    }
                    else if constexpr (std::is_same_v<T, Float>)
                    {
                        return std::to_string(arg);
                    }
                    else if constexpr (std::is_same_v<T, String>)
                    {
                        return generate_string(arg);
                    }
                    else if constexpr (std::is_same_v<T, Array>)
                    {
                        return generate_array(arg);
                    }
                    else if constexpr (std::is_same_v<T, Object>)
                    {
                        return generate_object(arg);
                    }
                },
                node.value);
        }

        static auto generate_string(const String &str) -> std::string
        {
            std::string json_str = "\"";
            json_str += str;
            json_str += '"';
            return json_str;
        }

        static auto generate_array(const Array &array) -> std::string
        {
            std::string json_str = "[";
            for (const auto &node : array)
            {
                json_str += generate(node);
                json_str += ',';
            }
            if (!array.empty())
            {
                json_str.pop_back(); // 如果数组不为空，移除最后一个元素后面的 ,，然后在字符串末尾添加 ]，表示 JSON 数组的结束。
            }
            json_str += ']';
            return json_str;
        }

        static auto generate_object(const Object &object) -> std::string
        {
            std::string json_str = "{";
            for (const auto &[key, node] : object)
            {
                json_str += generate_string(key);
                json_str += ':';
                json_str += generate(node);
                json_str += ',';
            }

            if (!object.empty())
            {
                json_str.pop_back();
            }
            return json_str;
        }
    };

    inline auto generate(const Node &node) -> std::string
    {
        return JsonGenerator::generate(node);
    }

    auto operator<<(std::ostream &out, const Node &t) -> std::ostream &
    {
        // 对'<<'进行重载，可用'<<'进行输出，使用'<<'进入JSON
        // 生成类，根据对应的类型生存对应的JSON
        out << JsonGenerator::generate(t);
        return out;
    }

}

using namespace json1;

int main()
{
    std::ifstream fin("json.txt"); // 这行代码创建了一个 std::ifstream对象fin,std::ifstream 是 C++ 中用于从文件中读取数据的输入流类。
    std::stringstream ss;          // fin.rdbuf() 返回文件流的缓冲区指针，ss << fin.rdbuf() 将文件内容写入到 ss 中。
    ss << fin.rdbuf();             // rdbuf无参数时返回调用者的流缓冲指针，可以实现一个流对象指向的内容用另一个流对象来输出
    std::string s{ss.str()};       // 通过 ss.str() 获取 ss 中的字符串内容，并赋值给 std::string 对象 s。
    auto x = parser(s).value();    // 解析json字符串

    std::cout << x << "\n";
    // x["configurations"].push({true});
    // std::cout<<x<<"\n";
    // x["configurations"].push({Null{}});
    // x["version"]={114514L};
    // std::cout<<x["age"]<<"\n";
    std::cout << std::endl;
}