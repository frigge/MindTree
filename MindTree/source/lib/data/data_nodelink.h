/*
    FRG Shader Editor, a Node-based Renderman Shading Language Editor
    Copyright (C) 2011  Sascha Fricke

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef NODELINK_H
#define NODELINK_H


#include "data/nodes/data_node_socket.h"

namespace MindTree
{
    

class DinSocket;
class DoutSocket;
class DSocket;
class DNSpace;

struct DNodeLink
{
	DNodeLink(DSocket *in=0, DSocket *out=0);
    DNodeLink(const DNodeLink &link) : out(link.out), in(link.in){};
    ~DNodeLink();
    DoutSocket *out;
    DinSocket *in;
};

} /* MindTree */
#endif // NODELINK_H
