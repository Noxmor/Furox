#include "Furox.h"
#include "input.txt.h"
extern i32 putchar(i32 c);extern i32 printf(char* format, ...);
i32 global_var;


char* std_String_Buffer(std_String* str)
{
    return str->buffer;
}
usize std_String_Length(std_String* str)
{
    return str->length;
}


usize Math_add(usize a, usize b)
{
    return (a + b);
}

b8 Math_IsEven(usize n)
{
    if(((n % 2) == 0))
    {
        return 1;
    }
    else
    {
        return 0;
    }

}


enum
{
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY = 99,
    FRIDAY,
    SATURDAY,
    SUNDAY
};

typedef u8 Day;

void exportedFunc(void)
{
}

char getChar(void)
{
    return '#';
}

u8 VariadicFunc(usize x, ...)
{
    return 2;
}

usize Main(void)
{
    usize foo = 8;
    usize bar = 16777215;
    usize buzz;
    usize value = 0;
    char c = 'c';
    usize test = ((10 * 20) + 30);
    usize otherTest = (((1 * 2) + 3) * 4);
    usize moduloTest = (8 % 3);
    usize paranthesisTest = (((3 + (7 + 8)) * 9) + 11);
    usize otherParanthesisTest = ((3 + (1 + 2)) * (((1 + 2) + 3) + 4));
    usize multipleParanthesisTest = 69;
    usize anotherParanthesisTest = ((42 + 64) * 69);
    usize anotherInterestingTest = ((13 + (14 + 15)) * 16);
    usize unaryTest = (!42);
    usize otherUnaryTest = (((!(!1)) + (-1)) * (!3));
    isize negativeNumber = (-((-4) * (-3)));
    usize boolTestAnd = (1 && 0);
    usize boolTestOr = (1 || 0);
    usize boolTestNegation = (!1);
    usize binaryTest = ((~9) | 12);
    usize otherBinaryTest = (7 & 4);
    usize anotherBinaryTest = (5 ^ 2);
    usize comparisonTest = (0 == 1);
    usize nextCompareTest = (2 < 3);
    usize anotherCompareTest = ((7 >= 9) < (4 > 2));
    usize functionCallTest = (getChar() + Math_add(5, 6));
    usize var = 0;
    var = 1;
    std_String str;
    str.length = 0;
    str.length = (str.length + var);
    char buffer[16];
    void* nonsense = (((*buffer) + (&buffer)) + buffer[8]);
    printf("Hello, World!\n");
    std_String* str_pointer = (&str);
    str_pointer->length = 15;
    printf("String length: %zu\n", str.length);
    u8 kw_value = ((0 + 1) + 0);
    usize i = 0;
    while((i < 10))
    {
        i = (i + 1);
        printf("%zu\n", i);
    }

    i = 0;
    do
    {
        i = (i + 1);
        printf("%zu\n", i);
    }
    while((i < 10) );

    for((i = 0);(i < 10);(i = (i + 1)))
    {
        printf("%zu\n", i);
    }

    if(1)
    {
        printf("This should print!\n");
    }

    if(0)
    {
        printf("This should not print!\n");
    }

    Day day = FRIDAY;
    return ((Math_add(1, (2 * 3)) * VariadicFunc(1, 2, 3, 'A', "Test")) + day);
}

