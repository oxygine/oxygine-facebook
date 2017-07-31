#include "FacebookSimulator.h"
#include "facebook.h"
#include "json/json.h"
#include "core/file.h"
#include "Box9Sprite.h"
#include "TextField.h"
#include "DebugActor.h"
#include "res/Resources.h"
#include "Stage.h"
#include "core/oxygine.h"

bool _isLoggedIn = true;
string _facebookToken = "";
string _userID = "";
string _appID = "";

Json::Value _facebook(Json::objectValue);

std::string getValue(const Json::Value& obj, const char* key);
static void save()
{
    _facebook["loggedIn"] = _isLoggedIn;

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
        addEventListener(TouchEvent::OUT, CLOSURE(this, &Btn::touch));
    }

    void setText(const string& txt)
    {
        _txt->setText(txt);
    }

    void touch(Event* ev)
    {
        if (ev->type == TouchEvent::OVER)
            setColor(Color::GreenYellow);
        if (ev->type == TouchEvent::OUT)
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
        _btnOk->setSize(70, 30);
        _btnOk->setText("Ok");
        addChild(_btnOk);

        _btnCancel = new Btn();
        _btnCancel->setSize(70, 30);
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



void facebookSimulatorLogin()
{
    getStage()->addTween(TweenDummy(), 1000)->addDoneCallback([](Event*)
    {
        spFacebookDialog dialog = new FacebookDialog;
        dialog->setScale(1.0f / getStage()->getScaleX());
        dialog->setSize(500, 300);
        getStage()->addChild(dialog);

        dialog->_btnOk->addClickListener([ = ](Event * e)
        {

            dialog->detach();
            e->removeListener();

            _isLoggedIn = true;
            _facebookToken = getValue(_facebook, "token");
            _userID = getValue(_facebook, "userID");
            save();

            facebook::internal::loginResult(true);
        });

        dialog->_btnCancel->addClickListener([ = ](Event * e)
        {
            dialog->detach();
            e->removeListener();

            facebook::internal::loginResult(false);
        });
    });
}

void facebookSimulatorLogout()
{
    _facebookToken.clear();
    _userID.clear();
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
    log::messageln("Facebook Simulator Init");

    _isLoggedIn = false;
    _facebookToken = "";
    _userID = "";
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

void facebookSimulatorNewMeRequest()
{
    string data = "{\"id\":\"YOUR_FACEBOOK_ID\",\"link\" : \"https://www.facebook.com/app_scoped_user_id/YOUR_FACEBOOK_ID/\",\"name\" : \"YOUR NAME\"}";
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

        dialog->_btnOk->addClickListener([=](Event * e)
        {
            dialog->detach();
            e->removeListener();

            facebook::internal::gameRequestResult("<fake>", false);
        });

        dialog->_btnCancel->addClickListener([=](Event * e)
        {
            dialog->detach();
            e->removeListener();

            facebook::internal::gameRequestResult("", true);
        });
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