#include "FacebookSimulator.h"
#include "facebook.h"
#include "oxygine/json/json.h"
#include "oxygine/core/file.h"
#include "oxygine/actor/Box9Sprite.h"
#include "oxygine/actor/TextField.h"
#include "oxygine/actor/DebugActor.h"
#include "oxygine/res/Resources.h"
#include "oxygine/actor/Stage.h"
#include "oxygine/core/oxygine.h"

bool _isLoggedIn = true;
string _facebookToken = "";
string _userID = "";
string _appID = "";
string _permissions = "";

Json::Value _facebook(Json::objectValue);

#if OXYGINE_VERSION >= 10
#define OUTX TouchEvent::OUTX
#else
#define OUTX TouchEvent::OUT
#endif

std::string getValue(const Json::Value& obj, const char* key);
static void save()
{
    _facebook["loggedIn"] = _isLoggedIn;
    _facebook["permissions"] = _permissions;

    Json::StyledWriter writer;
    string s = writer.write(_facebook);
    file::write(".facebook", s.c_str(), s.size());
}

DECLARE_SMART(Btn, spBtn);
class Btn : public Box9Sprite
{
public:
    Btn()
    {
        _txt = new TextField;
        _txt->setText("OK");
        _txt->setAlign(TextStyle::VALIGN_MIDDLE, TextStyle::HALIGN_MIDDLE);
        addChild(_txt);

        setColor(Color::Green);
        setResAnim(DebugActor::resSystem->getResAnim("btn"));

        addEventListener(TouchEvent::OVER, CLOSURE(this, &Btn::touch));
        addEventListener(OUTX, CLOSURE(this, &Btn::touch));

        setSize(70, 30);
    }

    void setText(const string& txt)
    {
        _txt->setText(txt);
    }

    void touch(Event* ev)
    {
        if (ev->type == TouchEvent::OVER)
            setColor(Color::GreenYellow);
        if (ev->type == OUTX)
            setColor(Color::Green);
    }

    void sizeChanged(const Vector2& size)
    {
        _txt->setSize(size);
    }

    spTextField _txt;
};

DECLARE_SMART(FacebookDialog, spFacebookDialog);
class FacebookDialog : public Actor
{
public:
    enum
    {
        EVENT_OK = 12323,
        EVENT_CANCEL
    };

    FacebookDialog()
    {
        setPriority(9999);

        spActor blocker = new Actor;
        blocker->setPosition(-getStage()->getSize());
        blocker->setSize(getStage()->getSize() * 3);
        addChild(blocker);

        _bg = new Box9Sprite;
        addChild(_bg);

        _title = new TextField;
        _title->setAlign(TextStyle::VALIGN_MIDDLE, TextStyle::HALIGN_MIDDLE);
        _title->setMultiline(true);
        _title->setColor(Color::Black);
        addChild(_title);

        _btnOk = new Btn();
        _btnOk->setText("Ok");
        addChild(_btnOk);

        _btnCancel = new Btn();
        _btnCancel->setText("Cancel");
        addChild(_btnCancel);
    }

    void setTitle(const string& txt)
    {
        _title->setText(txt);
    }

    void doRender(const RenderState& rs)
    {
        //Stage::render()
    }

    void sizeChanged(const Vector2& size)
    {
        _bg->setSize(size);

        Vector2 center = core::getDisplaySize().cast<Vector2>() / 2.0f;
        center = getStage()->parent2local(center);

        float sx = getStage()->getScaleX();
        setPosition(center - size / sx / 2);

        _btnOk->setPosition(size - _btnOk->getSize() - Vector2(10, 10));
        _btnCancel->setPosition(10, getHeight() - 10 - _btnCancel->getHeight());
        _title->setWidth(getWidth());
        _title->setHeight(_btnCancel->getY());
    }

    spBox9Sprite        _bg;
    spTextField         _title;
    spBtn               _btnOk;
    spBtn               _btnCancel;
};

spBtn _btnLike;
void facebookSimulatorShowLike(const string& url)
{
    _btnLike = new Btn;
    _btnLike->setText("LIKE");
    _btnLike->setAnchor(0.5f, 0.5f);
    _btnLike->attachTo(getStage());
    _btnLike->setPriority(31000);
}

void facebookSimulatorHideLike()
{
    if (_btnLike)
        _btnLike->detach();
    _btnLike = 0;
}

void facebookSimulatorLikeUpdate(float x, float y)
{
    _btnLike->setPosition(x * getStage()->getWidth(), y * getStage()->getHeight());
}

