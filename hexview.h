/**
 * @file hexview.h
 * @author Southern Sandbox
 * @date 2024-05-17
 * 
 * @copyright Copyright (c) 2024
 * 
 * @details
 *      1. hexview 是一种简易十六进制视图打印工具
 *      2. 支持最长打印长度为 0xffff，支持打印自定义备注信息
 *      3. 支持打印所在文件/所在函数/行数位置等信息
 *      4. 支持自定义输出接口
 *      5. 支持为指定数值/字符提供彩色打印支持（终端需支持 VT100 标准）
 *      6. 参考打印结果：
 * 
 * [  Hexview  ]
 * target  start
 * file    ../src/Main.c
 * in      _hv_cmd_exec -> L37
 * start   0x0x20001748
 * len     128
 * ------+------------------------------------------------
 *         00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
 * ------+------------------------------------------------
 *  0000 : 0d f0 01 63 00 00 00 00 18 00 00 00 b8 3e 00 00       ...c.........>..
 *  0010 : bc 3e 00 00 24 1f 00 00 0d f0 01 81 00 00 00 00       .>..$...........
 *  0020 : 30 00 00 00 08 3d 00 00 dc 3e 00 00 78 1f 00 00       0....=...>..x...
 *  0030 : 0d f0 01 c8 18 00 00 00 48 00 00 00 20 3f 00 00       ........H....?..
 *  0040 : 28 3f 00 00 ea 1e 00 00 0d f0 00 23 30 00 00 00       (?.........#0...
 *  0050 : ac 68 00 00 d5 a7 bd b7 e8 0b e6 d5 c3 0e c9 42       .h.............B
 *  0060 : 88 7e 48 31 52 4f 38 8d 1c e4 e0 9d bf f9 55 d9       .~H1RO8.......U.
 *  0070 : 0d 0e a4 36 e1 61 c7 ec 0a 67 35 48 cf d7 1b 5c       ...6.a...g5H...\
 * ------+------------------------------------------------
 * 
 */
#ifndef HEXVIEW_H
#define HEXVIEW_H

#include "hexview_conf.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"
#include "ctype.h"

/**
 * @brief 颜色码 
 * @note 遵循 VT100 标准
 */
typedef enum hv_color
{
    Hv_Color_Black = 30,    // 黑色
    Hv_Color_Red,           // 红色
    Hv_Color_Green,         // 绿色
    Hv_Color_Yellow,        // 黄色
    Hv_Color_Blue,          // 蓝色
    Hv_Color_Magenta,       // 品红
    Hv_Color_Cyan,          // 青涩
    Hv_Color_White,         // 白色
} hv_color_t;

/**
 * @brief 类型声明
*/
typedef unsigned char hv_uint8_t;
typedef unsigned short hv_uint16_t;
struct hexview;
typedef struct hexview* hexview_t;
typedef void (*print_fn_t)(void* buf, int len);
typedef hv_color_t (*mark_fn_t)(hv_uint8_t ch);

/**
 * @brief hexview 类
 */
struct hexview
{
    const char* name;   // 对象名称
    print_fn_t print;   // 打印函数
    mark_fn_t mark;     // 颜色标记函数
};

/**
 * @brief 从地址 start 开始打印 len 长度的十六进制视图
 * @note 推荐使用此宏进行打印
 * @see hv_print_chart
 */
#define hv_print(hv, start, len)            hv_print_chart(hv, #start, __FILE__, __func__, __LINE__, (void*) start, len, NULL)
#define hv_printn(hv, start, len, note)     hv_print_chart(hv, #start, __FILE__, __func__, __LINE__, (void*) start, len, note)


int hv_init(hexview_t hv, const char* name, print_fn_t print, mark_fn_t mark);
int hv_print_chart( hexview_t hv, 
                    const char* target, 
                    const char* file_name,
                    const char* fn_name,
                    const int line,
                    void* start, 
                    int len, 
                    const char* note);

#endif  // HEXVIEW_H