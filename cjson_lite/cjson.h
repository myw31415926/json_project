/**********************************************************
 * desc: cjson 头文件
 * file: cjson.h
 *
 * author:  myw31415926
 * date:    20200909
 * version: V0.1
 *
 * the closer you look, the less you see
 *********************************************************/

#ifndef _CJSON_H_
#define _CJSON_H_

#ifdef __cplusplus
extern "C" {
#endif

// cJSON 类型
#define cJSON_False     0
#define cJSON_True      1
#define cJSON_NULL      2
#define cJSON_Number    3
#define cJSON_String    4
#define cJSON_Array     5
#define cJSON_Object    6

// cJSON 结构体
typedef struct cJSON {
    struct cJSON *next, *prev;      // 前向、后向指针，前一项、后一项 cJSON 对象
    struct cJSON *child;            // 若是 Array/Object，则为孩子节点指针

    int     type;                   // 当前 cJSON 项的节点类型
    char*   valuestring;            // 当前 cJSON 项的字符串值, if type==cJSON_String
    int     valueint;               // 当前 cJSON 项的整型数值, if type==cJSON_Number
    double  valuedouble;            // 当前 cJSON 项的浮点型数值, if type==cJSON_Number

    char*   string;                 // 当前 cJSON 项的名称

} cJSON;

///////////////////////////////////////////////////////////////////////////////
// 解析一段 json 串，返回 cJSON 。完成后需要调用 cJSON_Delete 释放空间
extern cJSON* cJSON_Parse(const char* value);
// 释放 cJSON 的空间
extern void   cJSON_Delete(cJSON* c);

// 返回 cJSON 对象的输出字符串，带格式（缩进换行），使用完成后需要手动 free
extern char*  cJSON_Print(cJSON* item);
// 返回 cJSON 对象的输出字符串，不带格式（只有一行），使用完成后需要手动 free
extern char*  cJSON_PrintUnformatted(cJSON* item);

// 返回 cJSON 数组或对象的大小，即有多个个 next 指针
extern int    cJSON_GetArraySize(cJSON *array);
// 返回指定下标的 cJSON 对象
extern cJSON* cJSON_GetArrayItem(cJSON *array, int idx);
// 返回指定 key 值的 cJSON 对象，忽略大小写
extern cJSON* cJSON_GetObjectItem(cJSON *object, const char *string);

// 若解析错误，获取错误处的指针
extern const char* cJSON_GetErrorPtr(void);

// 创建不同类型的 cJSON 节点
extern cJSON* cJSON_CreateNull(void);
extern cJSON* cJSON_CreateTrue(void);
extern cJSON* cJSON_CreateFalse(void);
extern cJSON* cJSON_CreateBool(int b);
extern cJSON* cJSON_CreateNumber(double num);
extern cJSON* cJSON_CreateString(const char *string);
extern cJSON* cJSON_CreateArray(void);
extern cJSON* cJSON_CreateObject(void);

// 创建不同类型的 cJSON 数组
extern cJSON* cJSON_CreateIntArray(const int *numbers, int count);
extern cJSON* cJSON_CreateFloatArray(const float *numbers, int count);
extern cJSON* cJSON_CreateDoubleArray(const double *numbers, int count);
extern cJSON* cJSON_CreateStringArray(const char **strings, int count);

// 将 cJSON 节点添加到数组或对象中，对象要附上 key(string) 值
extern void cJSON_AddItemToArray(cJSON *array, cJSON *item);
extern void cJSON_AddItemToObject(cJSON *object,const char *string, cJSON *item);

// 宏定义：将数据添加到 cJSON 对象中
#define cJSON_AddNullToObject(object, name)       cJSON_AddItemToObject(object, name, cJSON_CreateNull())
#define cJSON_AddTrueToObject(object, name)       cJSON_AddItemToObject(object, name, cJSON_CreateTrue())
#define cJSON_AddFalseToObject(object, name)      cJSON_AddItemToObject(object, name, cJSON_CreateFalse())
#define cJSON_AddBoolToObject(object, name, b)    cJSON_AddItemToObject(object, name, cJSON_CreateBool(b))
#define cJSON_AddNumberToObject(object, name, n)  cJSON_AddItemToObject(object, name, cJSON_CreateNumber(n))
#define cJSON_AddStringToObject(object, name, s)  cJSON_AddItemToObject(object, name, cJSON_CreateString(s))

#ifdef __cplusplus
}
#endif

#endif /* _CJSON_H_ */