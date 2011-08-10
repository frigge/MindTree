/*frg.h*/

#ifndef  FRG_INC
#define  FRG_INC

#include "QDir"

class Project;
class VNSpace;
class DNSpace;
class NodeLib;
class frg_Shader_Author; 

namespace FRG
{
	extern Project *CurrentProject;
    extern VNSpace *Space;
	extern DNSpace *SpaceDataInFocus;
    extern frg_Shader_Author *Author;
    extern NodeLib *lib;

    namespace Utils
    {
        void moveDir(QDir source, QDir destination);
        void copyDir(QDir source, QDir destination);
    };
};
#endif   /* ----- #ifndef FRG_INC  ----- */
