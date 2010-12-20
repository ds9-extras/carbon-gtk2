/*
* oxygenwindowmanager.cpp
* pass some window mouse press/release/move event actions to window manager
* -------------------
*
* Copyright (c) 2010 Cédric Bellegarde <gnumdk@gmail.com>
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
*
* Largely inspired from Qtcurve style
* Copyright (C) Craig Drummond, 2003 - 2010 craig.p.drummond@gmail.com
*
* This  library is free  software; you can  redistribute it and/or
* modify it  under  the terms  of the  GNU Lesser  General  Public
* License  as published  by the Free  Software  Foundation; either
* version 2 of the License, or( at your option ) any later version.
*
* This library is distributed  in the hope that it will be useful,
* but  WITHOUT ANY WARRANTY; without even  the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License  along  with  this library;  if not,  write to  the Free
* Software Foundation, Inc., 51  Franklin St, Fifth Floor, Boston,
* MA 02110-1301, USA.
*/

#include "oxygenwindowmanager.h"
#include "oxygenstyle.h"
#include "config.h"

#include <gdk/gdkx.h>

namespace Oxygen
{


    //_________________________________________________
    WindowManager::WindowManager( void ):
        _mode( Full ),
        _drag( false ),
        _dragDistance( 4 ),
        _dragDelay( 500 ),
        _widget( 0L ),
        _lastRejectedEvent( 0L ),
        _x(-1),
        _y(-1)
    { initializeBlackList(); }

    //_________________________________________________
    WindowManager::~WindowManager( void )
    {
        _map.disconnectAll();
        _map.clear();
    }

    //_________________________________________________
    void WindowManager::registerWidget( GtkWidget* widget )
    {

        if( _map.contains( widget ) || widgetIsBlackListed( widget ) ) return;

        // Window with no decorations (set by app), let window manage it self
        if( GTK_IS_WINDOW( widget ) && !gtk_window_get_decorated( GTK_WINDOW( widget ) ) ) return;

        // widgets used in tabs also must be ignored (happens, unfortunately)
        GtkWidget* parent( gtk_widget_get_parent( widget ) );
        if( GTK_IS_NOTEBOOK( parent ) && Gtk::gtk_notebook_is_tab_label( GTK_NOTEBOOK( parent ), widget ) )
        { return; }

        #if OXYGEN_DEBUG
        std::cout
            << "Oxygen::WindowManager::registerWidget -"
            << " " << widget << " (" << G_OBJECT_TYPE_NAME( widget ) << ")"
            << " " << gtk_widget_get_name( widget )
            << std::endl;
        #endif

        // Force widget to listen to relevant events
        gtk_widget_add_events( widget,
            GDK_BUTTON_RELEASE_MASK |
            GDK_BUTTON_PRESS_MASK   |
            GDK_LEAVE_NOTIFY_MASK   |
            GDK_BUTTON1_MOTION_MASK );

        // allocate new Data object
        Data& data( _map.registerWidget( widget ) );

        // connect signals
        if( _mode != Disabled ) connect( widget, data );

    }

    //_________________________________________________
    void WindowManager::unregisterWidget( GtkWidget* widget )
    {
        if( !_map.contains( widget ) ) return;

        #if OXYGEN_DEBUG
        std::cout << "Oxygen::WindowManager::unregisterWidget - " << widget << " (" << G_OBJECT_TYPE_NAME( widget ) << ")" << std::endl;
        #endif

        _map.value( widget ).disconnect( widget );
        _map.erase( widget );

        if( _widget == widget )
        {
            _widget = 0L;
            _x = -1;
            _y = -1;
            _drag = false;
        }

    }

    //_________________________________________________
    void WindowManager::setMode( WindowManager::Mode mode )
    {
        if( mode == _mode ) return;

        // connect/disconnect all data in map, based on new and old mode
        if( mode == Disabled ) { _map.disconnectAll(); }
        else if( _mode == Disabled )
        {
            DataMap<Data>::Map& map( _map.map() );
            for( DataMap<Data>::Map::iterator iter = map.begin(); iter != map.end(); ++iter )
            { connect( iter->first, iter->second ); }
        }

        // assign new mode
        _mode = mode;

    }

