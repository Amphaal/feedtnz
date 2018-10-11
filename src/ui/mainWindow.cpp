#include "QtCore/QString"
#include "QtCore/QSize"
#include "QtCore/QObject"
#include "QtCore/QFileSystemWatcher"
#include "QtWidgets/QTabWidget"
#include "QtWidgets/QMainWindow"
#include "QtWidgets/QWidget"
#include "QtWidgets/QMenuBar"
#include "QtWidgets/QMenu"
#include "QtWidgets/QAction"
#include "QtWidgets/QSystemTrayIcon"
#include "QtWidgets/QMessageBox"
#include "QtGui/QCloseEvent"
#include "QtGui/QHideEvent"
#include "QtGui/QIcon"

#include "../helpers/configHelper.cpp"
#include "../helpers/platformHelper/platformHelper.h"
#include "./tabs/ShoutTab.cpp"
#include "./tabs/FeederTab.cpp"

class MainWindow : public QMainWindow {
   
    public:
        MainWindow(QString *title) : title(title), helper(ConfigHelper()), pHelper(PlatformHelper()) {     
            this->updateConfigValues();
            this->setWindowTitle(*title);
            this->_initUI();
            this->setupConfigFile();
        }
    
    private:
        bool forceQuitOnMacOS = false;
        QString *title;
        ConfigHelper helper;
        QSystemTrayIcon *trayIcon;
        vector<QAction*> myWTNZActions;
        nlohmann::json config;
        QFileSystemWatcher *watcher;
        string wtnzUrl;
        PlatformHelper pHelper;
        ShoutTab *shoutTab;
    
        ///
        ///UI instanciation
        ///

        void _initUI() {
            this->setMinimumSize(QSize(480, 400));
            this->_initUITabs();
            this->_initUITray();
            this->_initUIMenu();
        }

        void _initUITabs() {
            QTabWidget *tabs = new QTabWidget;
            this->shoutTab = new ShoutTab(tabs, &this->helper, this->config);
            FeederTab *feedTab = new FeederTab(tabs);

            tabs->addTab(shoutTab, "Shout!");
            tabs->addTab(feedTab, "Feeder");

            this->setCentralWidget(tabs);
        }

        void _initUIMenu() {
            QMenuBar *menuBar = new QMenuBar;
            menuBar->addMenu(this->_getMenu());
            this->setMenuWidget(menuBar);
        }

        void _initUITray() {
            QSystemTrayIcon *trayIcon = new QSystemTrayIcon;
            this->trayIcon = trayIcon;
            trayIcon->setIcon(QIcon(":/icons/feedtnz.png"));
            trayIcon->setToolTip(*this->title);

            //double it to the tray icon
            this->trayIcon->setContextMenu(this->_getMenu());

            trayIcon->show();
        }

        QMenu* _getMenu() {

            QMenu *fileMenuItem = new QMenu("File");

            //monitorAction
            QAction *monitorAction = new QAction("Open monitor...", fileMenuItem);
            QObject::connect(
                monitorAction, &QAction::triggered,
                this, &MainWindow::trueShow
            );

            //myWTNZAction
            QAction *myWTNZAction = new QAction("My WTNZ", fileMenuItem);
            myWTNZAction->setEnabled(false);
            QObject::connect(
                myWTNZAction, &QAction::triggered,
                this, &MainWindow::accessWTNZ
            );
            this->myWTNZActions.push_back(myWTNZAction);

            //updateConfigAction
            QAction *updateConfigAction = new QAction("Update configuration file", fileMenuItem);
            QObject::connect(
                updateConfigAction, &QAction::triggered,
                this, &MainWindow::openConfigFile
            );

            //quit
            QAction *quitAction = new QAction("Quit", fileMenuItem);
            QObject::connect(
                quitAction, &QAction::triggered,
                this, &MainWindow::forcedClose
            );

            fileMenuItem->addAction(monitorAction);
            fileMenuItem->addSeparator();
            fileMenuItem->addAction(myWTNZAction);
            fileMenuItem->addAction(updateConfigAction);
            fileMenuItem->addSeparator();
            fileMenuItem->addAction(quitAction);

            return fileMenuItem;
        }

        void updateConfigValues() {
            this->config = this->helper.accessConfig();
        }

        void setupConfigFile() {
            this->updateMenuItems();

            std::string f = this->helper.getConfigFileFullPath();
            QFileSystemWatcher *watcher = new QFileSystemWatcher(QStringList(f.c_str()), this);
            this->watcher = watcher;
            
            QObject::connect(watcher, &QFileSystemWatcher::fileChanged,
                    this, &MainWindow::updateMenuItems);
        }

        void updateMenuItems(const QString &path = NULL) {
            this->updateConfigValues();

            bool WTNZUrlAvailable = !this->config["targetUrl"].empty() && !this->config["user"].empty();
            if (WTNZUrlAvailable) {
                
                //set new WTNZ Url
                this->wtnzUrl = this->config["targetUrl"].get<std::string>();
                this->wtnzUrl += "/";
                this->wtnzUrl += this->config["user"].get<std::string>();

                //update action state
                for (QAction *action: this->myWTNZActions){action->setEnabled(true);}
            } else {
                //update action state
                for (QAction *action: this->myWTNZActions){action->setEnabled(false);}
            }
        }      

        ///
        /// Functionnalities helpers calls
        ///


        void accessWTNZ() {
            this->pHelper.openUrlInBrowser(this->wtnzUrl.c_str());
        }

        //open the config file into the OS browser
        void openConfigFile() {
            this->helper.openConfigFile();
        }

        ///
        /// Events handling
        ///

        //hide window on minimize, only triggered on windows
        void hideEvent(QHideEvent *event)
        {
            this->trueHide(event);
        }

        void closeEvent(QCloseEvent *event) {
            
            //apple specific behaviour, prevent closing
            #ifdef __APPLE__
                if(!this->forceQuitOnMacOS) {
                    return this->trueHide(event);
                }
            #endif

            //if running shout thread
            if(this->shoutTab->isWorkerRunning()) {
                auto msgboxRslt = QMessageBox::warning(this, QString("Shout worker running !"), 
                            QString("Shout worker is actually running : Are you sure you want to exit ?"), 
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                
                if(msgboxRslt == QMessageBox::Yes) {
                    //makes sure to wait for shoutThread to end. Limits COM app retention on Windows
                    this->shoutTab->endThread();
                } else {
                    event->ignore();
                    return;
                }
            }

            //hide trayicon on shutdown for Windows, refereshes the UI frames of system tray
            this->trayIcon->hide();
        }

        void trueShow() {
            this->showNormal();
            this->activateWindow();
            this->raise();
        }

        void trueHide(QEvent* event) {
            event->ignore();
            this->hide();
        }

        void forcedClose() {
            this->forceQuitOnMacOS = true;
            this->close();
        }
};