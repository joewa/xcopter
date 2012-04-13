/*
Der SpaceMouse-Treiber läuft in einem eigenen Thread. Dieser hat seine eigene (X11-) Eventloop.
Der zyklische Reglertakt fragt dann in jedem Schritt die aktuellen Koordinaten ab und ob eine Taste gedrückt wurde.
Eventueller Nachteil: viele Threads sind suboptimal, weil sie bei einem nicht-echtzeit-betriebssystem den zeitkritschen
Regler-Thread unterbrechen könnten.
*/
#include <string.h>
#include <math.h>
#include <stdio.h>

#define MagellanDEBUG
#define IBM

#include "qmagellan.h"
extern "C" {
 #include <X11/X.h>
 #include <X11/Xlib.h>
 #include <X11/Xutil.h>
 #include <X11/Xatom.h>
 //#include <X11/keysym.h>
 #include "xdrvlib.h"
}
using namespace std;


SpaceMouseData QMagellanThread::getData()
{
	SpaceMouseData data;
	data.x = x_; data.y = y_; data.z = z_;
	data.a = a_; data.b = b_; data.c = c_;
	data.b0 = 0; data.b1 = 0;
	// b0 und b1 zurücksetzen wenn gelesen
	return data;
}


double MagellanSensitivity = 1.0;

