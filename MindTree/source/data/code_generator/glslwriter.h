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

#ifndef GLSLWRITER_GX2BLQ0U

#define GLSLWRITER_GX2BLQ0U

#include "shaderwriter.h"

class GLSLWriter : public ShaderCodeGenerator
{
public:
    GLSLWriter(const MindTree::DNode *start);
    QString getCode();

protected:
    virtual void createHeader();

    inline bool isInputVar(const MindTree::DoutSocket*);

    void initVar(const MindTree::DSocket *socket);
    void outputVar(const MindTree::DinSocket *socket);

    QString writeVector(const MindTree::DoutSocket *socket);
    QString writeColor(const MindTree::DoutSocket *socket);
};

#endif /* end of include guard: GLSLWRITER_GX2BLQ0U */