    //_________________________________________________
    gboolean WindowManager::wmStyleSet( GtkWidget* widget, GtkStyle* , gpointer data )
    {
        static_cast<WindowManager*>(data)->unregisterWidget( widget );
        return false;
    }

    //_________________________________________________
    gboolean WindowManager::wmDestroy( GtkWidget* widget, gpointer data )
    {
        static_cast<WindowManager*>(data)->unregisterWidget( widget );
        return false;
    }

    //_________________________________________________
    gboolean WindowManager::wmButtonPress(GtkWidget *widget, GdkEventButton* event, gpointer data )
    {

        if( event->type == GDK_BUTTON_PRESS && event->button == Gtk::LeftButton )
        {

            #if OXYGEN_DEBUG
            std::cout << "Oxygen::WindowManager::wmButtonPress -"
                << " event: " << event
                << " widget: " << widget
                << " (" << G_OBJECT_TYPE_NAME( widget ) << ")"
                << " " << gtk_widget_get_name( widget )
                << std::endl;
            #endif

            return static_cast<WindowManager*>(data)->isWindowDragWidget( widget, event );

        } else return false;

    }

    //_________________________________________________
    gboolean WindowManager::wmButtonRelease(GtkWidget *widget, GdkEventButton* event, gpointer data )
    {

        #if OXYGEN_DEBUG
        std::cout << "Oxygen::WindowManager::wmButtonRelease -"
            << " event: " << event
            << " widget: " << widget
            << " (" << G_OBJECT_TYPE_NAME( widget ) << ")"
            << " " << gtk_widget_get_name( widget )
            << std::endl;
        #endif

        return static_cast<WindowManager*>( data )->finishDrag( widget );
    }

    //_________________________________________________
    gboolean WindowManager::wmLeave(GtkWidget *widget, GdkEventCrossing*, gpointer data )
    { return static_cast<WindowManager*>( data )->finishDrag( widget ); }

    //_________________________________________________
    gboolean WindowManager::wmMotion( GtkWidget *widget, GdkEventMotion* event, gpointer data )
    { return static_cast<WindowManager*>(data)->startDrag( widget, event ); }

    //_________________________________________________________________
    gboolean WindowManager::startDelayedDrag( gpointer data )
    {
        static_cast<WindowManager*>( data )->startDrag();
        return FALSE;
    }

    //_________________________________________________
    bool WindowManager::isWindowDragWidget( GtkWidget* widget, GdkEventButton* event )
    {
        if( _mode == Disabled ) return false;
        else if( (!_drag) && withinWidget(widget, event ) && useEvent( widget, event ) )
        {

            // store widget and event position
            _widget = widget;
            _x = int(event->x_root);
            _y = int(event->y_root);

            // start timer
            if( _timer.isRunning() ) _timer.stop();
            _timer.start( _dragDelay, (GSourceFunc)startDelayedDrag, this );

            // enable drag and accept
            _drag = true;
            return true;

        } else return false;
    }

    //_________________________________________________________________
    bool WindowManager::startDrag( GtkWidget* widget, GdkEventMotion* event )
    {

        // make sure drag is enabled
        if( !_drag ) return false;

        // check displacement with respect to drag start
        const int distance( abs( _x - int(event->x_root) ) + abs( _y - int(event->y_root) ) );

        if( distance > 0 && _timer.isRunning() ) _timer.stop();
        if( distance < _dragDistance ) return false;

        // start drag from current position
        return startDrag( widget, int(event->x_root), int(event->y_root) );

    }

