#include"Json.hpp"
#include<fstream>
#include<iostream>
using namespace json;
int main()
{
    std::ifstream fin("json.txt");//这行代码创建了一个 std::ifstream对象fin,std::ifstream 是 C++ 中用于从文件中读取数据的输入流类。
    std::stringstream ss; //fin.rdbuf() 返回文件流的缓冲区指针，ss << fin.rdbuf() 将文件内容写入到 ss 中。
    ss<<fin.rdbuf();//rdbuf无参数时返回调用者的流缓冲指针，可以实现一个流对象指向的内容用另一个流对象来输出
    std::string s{ss.str()}; //通过 ss.str() 获取 ss 中的字符串内容，并赋值给 std::string 对象 s。
    auto x=parser(s).value(); //解析json字符串
    std::cout<<x<<"\n";
    x["configurations"].push({true});
    std::cout<<x<<"\n";
    x["configurations"].push({Null{}});
    x["version"]={114514L};
    std::cout<<x<<"\n";
    std::cout<<std::endl;
}
// auto sum_2(int a,int b)
// {
//     return a+b;
// }
// int main()
// {
//     auto x=sum_2(1,2);
//     std::cout<<x<<std::endl;
//     std::cout<<typeid(x).name()<<std::endl;
// }