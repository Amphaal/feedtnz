#include <windows.h>

// #include "../../../libs/itunescom/iTunesEventHandler.cpp"
#include "../../../libs/itunescom/iTunesCOMInterface.h"

#include <ActiveQt/QAxBase>
#include <ActiveQt/QAxObject>

#include "../shout.h" 
#include "iTunesCOMHandler.h"
#include "../../../helpers/stringHelper.cpp"

#include <QMetaObject>
#include <QMetaMethod>
#include <QObject>
#include <QCoreApplication>

void ShoutWorker::run() {
    
    //start with log
    emit this->printLog("Waiting for iTunes to launch...");
    HWND iTunesWindowsHandler;

    do {
        
        //prepare for process search...
        iTunesWindowsHandler = FindWindowA(0, "iTunes");

        //search for iTunes...
        if(iTunesWindowsHandler != NULL) {

            //initiate COM object
            CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
            wchar_t* wch = nullptr;
            HRESULT hr = ::StringFromCLSID(CLSID_iTunesApp, &wch);
            QAxObject *iITunes = new QAxObject(QString::fromWCharArray(wch));
            
            iTunesCOMHandler *handler = new iTunesCOMHandler(iITunes, this);

            //bind events to sink handler
            bool oatputqe = QObject::connect(iITunes, iITunes->metaObject()->method(5), handler, handler->metaObject()->method(5));
            bool oppe = QObject::connect(iITunes, iITunes->metaObject()->method(9), handler, handler->metaObject()->method(6));
            bool opse = QObject::connect(iITunes, iITunes->metaObject()->method(11), handler, handler->metaObject()->method(7));

            //process events
            while(this->mustListen && !handler->iTunesShutdownRequested) {
                QCoreApplication::processEvents();
                this->msleep(200);
            }

            //clear COM usage
            handler->shoutHelper();
            iITunes->clear(); 
            CoUninitialize();

            //if iTunes is shutting down...
            if(this->mustListen && handler->iTunesShutdownRequested) {
                
                emit this->printLog("iTunes shutting down !");

                //wait for itunes to finally shutdown
                do {
                    iTunesWindowsHandler = FindWindowA(0, "iTunes");
                    if(iTunesWindowsHandler != NULL) this->sleep(1);
                } while(this->mustListen && iTunesWindowsHandler != NULL);
                

                emit this->printLog("Waiting for iTunes to launch again...");
            }

        } else {
            //if not found, wait and retry
            this->sleep(1);
        }
    } while (this->mustListen);

    //end with log
    emit this->printLog("Stopped listening to iTunes.");
};
