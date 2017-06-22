#pragma once

#include "oxygine/EventDispatcher.h"
#include "oxygine/Event.h"
#include "oxygine/json/json.h"
#include <string>
#include <list>

using namespace std;
using namespace oxygine;

namespace facebook
{

    void init();
    void free();

    class NewMeRequestEvent : public Event
    {
    public:
        enum { EVENT = sysEventID('f', 'b', 'n') };
        NewMeRequestEvent() : Event(EVENT) {}

        string id;
        //string link;
        string name;
        bool error = false;
    };

    class LoginEvent : public Event
    {
    public:
        enum { EVENT = sysEventID('f', 'b', 'l')};
        LoginEvent() : Event(EVENT) {}
        bool isLoggedIn = false;
    };

    class FriendsEvent : public Event
    {
        enum { EVENT = sysEventID('f', 'b', 'f')};
        FriendsEvent() : Event(EVENT) {}

        struct friend_Data
        {
            string name;
        };

        typedef list<friend_Data> friends_list;

        friends_list friends;
        bool error = false;

    };

    class TokenEvent : public Event
    {
    public:
        enum { EVENT = sysEventID('f', 'b', 't')};
        TokenEvent() : Event(EVENT) {};

        string token;
    };

    spEventDispatcher dispatcher();

    bool isLoggedIn();
    void login();
    void logout();
    void getFriends();
    void newMeRequest();
    bool appInviteDialog(const string& appLinkUrl, const string& previewImageUrl);

    string getAccessToken();
    string getUserID();
    string getAppID();

    namespace internal
    {
        typedef void(*cbInit)();
        typedef void(*cbFree)();
        typedef void(*cbLogin)();
        typedef void(*cbLogout)();
        typedef void(*cbNewMeRequest)();
        typedef void(*cbGetFriends)();

        typedef bool(*cbIsLoggedIn)();
        typedef std::string(*cbGetUserID)();
        typedef std::string(*cbGetAccessToken)();
        typedef std::string(*cbGetAppID)();

        extern cbInit          fInit;
        extern cbFree          fFree;
        extern cbLogin         fLogin;
        extern cbLogout        fLogout;
        extern cbNewMeRequest     fNewMeRequest;
        extern cbGetFriends         fGetFriends;
        extern cbIsLoggedIn         fIsLoggedIn;
        extern cbGetUserID           fGetUserID;
        extern cbGetAccessToken fGetAccessToken;
        extern cbGetAppID             fGetAppID;

        void newMeRequestResult(const string& data, bool error);
        void loginResult(bool value);
        void newToken(const string& value);
        void newMyFriendsRequestResult(const string& data, bool error);
    }
};
