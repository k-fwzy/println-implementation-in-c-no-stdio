#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <float.h>
#include <limits.h>

typedef enum { T_INT, T_UINT,
                T_ILL, T_ULL,
                T_D, T_STR, T_PTR } Tag;
typedef struct { Tag tag;
                union { long long ill;
                        unsigned long long ull;
                        double d;
                        const char* str;
                        const void* ptr; } val; } Box;
static Box
Box_int(
    int x
){ return (Box){ T_INT, .val.ill = x }; }
static Box
Box_ill(
    long long x
){ return (Box){ T_ILL, .val.ill = x }; }
static Box
Box_uint(
    unsigned x
){ return (Box){ T_UINT, .val.ull = x }; }
static Box
Box_ull(
    unsigned long long x
){ return (Box){ T_ULL, .val.ull = x }; }
static Box
Box_d(
    double x
){ return (Box){ T_D, .val.d = x }; }
static Box
Box_str(
    const char* x
){ return (Box){ T_STR, .val.str = x }; }
static Box
Box_ptr(
    const void* x
){ return (Box){ T_PTR, .val.ptr = x }; }

#define BOX(x) _Generic((x),        \
    char                : Box_int,  \
    short               : Box_int,  \
    int                 : Box_int,  \
    long                : Box_ill,  \
    long long           : Box_ill,  \
    unsigned char       : Box_uint, \
    unsigned short      : Box_uint, \
    unsigned            : Box_uint, \
    unsigned long       : Box_ull,  \
    unsigned long long  : Box_ull,  \
    float               : Box_d,    \
    double              : Box_d,    \
    char*               : Box_str,  \
    const char*         : Box_str,  \
    default             : Box_ptr   \
)(x)
#define GET(_1,_2,_3,_4,_5,_6,_7,_8,NAME,...) NAME
#define MAP1(f,a) f(a)
#define MAP2(f,a,b) f(a),\
                    f(b)
#define MAP3(f,a,b,c) f(a),\
                        f(b),\
                        f(c)
#define MAP4(f,a,b,c,d) f(a),\
                        f(b),\
                        f(c),\
                        f(d)
#define MAP5(f,a,b,c,d,e) f(a),\
                            f(b),\
                            f(c),\
                            f(d),\
                            f(e)
#define MAP6(f,a,b,c,d,e,g) f(a),\
                            f(b),\
                            f(c),\
                            f(d),\
                            f(e),\
                            f(g)
#define MAP7(f,a,b,c,d,e,g,h) f(a),\
                                f(b),\
                                f(c),\
                                f(d),\
                                f(e),\
                                f(g),\
                                f(h)
#define MAP8(f,a,b,c,d,e,g,h,i) f(a),\
                                f(b),\
                                f(c),\
                                f(d),\
                                f(e),\
                                f(g),\
                                f(h),\
                                f(i)
#define MAP(f,...) GET(__VA_ARGS__, \
                            MAP8,   \
                            MAP7,   \
                            MAP6,   \
                            MAP5,   \
                            MAP4,   \
                            MAP3,   \
                            MAP2,   \
                            MAP1)(f, __VA_ARGS__)
#define countof(arr)\
    sizeof arr / sizeof arr[0]

constexpr unsigned short
    bufsize = 1024;
typedef struct { char buf[bufsize];
                unsigned int cursor,
                    total_printed; } Buffer;
static void
Buffer_flush(
    Buffer* pbuf
){
    if(pbuf->cursor > 0){
        write(1, //stdout
            pbuf->buf,
            pbuf->cursor);
        pbuf->total_printed += pbuf->cursor;
        pbuf->cursor = 0;
    }
}

static inline void
Buffer_emitChar(
    Buffer* pbuf,
    char c
){
    pbuf->buf[pbuf->cursor++] = c;
    if(pbuf->cursor >= bufsize ||
            c == '\n')
        Buffer_flush(pbuf);
}

static void
Buffer_emitStr(
    Buffer* pbuf,
    const char* str
){
    if(!str)
        str = "((null))";
    while(*str)
        Buffer_emitChar(pbuf,
                        *str++);
}

static void
Buffer_emitUnsigned(
    Buffer* pbuf,
    unsigned long long v,
    unsigned int base,
    int upper
){
    const char*
        digits = upper
                    ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    : "0123456789abcdefghijklmnopqrstuvwxyz";

    if(base < 2 ||
        base > 36)
    {
        Buffer_emitStr(pbuf,
                "((bad base))");
        return;
    }

    constexpr unsigned char
        tmpsize = sizeof v * CHAR_BIT + 1;
    char tmp[tmpsize] = {0};
    char* p = tmp + sizeof tmp;
    *p = '\0';

    do{
        *--p = digits[v % base];
        v /= base;
    }while(v);
    Buffer_emitStr(pbuf, p);
}

static void
Buffer_emitSigned(
    Buffer* pbuf,
    long long n,
    int base,
    int upper
){
    unsigned long long u = 0;
    if(n < 0 && base == 10){
        Buffer_emitChar(pbuf, '-');
        u = -(unsigned long long)n;
    }else
        u = (unsigned long long)n;
    Buffer_emitUnsigned(pbuf,
                    u,
                (unsigned) base,
                    upper);
}