void QMagellanThread::run()
{
	QTextStream out(stdout);

Display *display;
Window root, window;

int screennumber,width,height;
XSizeHints *sizehints;
XWMHints *wmhints;
XClassHint *classhints;

char *WinName = "Magellan 3D Controller";
XTextProperty WindowName;
GC wingc;
XGCValues xgcvalues;

XEvent report;
MagellanFloatEvent MagellanEvent;

XComposeStatus compose;
KeySym keysym;

int MagellanDemoEnd = FALSE;
char MagellanBuffer[ 256 ];


/****************** Open a Window ******************************************/
 sizehints  = XAllocSizeHints();
 wmhints    = XAllocWMHints();
 classhints = XAllocClassHint();
 if ( (sizehints==NULL) || (wmhints==NULL) || (classhints==NULL) )
  {
   fprintf( stderr, "Can't allocate memory! Exit ... \n" );
   exit( -1 );
  };

 display = XOpenDisplay( NULL );
 if ( display == NULL )
  {
   fprintf( stderr, "Can't open display! Exit ... \n");
   exit( -1 );
  };

 screennumber = DefaultScreen(display);
 width  = DisplayWidth(display,screennumber);
 height = DisplayHeight(display,screennumber);
 root   = DefaultRootWindow(display);
 window = XCreateSimpleWindow( display, root, 0,0, /*width*/300,/*height*/40, 20,
			       BlackPixel(display,screennumber),
			       WhitePixel(display,screennumber) );

 printf("Magellan: xapp.c\n" );
 printf("Magellan Root Window=%08X \nMagellan Application Window=%08X \n\n",
 	 root, window );

 XStringListToTextProperty( &WinName, 1, &WindowName );

 wmhints->initial_state = NormalState;
 wmhints->input = TRUE;
 wmhints->flags = StateHint | InputHint;

 classhints->res_name = "Magellan 3D Controller";
 classhints->res_class = "BasicWindow";
 XSetWMProperties( display, window, &WindowName, NULL, /*argv*/0,
		   /*argc*/0, sizehints, wmhints, classhints );

 XMapWindow( display, window ); 
 xgcvalues.foreground = BlackPixel( display, 0 );
 xgcvalues.background = WhitePixel( display, 0 );
 wingc = XCreateGC( display, window, GCForeground | GCBackground, &xgcvalues );

 /************************* Create 3D Event Types ***************************/
 if ( !MagellanInit( display, window ) )
  {
   fprintf( stderr, "No driver is running. Exit ... \n" );
   exit(-1);
  };

 /************************* Main Loop ***************************************/
 XSelectInput( display, window, KeyPressMask | KeyReleaseMask );

 while( MagellanDemoEnd == FALSE )
  {
   XNextEvent( display, &report );
   switch( report.type )
    {
     case KeyRelease : /* ... */
                       break;
 
     case KeyPress :
      XLookupString( (XKeyEvent*)&report, MagellanBuffer,
     		     sizeof(MagellanBuffer), &keysym, &compose );
      MagellanDemoEnd = keysym == XK_Escape;
      break;
			
     case ClientMessage :
      switch( MagellanTranslateEvent( display, &report, &MagellanEvent, 1.0, 1.0 ) )
       {
        case MagellanInputMotionEvent :
         MagellanRemoveMotionEvents( display );
         x_ = MagellanEvent.MagellanData[ MagellanX ];
         y_ = MagellanEvent.MagellanData[ MagellanY ];
         z_ = MagellanEvent.MagellanData[ MagellanZ ];
         a_ = MagellanEvent.MagellanData[ MagellanA ];
         b_ = MagellanEvent.MagellanData[ MagellanB ];
         c_ = MagellanEvent.MagellanData[ MagellanC ];
         sprintf( MagellanBuffer, 
            "x=%+5.0lf y=%+5.0lf z=%+5.0lf a=%+5.0lf b=%+5.0lf c=%+5.0lf   ",
		  x_, y_, z_, a_, b_, c_ );

         XClearWindow( display, window );
	 XDrawString( display, window, wingc, 10,20,
	  	       MagellanBuffer, (int)strlen(MagellanBuffer) );
         XFlush( display );
	 break;
	 
        case MagellanInputButtonPressEvent :
         sprintf( MagellanBuffer, "Button pressed [%c]  ",
		   MagellanEvent.MagellanButton ==  9 ? '*' :
		   MagellanEvent.MagellanButton == 10 ? '+' :
		   MagellanEvent.MagellanButton == 11 ? '-' :
		             '0'+MagellanEvent.MagellanButton );
         XClearWindow( display, window );
         XDrawString( display, window, wingc, 10,40,
                       MagellanBuffer, (int)strlen(MagellanBuffer) );
         XFlush( display );

	 switch ( MagellanEvent.MagellanButton )
	  {
	   case 5: MagellanApplicationSensitivity( display, MagellanSensitivity / 2.0 ); break;
	   case 6: MagellanApplicationSensitivity( display, MagellanSensitivity * 2.0 ); break;
	   case 7: MagellanApplicationSensitivity( display, 1.0 ); break;
	  };

	 switch( MagellanEvent.MagellanButton )
	  {
 	   case 5: MagellanSensitivity = MagellanSensitivity <= 1.0/32.0 ? 1.0/32.0 : MagellanSensitivity/2.0; break;
 	   case 6: MagellanSensitivity = MagellanSensitivity >= 32.0 ? 32.0 : MagellanSensitivity*2.0; break;
	   case 7: MagellanSensitivity = 1.0;  break;
	  };

	 switch( MagellanEvent.MagellanButton )
	  {
	   case 5:
	   case 6:
	   case 7:
	   printf("Application Sensitivity = %lf \n", MagellanSensitivity );
	   break;
	  };
	 break;
	  
        case MagellanInputButtonReleaseEvent :
         sprintf( MagellanBuffer, "Button released [%c] ",
		   MagellanEvent.MagellanButton == 9  ? '*' :
		   MagellanEvent.MagellanButton == 10 ? '+' :
		   MagellanEvent.MagellanButton == 11 ? '-' :
		             '0'+MagellanEvent.MagellanButton ); 
         XClearWindow( display, window );
         XDrawString( display, window, wingc, 10,40,
                       MagellanBuffer, (int)strlen(MagellanBuffer) );
         XFlush( display );
         break;
	
       default : /* another ClientMessage event */
 	 break;
       };
      break;
     };
  };

 MagellanClose( display );
 XFree( sizehints );
 XFree( wmhints );
 XFree( classhints );
 XFreeGC( display, wingc );
 XDestroyWindow( display, window );
 XCloseDisplay( display );
}
