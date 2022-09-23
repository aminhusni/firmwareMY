#include "configuration.h"
#include "FSCommon.h"


bool copyFile(const char* from, const char* to)
{
#ifdef FSCom
    unsigned char cbuffer[16];
   
    File f1 = FSCom.open(from, FILE_O_READ);
    if (!f1){
        DEBUG_MSG("Failed to open source file %s\n", from);
        return false;
    }

    File f2 = FSCom.open(to, FILE_O_WRITE);
    if (!f2) {
        DEBUG_MSG("Failed to open destination file %s\n", to);
        return false;
    }
   
    while (f1.available() > 0) {
        byte i = f1.read(cbuffer, 16);
        f2.write(cbuffer, i);
    }
   
    f2.close();
    f1.close();
    return true;
#endif
}

bool renameFile(const char* pathFrom, const char* pathTo)
{
#ifdef FSCom
#ifdef ARCH_ESP32
    // rename was fixed for ESP32 IDF LittleFS in April
    return FSCom.rename(pathFrom, pathTo);
#else    
    if (copyFile(pathFrom, pathTo) && FSCom.remove(pathFrom) ) {
        return true;
    } else{
        return false;
    }
#endif
#endif
}

void listDir(const char * dirname, uint8_t levels, boolean del = false)
{
#ifdef FSCom
    char buffer[255];
    File root = FSCom.open(dirname, FILE_O_READ);
    if(!root){
        return;
    }
    if(!root.isDirectory()){
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory() && !String(file.name()).endsWith(".")) {
            if(levels){
#ifdef ARCH_ESP32
                listDir(file.path(), levels -1, del);
                if(del) { 
                    DEBUG_MSG("Removing %s\n", file.path());
                    strcpy(buffer, file.path());
                    file.close();
                    FSCom.rmdir(buffer);
                } else {
                    file.close();
                }
#else
                listDir(file.name(), levels -1, del);
                file.close();
#endif
            }
        } else {
#ifdef ARCH_ESP32
            if(del) {
                DEBUG_MSG("Deleting %s\n", file.path());
                strcpy(buffer, file.path());
                file.close();
                FSCom.remove(buffer);
            } else {
            DEBUG_MSG(" %s (%i Bytes)\n", file.path(), file.size());
                file.close();
            }
#else
            DEBUG_MSG(" %s (%i Bytes)\n", file.name(), file.size());
            file.close();
#endif            
        }
        file = root.openNextFile();
    }
#ifdef ARCH_ESP32    
    if(del) { 
        DEBUG_MSG("Removing %s\n", root.path());
        strcpy(buffer, root.path());
        root.close();
        FSCom.rmdir(buffer);
    } else {
        root.close();
    }
#else
    root.close();
#endif
#endif
}

void rmDir(const char * dirname)
{
#ifdef FSCom
#ifdef ARCH_ESP32

    FSCom.rmdir(dirname);
    listDir(dirname, 10, true);
#else
    // nRF52 implementation of LittleFS has a recursive delete function
    FSCom.rmdir_r(dirname);
#endif
#endif
}

void fsInit()
{
#ifdef FSCom
    if (!FSBegin())
    {
        DEBUG_MSG("ERROR filesystem mount Failed. Formatting...\n");
        assert(0); // FIXME - report failure to phone
    }
#ifdef ARCH_ESP32
    DEBUG_MSG("Filesystem files (%d/%d Bytes):\n", FSCom.usedBytes(), FSCom.totalBytes());
#else
    DEBUG_MSG("Filesystem files:\n");
#endif
    listDir("/", 10);
#endif
}
