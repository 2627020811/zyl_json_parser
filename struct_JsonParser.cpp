#include "Json.hpp"

namespace json
{
    void JsonParser::parse_whitespace()
    {
        while (pos < json_str.size() && std::isspace(json_str[pos]))
        {
            // 判断空格
            ++pos;
        }
    }

    std::optional<Value> JsonParser::parse_null()
    {
        if (json_str.substr(pos, 4) == "null")
        {
            pos += 4;
            return Null{};
        }
        return {};
    }

    std::optional<Value> JsonParser::parse_true()
    {
        if (json_str.substr(pos, 4) == "true")
        {
            pos += 4;
            return true;
        }
        return {};
    }

    std::optional<Value> JsonParser::parse_false()
    {
        if (json_str.substr(pos, 5) == "false")
        {
            pos += 5;
            return false;
        }
        return {};
    }

    std::optional<Value> JsonParser::parse_number()
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

    std::optional<Value> JsonParser::parse_string()
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

    std::optional<Value> JsonParser::parse_array()
    {
        pos++; //[
        Array arr;
        while (pos < json_str.size() && json_str[pos] != ']')
        {
            auto value = parse_value();
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

    std::optional<Value> JsonParser::parse_object()
    {
        pos++; //{ ：将 pos 向前移动一位，跳过当前位置的 { 字符，因为 JSON 对象的开始应该是 {。
        Object obj;
        while (pos < json_str.size() && json_str[pos] != '}')
        {
            auto key = parse_value(); // 解析键值
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
            auto val = parse_value(); // 解析value
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

    std::optional<Value> JsonParser::parse_value()
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

    std::optional<Node> JsonParser::parse()
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
}