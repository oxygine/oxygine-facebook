#include "facebook.h"

#ifdef __ANDROID__
#include "android/AndroidFacebook.h"

#elif __APPLE__
#include <TargetConditionals.h>
#include "ios/iosFacebook.h"
#else
#include "sim/FacebookSimulator.h"
#endif

#include "ox/json.hpp"
#include "ox/oxygine.hpp"
#include "ox/ThreadDispatcher.hpp"

namespace facebook
{
    namespace internal
    {
        using namespace std;

        cbInit          fInit = []() {};
        cbFree          fFree = []() {};
        cbLogin         fLogin = [](const vector<string>& permissions) {};
        cbLogout        fLogout = []() {};
        cbNewMeRequest  fNewMeRequest = []() {};
        cbGetFriends    fGetFriends = []() {};
        cbGameRequest   fGameRequest = [](const string& title, const string& text, const vector<string>& dest, const string& objectID, const std::string& userData) {};
        cbRequestInvitableFriends fRequestInvitableFriends = [](const vector<string>&) {};
        cbShareLink     fShareLink = [](const string &, const string &) { core::getMainThreadDispatcher().postCallback([]() {
            ShareEvent ev(true);
            dispatch(&ev);
        }); };

        cbIsLoggedIn     fIsLoggedIn = []() {return false; };
        cbGetUserID      fGetUserID = []() {return std::string(""); };
        cbGetAccessToken fGetAccessToken = []() {return std::string(""); };
        cbGetAppID       fGetAppID = []() {return std::string(""); };
        cbGetAccessTokenPermissions fGetAccessTokenPermissions = []() {return std::vector<string>();};
        cbLogPurchase    fLogPurchase = [](const double& price, const string& currency) {};
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
        fGetAccessTokenPermissions = jniFacebookGetAccessTokenPermissions;
        fRequestInvitableFriends = jniFacebookRequestInvitableFriends;
        fShareLink = jniFacebookShareLink;
        fLogPurchase = jniFacebookLogPurchase;
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
        fGetAccessTokenPermissions = iosFacebookGetPermissions;
        fRequestInvitableFriends = iosFacebookRequestInvitableFriends;
        fShareLink = iosFacebookShareLink;
#else
        fInit = facebookSimulatorInit;
        fFree = facebookSimulatorFree;
        fLogin = facebookSimulatorLogin;
        fLogout = facebookSimulatorLogout;
        fNewMeRequest = facebookSimulatorNewMeRequest;
        fGetFriends = facebookSimulatorGetFriends;
        fIsLoggedIn = facebookSimulatorIsLoggedIn;
        fGetUserID = facebookSimulatorGetUserID;
        fGetAccessToken = facebookSimulatorGetAccessToken;
        fGetAppID = facebookSimulatorGetAppID;
        fGameRequest = facebookSimulatorGameRequest;
        fGetAccessTokenPermissions = facebookSimulatorGetAccessTokenPermissions;
        fRequestInvitableFriends = facebookSimulatorInvitableFriendsRequest;    
        fShareLink = facebookSimulatorShareLink;
#endif


        logs::messageln("facebook::init");
        OX_ASSERT(_dispatcher == 0);
        _dispatcher = new EventDispatcher;

        fInit();

        logs::messageln("facebook::init done");
    }

    void free()
    {
        logs::messageln("facebook::free");

        OX_ASSERT(_dispatcher);

        fFree();

        if (_dispatcher)
            _dispatcher->removeAllEventListeners();
        _dispatcher = 0;
        logs::messageln("facebook::free done");
    }

    void login(const vector<string>& permissions)
    {
        logs::messageln("facebook::login");

        fLogin(permissions);

        logs::messageln("facebook::login done");
    }

    void logout()
    {

        logs::messageln("facebook::logout");
        fLogout();
        logs::messageln("facebook::logout done");
    }

    bool appInviteDialog(const string& appLinkUrl, const string& previewImageUrl)
    {
        logs::messageln("facebook::AppInviteDialog");

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
        logs::messageln("facebook::newMeRequest");
        fNewMeRequest();
        logs::messageln("facebook::newMeRequest done");
    }

