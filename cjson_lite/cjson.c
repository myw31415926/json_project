/**********************************************************
 * desc: cjson 源文件
 * file: cjson.c
 *
 * author:  myw31415926
 * date:    20200909
 * version: V0.1
 *
 * the closer you look, the less you see
 *********************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>      // DBL_EPSILON
#include <limits.h>     // INT_MAX INT_MIN
#include <ctype.h>
#include "cjson.h"

// 定义 printbuffer 格式：地址，长度，偏移
typedef struct {char *buffer; int length; int offset; } printbuffer;

// 错误指针 error_point，解析失败时，指向发生错误的地址
static const char* ep;

static void* (*cJSON_malloc)(size_t sz) = malloc;
static void  (*cJSON_free)(void* ptr) = free;

// 解析 json 字符串，并赋值给 item 节点，返回后续字符串的指针
static const char* parse_value(cJSON *item, const char *value);

// 输出 json 字符串
// @param item:  json 树结构的根节点
// @param depth: 节点的深度
// @param fmt:   是否带格式输出（缩进换行）
// @param p:     是否输出到指定的 buffer，这里为简便，忽略此参数
static char* print_value(cJSON *item, int depth, int fmt, printbuffer *p);

// 跳过非打印字符（回车换行符、结束符）和空格符 (32)
static const char* skip(const char *in)
{
    while (in && *in && (unsigned char)*in <= 32) in++;
    return in;
}

// 返回比x大的最小的2的N次方数
// 参考 https://blog.csdn.net/dlf1769/article/details/78918045
static int pow2gt(int x)
{
    --x;
    x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16;
    return x + 1;
}

// 确认 buffer 中是否有 needed 空间，若没有，则申请新的空间
static char* ensure(printbuffer *p, int needed)
{
    char *newbuffer;
    int newsize;
    if (!p || !p->buffer) return NULL;
    needed += p->offset;
    if (needed <= p->length) return p->buffer + p->offset;

    newsize = pow2gt(needed);
    newbuffer = (char*)cJSON_malloc(newsize);
    if (!newbuffer) { cJSON_free(p->buffer); p->length = 0; p->offset = 0; return NULL; }
    memcpy(newbuffer, p->buffer, p->length);
    cJSON_free(p->buffer);  // free old space
    p->length = newsize;
    p->buffer = newbuffer;
    return newbuffer + p->offset;
}

// 更新 printbuffer 中的偏移
static int update(printbuffer *p)
{
    char *str;
    if (!p || !p->buffer) return 0;
    str = p->buffer + p->offset;
    return p->offset + strlen(str);
}

// 生成一个 cJSON 节点对象，并置0
static cJSON* cJSON_New_Item(void)
{
    cJSON *node = cJSON_malloc(sizeof(cJSON));
    if (node) memset(node, 0, sizeof(cJSON));
    return node;
}

// 申请空间，并拷贝字符串，类似 linux strdup
static char* cJSON_strdup(const char *str)
{
    size_t len;
    char *copy;

    len = strlen(str) + 1;
    if (!(copy = (char*)cJSON_malloc(len))) return NULL;
    memcpy(copy, str, len);
    return copy;
}

// 忽略大小写的差异比较，类似 strcmp 函数
static int cJSON_strcasecmp(const char *s1, const char *s2)
{
    if (!s1) return (s1==s2) ? 0 : 1;
    if (!s2) return 1;

    for(; tolower(*s1) == tolower(*s2); ++s1, ++s2) if(*s1 == 0) return 0;
    return tolower(*s1) - tolower(*s2);
}

// 解析 json 字符串，并返回根节点
// @param value:                    json 字符串指针
// @param return_parse_end:         输出参数，返回解析结尾的指针
// @param require_null_terminated:  是否要求 json 字符串必须有结束符
cJSON* cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated)
{
    const char *end = NULL;
    cJSON *c = cJSON_New_Item();
    ep = 0;
    if (!c) return NULL;

    end = parse_value(c, skip(value));
    if (!end) { cJSON_Delete(c); return NULL; }

    // 判断是否需要检查结束符和返回解析结尾指针
    if (require_null_terminated) {
        end = skip(end);
        if(*end) {  // 非结束符结尾
            cJSON_Delete(c); ep = end; return NULL;
        }
    }
    if (require_null_terminated) *return_parse_end = end;

    return c;
}

// 解析一段 json 串，返回 cJSON 。完成后需要调用 cJSON_Delete 释放空间
cJSON* cJSON_Parse(const char* value)
{
    return cJSON_ParseWithOpts(value, NULL, 0);    // 不需要结束地址，不需要验证结束符
}

// 释放 cJSON 的空间
extern void cJSON_Delete(cJSON* c)
{
    cJSON *next;
    while (c) {
        // 需要遍历生成子节点
        next = c->next;
        if (c->child) cJSON_Delete(c->child);
        if (c->valuestring) cJSON_free(c->valuestring);
        if (c->string) cJSON_free(c->string);
        cJSON_free(c);
        c = next;
    }
}

// 返回 cJSON 对象的输出字符串，带格式（缩进换行），使用完成后需要手动 free
char*  cJSON_Print(cJSON* item)
{
    printbuffer p;
    p.offset = 0;
    p.length = 64;
    p.buffer = (char*)cJSON_malloc(p.length);

    return print_value(item, 0, 1, &p);
}

// 返回 cJSON 对象的输出字符串，不带格式（只有一行），使用完成后需要手动 free
extern char*  cJSON_PrintUnformatted(cJSON* item)
{
    printbuffer p;
    p.offset = 0;
    p.length = 64;
    p.buffer = (char*)cJSON_malloc(p.length);

    return print_value(item, 0, 0, &p);;
}

// 解析字符串格式的 json 串，返回后续字符串的指针
static const char* parse_string(cJSON *item, const char *str)
{
    const char *ptr = str + 1;
    char *ptr2, *out;
    int  len = 0;

    if (*str != '\"') { ep = str; return NULL; }    // 非字符串

    // 统计字符串长度，跳过斜线，斜线为转义符
    while (*ptr != '\"' && *ptr && ++len) { if (*ptr++ == '\\') ptr++; }

    out = (char*)cJSON_malloc(len + 1);
    if (!out) return NULL;

    ptr = str + 1; ptr2 = out;
    while (*ptr != '\"' && *ptr) {
        if (*ptr != '\\') *ptr2++ = *ptr++;         // 非转义字符，直接赋值
        else {
            ptr++;
            switch (*ptr) {
                case 'n': *ptr2++ = '\n'; break;    // 换行符
                case 't': *ptr2++ = '\t'; break;    // TAB 符
                // TODO: 源码中还有其他转移字符和 utf16 的字符，非重点，这里不做处理
                default: *ptr2++ = *ptr;  break;
            }
            ptr++;
        }
    }

    *ptr2 = 0;
    if (*ptr == '\"') ptr++;
    item->valuestring = out;
    item->type = cJSON_String;

    return ptr;
}

// 解析数据格式的 json 串，返回后续字符串的指针
static const char* parse_number(cJSON *item, const char *num)
{
    double n = 0, sign = 1, scale = 0;      // sign 符号位； scale 小数位
    int subscale = 0, signsubscale = 1;     // 科学计数法 signsubscale 符号位； subscale 指数

    if (*num == '-') sign = -1, num++;      // 负数
    if (*num == '0') num++;                 // 0 开头，忽略不计
    if (*num >= '1' && *num <= '9') { do n = (n * 10.0) + (*num++ - '0'); while(*num >= '0' && *num <= '9'); }
    if (*num == '.' && num[1] >= '0' && num[1] <= '9') {   // 小数点
        num++;                              // 注意统计小数位 scale
        do n = (n * 10.0) + (*num++ - '0'), scale--; while(*num >= '0' && *num <= '9'); 
    }
    if (*num == 'e' || *num == 'E') {       // 科学计数法
        num++;
        if (*num == '+') num++; else if (*num == '-') signsubscale = -1, num++;
        while (*num >= '0' && *num <= '9') subscale = (subscale * 10) + (*num++ - '0');
    }

    // 计算数据的值
    n = sign * n * pow(10.0, (scale + subscale * signsubscale));

    item->valuedouble = n;
    item->valueint = (int)n;
    item->type = cJSON_Number;

    return num;
}

// 解析数组 [] 格式的 json 串，返回后续字符串的指针
static const char* parse_array(cJSON *item, const char *value)
{
    cJSON *child;
    if (*value != '[') {ep = value; return NULL; }  // 非数组

    item->type = cJSON_Array;
    value = skip(value + 1);
    if (*value == ']') return value + 1;            // 空数组

    item->child = child = cJSON_New_Item();
    if (!item->child) return NULL;
    value = skip(parse_value(child, skip(value)));          // 跳过空格，解析数组中的字符串
    if (!value) return NULL;

    while (*value == ',') {                                 // 作为子节点的 next 项处理数组
        cJSON *new_item = cJSON_New_Item();
        if (!new_item) return NULL;
        child->next = new_item;                             // 连接双向链表
        new_item->prev = child;
        child = new_item;
        value = skip(parse_value(child, skip(value + 1)));  // 跳过空格，解析数组中的字符串
        if (!value) return NULL;
    }

    if (*value == ']') return value + 1;                    // 数组解析结束
    ep = value; return NULL;                                // 解析错误
}

// 解析对象 {} 格式的 json 串，返回后续字符串的指针
static const char* parse_object(cJSON *item, const char *value)
{
    cJSON *child;
    if (*value != '{') {ep = value; return NULL; }  // 非对象

    item->type = cJSON_Object;
    value = skip(value + 1);
    if (*value == '}') return value + 1;            // 空对象

    item->child = child = cJSON_New_Item();
    if (!item->child) return NULL;
    value = skip(parse_string(child, skip(value)));     // 跳过空格，解析对象中的 key 值（字符串）
    if (!value) return NULL;
    child->string = child->valuestring; child->valuestring = NULL;

    if (*value != ':') {ep = value; return NULL; }      // 对象格式错误，必是 key:value
    value = skip(parse_value(child, skip(value + 1)));  // 跳过空格，解析对象中的 value
    if (!value) return NULL;

    while (*value == ',') {                             // 作为子节点的 next 项处理数组
        cJSON *new_item = cJSON_New_Item();
        if (!new_item) return NULL;
        child->next = new_item;                         // 连接双向链表
        new_item->prev = child;
        child = new_item;

        value = skip(parse_string(child, skip(value + 1))); // 跳过空格，解析对象中的 key 值（字符串）
        if (!value) return NULL;
        child->string = child->valuestring; child->valuestring = NULL;

        if (*value != ':') {ep = value; return NULL; }      // 对象格式错误，必是 key:value
        value = skip(parse_value(child, skip(value + 1)));  // 跳过空格，解析对象中的 value
        if (!value) return NULL;
    }

    if (*value == '}') return value + 1;            // 数组解析结束
    ep = value; return NULL;                        // 解析错误
}

// 解析 json 字符串，并赋值给 item 节点，返回后续字符串的指针
static const char* parse_value(cJSON *item, const char *value)
{
    if (!value)                      return NULL;
    if (!strncmp(value, "null", 4))  { item->type = cJSON_NULL; return value + 4; }
    if (!strncmp(value, "false", 5)) { item->type = cJSON_False; return value + 5; }
    if (!strncmp(value, "true", 4))  { item->type = cJSON_True; item->valueint = 1; return value + 4; }
    if (*value == '\"')              { return parse_string(item, value); }
    if (*value == '-' || (*value >= '0' && *value <= '9')) { return parse_number(item, value); }
    if (*value == '[')               { return parse_array(item, value); }
    if (*value == '{')               { return parse_object(item, value); }

    // 错误格式
    ep = value;
    return NULL;
}

// 输出 json 数字
static char* print_number(cJSON *item, printbuffer *p)
{
    char *str = NULL;
    double d  = item->valuedouble;

    if (d == 0) {
        str = ensure(p, 2);
        if (str) strcpy(str, "0");
    }
    else if (fabs((double)item->valueint - d) <= DBL_EPSILON && d <= INT_MAX && d >= INT_MIN) {
        // 输出为整数
        str = ensure(p, 21);                // /* 2^64+1 can be represented in 21 chars. */
        if (str) sprintf(str, "%d", item->valueint);
    }
    else {
        str = ensure(p, 64);
        if (str) sprintf(str, "%f", d);     // 源码中处理更复杂一些，这里直接输出
    }

    return str;
}

