/**********************************************************
 * desc: cjson 程序测试
 * file: cjson_test.c
 *
 * author:  myw31415926
 * date:    20200909
 * version: V0.1
 *
 * the closer you look, the less you see
 *********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "cjson.h"

int parse_json_pirnt(char* json, int fmt)
{
    char *out;
    /* code */
    cJSON* c = cJSON_Parse(json);
    if (!c) {
        printf("cJSON_Parse return null, error: %s\n", cJSON_GetErrorPtr());
        return -1;
    }

    printf("cjson test: %d\n", c->type);

    if (fmt) out = cJSON_Print(c);
    else out = cJSON_PrintUnformatted(c);
    printf("cjson: \n%s\n", out);
    free(out);

    cJSON_Delete(c);

    return 0;
}

int get_json_value(char* json)
{
    int   i, sz = 0;
    char  *out;
    cJSON *item;

    /* code */
    cJSON* c = cJSON_Parse(json);
    if (!c) {
        printf("cJSON_Parse return null, error: %s\n", cJSON_GetErrorPtr());
        return -1;
    }

    if (c->type == cJSON_Array || c->type == cJSON_Object) {
        sz = cJSON_GetArraySize(c);
        for (i = 0; i < sz; ++i) {
            item = cJSON_GetArrayItem(c, i);
            if (item->string) {
                out = cJSON_PrintUnformatted(item);
                printf("array|object[%d]: %s: %s\n", i, item->string, out);
                free(out);
            }
            else {
                out = cJSON_PrintUnformatted(item);
                printf("array|object[%d]: %s\n", i, out);
                free(out);
            }
        }
    }

    if (c->type == cJSON_Object) {
        item = cJSON_GetObjectItem(c, "name");
        if (item) {
            out = cJSON_PrintUnformatted(item);
            printf("name: %s\n", out);
            free(out);
        }
    }

    cJSON_Delete(c);

    return 0;
}

// 一条地址记录，用于测试
struct record {
    const char *precision;
    double lat,lon;
    const char *address,*city,*state,*zip,*country;
};

void create_json_objects()
{
    int  i;
    char *out;
    cJSON *root, *fmt, *img, *thm, *fld;

    // Our "days of the week" array
    const char *strings[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    int numbers[3][3]={{0,-1,0}, {1,0,0}, {0,0,1}};
    // Our "gallery" item: 
    int ids[4]={116, 943, 234, 38793};
    // Our array of "records": 
    struct record fields[2]={
        {"zip", 37.7668,   -1.223959e+2, "", "SAN FRANCISCO", "CA", "94107", "US"},
        {"zip", 37.371991, -1.22026e+2,  "", "SUNNYVALE",     "CA", "94085", "US"}};

    // 构造 json 节点
    root = cJSON_CreateObject();  
    cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
    cJSON_AddItemToObject(root, "format", fmt = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt, "type", "rect");
    cJSON_AddNumberToObject(fmt, "width",  1920);
    cJSON_AddNumberToObject(fmt, "height", 1080);
    cJSON_AddFalseToObject (fmt, "interlace");
    cJSON_AddNumberToObject(fmt, "frame rate", 24);

    out = cJSON_Print(root); printf("json1:\n%s\n",out);  free(out); cJSON_Delete(root);

    // 构造数组
    root = cJSON_CreateStringArray(strings, 7);

    out = cJSON_Print(root); printf("json2:\n%s\n",out);  free(out); cJSON_Delete(root);


    // 构造多维数组
    root = cJSON_CreateArray();
    for (i = 0; i < 3; i++) cJSON_AddItemToArray(root, cJSON_CreateIntArray(numbers[i], 3));

    out = cJSON_Print(root); printf("json3:\n%s\n",out);  free(out); cJSON_Delete(root);


    // 构造 json 对象
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "Image", img = cJSON_CreateObject());
    cJSON_AddNumberToObject(img, "Width", 800);
    cJSON_AddNumberToObject(img, "Height", 600);
    cJSON_AddStringToObject(img, "Title", "View from 15th Floor");
    cJSON_AddItemToObject(img, "Thumbnail", thm = cJSON_CreateObject());
    cJSON_AddStringToObject(thm, "Url", "http:/*www.example.com/image/481989943");
    cJSON_AddNumberToObject(thm, "Height", 125);
    cJSON_AddStringToObject(thm, "Width", "100");
    cJSON_AddItemToObject(img, "IDs", cJSON_CreateIntArray(ids, 4));

    out = cJSON_Print(root); printf("json4:\n%s\n",out);  free(out); cJSON_Delete(root);

    // 创造对象数组
    root = cJSON_CreateArray();
    for (i = 0; i < 2; i++) {
        cJSON_AddItemToArray(root, fld = cJSON_CreateObject());
        cJSON_AddStringToObject(fld, "precision", fields[i].precision);
        cJSON_AddNumberToObject(fld, "Latitude", fields[i].lat);
        cJSON_AddNumberToObject(fld, "Longitude", fields[i].lon);
        cJSON_AddStringToObject(fld, "Address", fields[i].address);
        cJSON_AddStringToObject(fld, "City", fields[i].city);
        cJSON_AddStringToObject(fld, "State", fields[i].state);
        cJSON_AddStringToObject(fld, "Zip", fields[i].zip);
        cJSON_AddStringToObject(fld, "Country", fields[i].country);
    }
    

    out = cJSON_Print(root); printf("json5:\n%s\n",out);  free(out); cJSON_Delete(root);
}

