#ifndef STATUS_H
#define STATUS_H

typedef struct KettleStatus_t {
    float targetTemperature = 0;
    float currentTemperature = 0;
    bool isHeating = false;
    bool isOn = false;
    
} KettleStatus_t;

typedef struct Point_t{
    int x = 0;
    int y = 0;
} Point_t;

#endif //STATUS_H