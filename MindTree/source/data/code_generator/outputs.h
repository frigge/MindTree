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

#ifndef OUTPUTS_QEX7LSFP

#define OUTPUTS_QEX7LSFP

#include "data/nodes/data_node.h"

class AbstractOutputNode : public MindTree::DNode
{
public:
    AbstractOutputNode(QString name, bool raw=false);
    AbstractOutputNode(const AbstractOutputNode* node);
    virtual ~AbstractOutputNode();

    virtual QString writeCode()=0;
    //SourceDock* getSourceEdit() const;
    QString getShaderName() const;
    QString getFileName() const;
    virtual int compile()const =0;

protected:

private:
    //SourceDock *sedit;
};

class RSLOutputNode : public AbstractOutputNode
{
public:
    RSLOutputNode(QString name, bool raw=false);
    RSLOutputNode(const RSLOutputNode* node);
    QString writeCode();
    int compile()const;
};

class SurfaceOutputNode : public RSLOutputNode
{
public:
    SurfaceOutputNode(bool raw=false);
    SurfaceOutputNode(const SurfaceOutputNode* node);
};

class DisplacementOutputNode : public RSLOutputNode
{
public:
    DisplacementOutputNode(bool raw=false);
    DisplacementOutputNode(const DisplacementOutputNode* node);
};

class VolumeOutputNode : public RSLOutputNode
{
public:
    VolumeOutputNode(bool raw=false);
    VolumeOutputNode(const VolumeOutputNode* node);
};

class LightOutputNode : public RSLOutputNode
{
public:
    LightOutputNode(bool raw=false);
    LightOutputNode(const LightOutputNode* node);
};

class GLSLOutputNode : public AbstractOutputNode
{
public:
    GLSLOutputNode(QString name, bool raw=false);
    GLSLOutputNode(const GLSLOutputNode *node);
    QString writeCode();

protected:
    void setCode(QString value);
    QString getCode()const;

private:
    QString code;
};

class FragmentOutputNode : public GLSLOutputNode
{
public:
    FragmentOutputNode(bool raw=false);
    FragmentOutputNode(const FragmentOutputNode *node);
    int compile()const;
};

class VertexOutputNode : public GLSLOutputNode
{
public:
    VertexOutputNode(bool raw=false);
    VertexOutputNode(const VertexOutputNode *node);
    int compile()const;
};

class GeometryOutputNode : public GLSLOutputNode
{
public:
    GeometryOutputNode(bool raw=false);
    GeometryOutputNode(const GeometryOutputNode *node);
    int compile()const;
};

#endif /* end of include guard: OUTPUTS_QEX7LSFP */
