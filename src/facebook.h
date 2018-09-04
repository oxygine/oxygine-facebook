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

        string data;

        bool error = false;
    };

    class LoginEvent : public Event
    {
    public:
        enum { EVENT = sysEventID('f', 'b', 'l')};
        LoginEvent() : Event(EVENT) {}

        bool isLoggedIn = false;
        string userID;
        string token;
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

    class InvitableFriendsEvent: public Event
    {
    public:
        enum {EVENT = sysEventID('f', 'I', 'F')};

        InvitableFriendsEvent(): Event(EVENT), status(-1) {}
        struct Friend
        {
            string id;
            string name;
            string url;
        };

        //vector<Friend> friends;
        string data;
        int status;//ok = 0; end = -1, error = -2

    };

    class TokenEvent : public Event
    {
    public:
        enum { EVENT = sysEventID('f', 'b', 't')};
        TokenEvent() : Event(EVENT) {};

        string token;
    };

    class GameRequestEvent: public Event
    {
    public:
        enum {EVENT = sysEventID('f', 'g', 'r')};
        GameRequestEvent(const string& Data, bool Canceled);

        string data;
        bool canceled;


        string request;
        vector<string> to;
    };

    class ShareEvent: public Event
    {
    public:
        enum {EVENT = sysEventID('f', 'S', 'h')};
        bool canceled;

        ShareEvent(bool canceled);
    };

    spEventDispatcher dispatcher();

    bool isLoggedIn();
    void login(const vector<string>& permissions);
    void logout();
    void getFriends();
    void newMeRequest();
    void gameRequest(const string& title, const string& text, const vector<string>& dest, const string& objectID, const string& userData);
    void requestInvitableFriends(const vector<string>& exclude);
    void shareLink(const string& url, const string& quote);
    void logPurchase(const double& price, const string& currency);

    bool appInviteDialog(const string& appLinkUrl, const string& previewImageUrl);

    string getAccessToken();
    string getUserID();
    string getAppID();
    vector<string> getAccessTokenPermissions();

    namespace internal
    {
        typedef void(*cbInit)();
        typedef void(*cbFree)();
        typedef void(*cbLogin)(const vector<string>& permissions);
        typedef void(*cbLogout)();
        typedef void(*cbNewMeRequest)();
        typedef void(*cbGameRequest)(const string& title, const string& text, const vector<string>& dest, const string& objectID, const std::string& userData);
        typedef void(*cbGetFriends)();
        typedef void(*cbRequestInvitableFriends)(const vector<string>&);
        typedef void(*cbShareLink)(const string& url, const string& quote);
        typedef void(*cbLogPurchase)(const double& price, const string& currency);
        typedef bool(*cbIsLoggedIn)();
        typedef std::string(*cbGetUserID)();
        typedef std::string(*cbGetAccessToken)();
        typedef std::string(*cbGetAppID)();

        typedef std::vector<std::string>(*cbGetAccessTokenPermissions)();

        extern cbInit                       fInit;
        extern cbFree                       fFree;
        extern cbLogin                      fLogin;
        extern cbLogout                     fLogout;
        extern cbNewMeRequest               fNewMeRequest;
        extern cbGetFriends                 fGetFriends;
        extern cbRequestInvitableFriends    fRequestInvitableFriends;
        extern cbGameRequest                fGameRequest;
        extern cbIsLoggedIn                 fIsLoggedIn;
        extern cbGetUserID                  fGetUserID;
        extern cbGetAccessToken             fGetAccessToken;
        extern cbGetAppID                   fGetAppID;
        extern cbGetAccessTokenPermissions  fGetAccessTokenPermissions;
        extern cbShareLink                  fShareLink;
        extern cbLogPurchase                fLogPurchase;

        void newMeRequestResult(const string& data, bool error);
        void loginResult(bool value, const string& userID, const string& token);
        void newToken(const string& value);
        void newMyFriendsRequestResult(const string& data, bool error);

        void gameRequestResult(const string& id, bool canceled);
        //void resultInvitableFriends(int page, vector<);

        void dispatch(Event*);
    }
};
