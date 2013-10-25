#include "GL/glew.h"
#include "QGLContext"
#include "QGLFormat"
#include "memory"
#include "iostream"
#include "init.h"

void MindTree::GL::init()    
{
    QGLFormat fm = QGLFormat::defaultFormat();
    fm.setVersion(3, 2);
    fm.setProfile(QGLFormat::CoreProfile);

    std::unique_ptr<QGLContext> ctx(new QGLContext(fm));
    if(ctx->create()) {
        ctx->makeCurrent();
        glewInit();
    }
    else
        std::cout<<"context creation failed"<<std::endl;
}
