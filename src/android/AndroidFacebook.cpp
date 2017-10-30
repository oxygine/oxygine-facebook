#include <jni.h>
#include <android/log.h>
#include <assert.h>
#include "ox/oxygine.hpp"
#include "ox/Object.hpp"
#include "ox/ThreadMessages.hpp"
#include "ox/oxygine.hpp"
#include "oxygine/core/android/jniHelper.h"
#include "oxygine/core/android/jniUtils.h"
#include "ox/json.hpp"
#include "src/facebook.h"


using namespace oxygine;

jclass _jFacebookClass = 0;
jobject _jFacebookObject = 0;

bool isFacebookEnabled()
{
    return _jFacebookClass && _jFacebookObject;
}


extern "C"
{
    JNIEXPORT void JNICALL Java_org_oxygine_facebook_FacebookAdapter_newMeRequestResult(JNIEnv* env, jobject obj, jstring json_data, jboolean error)
    {
        string data = jniGetString(env, json_data);

        core::getMainThreadDispatcher().postCallback([ = ]()
        {
            facebook::internal::newMeRequestResult(data , (bool) error) ;
        });
    }

    JNIEXPORT void JNICALL Java_org_oxygine_facebook_FacebookAdapter_newToken(JNIEnv* env, jobject obj, jstring json_data)
    {
        string data = jniGetString(env, json_data);

        core::getMainThreadDispatcher().postCallback([ = ]()
        {
            facebook::internal::newToken(data) ;
        });
    }

    JNIEXPORT void JNICALL Java_org_oxygine_facebook_FacebookAdapter_loginResult(JNIEnv* env, jobject obj, jboolean value)
    {
        core::getMainThreadDispatcher().postCallback([ = ]()
        {
            facebook::internal::loginResult((bool) value) ;
        });
    }

    JNIEXPORT void JNICALL Java_org_oxygine_facebook_FacebookAdapter_newMyFriendsRequestResult(JNIEnv* env, jobject obj, jstring json_data, jboolean error)
    {
        string data = jniGetString(env, json_data);

        core::getMainThreadDispatcher().postCallback([ = ]()
        {
            facebook::internal::newMyFriendsRequestResult(data, (bool) error);
        });
    }

    JNIEXPORT void JNICALL Java_org_oxygine_facebook_FacebookAdapter_nativeGameRequest(JNIEnv* env, jobject obj, jstring jData, jboolean error)
    {
        string data = jniGetString(env, jData);

        core::getMainThreadDispatcher().postCallback([=]()
        {
            facebook::internal::gameRequestResult(data, (bool)error);
        });
    }

    JNIEXPORT void JNICALL Java_org_oxygine_facebook_FacebookAdapter_nativeShareResult(JNIEnv* env, jobject obj, jstring jData, jboolean canceled)
    {
        string data = jniGetString(env, jData);

        core::getMainThreadDispatcher().postCallback([=]()
        {
            facebook::ShareEvent se((bool)canceled);
            facebook::internal::dispatch(&se);
        });
    }

    JNIEXPORT void JNICALL Java_org_oxygine_facebook_FacebookAdapter_nativeResponseInvitableFriends(JNIEnv* env, jobject obj, jstring jdata, int page)
    {
        string data = jniGetString(env, jdata);

        core::getMainThreadDispatcher().postCallback([ = ]()
        {
            facebook::InvitableFriendsEvent ev;
            ev.data = data;
            ev.status = page;
            facebook::internal::dispatch(&ev);
        });
    }
}


void jniFacebookInit()
{
    try
    {
        JNIEnv* env = jniGetEnv();
        LOCAL_REF_HOLDER(env);
        JNI_NOT_NULL(env);

        _jFacebookClass = env->FindClass("org/oxygine/facebook/FacebookAdapter");
        JNI_NOT_NULL(_jFacebookClass);

        _jFacebookClass = (jclass) env->NewGlobalRef(_jFacebookClass);
        JNI_NOT_NULL(_jFacebookClass);

        _jFacebookObject = env->NewGlobalRef(jniFindExtension(env, _jFacebookClass));
        JNI_NOT_NULL(_jFacebookObject);
    }
    catch (const notFound&)
    {
        log::error("jniFacebookInit failed, class/member not found");
    }
}

