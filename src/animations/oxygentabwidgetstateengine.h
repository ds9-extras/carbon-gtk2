#ifndef oxygentabwidgetstateengine_h
#define oxygentabwidgetstateengine_h
/*
* this file is part of the oxygen gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
*
* This  library is free  software; you can  redistribute it and/or
* modify it  under  the terms  of the  GNU Lesser  General  Public
* License  as published  by the Free  Software  Foundation; either
* version 2 of the License, or(at your option ) any later version.
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

#include "../oxygenanimationdata.h"
#include "../oxygenstyleoptions.h"
#include "oxygenanimationengine.h"
#include "oxygengenericengine.h"
#include "oxygendatamap.h"
#include "oxygentabwidgetstatedata.h"

#include <gtk/gtk.h>

namespace Oxygen
{
    //! forward declaration
    class Animations;

    //! stores data associated to editable tabwidgetstatees
    /*!
    ensures that the text entry and the button of editable tabwidgetstatees
    gets hovered and focus flags at the same time
    */
    class TabWidgetStateEngine: public GenericEngine<TabWidgetStateData>, public AnimationEngine
    {

        public:

        //! constructor
        TabWidgetStateEngine( Animations* widget ):
            GenericEngine<TabWidgetStateData>( widget ),
            _duration( 150 )
            {}

        //! destructor
        virtual ~TabWidgetStateEngine( void )
        {}

        //! enabled state
        virtual void setEnabled( bool );

        //! transition duration
        virtual void setDuration( int );

        //! retrieve animation data matching a given widget for provided options
        /*! note: for convenience, this method also calls ::registerWidget and ::updateState */
        virtual AnimationData get( GtkWidget* widget, int index, const StyleOptions& options )
        {

            // check widget
            if( !( enabled() && widget ) ) return AnimationData();

            // register
            registerWidget( widget );

            TabWidgetStateData& data( this->data().value( widget ) );
            data.updateState( index, (options&Hover) && !(options&Disabled) );

            return data.isAnimated( index ) ? AnimationData( data.opacity( index ), AnimationHover ):AnimationData();

        }

        //@}

        protected:

        //!@name protected modifiers
        //@{

        //! register widget [overloaded]
        virtual bool registerWidget( GtkWidget* widget )
        {
            const bool registered( GenericEngine<TabWidgetStateData>::registerWidget( widget ) );
            if( registered ) data().value( widget ).setDuration( _duration );
            return registered;
        }

        //@}

        private:

        //! transition duration
        int _duration;

    };

}

#endif