int main(int argc, char *argv[])
{
    char text1[]="{\n\"name\": \"Jack (\\\"Bee\\\") Nimble\", \n\"format\": {\"type\":       \"rect\", \n\"width\":      1920, \n\"height\":     1080, \n\"interlace\":  false,\"frame rate\": 24\n}\n}";   
    char text2[]="[\"Sunday\", \"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";
    char text3[]="[\n    [0, -1, 0],\n    [1, 0, 0],\n    [0, 0, 1]\n   ]\n";
    char text4[]="{\n       \"Image\": {\n          \"Width\":  800,\n          \"Height\": 600,\n          \"Title\":  \"View from 15th Floor\",\n         \"Thumbnail\": {\n              \"Url\":    \"http:/*www.example.com/image/481989943\",\n               \"Height\": 125,\n              \"Width\":  \"100\"\n           },\n            \"IDs\": [116, 943, 234, 38793]\n       }\n }";
    char text5[]="[\n    {\n     \"precision\": \"zip\",\n   \"Latitude\":  37.7668,\n   \"Longitude\": -122.3959,\n     \"Address\":   \"\",\n  \"City\":      \"SAN FRANCISCO\",\n     \"State\":     \"CA\",\n    \"Zip\":       \"94107\",\n     \"Country\":   \"US\"\n     },\n    {\n     \"precision\": \"zip\",\n   \"Latitude\":  37.371991,\n     \"Longitude\": -122.026020,\n   \"Address\":   \"\",\n  \"City\":      \"SUNNYVALE\",\n     \"State\":     \"CA\",\n    \"Zip\":       \"94085\",\n     \"Country\":   \"US\"\n     }\n     ]";

    printf("\n****** parse_json_pirnt with fmt = 1 ******\n");
    parse_json_pirnt("[\"age\", 2]", 1);
    parse_json_pirnt("{\"name\" : \"mayw\", \"age\" : 21}", 1);
    parse_json_pirnt(text1, 1);
    parse_json_pirnt(text2, 1);
    parse_json_pirnt(text3, 1);
    parse_json_pirnt(text4, 1);
    parse_json_pirnt(text5, 1);

    printf("\n****** parse_json_pirnt with fmt = 0 ******\n");
    parse_json_pirnt(text1, 0);
    parse_json_pirnt(text2, 0);
    parse_json_pirnt(text3, 0);
    parse_json_pirnt(text4, 0);
    parse_json_pirnt(text5, 0);

    printf("\n****** get_json_value ******\n");
    get_json_value("[\"age\", 2]");
    get_json_value("{\"name\" : \"mayw\", \"age\" : 21}");
    get_json_value(text1);
    get_json_value(text2);
    get_json_value(text3);
    get_json_value(text4);
    get_json_value(text5);

    printf("\n****** create_json_objects ******\n");
    create_json_objects();

    return 0;
}