    //_________________________________________________________________
    bool WindowManager::startDrag( GtkWidget* widget, int x, int y )
    {
        // create xevent and send.
        XEvent     xev;
        GtkWindow  *topLevel = GTK_WINDOW( gtk_widget_get_toplevel( widget ) );
        GdkWindow  *window = gtk_widget_get_window( GTK_WIDGET( topLevel ) );
        GdkDisplay *display = gtk_widget_get_display( GTK_WIDGET( topLevel ) );
        GdkWindow  *root = gdk_screen_get_root_window( gtk_window_get_screen( topLevel ) );

        xev.xclient.type = ClientMessage;
        xev.xclient.message_type = gdk_x11_get_xatom_by_name_for_display(display, "_NET_WM_MOVERESIZE");
        xev.xclient.display = GDK_DISPLAY_XDISPLAY(display);
        xev.xclient.window = GDK_WINDOW_XID(window);
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = x;
        xev.xclient.data.l[1] = y;
        xev.xclient.data.l[2] = 8; // NET::Move
        xev.xclient.data.l[3] = Button1;
        xev.xclient.data.l[4] = 0;
        XUngrabPointer(GDK_DISPLAY_XDISPLAY(display), CurrentTime);

        XSendEvent(
            GDK_DISPLAY_XDISPLAY(display),
            GDK_WINDOW_XID(root),
            False,
            SubstructureRedirectMask | SubstructureNotifyMask,
            &xev);

        // force a release as some widgets miss it...
        // wmButtonRelease( widget, 0L, this );
        return true;
    }

    //_________________________________________________
    bool WindowManager::finishDrag( GtkWidget* widget )
    {

        _widget = 0L;
        _lastRejectedEvent = 0L;
        _x = -1;
        _y = -1;

        if( _drag )
        {

            gtk_grab_remove(widget);
            gdk_pointer_ungrab( CurrentTime );
            _drag = false;
            return true;

        } else return false;

    }

    //_________________________________________________
    bool WindowManager::withinWidget( GtkWidget* widget, GdkEventButton* event ) const
    {

        // get widget window
        GdkWindow *window( gtk_widget_get_window( widget ) );

        // Some widgets aren't realized (GeditWindow for exemple) ...
        if( !window ) return true;

        GtkAllocation allocation;

        if( GTK_IS_NOTEBOOK( widget ) )
        {

            Gtk::gtk_notebook_get_tabbar_rect( GTK_NOTEBOOK( widget ), &allocation );

        } else {

            #if GTK_CHECK_VERSION(2, 18, 0)
            gtk_widget_get_allocation( widget, &allocation );
            #else
            allocation = widget->allocation;
            #endif

        }

        // translate to current window
        int nx(0);
        int ny(0);
        gdk_window_get_geometry( window, &nx, &ny, 0L, 0L, 0L );
        allocation.x -= nx;
        allocation.y -= ny;

        // translate absolute coordinates
        gdk_window_get_origin(window, &nx, &ny );
        allocation.x += nx;
        allocation.y += ny;

        return Gtk::gdk_rectangle_contains( &allocation, int(event->x_root), int(event->y_root) );

    }

    //_________________________________________________
    bool WindowManager::useEvent( GtkWidget* widget, GdkEventButton* event )
    {

        // check against mode
        if( _mode == Disabled ) return false;
        if( _mode == Minimal && !( GTK_IS_TOOLBAR( widget ) || GTK_IS_MENU_BAR( widget ) ) ) return false;
        if( _lastRejectedEvent && event == _lastRejectedEvent ) return false;

        // always accept if widget is not a container
        if( !GTK_IS_CONTAINER( widget ) ) return true;

        // if widget is a notebook, accept if there is no hovered tab
        bool useEvent( true );
        if( GTK_IS_NOTEBOOK( widget ) )
        {

            useEvent =
                ( !Gtk::gtk_notebook_has_visible_arrows( GTK_NOTEBOOK( widget ) ) ) &&
                Style::instance().animations().tabWidgetEngine().contains( widget ) &&
                Style::instance().animations().tabWidgetEngine().hoveredTab( widget ) == -1 &&
                childrenUseEvent( widget, event, false );

        } else {

            useEvent = childrenUseEvent( widget, event, false );

        }

        if( !useEvent ) _lastRejectedEvent = event;
        return useEvent;

    }

