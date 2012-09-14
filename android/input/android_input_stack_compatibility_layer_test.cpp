#include "input_stack_compatibility_layer.h"
#include "input_stack_compatibility_layer_flags.h"

#include <signal.h>

#include <stdio.h>

namespace
{

bool g_stop = false;

void signal_handler(int)
{
    g_stop = true;
}

void on_new_event(Event* event, void* context)
{
    printf("%s", __PRETTY_FUNCTION__);

    printf("\tEventType: %d \n", event->type);
    printf("\tdevice_id: %d \n", event->device_id);
    printf("\tsource_id: %d \n", event->source_id);
    printf("\taction: %d \n", event->action);
    printf("\tflags: %d \n", event->flags);
    printf("\tmeta_state: %d \n", event->meta_state);
    switch(event->type)
    {
        case MOTION_EVENT_TYPE:
            
            if (ISCL_MOTION_EVENT_ACTION_DOWN == (~ISCL_MOTION_EVENT_ACTION_POINTER_INDEX_MASK & event->action))
                printf("ISCL_MOTION_EVENT_ACTION_DOWN\n");
            if (ISCL_MOTION_EVENT_ACTION_UP == (~ISCL_MOTION_EVENT_ACTION_POINTER_INDEX_MASK & event->action))
                printf("ISCL_MOTION_EVENT_ACTION_UP\n");
            if (ISCL_MOTION_EVENT_ACTION_MOVE == (~ISCL_MOTION_EVENT_ACTION_POINTER_INDEX_MASK & event->action))
                printf("ISCL_MOTION_EVENT_ACTION_MOVE\n");
            
            printf("\tdetails.motion.edge_flags: %d \n", event->details.motion.edge_flags);

            if (ISCL_MOTION_EVENT_EDGE_FLAG_TOP & event->details.motion.edge_flags)
                printf("\tdetails.motion.edge_flags: Top edge \n");
            if (ISCL_MOTION_EVENT_EDGE_FLAG_BOTTOM & event->details.motion.edge_flags)
                printf("\tdetails.motion.edge_flags: Bottom edge \n");
            if (ISCL_MOTION_EVENT_EDGE_FLAG_LEFT & event->details.motion.edge_flags)
                printf("\tdetails.motion.edge_flags: Left edge \n");
            if (ISCL_MOTION_EVENT_EDGE_FLAG_RIGHT & event->details.motion.edge_flags)
                printf("\tdetails.motion.edge_flags: Right edge \n");

            printf("\tdetails.motion.event_time: %d\n", event->details.motion.event_time);
            printf("\tdetails.motion.pointer_coords.x: %f\n", event->details.motion.pointer_coordinates[0].x);
            printf("\tdetails.motion.pointer_coords.y: %f\n", event->details.motion.pointer_coordinates[0].y);
            break;
        default:
            break;
    }
}

}

int main(int argc, char** argv)
{
    g_stop = false;
    signal(SIGINT, signal_handler);
    
    AndroidEventListener listener;
    listener.on_new_event = on_new_event;
    listener.context = NULL;

    InputStackConfiguration config = {
  enable_touch_point_visualization : true,
  default_layer_for_touch_point_visualization : 10000
    };

    android_input_stack_initialize(&listener, &config);
    android_input_stack_start_waiting_for_flag(&g_stop);
    
    android_input_stack_stop();
    android_input_stack_shutdown();
}

