#ifndef oxygenmenustateengine_h
#define oxygenmenustateengine_h
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

#include "oxygenanimationengine.h"
#include "oxygengenericengine.h"
#include "oxygendatamap.h"
#include "oxygenmenustatedata.h"

#include <gtk/gtk.h>

namespace Oxygen
{
    //! forward declaration
    class Animations;

    //! stores data associated to editable comboboxes
    /*!
    ensures that the text entry and the button of editable comboboxes
    gets hovered and focus flags at the same time
    */
    class MenuStateEngine: public GenericEngine<MenuStateData>, public AnimationEngine
    {

        public:

        //! constructor
        MenuStateEngine( Animations* parent ):
            GenericEngine<MenuStateData>( parent ),
            _animationsEnabled( true ),
            _followMouse( false ),
            _followMouseAnimationsDuration( 40 )
            {}

        //! destructor
        virtual ~MenuStateEngine( void )
        {}

        //! register widget [overloaded]
        virtual bool registerWidget( GtkWidget* widget )
        {
            const bool registered( GenericEngine<MenuStateData>::registerWidget( widget ) );
            if( registered )
            {
                data().value( widget ).setDuration( duration() );
                data().value( widget ).setAnimationsEnabled( _animationsEnabled );
                data().value( widget ).setFollowMouse( _followMouse );
                data().value( widget ).setFollowMouseAnimationsDuration( _followMouseAnimationsDuration );
            }
            return registered;
        }

        //!@name modifiers
        //@{

        //! enable animations
        bool setAnimationsEnabled( bool value )
        {
            if( _animationsEnabled == value ) return false;
            _animationsEnabled = value;

            for( DataMap<MenuStateData>::Map::iterator iter = data().map().begin(); iter != data().map().end(); iter++ )
            { iter->second.setAnimationsEnabled( value && !widgetIsBlackListed( iter->first ) ); }
            return true;
        }

        //! transition duration
        virtual bool setDuration( int value )
        {
            if( !AnimationEngine::setDuration( value ) ) return false;
            for( DataMap<MenuStateData>::Map::iterator iter = data().map().begin(); iter != data().map().end(); iter++ )
            { iter->second.setDuration( value ); }
            return false;
        }

        //! enable follow-mouse animation
        bool setFollowMouse( bool value )
        {
            if( _followMouse == value ) return false;
            _followMouse = value;

            for( DataMap<MenuStateData>::Map::iterator iter = data().map().begin(); iter != data().map().end(); iter++ )
            { iter->second.setFollowMouse( value && !widgetIsBlackListed( iter->first ) ); }
            return true;
        }

        //! follow-mouse animations duration
        bool setFollowMouseAnimationsDuration( int value )
        {
            if( _followMouseAnimationsDuration == value ) return false;
            _followMouseAnimationsDuration = value;

            for( DataMap<MenuStateData>::Map::iterator iter = data().map().begin(); iter != data().map().end(); iter++ )
            { iter->second.setFollowMouseAnimationsDuration( value ); }
            return true;
        }

        //@}

        //!@name accessors
        //@{

        //! true if animated
        bool isAnimated( GtkWidget* widget )
        { return data().value( widget ).isAnimated(); }

        //! true if given animation type is animated
        bool isAnimated( GtkWidget* widget, const AnimationType& type )
        { return data().value( widget ).isAnimated( type ); }

        //! animated widget for given parent and type
        const GtkWidget* widget( GtkWidget* widget, const AnimationType& type )
        { return data().value( widget ).widget( type ); }

        //! animated rect for given widget and type
        const GdkRectangle& rectangle( GtkWidget* widget, const AnimationType& type )
        { return data().value( widget ).rectangle( type ); }

        //! animation data for given widget and type
        AnimationData animationData( GtkWidget* widget, const AnimationType& type )
        { return data().value( widget ).animationData( type ); }

        //! returns true if animated rectangle is valid
        bool animatedRectangleIsValid( GtkWidget* widget )
        { return data().value( widget ).animatedRectangleIsValid(); }

        //! animated rectangle
        const GdkRectangle& animatedRectangle( GtkWidget* widget )
        { return data().value( widget ).animatedRectangle(); }

        //@}

        private:

        //! enable animations
        bool _animationsEnabled;

        //! follow-mouse enabled
        bool _followMouse;

        //! follow-mouse animation duration
        int _followMouseAnimationsDuration;

    };

}

#endif