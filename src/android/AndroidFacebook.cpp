#include <jni.h>
#include <android/log.h>
#include <assert.h>
#include "core/oxygine.h"
#include "core/Object.h"
#include "core/ThreadMessages.h"
#include "core/oxygine.h"
#include "core/android/jniHelper.h"
#include "core/android/jniUtils.h"
#include "json/json.h"
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

    JNIEXPORT void JNICALL Java_org_oxygine_facebook_FacebookAdapter_nativeGameRequest(JNIEnv* env, jobject obj, jstring jrequestID, jboolean error)
    {
        string requestID = jniGetString(env, jrequestID);

        core::getMainThreadDispatcher().postCallback([ = ]()
        {
            facebook::internal::gameRequestResult(requestID, (bool) error);
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

    try
    {
        JNIEnv* env = jniGetEnv();
        LOCAL_REF_HOLDER(env);

        env->DeleteGlobalRef(_jFacebookClass);
        _jFacebookClass = 0;

        env->DeleteGlobalRef(_jFacebookObject);
        _jFacebookObject = 0;
    }
    catch (const notFound&)
    {

    }
}

bool jniFacebookIsLoggedIn()
{
    if (!isFacebookEnabled())
        return false;

    bool result = false;
    try
    {
        JNIEnv* env = jniGetEnv();
        LOCAL_REF_HOLDER(env);

        jmethodID jisLoggedIn = env->GetMethodID(_jFacebookClass, "isLoggedIn", "()Z");
        JNI_NOT_NULL(jisLoggedIn);

        jboolean jb = env->CallBooleanMethod(_jFacebookObject, jisLoggedIn);
        result = (bool) jb;

    }
    catch (const notFound&)
    {
        log::error("jniFacebookIsLoggedIn failed, class/member not found");
    }

    return result;
}

void jniFacebookNewMeRequest()
{
    if (!isFacebookEnabled())
        return;

    try
    {
        JNIEnv* env = jniGetEnv();
        LOCAL_REF_HOLDER(env);

        jmethodID jnewMeRequest = env->GetMethodID(_jFacebookClass, "newMeRequest", "()V");
        JNI_NOT_NULL(jnewMeRequest);
        env->CallVoidMethod(_jFacebookObject, jnewMeRequest);

    }
    catch (const notFound&)
    {
        log::error("jniFacebookNewMeRequest failed, class/member not found");
    }
}

string jniFacebookGetAccessToken()
{
    if (!isFacebookEnabled())
        return "";

    try
    {
        JNIEnv* env = jniGetEnv();
        LOCAL_REF_HOLDER(env);

        jmethodID jgetToken = env->GetMethodID(_jFacebookClass, "getAccessToken", "()Ljava/lang/String;");
        JNI_NOT_NULL(jgetToken);

        jobject obj = env->CallObjectMethod(_jFacebookObject, jgetToken);
        string data = jniGetString(env, (jstring) obj);
        return data;

    }
    catch (const notFound&)
    {
        log::error("jniFacebookGetAccessToken failed, class/member not found");
    }

    return "";
}

string jniFacebookGetUserID()
{
    if (!isFacebookEnabled())
        return "";

    try
    {
        JNIEnv* env = jniGetEnv();
        LOCAL_REF_HOLDER(env);

        jmethodID jgetUserID = env->GetMethodID(_jFacebookClass, "getUserID", "()Ljava/lang/String;");
        JNI_NOT_NULL(jgetUserID);

        jobject obj = env->CallObjectMethod(_jFacebookObject, jgetUserID);
        string data = jniGetString(env, (jstring)obj);
        return data;

    }
    catch (const notFound&)
    {
        log::error("jniFacebookGetUserID failed, class/member not found");
    }

    return "";
}



string jniFacebookGetAppID()
{
    if (!isFacebookEnabled())
        return "";

    try
    {
        JNIEnv* env = jniGetEnv();
        LOCAL_REF_HOLDER(env);

        jmethodID jm = env->GetMethodID(_jFacebookClass, "getAppID", "()Ljava/lang/String;");
        JNI_NOT_NULL(jm);

        jobject obj = env->CallObjectMethod(_jFacebookObject, jm);
        string data = jniGetString(env, (jstring)obj);
        return data;

    }
    catch (const notFound&)
    {
        log::error("jniFacebookGetAppID failed, class/member not found");
    }

    return "";
}

bool jniFacebookAppInviteDialog(const string& appLinkUrl, const string& previewImageUrl)
{
    if (!isFacebookEnabled())
        return false;

    try
    {
        JNIEnv* env = jniGetEnv();
        LOCAL_REF_HOLDER(env);

        jmethodID jappInviteDialog = env->GetMethodID(_jFacebookClass, "appInviteDialog", "([Ljava/lang/String;[Ljava/lang/String;)Z");
        JNI_NOT_NULL(jappInviteDialog);

        jstring jappLinkUrl = env->NewStringUTF(appLinkUrl.c_str());
        jstring jpreviewImageUrl = env->NewStringUTF(previewImageUrl.c_str());

        env->CallVoidMethod(_jFacebookObject, jappInviteDialog, jappLinkUrl, jpreviewImageUrl);

    }
    catch (const notFound&)
    {
        log::error("jniFacebookGetFriends failed, class/member not found");
        return false;
    }
    return true;
}

void jniFacebookGetFriends()
{
    if (!isFacebookEnabled())
        return;

    try
    {
        JNIEnv* env = jniGetEnv();
        LOCAL_REF_HOLDER(env);

        jmethodID jgetFriends = env->GetMethodID(_jFacebookClass, "newMyFriendsRequest", "()V");
        JNI_NOT_NULL(jgetFriends);
        env->CallVoidMethod(_jFacebookObject, jgetFriends);

    }
    catch (const notFound&)
    {
        log::error("jniFacebookGetFriends failed, class/member not found");
    }

}

void jniFacebookLogin()
{
    if (!isFacebookEnabled())
        return;

    try
    {
        JNIEnv* env = jniGetEnv();
        LOCAL_REF_HOLDER(env);

        jmethodID jlogin = env->GetMethodID(_jFacebookClass, "login", "()V");
        JNI_NOT_NULL(jlogin);
        env->CallVoidMethod(_jFacebookObject, jlogin);

    }
    catch (const notFound&)
    {
        log::error("jniFacebookLogin failed, class/member not found");
    }
}

void jniFacebookLogout()
{
    if (!isFacebookEnabled())
        return;

    try
    {
        JNIEnv* env = jniGetEnv();
        LOCAL_REF_HOLDER(env);

        jmethodID jlogout = env->GetMethodID(_jFacebookClass, "logout", "()V");
        JNI_NOT_NULL(jlogout);
        env->CallVoidMethod(_jFacebookObject, jlogout);

    }
    catch (const notFound&)
    {
        log::error("jniFacebookLogout failed, class/member not found");
    }
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