    //_________________________________________________
    bool WindowManager::childrenUseEvent( GtkWidget* widget, GdkEventButton* event, bool inNoteBook ) const
    {
        // accept, by default
        bool usable = true;

        // get children and check
        GList *children = gtk_container_get_children( GTK_CONTAINER( widget ) );
        for( GList *child = g_list_first( children ); child && usable ; child = g_list_next( child ) )
        {

            // cast child to GtkWidget
            if( !GTK_IS_WIDGET( child->data ) ) continue;
            GtkWidget* childWidget( GTK_WIDGET( child->data ) );

            // check widget state and type
            if( gtk_widget_get_state( childWidget ) == GTK_STATE_PRELIGHT )
            {

                // if widget is prelight, we don't need to check where event happen,
                // any prelight widget indicate we can't do a move
                usable = false;
                continue;

            }

            GdkWindow *window = gtk_widget_get_window( childWidget );
            if( !( window && gdk_window_is_visible( window ) ) )
            { continue; }

            if( GTK_IS_NOTEBOOK( childWidget ) ) inNoteBook = true;

            if( !( event && withinWidget( childWidget, event ) ) )
            { continue; }

            // check special cases for which grab should not be enabled
            if(
                ( widgetIsBlackListed( childWidget ) ) ||
                ( GTK_IS_NOTEBOOK( widget ) && Gtk::gtk_notebook_is_tab_label( GTK_NOTEBOOK( widget ), childWidget ) ) ||
                ( GTK_IS_BUTTON( childWidget ) && gtk_widget_get_state( childWidget ) != GTK_STATE_INSENSITIVE ) ||
                ( gtk_widget_get_events ( childWidget ) & (GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK) ) ||
                ( GTK_IS_MENU_ITEM( childWidget ) ) ||
                ( GTK_IS_SCROLLED_WINDOW( childWidget ) && ( !inNoteBook || gtk_widget_is_focus( childWidget ) ) ) )
            { usable = false; }

            // if child is a container and event has been accepted so far, also check it, recursively
            if( usable && GTK_IS_CONTAINER( childWidget ) )
            { usable = childrenUseEvent( childWidget, event, inNoteBook); }

            #if OXYGEN_DEBUG
            std::cout << "Oxygen::WindowManager::childrenUseEvent - "
                << widget
                << " (" << G_OBJECT_TYPE_NAME( childWidget ) << ")"
                << " " << gtk_widget_get_name( childWidget )
                << " usable: " << usable
                << std::endl;
            #endif

        }

        // free list
        if( children ) g_list_free( children );

        return usable;
    }

    //_________________________________________________
    void WindowManager::initializeBlackList( void )
    {
        // clear list
        _blackList.clear();
        _blackList.push_back( "GtkPizza" );
        _blackList.push_back( "MetaFrames" );
        _blackList.push_back( "GladeDesignLayout" );
        _blackList.push_back( "SPHRuler" );
        _blackList.push_back( "SPVRuler" );
    }

    //________________________________________________________________________________
    void WindowManager::connect( GtkWidget* widget, WindowManager::Data& data )
    {
        data._destroyId.connect( G_OBJECT( widget ), "destroy", G_CALLBACK( wmDestroy ), this );
        data._styleId.connect( G_OBJECT( widget ), "style-set", G_CALLBACK( wmStyleSet ), this );

        data._pressId.connect( G_OBJECT( widget ), "button-press-event", G_CALLBACK( wmButtonPress ), this );
        data._releaseId.connect( G_OBJECT( widget ), "button-release-event", G_CALLBACK( wmButtonRelease ), this );
        data._leaveId.connect( G_OBJECT( widget ), "leave-notify-event", G_CALLBACK( wmLeave ), this );
        data._motionId.connect( G_OBJECT( widget ), "motion-notify-event", G_CALLBACK( wmMotion ), this );
    }

    //_________________________________________________
    void WindowManager::Data::disconnect( GtkWidget* )
    {

        _leaveId.disconnect();
        _destroyId.disconnect();
        _pressId.disconnect();
        _releaseId.disconnect();
        _motionId.disconnect();
        _styleId.disconnect();

    }

}
