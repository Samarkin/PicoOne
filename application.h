#ifndef __APP_H__
#define __APP_H__

typedef struct _application {
    char* name;
    void (*start)(void);
    void (*run)(void);
    void (*stop)(void);
} application_t;

#endif // __APP_H__