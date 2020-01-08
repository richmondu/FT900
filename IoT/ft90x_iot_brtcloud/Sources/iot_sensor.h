#ifndef SOURCES_IOT_SENSOR_H_
#define SOURCES_IOT_SENSOR_H_


void pots_class_task    ( void *pvParameters );
void temp_class_task    ( void *pvParameters );

void beep_class_task    ( void *pvParameters );
void light_class_task   ( void *pvParameters );
void display_class_task ( void *pvParameters );


#endif /* SOURCES_IOT_SENSOR_H_ */
