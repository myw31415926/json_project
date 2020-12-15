cJSON源码分析
https://www.jianshu.com/p/838f69db2f71
https://codeload.github.com/lanxinyuchs/books/zip/master

cJSON是C语言中的一个JSON编解码器，非常轻量级，C文件只有不到一千行，代码的可读性也很好，很适合作为C语言项目进行学习。项目主页：
https://sourceforge.net/projects/cjson/

对于json格式编码与解码，其实就是类似于一个解释器，主要原理还是运用递归。个人认为，如果能用一些支持面向对象的语言来做这个项目，代码实现起来应该会更加优雅。

先来看一下cJSON的数据结构：

/* The cJSON structure: */
typedef struct cJSON {
    struct cJSON *next,*prev;   /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct cJSON *child;        /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

    int type;                   /* The type of the item, as above. */

    char *valuestring;          /* The item's string, if type==cJSON_String */
    int valueint;               /* The item's number, if type==cJSON_Number */
    double valuedouble;         /* The item's number, if type==cJSON_Number */

    char *string;               /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} cJSON;