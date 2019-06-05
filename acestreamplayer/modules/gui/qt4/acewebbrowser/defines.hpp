#ifndef AWB_DEFINES_H
#define AWB_DEFINES_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <p2p_object.h>

#include <QMetaType>
#include <QDebug>
#include <QSettings>
#include <QProcess>
#include <QDir>
#include <QUrl>

namespace AceWebBrowser {

#ifdef TORRENT_STREAM
#define ACE_INSTALL_KEY "\\Software\\TorrentStream\\"
#else
#define ACE_INSTALL_KEY "\\Software\\ACEStream\\"
#endif

static QString engine_location = "";
static void OpenInAceWeb(const QUrl &url, const QString &args)
{
    if(url.isValid()) {
#ifdef Q_OS_WIN
        if(engine_location.isEmpty()) {
            QSettings hkcu("HKEY_CURRENT_USER" ACE_INSTALL_KEY, QSettings::NativeFormat);
            if(hkcu.contains("EnginePath")) {
                engine_location = QDir::toNativeSeparators(hkcu.value("EnginePath", "").toString());
            }
            else {
                QSettings hklm("HKEY_LOCAL_MACHINE" ACE_INSTALL_KEY, QSettings::NativeFormat);
                engine_location = QDir::toNativeSeparators(hklm.value("EnginePath", "").toString());
            }
        }
        QString aceWebLocation = engine_location.replace("ace_engine", "ace_web");
        QStringList argslist = args.split(" ");
        argslist.append(url.toString());
        QProcess::startDetached(aceWebLocation, argslist);
#endif
    }
}

enum BrowserType {
    BTYPE_UNDEFINED = -1,
    BTYPE_PAUSE = 0,
    BTYPE_STOP,
    BTYPE_OVERLAY,
    BTYPE_PREROLL,
    BTYPE_SLIDER,
    BTYPE_HIDDEN,
    BTYPE_PREPLAY,
    BTYPE_WEBSTAT // 7,8,9,10
};

enum BrowserCookies {
    BCOOK_OUR = 1,
    BCOOK_ALL,
    BCOOK_NONE
};

enum BrowserUserAgent {
    BUA_OUR = 1,
    BUA_HOST
};

enum BrowserHolderPlayerState {
    BHPS_NONE = 0,
    BHPS_PLAYING = 2,
    BHPS_PAUSED = 3,
    BHPS_STOPPED = 4,
    BHPS_ERROR = 5
};

enum BrowserState {
    BS_UNLOADED,
    BS_LOADING,
    BS_LOADED,
    BS_ERROR
};

enum BrowserAction {
    BA_UNDEF,
    BA_SHOW,
    BA_HIDE,
    BA_CLOSE
};

#define USER_AGENT "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/534.34 (KHTML, like Gecko) Qt/"QT_VERSION_STR" AceStream/"P2P_STD_VERSION"";

#define ADD_JS_SCRIPT_JS "" \
    "(function(){" \
    "  function embed(tries) {" \
    "      if(tries <= 0) return;" \
    "      if(!document || !document.head) {" \
    "          setTimeout(function() { embed(tries-1); }, 500);" \
    "          return;" \
    "      }" \
    "      var scripts = document.head.getElementsByTagName('script');" \
    "      var has_script = false;" \
    "      for(var i = 0; i < scripts.length; i++) {" \
    "          if(scripts[i].getAttribute('src') === '%SCRIPT_URL%') { has_script = true; }" \
    "      }" \
    "      if(!has_script) {" \
    "          var jsref = document.createElement('script');" \
    "          jsref.setAttribute('type','text/javascript');" \
    "          jsref.setAttribute('src', '%SCRIPT_URL%');" \
    "          document.head.appendChild(jsref);" \
    "      }" \
    "  }" \
    "  embed(50);" \
    "})();"

#define ADD_JS_CODE_JS "" \
    "(function(){" \
    "    function embed(tries) {" \
    "        if(tries <= 0) return;" \
    "        if(!document || !document.head) {" \
    "            setTimeout(function() { embed(tries-1); }, 500);" \
    "            return;" \
    "        }" \
    "        var js = document.createElement('script');" \
    "        js.setAttribute('type', 'text/javascript');" \
    "        js.innerHTML = '%SCRIPT_CODE%';" \
    "        document.head.appendChild(js);" \
    "    }" \
    "    embed(50);" \
    "})();"

}

Q_DECLARE_METATYPE(AceWebBrowser::BrowserType)

#endif // AWB_DEFINES_H
