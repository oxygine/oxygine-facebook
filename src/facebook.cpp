#include "facebook.h"

#ifdef __ANDROID__
#include "android/AndroidFacebook.h"

#elif __APPLE__
#include <TargetConditionals.h>
#include "ios/iosFacebook.h"
#else
#include "sim/FacebookSimulator.h"
#endif



namespace facebook
{
    namespace internal
    {
        using namespace std;
        
        cbInit          fInit = []() {};
        cbFree          fFree = []() {};
        cbLogin         fLogin = []() {};
        cbLogout        fLogout = []() {};
        cbNewMeRequest  fNewMeRequest = []() {};
        cbGetFriends    fGetFriends = []() {};
        cbGameRequest   fGameRequest = [](const string &title, const string &text, const vector<string>& dest, const string &objectID, const std::string &userData){};

        cbIsLoggedIn     fIsLoggedIn = []() {return false; };
        cbGetUserID      fGetUserID = []() {return std::string(""); };
        cbGetAccessToken fGetAccessToken = []() {return std::string(""); };
        cbGetAppID       fGetAppID = []() {return std::string(""); };
    }

    using namespace internal;


    spEventDispatcher _dispatcher;

    spEventDispatcher dispatcher()
    {
        return _dispatcher;
    }

    void init()
    {
#ifdef __ANDROID__
        fInit = jniFacebookInit;
        fFree = jniFacebookFree;

        fLogin = jniFacebookLogin;
        fLogout = jniFacebookLogout;
        fNewMeRequest = jniFacebookNewMeRequest;
        fGetFriends = jniFacebookGetFriends;
        fIsLoggedIn = jniFacebookIsLoggedIn;
        fGetUserID = jniFacebookGetUserID;
        fGetAccessToken = jniFacebookGetAccessToken;
        fGetAppID = jniFacebookGetAppID;
        fGameRequest = jniFacebookGameRequest;
#elif TARGET_OS_IPHONE
        fInit = iosFacebookInit;
        fFree = iosFacebookFree;
        fLogin = iosFacebookLogin;
        fLogout = iosFacebookLogout;
        fNewMeRequest = iosFacebookRequestMe;
        fGetFriends = []() {OX_ASSERT(0); };
        fIsLoggedIn = []() {OX_ASSERT(0); return false; };
        fGetUserID = iosFacebookGetUserID;
        fGetAccessToken = iosFacebookGetAccessToken;
        fGetAppID = []() {OX_ASSERT(0); return std::string(""); };
        fGameRequest = iosFacebookGameRequest;
#else
        fInit = facebookSimulatorInit;

        fLogin = facebookSimulatorLogin;
        fLogout = facebookSimulatorLogout;
        fNewMeRequest = facebookSimulatorNewMeRequest;
        fGetFriends = facebookSimulatorGetFriends;
        fIsLoggedIn = facebookSimulatorIsLoggedIn;
        fGetUserID = facebookSimulatorGetUserID;
        fGetAccessToken = facebookSimulatorGetAccessToken;
        fGetAppID = facebookSimulatorGetAppID;
#endif


        log::messageln("facebook::init");
        OX_ASSERT(_dispatcher == 0);
        _dispatcher = new EventDispatcher;

        fInit();

        log::messageln("facebook::init done");
    }

    void free()
    {
        log::messageln("facebook::free");

        OX_ASSERT(_dispatcher);

        fFree();

        _dispatcher->removeAllEventListeners();
        _dispatcher = 0;
        log::messageln("facebook::free done");
    }

    void login()
    {
        log::messageln("facebook::login");

        fLogin();

        log::messageln("facebook::login done");
    }

    void logout()
    {

        log::messageln("facebook::logout");
        fLogout();
        log::messageln("facebook::logout done");
    }

    bool appInviteDialog(const string& appLinkUrl, const string& previewImageUrl)
    {
        log::messageln("facebook::AppInviteDialog");

#ifdef __ANDROID__
        return jniFacebookAppInviteDialog(appLinkUrl, previewImageUrl);
#elif TARGET_OS_IPHONE
#else
        return facebookSimulatorAppInviteDialog(appLinkUrl, previewImageUrl);
#endif
        return false;

    }

    void newMeRequest()
    {
        log::messageln("facebook::newMeRequest");
        fNewMeRequest();
        log::messageln("facebook::newMeRequest done");
    }
    
    void gameRequest()
    {
        vector<string> dest = {"1531304915"};
        fGameRequest("Hello", "Text", dest, "", "");
    }

    void getFriends()
    {
        log::messageln("facebook::getFriends");
        fGetFriends();
        log::messageln("facebook::getFriends done");
    }

    bool isLoggedIn()
    {
        log::messageln("facebook::isLoggined");
        return fIsLoggedIn();
    }

    string getAccessToken()
    {
        log::messageln("facebook::getAccessToken");
        string token = fGetAccessToken();
        log::messageln("%s", token.c_str());
        return token;
    }

    string getUserID()
    {
        log::messageln("facebook::getUserID");
        string id = fGetUserID();

        log::messageln("%s", id.c_str());

        return id;
    }

    string getAppID()
    {
        return fGetAppID();
    }

    namespace internal
    {
        void newToken(const string& value)
        {
            log::messageln("facebook::internal::newToken %s", value.c_str());
            TokenEvent ev;
            ev.token = value;
            if (_dispatcher)
                _dispatcher->dispatchEvent(&ev);
        }

        void loginResult(bool value)
        {
            log::messageln("facebook::internal::loginResult %d", value);
            LoginEvent ev;
            ev.isLoggedIn = value;
            if (_dispatcher)
                _dispatcher->dispatchEvent(&ev);
        }

        void newMeRequestResult(const string& data, bool error)
        {
            log::messageln("facebook::internal::newMeRequestResult %s", data.c_str());

            NewMeRequestEvent event;
            Json::Reader reader;
            Json::Value root;
            bool parsingSuccessful = reader.parse(data.c_str(), root);     //parse process
            if (!parsingSuccessful || error)
            {
                event.error = true;
                log::messageln("newMeRequestResult error %s", error ? "response error" : "parse error");
                return;
            }
            else
            {
                event.id = root["id"].asCString();
                //event.link = root["link"].asCString();
                event.name = root["name"].asCString();
            }

            if (_dispatcher)
                _dispatcher->dispatchEvent(&event);
        }

        void newMyFriendsRequestResult(const string& data, bool error)
        {
            log::messageln("facebook::internal::newMyFriendsRequestResult %s", data.c_str());
        }
        
        void gameRequestResult(const string &id, bool canceled)
        {
            log::messageln("facebook::internal::gameRequestResult %s", id.c_str());
            GameRequestEvent ev(id, canceled);
            if (_dispatcher)
                _dispatcher->dispatchEvent(&ev);
        }
    }
}

