# 一、简介
Hexview 是一种适用于单片机平台的十六进制视图打印库。
Hexview 一般用于在单片机中通过串口打印指定长度的内存的十六进制视图表，帮助使用者实时监控指定内存的数据情况。

# 二、使用方式
## 2.1 struct hexview 对象
Hexview 采用简单的面向对象的编程方式，初始化时需要创建 struct hexview 对象，并提供必要的回调函数。
调用 hv_init() 对对象进行初始化。
```c
static struct hexview hv;

static void _hv_write(void* buf, int len)
{
    //...
}

static hv_color_t _hv_mark(uint8_t ch)
{
    //...
}

int mdw_hexview_init(void)
{
    hv_init(&hv, "hv", _hv_write, _hv_mark);
    return 0;
}

```

## 2.2 打印函数 print_fn_t 
在 2.1 中，我们需要向 hv_init() 提供打印函数 print_fn_t，之后 Hexview 便会通过用户指定的函数进行输出。函数的实现可以参考以下代码：
```c
#include "stdlib.h"

static void _hv_write(void* buf, int len)
{
    fwrite(buf, sizeof(char), len, stdout);
}
```

## 2.3 颜色标记函数 
在 2.1 中，我们需要向 hv_init() 提供打印函数 mark_fn_t。当用户向 hv_init() 提供的标记函数为 NULL，Hexview 将打印普通颜色的字符；如果用户提供了颜色标记函数，且用户终端支持 VT100 颜色码，则 Hexview 就可以依据用户提供的颜色标记函数对特定的数值做彩色打印。
在下面例子中，_hv_mark() 获取此时需要打印的数值 ch，依据 ch 的值打印不同的颜色，比如对应到 ASCII 中，字母显示为青色，数字显示为黄色，0x00 显示为黑色，0xf0 和 0x0d 显示为红色，0x01 显示为绿色，其他字符显示为白色。
```c
static hv_color_t _hv_mark(uint8_t ch)
{
    if(isalnum(ch))
    {
        if(isalpha(ch))
            return Hv_Color_Cyan;
        else
            return Hv_Color_Yellow;
    }

    switch(ch)
    {
        case 0x00: return Hv_Color_Black;
        case 0xf0:
        case 0x0d: return Hv_Color_Red;
        case true: return Hv_Color_Green;
    }
    return Hv_Color_White;
}
```

## 2.4 API 接口函数
Hexview 提供 hv_init() 和 hv_print_chart()，分别用于初始化和视图打印。
```c
int hv_init(hexview_t hv, const char* name, print_fn_t print, mark_fn_t mark);
int hv_print_chart( hexview_t hv, 
                    const char* target, 
                    const char* file_name,
                    const char* fn_name,
                    const int line,
                    void* start, 
                    int len, 
                    const char* note);
```

## 2.5 推荐使用的宏函数
由于 hv_print_chart() 参数太多，Hexview 库提供了两个宏函数来简洁的使用 Hexview 的打印功能。
一般情况下，也推荐使用这两个宏函数来进行视图的打印。
```c
#define hv_print(hv, start, len)            hv_print_chart(hv, #start, __FILE__, __func__, __LINE__, (void*) start, len, NULL)
#define hv_printn(hv, start, len, note)     hv_print_chart(hv, #start, __FILE__, __func__, __LINE__, (void*) start, len, note)
```
## 2.6 进一步封装
由于 Hexview 的设计之初，为支持多种输出方式采用了面向对象的设计，即每次使用打印函数，都需要提供 struct hexview 对象。但这种方式对应跨源文件使用不太友好，因为很多时候我们仅使用串口来打印结果。
这里推荐通过 API 封装的进行提供 struct hexivew 对象，而非直接 extern，例如：
在新创建的 my_hv.c:
```c
static struct hexview hv;
static void _hv_write(void* buf, int len)
{
    //...
}

static hv_color_t _hv_mark(uint8_t ch)
{
    //...
}

int mdw_hexview_init(void)
{
    hv_init(&hv, "hv", _hv_write, _hv_mark);
    return 0;
}

struct hexview* mdw_hexview_get_act(void)
{
    return & hv
}
```
在新创建的 my_hv.h 声明自定义的接口
```c
int mdw_hexview_init(void);
struct hexview* mdw_hexview_get_act(void);
```
在 main 中进行打印
```c
#include "my_hv.h"
static char buf[32] = {0};
int main(void)
{
    // 忽略无关内容
    struct hexview* hv = mdw_hexview_get_act();
    hv_print(hv, buf, sizeof(buf));
    return 0;
}
```
# 在支持 VT100 颜色码的终端的串口打印效果
使用 MobaXterm 的彩色打印效果
![image](https://github.com/SouthernSandbox/hexview/assets/75567944/6ac7c453-b3d0-4230-882e-a86e3bd478b1)

