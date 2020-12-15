/**********************************************************
 * desc: cjson 和 rapidjson 简单性能测试
 * file: cjson_rapidjson_perf.cpp
 *
 * author:  myw31415929
 * date:    20201207
 * version: V0.1
 *
 * the closer you look, the less you see
 *********************************************************/

#include <iostream>
#include <string>
#include "cJSON.h"
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

/* cJSON 生成 json 对象，并解析 */
void cjson_perf()
{
    char *json;
    cJSON *root, *img, *thm, *out_root;
    int ids[4]={100, 200, 300, 400};

    // 构造 json 对象
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "Title", "index.html");
    cJSON_AddTrueToObject(root, "Defalult");
    cJSON_AddStringToObject(root, "Text", "This is index page");
    cJSON_AddItemToObject(root, "Image", img = cJSON_CreateObject());
    cJSON_AddNumberToObject(img, "Width", 800);
    cJSON_AddNumberToObject(img, "Height", 600);
    cJSON_AddStringToObject(img, "Title", "Index image");
    cJSON_AddItemToObject(img, "Thumbnail", thm = cJSON_CreateObject());
    cJSON_AddStringToObject(thm, "Url", "http://www.example.com/image/481989943");
    cJSON_AddNumberToObject(thm, "Height", 125.5);
    cJSON_AddNumberToObject(thm, "Width", 100.5);
    cJSON_AddItemToObject(img, "IDs", cJSON_CreateIntArray(ids, 4));

    json = cJSON_PrintUnformatted(root);
    // std::cout << "cJSON output: " << json << std::endl;

    out_root = cJSON_Parse(json);
    if (!out_root) {
        std::cout << "cJSON_Parse error, json: " << cJSON_GetErrorPtr() << std::endl;
    }

    if (json) free(json);
    if (out_root) cJSON_Delete(out_root);

    cJSON_Delete(root);
}

/* rapidjson 生成 json 对象，并解析 */
void rapidjson_perf()
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    // rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);    // 可换行

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

    const char *json = buffer.GetString();
    // std::cout << "rapidjson output: " << json << std::endl;

    rapidjson::Document d;
    if (d.Parse(json).HasParseError()) {
        std::cout << "rapidjson Parse error, json: " << json << std::endl;
    }
}

int main(int argc, char const *argv[])
{
    int flag  = 0;          // 0: cJSON; 1: rapidjson
    int count = 1;

    if (argc == 3) {
        flag = atoi(argv[1]);
        count = atoi(argv[2]);
    }
    std::cout << (flag == 0 ? "cJSON run count " : "rapidjson run count ") << count << std::endl;

    if (flag == 0)
        for (int i = 0; i < count; i++) cjson_perf();
    else
        for (int i = 0; i < count; i++) rapidjson_perf();

    return 0;
}