void facebookSimulatorLogin(const vector<string>& perm)
{
    getStage()->addTween(TweenDummy(), 1000)->addDoneCallback([=](Event*)
    {
        spFacebookDialog dialog = new FacebookDialog;
        dialog->setScale(1.0f / getStage()->getScaleX());
        dialog->setSize(500, 300);
        string st;
        for (auto s : perm)
            st += s + "\n";
        dialog->setTitle(st);
        getStage()->addChild(dialog);

        FacebookDialog* ptr = dialog.get();

        dialog->_btnOk->addClickListener([ = ](Event * e)
        {

            ptr->detach();
            e->removeListener();

            _isLoggedIn = true;
            _facebookToken = getValue(_facebook, "token");
            _userID = getValue(_facebook, "userID");

            string str = _permissions;
            if (!perm.empty())
            {
                if (!str.empty())
                    str += ",";
                for (auto n : perm)
                    str = str + n + ",";
                if (!str.empty())
                    str.pop_back();
            }
            _permissions = str;// getValue(_facebook, "permissions");
            save();

            facebook::internal::loginResult(true, _userID, _facebookToken);
        });

        dialog->_btnCancel->addClickListener([ = ](Event * e)
        {
            ptr->detach();
            e->removeListener();

            facebook::internal::loginResult(false, "", "");
        });
    });
}

void facebookSimulatorLogout()
{
    _facebookToken.clear();
    _userID.clear();
    _permissions.clear();
    _isLoggedIn = false;
    save();
}

std::string getValue(const Json::Value& obj, const char* key)
{
    if (obj[key].empty())
        return "";
    return obj[key].asString();
}

void facebookSimulatorInit()
{
    logs::messageln("Facebook Simulator Init");

    _isLoggedIn = false;
    _facebookToken = "";
    _userID = "";
    _permissions = "";
    _appID = "";

    file::buffer bf;

    if (file::read(".facebook", bf, ep_ignore_error))
    {
        Json::Reader reader;
        reader.parse((char*)&bf.front(), (char*)&bf.front() + bf.size(), _facebook, false);


        _appID = getValue(_facebook, "appID");

        if (_facebook["loggedIn"].asBool())
        {
            _isLoggedIn = true;
            _facebookToken = getValue(_facebook, "token");
            _userID = getValue(_facebook, "userID");
            _permissions = getValue(_facebook, "permissions");
            if (_permissions.empty())
                _permissions = "public_profile";
        }
    }

#ifdef EMSCRIPTEN
    char token[255];
    char id[255];

    EM_ASM_INT(
    {
        var id = getURLParameter("id") || "";
        stringToUTF8(id, $0, 255);

        var token = getURLParameter("token") || "";
        stringToUTF8(token, $1, 255);

    }, id, token);

    if (id[0])
        _userID = id;
    if (token[0])
        _facebookToken = token;
#endif
}

void facebookSimulatorFree()
{
    _btnLike = 0;
}

void facebookSimulatorNewMeRequest()
{
    Json::Value me(Json::objectValue);
    me["id"] = _userID;
    me["name"] = "Your Name";
    Json::StyledWriter writer;
    string data = Json::StyledWriter().write(me);
    facebook::internal::newMeRequestResult(data, false);
}


void facebookSimulatorGetFriends()
{

}

bool facebookSimulatorAppInviteDialog(const string& appLinkUrl, const string& previewImageUrl)
{
    return false;
}

void facebookSimulatorGameRequest(const std::string& title, const std::string& text, const std::vector<std::string>& dest, const std::string& objectID, const std::string& userData)
{
    getStage()->addTween(TweenDummy(), 1000)->addDoneCallback([](Event*)
    {
        spFacebookDialog dialog = new FacebookDialog;
        dialog->setScale(1.0f / getStage()->getScaleX());
        dialog->setSize(500, 300);
        getStage()->addChild(dialog);

        FacebookDialog* ptr = dialog.get();

        dialog->_btnOk->addClickListener([ = ](Event * e)
        {
            ptr->detach();
            e->removeListener();

            facebook::internal::gameRequestResult("<fake>", false);
        });

        dialog->_btnCancel->addClickListener([ = ](Event * e)
        {
            ptr->detach();
            e->removeListener();

            facebook::internal::gameRequestResult("", true);
        });
    });
}