void jniFacebookFree()
{
    if (!isFacebookEnabled())
        return;


    JNIEnv* env = jniGetEnv();
    LOCAL_REF_HOLDER(env);

    env->DeleteGlobalRef(_jFacebookClass);
    _jFacebookClass = 0;

    env->DeleteGlobalRef(_jFacebookObject);
    _jFacebookObject = 0;
}

bool jniFacebookIsLoggedIn()
{
    if (!isFacebookEnabled())
        return false;

    bool result = false;

    JNIEnv* env = jniGetEnv();
    LOCAL_REF_HOLDER(env);

    jmethodID jisLoggedIn = env->GetMethodID(_jFacebookClass, "isLoggedIn", "()Z");
    JNI_NOT_NULL(jisLoggedIn);

    jboolean jb = env->CallBooleanMethod(_jFacebookObject, jisLoggedIn);
    result = (bool) jb;

    return result;
}

void jniFacebookNewMeRequest()
{
    if (!isFacebookEnabled())
        return;

    JNIEnv* env = jniGetEnv();
    LOCAL_REF_HOLDER(env);

    jmethodID jnewMeRequest = env->GetMethodID(_jFacebookClass, "newMeRequest", "()V");
    JNI_NOT_NULL(jnewMeRequest);
    env->CallVoidMethod(_jFacebookObject, jnewMeRequest);
}

string jniFacebookGetAccessToken()
{
    if (!isFacebookEnabled())
        return "";


    JNIEnv* env = jniGetEnv();
    LOCAL_REF_HOLDER(env);

    jmethodID jgetToken = env->GetMethodID(_jFacebookClass, "getAccessToken", "()Ljava/lang/String;");
    JNI_NOT_NULL(jgetToken);

    jobject obj = env->CallObjectMethod(_jFacebookObject, jgetToken);
    string data = jniGetString(env, (jstring) obj);

    return data;
}

vector<string> jniFacebookGetAccessTokenPermissions()
{
    if (!isFacebookEnabled())
        return {};


    JNIEnv* env = jniGetEnv();
    LOCAL_REF_HOLDER(env);

    jmethodID jFn = env->GetMethodID(_jFacebookClass, "getAccessTokenPermissions", "()[Ljava/lang/String;");
    JNI_NOT_NULL(jFn);

    jobjectArray obj = (jobjectArray)env->CallObjectMethod(_jFacebookObject, jFn);

    vector<string> res;
    jniGetStringArray(res, env, obj);

    return res;
}

string jniFacebookGetUserID()
{
    if (!isFacebookEnabled())
        return "";

    JNIEnv* env = jniGetEnv();
    LOCAL_REF_HOLDER(env);

    jmethodID jgetUserID = env->GetMethodID(_jFacebookClass, "getUserID", "()Ljava/lang/String;");
    JNI_NOT_NULL(jgetUserID);

    jobject obj = env->CallObjectMethod(_jFacebookObject, jgetUserID);
    string data = jniGetString(env, (jstring)obj);
    return data;
}



string jniFacebookGetAppID()
{
    if (!isFacebookEnabled())
        return "";


    JNIEnv* env = jniGetEnv();
    LOCAL_REF_HOLDER(env);

    jmethodID jm = env->GetMethodID(_jFacebookClass, "getAppID", "()Ljava/lang/String;");
    JNI_NOT_NULL(jm);

    jobject obj = env->CallObjectMethod(_jFacebookObject, jm);
    string data = jniGetString(env, (jstring)obj);
    return data;
}

