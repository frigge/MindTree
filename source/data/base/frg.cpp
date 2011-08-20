
#include "frg.h"

#include "source/graphics/shaderpreview.h"

Project *FRG::CurrentProject = 0;
VNSpace *FRG::Space = 0;
DNSpace *FRG::SpaceDataInFocus = 0;
frg_Shader_Author *FRG::Author = 0;
NodeLib *FRG::lib = 0;
PreviewSceneEditor *FRG::previewEditor = 0;
PreviewSceneCache FRG::previewScenes;

void FRG::Utils::moveDir(QDir source, QDir destination)
{
    QFileInfo srcinfo(source.path());
    QDir destdir(destination);
    if (srcinfo.isDir())
    {
        QString srcdirname = source.dirName();
        destdir.mkdir(srcdirname);
        destdir.cd(srcdirname);

        QDir sourceparent(source);
        sourceparent.cdUp();
        QString sourcedirname(source.dirName());
        if (!sourceparent.rmdir(sourcedirname))
        {
            QFileInfoList sourceinfolist = source.entryInfoList();
            foreach (QFileInfo dirinfo, sourceinfolist)
            {
                QString dirinfofilepath = dirinfo.filePath();
                if (dirinfo.baseName() != "")
                    moveDir(QDir(dirinfo.filePath()), destdir);
            }
            sourceparent.rmdir(sourcedirname);
        }
    } else
    {
        QFile srcfile(source.path());
        QString filename(srcinfo.fileName());
        QString destfilepath(destdir.filePath(filename));
        if (srcfile.copy(destfilepath))
            srcfile.remove();
    }
}

void FRG::Utils::copyDir(QDir source, QDir destination)
{
    QFileInfo srcinfo(source.path());
    QDir destdir(destination);
    if (srcinfo.isDir())
    {
        QString srcdirname = source.dirName();
        destdir.mkdir(srcdirname);
        destdir.cd(srcdirname);

        QDir sourceparent(source);
        sourceparent.cdUp();
        QString sourcedirname(source.dirName());

        QFileInfoList sourceinfolist = source.entryInfoList();
        foreach (QFileInfo dirinfo, sourceinfolist)
        {
            QString dirinfofilepath = dirinfo.filePath();
            if (dirinfo.baseName() != "")
                copyDir(QDir(dirinfo.filePath()), destdir);
        }

    } else
    {
        QFile srcfile(source.path());
        QString filename(srcinfo.fileName());
        QString destfilepath(destdir.filePath(filename));
        srcfile.copy(destfilepath);
    }
}

void FRG::Utils::remove(QString path)    
{
    QFileInfo fileinfo(path);
    QDir data(path);
    QDir parentDir(path);
    parentDir.cdUp();
    QString name = data.dirName();
    if (fileinfo.isDir())
        if(data.count() == 2)
            parentDir.rmdir(name);
        else
        {
            foreach(QFileInfo item, data.entryInfoList())
                if(item.absoluteFilePath() != path
                        &&item.absoluteFilePath() != parentDir.absolutePath())
                    remove(item.absoluteFilePath());
            
            parentDir.rmdir(name);
        }
    else
        parentDir.remove(name);
}
