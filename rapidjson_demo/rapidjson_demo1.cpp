/**********************************************************
 * desc: rapidjson 简单实例
 * file: rapidjson_demo1.cpp
 *
 * author:  myw31415929
 * date:    20201207
 * version: V0.1
 *
 * the closer you look, the less you see
 *********************************************************/

#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

int main(int argc, char const *argv[])
{
    // 1. 将 json 解析为 DOM
    const char *json = "{\"project\":\"rapidjson\",\"stars\":10}";
    rapidjson::Document d;
    d.Parse(json);

    // 2. 利用 DOM 作出修改
    rapidjson::Value &s = d["stars"];
    s.SetInt(s.GetInt() + 1);

    // 3. 使用 StringBuffer 将 DOM 转换成 json
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);
    std::cout << "Output: "<< buffer.GetString() << std::endl;
    return 0;
}