// 输出 json 字符串
static char *print_string_ptr(const char *str, printbuffer *p)
{
    const char *ptr;
    char *ptr2, *out;
    int len;

    // 源码中会处理转义字符，这里不做处理，可以参考源码
    if (!str) {
        out = ensure(p, 3);
        if (!out) return NULL;
        strcpy(out, "\"\"");
    }
    else {
        for (ptr = str; *ptr; ptr++) ;
        len = ptr - str;

        out = ensure(p, len + 3);   // 双引号 + 结束符
        if (!out) return NULL;

        ptr2 = out; *ptr2++ = '\"';
        strcpy(ptr2, str);
        ptr2[len] = '\"';
        ptr2[len + 1] = 0;
    }

    return out;
}

// 输出 json 字符串
static char* print_string(cJSON *item, printbuffer *p)
{
    return print_string_ptr(item->valuestring, p);
}

// 输出 json 数组
static char* print_array(cJSON *item, int depth, int fmt, printbuffer *p)
{
    int numentries = 0, i = 0, len;
    char *out, *ptr;
    cJSON *child = item->child;

    // 计算数组中总共有多少数据
    while (child) { numentries++; child = child->next; }

    // 空数组
    if (!numentries) {
        out = ensure(p, 3);
        if (out) strcpy(out, "[]");
        return out;
    }

    i = p->offset;                  // 记录输出位置
    ptr = ensure(p, 1);
    if (!ptr) return NULL;
    *ptr = '['; p->offset++;

    child = item->child;
    while (child) {
        out = print_value(child, depth + 1, fmt, p);
        p->offset = update(p);      // 此处统一计算偏移

        if (child->next) {          // 若有 next 计算逗号和分隔符
            len = fmt ? 2 : 1;
            ptr = ensure(p, len + 1);
            if (!ptr) return 0;
            *ptr++ = ',';
            if (fmt) *ptr++ = ' ';
            *ptr = 0;
            p->offset += len;
        }
        child = child->next;
    }

    ptr = ensure(p, 2);
    if (!ptr) return NULL;
    *ptr++ = ']'; *ptr = 0;
    out = p->buffer + i;

    return out;
}

