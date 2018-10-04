#pragma once

#include "QtCore/QThread"

class ITNZWorker : public QThread {
    
    Q_OBJECT

    signals:
        void printLog(const std::string &message);
};