static void
Buffer_emitFloat(
    Buffer* pbuf,
    double f,
    int afterpoint
){
    if(afterpoint < 0)
        afterpoint = 6;
    if(afterpoint > 17)
        afterpoint = 17;

    if(f != f){
        Buffer_emitStr(pbuf,
                        "NaN");
        return;
    }

    if(f > DBL_MAX){
        Buffer_emitStr(pbuf,
                        "inf");
        return;
    }

    if(f < -DBL_MAX){
        Buffer_emitStr(pbuf,
                        "-inf");
        return;
    }

    if(f < 0.0 || 
        (f == 0.0 && 
            1.0 / f < 0.0))
    {
        Buffer_emitChar(pbuf, '-');
        f = -f;
    }

    double half = 0.5;
    for(int i = 0; i < afterpoint; i++)
        half /= 10.0;
    f += half;

    if(f >= 1.8446744073709552e19){
        Buffer_emitUnsigned(pbuf,
                        ULLONG_MAX,
                        10, 0);
        return;
    }

    unsigned long long ipart =
        (unsigned long long)f;
    Buffer_emitUnsigned(pbuf,
                    ipart,
                        10, 0);
    f -= (double)ipart;

    if(afterpoint == 0)
        return;

    Buffer_emitChar(pbuf, '.');
    for(int i = 0; i < afterpoint; i++){
        f *= 10.0;
        int digit = (int) f;
        if(digit > 9)
            digit = 9;
        if(digit < 0)
            digit = 0;
        Buffer_emitChar(pbuf, (char)('0' + digit));
        f -= (double)digit;
    }
}

static void
Buffer_emitBox(
    Buffer* pbuf,
    const Box* b,
    int base,
    int upper,
    int afterpoint
){
    switch(b->tag){
        case T_INT: case T_ILL:
            Buffer_emitSigned(pbuf,
                                b->val
                                  .ill,
                            base,
                            upper);
            break;
        case T_UINT: case T_ULL:
            Buffer_emitUnsigned(pbuf,
                                b->val
                                  .ull,
                            (unsigned)base,
                                upper);
            break;
        case T_D: Buffer_emitFloat(pbuf,
                                    b->val
                                      .d,
                                    afterpoint);
            break;
        case T_STR: Buffer_emitStr(pbuf,
                                    b->val
                                      .str);
            break;
        case T_PTR:
            Buffer_emitStr(pbuf, "0x");
            Buffer_emitUnsigned(pbuf,
                                (uintptr_t)b->val
                                             .ull,
                                16, 0);
            break;
    }
}
static void
Error(
    const char *msg
){
    size_t n = 0;
    while(msg[n++]);
    write(2, msg, n);
}

int
printlnf_impl(
    const char* fmt,
    size_t argc,
    Box* argv
){
    if(!fmt)
        return -1;

    Buffer buf = {0};
    size_t cursor = 0,
        next = 0,
        placeholders = 0;
    int errs = 0;

    while(fmt[cursor]){
        if(fmt[cursor] == '{' &&
            fmt[cursor + 1] != '{')
        {
            placeholders++;
            int idx = cursor + 1;

            int base = 10,
                afterpoint = -1,
                upper = 0;

            while(fmt[idx] &&
                fmt[idx] != '}')
            {
                if(fmt[idx] == ':'){
                    idx++;
                    switch(fmt[idx]){
                        case '.':
                            idx++;
                            base = 10; 
                            int prec = 0,
                                got = 0;
                            while(fmt[idx] >= '0' && 
                                    fmt[idx] <= '9')
                            {
                                prec = prec * 10 + (fmt[idx] - '0');
                                idx++;
                                got = 1;
                            }

                            if(got)
                                afterpoint = prec;
                            break;
                        case 'b': base = 2;
                                    idx += 2; break;
                        case 'o': base = 8;
                                    idx += 2; break;
                        case 'x': base = 16; 
                                    idx += 2; break;
                        case 'X': base = 16;
                                    upper = 1;
                                    idx += 2; break;
                        case 'p': base = 16;
                                    idx += 2; break;
                        default: idx += 1; break;
                    }
                }else
                    idx++;
            }
            cursor = idx + 1;

            if(fmt[idx] != '}'){
                Error("println: unterminated '{'\n");
                break;
            }

            if(next < argc){
                Buffer_emitBox(&buf,
                            &argv[next++],
                            base,
                            upper,
                        afterpoint);
            }else{
                Buffer_emitStr(&buf,
                            "((missing))");
                errs++;
            }
        }else if(fmt[cursor] == '{' &&
                    fmt[cursor + 1] == '{')
        {
            Buffer_emitChar(&buf,
                                '{');
            cursor += 2;
        }else if(fmt[cursor] == '}' &&
                fmt[cursor + 1] == '}')
        {
            Buffer_emitChar(&buf,
                                '}');
            cursor += 2;
        }else
            Buffer_emitChar(&buf, fmt[cursor++]);
    }

    if(next < argc){
        Buffer_flush(&buf);
        Error("println: extra argument(s) ignored\n");
        errs++;
    }

    Buffer_emitChar(&buf, '\n');
    Buffer_flush(&buf);
    return errs
            ? -1
            : buf.total_printed;
}

#define println(str)\
    printlnf_impl((str), 0, NULL);
#define printlnf(fmt, ...)                                                  \
    printlnf_impl((fmt),                                                    \
                    countof((Box[]){ __VA_OPT__(MAP(BOX, __VA_ARGS__)) }),  \
                    (Box[]){ __VA_OPT__(MAP(BOX, __VA_ARGS__)) });

int
main(
    void
){
    const char* msg = "hello world";
    printlnf("{} {{ {} }}\n {:.4} \n\t{}", msg, 21, 3.1415);
    println("we are charlie kirk");

    return 0;
}