// 输出 json 对象
static char* print_object(cJSON *item, int depth, int fmt, printbuffer *p)
{
    int numentries = 0, i = 0, j, len;
    char *out, *ptr;
    cJSON *child = item->child;

    // 计算对象中总共有多少数据
    while (child) { numentries++; child = child->next; }

    // 空对象
    if (!numentries) {
        out = ensure(p, 3);
        if (out) strcpy(out, "{}");
        return out;
    }

    i = p->offset;                  // 记录输出位置

    len = fmt ? 2 : 1;
    ptr = ensure(p, len + 1);
    if (!ptr) return NULL;
    *ptr++ = '{';
    if (fmt) *ptr++ = '\n'; *ptr = 0;     // 格式化输出，要换行
    p->offset += len;

    child = item->child; depth++;
    while (child) {
        if (fmt) {
            ptr = ensure(p, depth);
            if (!ptr) return NULL;
            for (j = 0; j < depth; j++) *ptr++ = '\t';
            p->offset += depth;
        }

        print_string_ptr(child->string, p);
        p->offset = update(p);

        ptr=ensure(p, 1);
        if (!ptr) return NULL;
        *ptr++ = ':';
        p->offset++;

        out = print_value(child, depth, fmt, p);
        p->offset = update(p);

        // 判断是否有下一项
        len = (fmt ? 1 : 0) + (child->next ? 1 : 0);
        ptr=ensure(p, len + 1);
        if (!ptr) return NULL;
        if (child->next) *ptr++ =',';
        if (fmt) *ptr++ ='\n';
        *ptr = 0; p->offset += len;

        child = child->next;
    }

    ptr = ensure(p, fmt ? (depth + 1) : 2);
    if (!ptr) return NULL;
    if (fmt) for(j = 0; j < depth - 1; j++) *ptr++ = '\t';
    *ptr++ = '}'; *ptr = 0;
    out = p->buffer + i;

    return out;
}

