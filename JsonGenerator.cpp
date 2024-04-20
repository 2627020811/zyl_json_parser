#include "Json.hpp"

namespace json
{
    std::string JsonGenerator::generate(const Node &node)
    {
        // 用于根据 Node 对象生成对应的 JSON 字符串表示。

        // std::visit 是 C++17 引入的 std::variant 的访问器，用于根据 node.value 的类型执行不同的操作。
        // 它接收一个 lambda 函数和一个 std::variant 类型的值 node.value，根据 node.value 的实际类型执行不同的逻辑。
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

    std::string JsonGenerator::generate_string(const String &str)
    {
        std::string json_str = "\"";
        json_str += str;
        json_str += '"';
        return json_str;
    }

    std::string JsonGenerator::generate_array(const Array &array) 
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

    std::string JsonGenerator::generate_object(const Object &object)
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

}