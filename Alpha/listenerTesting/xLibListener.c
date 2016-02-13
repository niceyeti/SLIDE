#include "X11/Xlib.h"
#include "stdio.h"

int main(int argc, char *argv[])
{
    Display *display;
    Window root_window;
    XEvent event;

    display = XOpenDisplay(0);
    root_window = XRootWindow(display, 0);
    XSelectInput(display, root_window, PointerMotionMask );

    while(1) {
        XNextEvent( display, &event );
        switch( event.type ) {
            case MotionNotify:
                printf("x %d y %d\n", event.xmotion.x, event.xmotion.y );
                break;
        }
    }

    return 0;
}
