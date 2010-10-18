/*
* this file is part of the oxygen gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
*
* inspired notably from kdelibs/kdeui/color/kcolorscheme.h
* Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
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

#include "oxygenpalette.h"
#include "oxygencoloreffect.h"
#include "oxygencolorutils.h"

namespace Oxygen
{

    //_______________________________________________________
    void Palette::generate( Group from, Group to, const ColorUtils::Effect& effect, bool changeSelectionColor )
    {
        // start by copying palette
        colorList(to) = colorList(from);

        const ColorUtils::Rgba::List& source = colorList(from);
        ColorUtils::Rgba::List &destination = colorList(to);

        // apply effects to background
        destination[Window] = effect.color(source[Window]);
        destination[Button] = effect.color(source[Button]);
        destination[Base] = effect.color(source[Base]);
        destination[BaseAlternate] = effect.color(source[BaseAlternate]);

        if( changeSelectionColor ) destination[Selected] = effect.color( ColorUtils::tint( source[Window], source[Selected], 0.4 ) );
        else destination[Selected] = effect.color( source[Selected] );

        // apply effects to hover and focus and text
        destination[WindowText] = effect.color(source[WindowText], source[Window] );
        destination[ButtonText] = effect.color(source[ButtonText], source[Button] );
        destination[Text] = effect.color(source[Text], source[Base] );
        destination[Focus] = effect.color(source[Focus], source[Base] );
        destination[Hover] = effect.color(source[Hover], source[Base] );

    }

}