bool jniFacebookAppInviteDialog(const string& appLinkUrl, const string& previewImageUrl)
{
    if (!isFacebookEnabled())
        return false;


    JNIEnv* env = jniGetEnv();
    LOCAL_REF_HOLDER(env);

    jmethodID jappInviteDialog = env->GetMethodID(_jFacebookClass, "appInviteDialog", "([Ljava/lang/String;[Ljava/lang/String;)Z");
    JNI_NOT_NULL(jappInviteDialog);

    jstring jappLinkUrl = env->NewStringUTF(appLinkUrl.c_str());
    jstring jpreviewImageUrl = env->NewStringUTF(previewImageUrl.c_str());

    env->CallVoidMethod(_jFacebookObject, jappInviteDialog, jappLinkUrl, jpreviewImageUrl);

    return true;
}

void jniFacebookGetFriends()
{
    if (!isFacebookEnabled())
        return;

    JNIEnv* env = jniGetEnv();
    LOCAL_REF_HOLDER(env);

    jmethodID jgetFriends = env->GetMethodID(_jFacebookClass, "newMyFriendsRequest", "()V");
    JNI_NOT_NULL(jgetFriends);
    env->CallVoidMethod(_jFacebookObject, jgetFriends);
}

void jniFacebookLogin(const std::vector<std::string>& permissions)
{
    if (!isFacebookEnabled())
        return;

    JNIEnv* env = jniGetEnv();
    LOCAL_REF_HOLDER(env);

    jmethodID jlogin = env->GetMethodID(_jFacebookClass, "login", "([Ljava/lang/String;)V");
    JNI_NOT_NULL(jlogin);
    env->CallVoidMethod(_jFacebookObject, jlogin, jniGetObjectStringArray(permissions, env));
}

void jniFacebookLogout()
{
    if (!isFacebookEnabled())
        return;

    JNIEnv* env = jniGetEnv();
    LOCAL_REF_HOLDER(env);

    jmethodID jlogout = env->GetMethodID(_jFacebookClass, "logout", "()V");
    JNI_NOT_NULL(jlogout);
    env->CallVoidMethod(_jFacebookObject, jlogout);
}

void jniFacebookGameRequest(const string& title, const string& text, const vector<string>& dest, const string& objectID, const std::string& userData)
{
    if (!isFacebookEnabled())
        return;

    JNIEnv* env = jniGetEnv();
    LOCAL_REF_HOLDER(env);

    jmethodID jfunc = env->GetMethodID(_jFacebookClass, "sendGameRequest", "(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");

    jstring jTitle = 0;
    if (!title.empty())
        jTitle = env->NewStringUTF(title.c_str());

    jstring jText = 0;
    if (!text.empty())
        jText = env->NewStringUTF(text.c_str());

    jstring jobjectID = 0;
    if (!objectID.empty())
        jobjectID = env->NewStringUTF(objectID.c_str());

    jstring juserData = 0;
    if (!userData.empty())
        env->NewStringUTF(userData.c_str());

    jobjectArray jdest = jniGetObjectStringArray(dest, env);

    env->CallVoidMethod(_jFacebookObject, jfunc, jTitle, jText, jdest, jobjectID, juserData);
}

void jniFacebookRequestInvitableFriends(const vector<string>& exc)
{
    if (!isFacebookEnabled())
        return;

    JNIEnv* env = jniGetEnv();
    LOCAL_REF_HOLDER(env);

    jmethodID jFn = env->GetMethodID(_jFacebookClass, "requestInvitableFriends", "([Ljava/lang/String;)V");
    JNI_NOT_NULL(jFn);

    jobjectArray arr =  jniGetObjectStringArray(exc, env);
    env->CallVoidMethod(_jFacebookObject, jFn, arr);
}

void jniFacebookShareLink(const string &url, const string &quote)
{
    if (!isFacebookEnabled())
        return;


    JNIEnv* env = jniGetEnv();
    LOCAL_REF_HOLDER(env);

    jmethodID jfn = env->GetMethodID(_jFacebookClass, "shareLink", "(Ljava/lang/String;Ljava/lang/String;)V");
    JNI_NOT_NULL(jfn);

    env->CallVoidMethod(_jFacebookObject, jfn, env->NewStringUTF(url.c_str()), env->NewStringUTF(quote.c_str()));
}