// 将 json 节点输出为 json 字符串
// @param item:  json 树结构的根节点
// @param depth: 节点的深度
// @param fmt:   是否带格式输出（缩进换行）
// @param p:     是否输出到指定的 buffer
// 这里为简便，只处理 p != NULL 的情况，避免频繁申请空间
static char* print_value(cJSON *item, int depth, int fmt, printbuffer *p)
{
    char *out = NULL;
    if (!item) return NULL;

    switch (item->type & 255) {
        case cJSON_NULL:   out = ensure(p, 5); if (out) strcpy(out, "null"); break;
        case cJSON_False:  out = ensure(p, 6); if (out) strcpy(out, "false"); break;
        case cJSON_True:   out = ensure(p, 5); if (out) strcpy(out, "true"); break;
        case cJSON_Number: out = print_number(item, p); break;
        case cJSON_String: out = print_string(item, p); break;
        case cJSON_Array:  out = print_array(item, depth, fmt, p); break;
        case cJSON_Object: out = print_object(item, depth, fmt, p); break;
    }
    return out;
}


// 返回 cJSON 数组或对象的大小，即有多个个 next 指针
int    cJSON_GetArraySize(cJSON *array)
{
    int i = 0;
    cJSON *c = array->child;
    while (c) i++, c = c->next;
    return i;
}

// 返回指定下标的 cJSON 对象
cJSON* cJSON_GetArrayItem(cJSON *array, int idx)
{
    cJSON *c = array->child;
    while (c && idx > 0) idx--, c = c->next;
    return c;
}

