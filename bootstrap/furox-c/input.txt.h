#ifndef FUROX_C_INPUT_TXT_H
#define FUROX_C_INPUT_TXT_H



typedef struct std_String
{
    char* buffer;
    usize length;
} std_String;

char* std_String_Buffer(std_String* str);usize std_String_Length(std_String* str);



b8 Math_IsEven(usize n);



void exportedFunc(void);




#endif
