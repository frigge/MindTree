/*frg.h*/

#ifndef  FRG_INC
#define  FRG_INC

#include "QDir"

#define FRG_PROJECT_HEADER QString("frgse")
#define FRG_PROJECT_HEADER_CHECK QString header;\
                                    QString supheader("frgse");\
                                    stream>>header;\
                                    if(header == supheader)

#define FRG_NODE_HEADER QString("frgn")
#define FRG_NODE_HEADER_CHECK QString header;\
                                QString supheader("frgn");\
                                stream>>header;\
                                if(header == supheader)

class NodeLib;
class frg_Shader_Author; 
class PropertiesEditor;
class NodeGraphWidget;
namespace MindTree
{
    
class Project;
class DNSpace;

namespace FRG
{
	extern Project *CurrentProject;
	extern DNSpace *SpaceDataInFocus;
    extern frg_Shader_Author *Author;
    extern NodeLib *lib;
    extern PropertiesEditor *propEditor;
    extern NodeGraphWidget *Graph;

    namespace Utils
    {
        void moveDir(QDir source, QDir destination);
        void copyDir(QDir source, QDir destination);
        void remove(QString path);
    };
};
} /* MindTree */
#endif   /* ----- #ifndef FRG_INC  ----- */
