#ifndef oxygenstyleoptions_h
#define oxygenstyleoptions_h

/*
* this file is part of the oxygen gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
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

#include <gtk/gtk.h>

namespace Oxygen
{

    //! internal option flags to pass arguments around
    enum StyleOption
    {
        None = 0,
        Blend = 1<<0,
        Sunken = 1<<1,
        Flat = 1<<2,
        Focus = 1<<3,
        Hover = 1<<4,
        NoFill = 1<<5,
        Vertical = 1<<6,
        Alpha = 1<<7,
        Contrast = 1<<8,
        Selected = 1<<9,
        Disabled = 1<<10
    };

    typedef unsigned long StyleOptions;

    //! return style options matching widget state and shadow
    inline StyleOptions styleOptions( GtkWidget* widget, GtkStateType state, GtkShadowType shadow = GTK_SHADOW_NONE )
    {
        StyleOptions options( None );
        if( state == GTK_STATE_INSENSITIVE ) options |= Disabled;
        else if( state == GTK_STATE_PRELIGHT ) options |= Hover;
        else if( state == GTK_STATE_SELECTED ) options |= Selected;

        if( shadow == GTK_SHADOW_IN ) options |= Sunken;
        if( GTK_WIDGET_HAS_FOCUS(widget) ) options|=Oxygen::Focus;
        return options;
    }

};

#endif
