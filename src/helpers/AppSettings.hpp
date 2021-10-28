#pragma once

#include <QSettings>

class AppSettings : public QSettings {
 public:
    struct ConnectivityInfos {
        const QString platformHostUrl = QString();
        const QString username = QString();
        const QString password = QString();
        bool areOK = false;

        const QString getPlaformHomeUrl() const {
            if(!areOK) return QString();
            return platformHostUrl + '/' + username;
        }
    };

    struct UploadInfos {
        const QString path;
        const QString outputFileName;
    };

    const ConnectivityInfos getConnectivityInfos() const {
        auto hostUrl =  this->value(PLATFORM_HOST_URL).toString();
        auto username = this->value(PLATFORM_USERNAME).toString();
        auto password = this->value(PLATFORM_PASSWORD).toString();

        if(username.isEmpty() || hostUrl.isEmpty() || password.isEmpty())
            return {};
        
        return {
            hostUrl,
            username,
            password,
            true
        };
    }

    static const UploadInfos getShoutUploadInfos() {
        return { "/uploadShout", "shout_file" };
    }

    static const UploadInfos getFeederUploadInfos() {
        return { "/uploadLib", "wtnz_file" };
    }


    static inline const char * MUST_AUTORUN_SHOUT = "AutoRunShout";
    static inline const char * PLATFORM_USERNAME = "PlatformUsername";
    static inline const char * PLATFORM_PASSWORD = "PlatformPassword";
    static inline const char * PLATFORM_HOST_URL = "PlatformHostUrl";
    static inline const char * MUST_RUN_AT_STARTUP = "MustRunAtStartup";
};