void facebookSimulatorShareLink(const string &link, const string &quote)
{
    getStage()->addTween(TweenDummy(), 1000)->addDoneCallback([=](Event*)
    {
        spFacebookDialog dialog = new FacebookDialog;
        dialog->setScale(1.0f / getStage()->getScaleX());
        dialog->setSize(500, 300);
        dialog->setTitle(link);
        getStage()->addChild(dialog);

        FacebookDialog* ptr = dialog.get();

        dialog->_btnOk->addClickListener([=](Event * e)
        {
            ptr->detach();
            e->removeListener();

            facebook::ShareEvent se(false);
            facebook::internal::dispatch(&se);
        });

        dialog->_btnCancel->addClickListener([=](Event * e)
        {
            ptr->detach();
            e->removeListener();

            facebook::ShareEvent se(true);
            facebook::internal::dispatch(&se);
        });
    });
}

static bool splitStrSep(const string& str, char sep, string& partA, string& partB, error_policy ep)
{
    string copy = str;
    partA = str;

    size_t pos = str.find_first_of(sep);
    if (pos == str.npos)
    {
        handleErrorPolicy(ep, "can't split string: %s", str.c_str());
        return false;
    }

    partA = copy.substr(0, pos);
    partB = copy.substr(pos + 1, str.size() - pos - 1);
    return true;
}

vector<string> facebookSimulatorGetAccessTokenPermissions()
{
    vector<string> perm;
    string p;
    string data = _permissions;
    while (splitStrSep(data, ',', p, data, ep_ignore_error))
        perm.push_back(p);
    perm.push_back(data);

    return perm;
}