// 返回指定 key 值的 cJSON 对象，忽略大小写
cJSON* cJSON_GetObjectItem(cJSON *object, const char *string)
{
    cJSON *c = object->child;
    while (c && cJSON_strcasecmp(c->string, string)) c = c->next;
    return c;
}

// 若解析错误，获取错误处的指针
const char* cJSON_GetErrorPtr(void)
{
    return ep;
}

// 创建不同类型的 cJSON 节点
cJSON* cJSON_CreateNull(void)
{
    cJSON *item=cJSON_New_Item();
    if(item) item->type = cJSON_NULL;
    return item;
}

cJSON* cJSON_CreateTrue(void)
{
    cJSON *item=cJSON_New_Item();
    if(item) item->type = cJSON_True;
    return item;
}

cJSON* cJSON_CreateFalse(void)
{
    cJSON *item=cJSON_New_Item();
    if(item) item->type = cJSON_False;
    return item;
}

cJSON* cJSON_CreateBool(int b)
{
    cJSON *item=cJSON_New_Item();
    if(item) item->type = b ? cJSON_True : cJSON_False;
    return item;
}

cJSON* cJSON_CreateNumber(double num)
{
    cJSON *item=cJSON_New_Item();
    if(item) {
        item->type = cJSON_Number;
        item->valuedouble = num;
        item->valueint = (int)num;
    }
    return item;
}

cJSON* cJSON_CreateString(const char *string)
{
    cJSON *item=cJSON_New_Item();
    if(item) {
        item->type = cJSON_String;
        item->valuestring = cJSON_strdup(string);
    }
    return item;
}

cJSON* cJSON_CreateArray(void)
{
    cJSON *item=cJSON_New_Item();
    if(item) item->type = cJSON_Array;
    return item;
}

cJSON* cJSON_CreateObject(void)
{
    cJSON *item=cJSON_New_Item();
    if(item) item->type = cJSON_Object;
    return item;
}

// 创建不同类型的 cJSON 数组
cJSON* cJSON_CreateIntArray(const int *numbers, int count)
{
    int i;
    cJSON *n = NULL, *p = NULL, *item = cJSON_CreateArray();

    for (i = 0; item && i < count; i++) {
        n = cJSON_CreateNumber(numbers[i]);
        if (i == 0) {   // 头节点
            item->child = n;
        }
        else {          // 非头节点，连接链表
            p->next = n;
            n->prev = p;
        }
        p = n;
    }
    return item;
}

cJSON* cJSON_CreateFloatArray(const float *numbers, int count)
{
    int i;
    cJSON *n = NULL, *p = NULL, *item = cJSON_CreateArray();

    for (i = 0; item && i < count; i++) {
        n = cJSON_CreateNumber(numbers[i]);
        if (i == 0) {   // 头节点
            item->child = n;
        }
        else {          // 非头节点，连接链表
            p->next = n;
            n->prev = p;
        }
        p = n;
    }
    return item;
}

cJSON* cJSON_CreateDoubleArray(const double *numbers, int count)
{
    int i;
    cJSON *n = NULL, *p = NULL, *item = cJSON_CreateArray();

    for (i = 0; item && i < count; i++) {
        n = cJSON_CreateNumber(numbers[i]);
        if (i == 0) {   // 头节点
            item->child = n;
        }
        else {          // 非头节点，连接链表
            p->next = n;
            n->prev = p;
        }
        p = n;
    }
    return item;
}

cJSON* cJSON_CreateStringArray(const char **strings, int count)
{
    int i;
    cJSON *n = NULL, *p = NULL, *item = cJSON_CreateArray();

    for (i = 0; item && i < count; i++) {
        n = cJSON_CreateString(strings[i]);
        if (i == 0) {   // 头节点
            item->child = n;
        }
        else {          // 非头节点，连接链表
            p->next = n;
            n->prev = p;
        }
        p = n;
    }
    return item;
}

// 将 cJSON 节点添加到数组或对象中，对象要附上 key(string) 值
void cJSON_AddItemToArray(cJSON *array, cJSON *item)
{
    cJSON *c = array->child;
    if (!item) return;
    if (!c) array->child = item;    // 直接作为孩子节点
    else {                          // 作为右子树节点
        while (c && c->next) c = c->next;
        c->next = item;
        item->prev = c;
    }
}

void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
    if (!item) return;
    if (item->string) cJSON_free(item->string);     // 释放节点空间
    item->string = cJSON_strdup(string);
    cJSON_AddItemToArray(object, item);
}
