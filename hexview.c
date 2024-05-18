/**
 * @file hexview.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-08
 * 
 * @copyright Copyright (c) 2024
 */
#include "hexview.h"
#include "stdio.h"

/**
 * @brief 格式化输出
 * @param hv 
 * @param fmt 
 * @param ... 
 */
static void _hv_printf(hexview_t hv, const char* fmt, ...)
{
    char buf[128] = {0};
    va_list list;
    va_start(list, fmt);
    vsprintf(buf, fmt, list);
    hv->print(buf, strlen(buf));
    va_end(list);
}

/**
 * @brief 绘制重复字符
 * @note  例如，若 ch = 1, len = 4, repeat = 3，则函数效果为：
 *          1111 1111 1111 
 * @param hv 
 * @param len 字符连续打印的长度
 * @param repeat 重复次数
 */
static void _draw_repeat_char(hexview_t hv, char ch, int len, int repeat)
{
    while(repeat--)
    {
        int i = 0;
        for(i = 0; i < len; ++i)
            hv->print(&ch, sizeof(char));
    }
}

/**
 * @brief 绘制图表基本信息
 * @param hv 
 * @param target 
 * @param file_name 
 * @param fn_name 
 * @param line 
 * @param start 
 * @param len 
 */
static void _draw_basic_info( hexview_t hv, 
                              const char* target, 
                              const char* file,
                              const char* fn,
                              const int line,
                              void* start, 
                              int len, 
                              const char* note)
{
    _hv_printf(hv, "\r\n[  Hexview  ]\r\n");
    _hv_printf(hv, "target  %s\r\n", target);
    _hv_printf(hv, "file    %s\r\n", file);
    _hv_printf(hv, "in      %s -> L%d\r\n", fn, line);
    _hv_printf(hv, "start   %p\r\n", start);
    _hv_printf(hv, "len     %d\r\n", len);
    if(note)
        _hv_printf(hv, "note    %s\r\n", note);
}

/**
 * @brief 绘制水平分割器
 * @param hv 
 */
static void _draw_hor_splitter(hexview_t hv)
{
    _draw_repeat_char(hv, '-', HV_CHART_HIGH_ORDER_LEN + 2, 1);
    _draw_repeat_char(hv, '+', 1, 1);
    _draw_repeat_char(hv, '-', 3, 16);
    hv->print("\r\n", strlen("\r\n"));
}

/**
 * @brief 绘制顶部信息栏
 * @param hv 
 * @param start 
 */
static void _draw_top_bar(hexview_t hv)
{
    register hv_uint16_t addr;

    _draw_hor_splitter(hv);

    /** 绘制高位偏移量留白 **/
    _draw_repeat_char(hv, ' ', HV_CHART_HIGH_ORDER_LEN + 3, 1);

    /** 绘制地址列 **/
    for(addr = 0; addr <= 0x0f; ++addr)
        _hv_printf(hv, " %02x", (hv_uint16_t)addr);
    hv->print("\r\n", strlen("\r\n"));

    _draw_hor_splitter(hv);
}

/**
 * @brief 绘制图表的单行十六进制数据
 * @param hv 
 * @param start 
 * @param row 
 * @param cols 
 */
static void _draw_row_hex(hexview_t hv, const void* const start, hv_uint8_t row, hv_uint8_t cols)
{
    hv_uint8_t col;
    _hv_printf(hv, " %04x :", (hv_uint16_t)(row << 4)); // 表头

    /** 绘制十六进制数据 **/
    for (col = 0; col < cols; col++)
    {
        hv_uint8_t ch = (hv_uint8_t)((hv_uint8_t *)start)[(row << 4) | col];
        if(hv->mark == NULL)
            _hv_printf(hv, " %02x", (hv_uint16_t)ch);
        else 
        {
            hv_color_t color = hv->mark(ch);
            _hv_printf(hv, "\033[1;%dm %02x\033[0m", (hv_uint16_t)color, (hv_uint16_t)ch);
        }
    }
    _draw_repeat_char(hv, ' ', (0x10 - cols)*3 + 7, 1);

    /** 绘制 ascii 可打印字符 **/
    for (col = 0; col < cols; col++)
    {
        hv_uint8_t ch = ((hv_uint8_t *)start)[(row << 4) | col];
        if (isgraph(ch))
        {
            if(hv->mark == NULL)
                _hv_printf(hv, "%c", ch);
            else 
            {
                hv_color_t color = hv->mark(ch);
                _hv_printf(hv, "\033[1;%dm%c\033[0m", (hv_uint16_t)color, (hv_uint16_t)ch);
            }
        }
        else
        {
            if(hv->mark == NULL)
                _hv_printf(hv, ".", ch);
            else 
            {
                hv_color_t color = hv->mark(ch);
                _hv_printf(hv, "\033[1;%dm.\033[0m", (hv_uint16_t)color);
            }
        }
    }
    _hv_printf(hv, "\r\n");
}

/**
 * @brief 绘制数据图表
 * @param hv 
 * @param start 
 * @param len 
 */
static void _draw_chart(hexview_t hv, void* start, int len)
{
    hv_uint8_t rows = (hv_uint8_t)(len >> 4);
    hv_uint8_t row = 0;
    hv_uint8_t hole = (hv_uint8_t)len - (rows << 4); 

    for( row = 0; row < rows; row++ )
        _draw_row_hex(hv, start, row, 0x10);

    if(hole)
        _draw_row_hex(hv, start, row, hole);

    _draw_hor_splitter(hv);
}

/**
 * @brief 初始化 hexview 对象
 * @param hv 
 * @param name 
 * @param print 
 * @param mark 
 * @return int 
 */
int hv_init(hexview_t hv, const char* name, print_fn_t print, mark_fn_t mark)
{
    memset(hv, 0, sizeof(struct hexview));
    hv->name = name;
    hv->print = print;
    hv->mark = mark;
    return 0;
}

/**
 * @brief 打印十六进制视图
 * @param hv struct hexview 对象
 * @param target 目标名称
 * @param file_name 所在文件的名称
 * @param fn_name 调用此函数的函数名
 * @param line 本函数调用时所在文件的行数
 * @param start 视图打印的起始地址
 * @param len 被打印的长度
 * @return int 
 */
int hv_print_chart( hexview_t hv, 
                    const char* target, 
                    const char* file,
                    const char* fn,
                    const int line,
                    void* start, 
                    int len, 
                    const char* note)
{
    _draw_basic_info(hv, target, file, fn, line, start, len, note);
    _draw_top_bar(hv);
    _draw_chart(hv, start, len);
    return 0;
}