void facebookSimulatorInvitableFriendsRequest(const vector<string>& exclude)
{
    getStage()->addTween(TweenDummy(), 1000)->addDoneCallback([ = ](Event*)
    {
        facebook::InvitableFriendsEvent ev;
        ev.status = -1;

        string data =
            R"({
            "data":[
        {
            "id":"AVkqRdGtDgS67je5O9tdLyQcCXS7nxDInzlM7QJTLsWoKDDdvleqOb85auiIlj1ZpMdlLZYhnZ7LfsFocyBG6Dsd1IO6RJVsGJZYTkPb4frTKA",
                "name" : "Andy Megowan",
                "picture" : {
                "data":{
                    "is_silhouette":false,
                        "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/p50x50\/12391826_10153382144255829_7612357279154946740_n.jpg?oh=ae41113655deaa4fa0ff3b5d527d685d&oe=5A2843FD"
                }
            }
        },
        {
            "id":"AVkXSuD-eM8tn1Z1h8UjFFmHxoq2jFx24RpksCK_7tvpxukps-V_wYxLjVirVwOl8rebtyX9VTkY_fr5szJnja0KrNJyN2LX1gf-pBW26Y24Og",
            "name" : "Betty Tikker Davis",
            "picture" : {
            "data":{
                "is_silhouette":false,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/c0.6.50.50\/p50x50\/562726_10151098323433774_2054822595_n.jpg?oh=110e4cf44bfa98b2a43562cf8e3f598c&oe=59F55B2A"
            }
        }
        },
        {
            "id":"AVkP59M4VE9vbH9TNRACk86WJ4B8CFNdUhtQEKdNqIO69ayUysiQQ44UxCyQbswKjBwKu9whlCabeFQN5f2IfMM9qddvd1iYCKVY8S-5ThFM3w",
            "name" : "Nataliia IWin",
            "picture" : {
            "data":{
                "is_silhouette":false,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/c33.33.414.414\/s50x50\/67123_178884815473629_3775692_n.jpg?oh=a2c5759f348714c1288af23d63dbd8a1&oe=5A331062"
            }
        }
        },
        {
            "id":"AVnNMYomg-PI5u4cnpqAanj3peBd8ysqP3isMH7DN-o-AT2__u6uNflKkjmDOR-6ospqEIn-0VsADo7MENDDUgz2zG0WC73VzqzpX-94g06deQ",
            "name" : "Dmitry Climber Koropchenko",
            "picture" : {
            "data":{
                "is_silhouette":false,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/c0.14.50.50\/p50x50\/1913963_100859886603396_6133185_n.jpg?oh=d78fc127bd3296ffa90b4e189bedb5bd&oe=59F17103"
            }
        }
        },
        {
            "id":"AVnUbK6xmYqtYpXvr4Nr1-z654j1fB8DBrtQyf-b8SM55bJjCzrDSH8M05nqXEabCLnhVoMkRb9JS3b_IbOxru4NRKF-zUqn2YpDVnQsHy4Jsw",
            "name" : "Tagir Kudashev",
            "picture" : {
            "data":{
                "is_silhouette":false,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/c0.8.50.50\/p50x50\/12481_781460375198728_1980160723_n.jpg?oh=d373637cd7b71a4a3ca0a04ef6bea844&oe=59F1A210"
            }
        }
        },
        {
            "id":"AVkbWWZGC6-PdQGxNvSH_aSmOpPe94cCMmhhYvyTg7mz2cwXwWrSPzY52ZodCaz2Wk7UxruuUAjtCX2dBlQAauqWaa1CloHxjk58AieK33F70Q",
            "name" : "Slavyana Korobitsyna",
            "picture" : {
            "data":{
                "is_silhouette":false,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/c0.0.50.50\/p50x50\/549013_646020848749690_661025531_n.jpg?oh=3d5cec7a20f121815ebe25c123d668c9&oe=5A32774E"
            }
        }
        },
        {
            "id":"AVkFmbqxnjPHOOZWPQyCX9okhS8kSRotmi0UInX35fpEBzlL7XYw0Po5QxlewBrQYQXW6anzKlSiN7TQs0ev4mWT5LY1Opvsx_VPK1vko9hsKQ",
            "name" : "Pavel Shklovsky",
            "picture" : {
            "data":{
                "is_silhouette":false,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/p50x50\/17425011_1371244766231712_269159504467186983_n.jpg?oh=d1141a7edcc19900fdf60f542a858088&oe=59F83DEF"
            }
        }
        },
        {
            "id":"AVkC6WvgxEGZnHxiql_Q3g0Ws_Vzgd9MWLSrgGk3Xxe5ok-0KOqnZ-_yfXU8r_ccvWWLM5WRL-77rNvwz9-J9jA1gxdaOyOA1fePRDOluaOwJQ",
            "name" : "Ivan Spogreev",
            "picture" : {
            "data":{
                "is_silhouette":false,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/c8.0.50.50\/p50x50\/1939861_702533369767486_1910457867_n.jpg?oh=84dc548d4f73d35fef307951b8edf649&oe=5A352A84"
            }
        }
        },
        {
            "id":"AVmmbjHLtvupdH74hJbw7x-TRxzwRWPTh2MjaNo-guKgN8jafcswEMh83mOrtaO-8ecwIQGsLmPq77L-C6jKfGeVFq3W6XcQwUXCGeTBYABR5g",
            "name" : "Adelya Ibraeva",
            "picture" : {
            "data":{
                "is_silhouette":false,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/c0.8.50.50\/p50x50\/25863_1410278946983_2459805_n.jpg?oh=a8c942a1bd5af662f1fdeea7d28be96c&oe=5A32ABEC"
            }
        }
        },
        {
            "id":"AVnHUcMCqYSSWtrWgekvqF_e2COGIu5LHF8fpbfmU4zuHU5HDEOAppEBdbbG-jHZSLpk-1fBoMAxAyAWF_jxlYPZ11HoTjJb4gvUn3tvrWMU2g",
            "name" : "Michael Plotkin",
            "picture" : {
            "data":{
                "is_silhouette":false,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/c0.0.50.50\/p50x50\/11825037_1105949796099619_9100106404943056990_n.jpg?oh=63e59ee716541da26dcaaef2e563d478&oe=59EE37C3"
            }
        }
        },
        {
            "id":"AVn89VyUuIq8iObFn54HT1_4hV02FQ_2R1emVolnRtwOArbKbRXlf3wE53NsX6ApbrVTKWz1nXJgcLi8o2GckZw0DduJdS0baByaslVqHGNLIg",
            "name" : "Denis Pugachev",
            "picture" : {
            "data":{
                "is_silhouette":false,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/c0.8.50.50\/p50x50\/1621872_691930380828596_914166516_n.jpg?oh=8940490733d9790bc27087e052999c52&oe=5A367E1D"
            }
        }
        },
        {
            "id":"AVnnlMcCfP6xUaOgaArWyHEzC6j53FpWgm0Ry9vK7uwn8E24Jp335ZGf9tFkwElxuY7X-U7mSJr-uisRdKiWlHblLrL07KUdO3YuTSfpvFWCVw",
            "name" : "Arcadiy Yudaev",
            "picture" : {
            "data":{
                "is_silhouette":false,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/p50x50\/1911717_734465343238479_1963763507_n.jpg?oh=51290f6e89f197115c34a62bc3474e49&oe=59FD0683"
            }
        }
        },
        {
            "id":"AVk5knisnYII4T4Kj8W3Xb974LJ8b74fpR035JfxUZ6uYd_Jq5IhiCP-k37XURoXpdnhacMURYqb6knHoISjiJD1ArjGsguz5FQ2oEr4rMpS7g",
            "name" : "Anna  Sergeevna",
            "picture" : {
            "data":{
                "is_silhouette":false,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/c31.31.391.391\/s50x50\/551663_102595446549149_1830582002_n.jpg?oh=f701b836bab0d11fc8fb8c75d69e0180&oe=5A366E76"
            }
        }
        },
        {
            "id":"AVk5tSO0_cKMMvcPZo6NQSbq1DuG7dgdl2acZb1l5x7c_nzQgsatyqEUnwRcQYTZFJDMj-AC13ETQRRHa5cSWxvTTs3-LsfyO8gHPXggsRoGpw",
            "name" : "Anna  Fadeeva",
            "picture" : {
            "data":{
                "is_silhouette":true,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/c15.0.50.50\/p50x50\/1379841_10150004552801901_469209496895221757_n.jpg?oh=2ea25ecb9f05e42469f1d7a4bb6cf0de&oe=59F16C33"
            }
        }
        },
        {
            "id":"AVm436dMbEXzNjai_br-1UJOUwzAySehUICwLAIQve7YF5yRr2T1fMHo5VIDeGPj-WinO2qJHbRtLWzBpW4zPh_kZfkL9bHNeY2dTLDfM7ue4A",
            "name" : "Sofya Bazhenova",
            "picture" : {
            "data":{
                "is_silhouette":false,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/c11.0.50.50\/p50x50\/1379803_619498608101294_1356190505_n.jpg?oh=9c5fa0effa5f99c43673fc77878ba466&oe=59F16794"
            }
        }
        },
        {
            "id":"AVn0ruYE9VmAqbSx2xfBQQ9JLhoWJvhDZhAYxU3pRM91g3cCvYbj5WKxBV92vPh33syBwxvXysKyGqNSUeQMESYm6KgWH26qwgfj3FTtmfcEqg",
            "name" : "Daniil Tutubalin",
            "picture" : {
            "data":{
                "is_silhouette":false,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/p50x50\/13654325_10202055994877219_6218654228739896779_n.jpg?oh=eb0f087ae8fb97ba1e64f33094ae8b78&oe=5A2EB970"
            }
        }
        },
        {
            "id":"AVkHIpIvNLS60c69gfj-9I2VXsxezmV8NTl02aTfH-bzzoKeyFLzkIq79fk0yc1e568KLW0VdhItWe4YUDp8RupF8lRAg4Jvcbctnh6XG4FM1g",
            "name" : "Andrey Sundukov",
            "picture" : {
            "data":{
                "is_silhouette":false,
                    "url" : "https:\/\/z-p3-scontent.xx.fbcdn.net\/v\/t1.0-1\/p50x50\/1506848_10152305182674059_308024372_n.jpg?oh=494373c5d83b402f28d80b0c78bc907e&oe=59F860CA"
            }
        }
        }
            ],
            "paging":{
            "cursors":{
                "before":"QVFIUlRxMTJxNThWUlhWM21VQUJCS21kQmhPQ3RweVVFLXhpMGxEVTVEUnNjRThEeGhuLU9ROGxDSkQ0MmNaaFlreXB5Rnk1c2QyMTYtNjR2LVdpTF9ya2xn",
                    "after" : "QVFIUklTNG9Rdk1wNDZANSlBpMHk2djhKeFFLVEd4Yms3anhKQl9CWF9UODFrUW5UVW5oc1JkeGdndDhfTFZAPa1ZATZAFdNSXFzSi1LbUFjS2ZAPM1dzWTA2b2hB"
            },
                "next" : ""
        }
        })";

        ev.data = data;


        if ((rand() % 3) < 2)
        {
            ev.status = 0;
            facebookSimulatorInvitableFriendsRequest(exclude);
        }
        facebook::internal::dispatch(&ev);
    });
}

bool facebookSimulatorIsLoggedIn()
{
    return _isLoggedIn;
}

std::string facebookSimulatorGetAccessToken()
{
    return _facebookToken;
}

std::string facebookSimulatorGetUserID()
{
    return _userID;
}

std::string facebookSimulatorGetAppID()
{
    return _appID;
}