    void gameRequest(const string& title, const string& text, const vector<string>& dest, const string& objectID, const string& userData)
    {
        fGameRequest(title, text, dest, objectID, userData);
    }

    void requestInvitableFriends(const vector<string>& exclude)
    {
        logs::messageln("facebook::requestInvitableFriends");
        fRequestInvitableFriends(exclude);
    }

    void shareLink(const string &url, const string &quote)
    {
        logs::messageln("facebook::shareLink");
        fShareLink(url, quote);
    }

    void logPurchase(const double & price, const string & currency)
    {
        logs::messageln("facebook::logPurchase");
        fLogPurchase(price, currency);
        logs::messageln("facebook::logPurchase done");
    }

    void getFriends()
    {
        logs::messageln("facebook::getFriends");
        fGetFriends();
        logs::messageln("facebook::getFriends done");
    }

    bool isLoggedIn()
    {
        logs::messageln("facebook::isLoggined");
        return fIsLoggedIn();
    }

    string getAccessToken()
    {
        logs::messageln("facebook::getAccessToken");
        string token = fGetAccessToken();
        logs::messageln("%s", token.c_str());
        return token;
    }

    vector<string> getAccessTokenPermissions()
    {
        logs::messageln("facebook::getAccessToken");
        vector<string> res = fGetAccessTokenPermissions();
        string str;
        for (const auto& s : res)
            str += s + ",";
        if (!str.empty())
            str.pop_back();
        logs::messageln("permissions: %s", str.c_str());

        return res;
    }

    string getUserID()
    {
        logs::messageln("facebook::getUserID");
        string id = fGetUserID();
        
        logs::messageln("%s", id.c_str());

        return id;
    }

    string getAppID()
    {
        return fGetAppID();
    }

    GameRequestEvent::GameRequestEvent(const string& Data, bool Canceled) : Event(EVENT), data(Data), canceled(Canceled)
    {
        Json::Reader reader;
        Json::Value value;

        bool ok = !Data.empty();
        if (ok)
            ok = reader.parse((char*)&Data.front(), (char*)&Data.front() + Data.size(), value, false);
        if (ok)
        {
            const Json::Value& jr = value["request"];
            if (!jr.isNull())
                request = jr.asString();

            const Json::Value& jto = value["to"];
            if (!jto.isNull())
            {
                for (Json::ArrayIndex i = 0; i < jto.size(); ++i)
                {
                    to.push_back(jto[i].asString());
                }
            }
        }
    }
    
    
    ShareEvent::ShareEvent(bool Canceled):Event(EVENT), canceled(Canceled){}

    namespace internal
    {
        void newToken(const string& value)
        {
            logs::messageln("facebook::internal::newToken %s", value.c_str());
            TokenEvent ev;
            ev.token = value;
            if (_dispatcher)
                _dispatcher->dispatchEvent(&ev);
        }

        void loginResult(bool value)
        {
            logs::messageln("facebook::internal::loginResult %d", value);
            LoginEvent ev;
            ev.isLoggedIn = value;
            if (_dispatcher)
                _dispatcher->dispatchEvent(&ev);
        }

        void newMeRequestResult(const string& data, bool error)
        {
            logs::messageln("facebook::internal::newMeRequestResult %s", data.c_str());

            NewMeRequestEvent event;
            Json::Reader reader;
            Json::Value root;
            bool parsingSuccessful = reader.parse(data.c_str(), root);     //parse process
            if (!parsingSuccessful || error)
            {
                event.error = true;
                logs::messageln("newMeRequestResult error %s", error ? "response error" : "parse error");
                return;
            }
            else
            {
                event.data = data;
            }

            if (_dispatcher)
                _dispatcher->dispatchEvent(&event);
        }

        void newMyFriendsRequestResult(const string& data, bool error)
        {
            logs::messageln("facebook::internal::newMyFriendsRequestResult %s", data.c_str());
        }

        void gameRequestResult(const string& data, bool canceled)
        {
            logs::messageln("facebook::internal::gameRequestResult %s", data.c_str());
            GameRequestEvent ev(data, canceled);
            if (_dispatcher)
                _dispatcher->dispatchEvent(&ev);
        }

        void dispatch(Event* ev)
        {
            if (_dispatcher)
                _dispatcher->dispatchEvent(ev);
        }
    }
}

