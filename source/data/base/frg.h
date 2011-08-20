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

class Project;
class VNSpace;
class DNSpace;
class NodeLib;
class frg_Shader_Author; 
class PreviewSceneEditor;
class PreviewSceneCache;

namespace FRG
{
	extern Project *CurrentProject;
    extern VNSpace *Space;
	extern DNSpace *SpaceDataInFocus;
    extern frg_Shader_Author *Author;
    extern NodeLib *lib;
    extern PreviewSceneEditor *previewEditor;
    extern PreviewSceneCache previewScenes;

    namespace Utils
    {
        void moveDir(QDir source, QDir destination);
        void copyDir(QDir source, QDir destination);
        void remove(QString path);
    };
};
#endif   /* ----- #ifndef FRG_INC  ----- */
