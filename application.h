#ifndef __APP_H__
#define __APP_H__

void pico_application_stop(void);
extern void* pico_application_data;

typedef struct _application {
    char* name;
    int data_size;
    void (*start)(void);
    void (*run)(void);
    void (*stop)(void);
} application_t;

#endif // __APP_H__