/**********************************************************
 * desc: rapidjson 解析和构造 json 实例
 * file: rapidjson_demo2.cpp
 *
 * author:  myw31415929
 * date:    20201207
 * version: V0.1
 *
 * the closer you look, the less you see
 *********************************************************/

#include <iostream>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

/**
 * test json:
{
    "Title": "index.html",
    "Defalult": true,
    "Text":"This is index page",
    "Image":{
        "Width":800,
        "Height":600,
        "Title":"Index image",
        "Thumbnail":{
            "Url":"http://www.example.com/image/481989943",
            "Height":125.5,
            "Width":"100.5"
        },
        "IDs":[100, 200, 300, 400]
    }
}
*/

/* 解析 json 串 */
int test_rapidjson_parse(const std::string json)
{
    rapidjson::Document d;
    if (d.Parse(json.c_str()).HasParseError()) {
        std::cout << "rapidjson Parse error, json: " << json << std::endl;
        return -1;
    }

    if (d.HasMember("Title") && d["Title"].IsString()) {
        std::cout << "Title: " << d["Title"].GetString() << std::endl;
    }

    if (d.HasMember("Defalult") && d["Defalult"].IsBool()) {
        std::cout << "Defalult: " << (d["Defalult"].GetBool() ? "true" : "false") << std::endl;
    }

    if (d.HasMember("Text") && d["Text"].IsString()) {
        std::cout << "Text: " << d["Text"].GetString() << std::endl;
    }

    if (d.HasMember("Image") && d["Image"].IsObject()) {
        const rapidjson::Value& obj = d["Image"];

        if (obj.HasMember("Width") && obj["Width"].IsInt()) {
            std::cout << "\tWidth: " << obj["Width"].GetInt() << std::endl;
        }

        if (obj.HasMember("Height") && obj["Height"].IsInt()) {
            std::cout << "\tHeight: " << obj["Height"].GetInt() << std::endl;
        }

        if (obj.HasMember("Thumbnail") && obj["Thumbnail"].IsObject()) {
            const rapidjson::Value& tobj = obj["Thumbnail"];

            if (tobj.HasMember("Url") && tobj["Url"].IsString()) {
                std::cout << "\t\tUrl: " << tobj["Url"].GetString() << std::endl;
            }

            if (tobj.HasMember("Height") && tobj["Height"].IsDouble()) {
                std::cout << "\t\tHeight: " << tobj["Height"].GetDouble() << std::endl;
            }

            if (tobj.HasMember("Width") && tobj["Width"].IsFloat()) {
                std::cout << "\t\tWidth: " << tobj["Width"].GetFloat() << std::endl;
            }
        }

        if (obj.HasMember("IDs") && obj["IDs"].IsArray()) {
            const rapidjson::Value& arr = obj["IDs"];

            std::cout << "\tIDs: ";
            for (size_t i = 0; i < arr.Size(); ++i) {
                if (arr[i].IsInt()) 
                    if (i == 0) std::cout << arr[i].GetInt();
                    else std::cout << ", " << arr[i].GetInt();
                else
                    std::cout << "array is not int" << std::endl;
            }
            std::cout << std::endl;
        }
    }

    return 0;
}

/* 解析 json 对象，返回 json 串 */
std::string test_rapidjson_write()
{
    rapidjson::StringBuffer buffer;
    // rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);    // 可换行

    writer.StartObject();

    writer.Key("Title");
    writer.String("index.html");
    writer.Key("Defalult");
    writer.Bool(true);
    writer.Key("Text");
    writer.String("This is index page");

    writer.Key("Image");
    writer.StartObject();

    writer.Key("Width");
    writer.Int(800);
    writer.Key("Height");
    writer.Int(600);
    writer.Key("Title");
    writer.String("Index image");

    writer.Key("Thumbnail");
    writer.StartObject();

    writer.Key("Url");
    writer.String("http://www.example.com/image/481989943");
    writer.Key("Height");
    writer.Double(125.5);
    writer.Key("Width");
    writer.Double(100.5);

    writer.EndObject();     // end Thumbnail

    writer.Key("IDs");
    writer.StartArray();
    writer.Int(100);
    writer.Int(200);
    writer.Int(300);
    writer.Int(400);
    writer.EndArray();

    writer.EndObject();     // end Image
    writer.EndObject();     // end root

    std::string json = buffer.GetString();
    std::cout << "test_rapidjson_write: "<< json << std::endl;

    return json;
}


int main(int argc, char const *argv[])
{
    std::cout << "test_rapidjson_write start ... " << std::endl;
    std::string json = test_rapidjson_write();

    std::cout << "test_rapidjson_parse start ... " << std::endl;
    test_rapidjson_parse(json);

    return 0;
}
