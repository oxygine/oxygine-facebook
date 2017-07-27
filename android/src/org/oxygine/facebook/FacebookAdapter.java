package org.oxygine.facebook;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import com.facebook.AccessToken;
import com.facebook.AccessTokenTracker;
import com.facebook.CallbackManager;
import com.facebook.FacebookCallback;
import com.facebook.FacebookException;
import com.facebook.FacebookRequestError;
import com.facebook.FacebookSdk;
import com.facebook.GraphRequest;
import com.facebook.GraphResponse;
import com.facebook.Profile;
import com.facebook.ProfileTracker;
import com.facebook.appevents.AppEventsLogger;
import com.facebook.login.LoginManager;
import com.facebook.login.LoginResult;
import com.facebook.share.model.AppInviteContent;
import com.facebook.share.model.GameRequestContent;
import com.facebook.share.widget.AppInviteDialog;

import com.facebook.share.widget.GameRequestDialog;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.oxygine.lib.extension.ActivityObserver;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

import android.util.Base64;

public class FacebookAdapter extends ActivityObserver
{
    private static String TAG = "FacebookAdapter";

    ProfileTracker profileTracker;
    AccessToken accessToken;
    AccessTokenTracker accessTokenTracker;
    CallbackManager callbackManager;
    Activity activity;
    JSONObject userData;
    GameRequestDialog requestDialog;

    public native void loginResult(boolean value);
    public native void newToken(String value);
    public native void newMyFriendsRequestResult(String data, boolean error);
    public native void newMeRequestResult(String data, boolean error);
    public native void nativeGameRequest(String request, boolean error);

    public void logout()
    {
        Log.i(TAG, "logout");
        userData = null;
        LoginManager.getInstance().logOut();
    }

    public void login()
    {
        Log.i(TAG, "login");
        LoginManager.getInstance().logInWithReadPermissions(activity, Arrays.asList("public_profile", "user_friends"));
    }

    public void printKeyHash()
    {
        try {
            PackageInfo info = activity.getPackageManager().getPackageInfo(
               activity.getPackageName(), PackageManager.GET_SIGNATURES);

            for (Signature signature : info.signatures) {
                MessageDigest md = MessageDigest.getInstance("SHA");
                md.update(signature.toByteArray());
                Log.d("KeyHash:", Base64.encodeToString(md.digest(), Base64.DEFAULT));
            }
        } catch (Exception e) {         
        }
    }

    @Override
    public void onCreate()
    {
        printKeyHash();

        requestDialog = new GameRequestDialog(_activity);
        requestDialog.registerCallback(callbackManager,
                new FacebookCallback<GameRequestDialog.Result>() {
                    public void onSuccess(GameRequestDialog.Result result) {
                        //String id = result.getId();
                        nativeGameRequest(result.getRequestId(), false);
                    }
                    public void onCancel() {
                        nativeGameRequest(null, true);
                    }
                    public void onError(FacebookException error) {
                        nativeGameRequest(null, true);
                    }
                }
        );
    }

    public FacebookAdapter(Activity a)
    {
        Log.i(TAG, "FacebookAdapter");

        activity = a;
        FacebookSdk.sdkInitialize(a.getApplicationContext());

        callbackManager = CallbackManager.Factory.create();

        LoginManager.getInstance().registerCallback(callbackManager,
                new FacebookCallback<LoginResult>() {
                    @Override
                    public void onSuccess(LoginResult loginResult) {
                        Log.i(TAG, "Login::onSuccess");
                        accessToken = loginResult.getAccessToken();
                        loginResult(true);
                    }

                    @Override
                    public void onCancel() {
                        Log.i(TAG, "Login::onCancel");
                        loginResult(false);
                    }

                    @Override
                    public void onError(FacebookException exception) {
                        Log.i(TAG, "Login::onError");
                        loginResult(false);
                    }
                });


        accessTokenTracker = new AccessTokenTracker() {
            @Override
            protected void onCurrentAccessTokenChanged(
                    AccessToken oldAccessToken,
                    AccessToken currentAccessToken) {
                // Set the access token using
                // currentAccessToken when it's loaded or set.
                accessToken = currentAccessToken;
                if (accessToken != null)
                	newToken(accessToken.getToken());
            }
        };
        // If the access token is available already assign it.
        accessToken = AccessToken.getCurrentAccessToken();

/*
        profileTracker = new ProfileTracker() {
            @Override
            protected void onCurrentProfileChanged(
                    Profile oldProfile,
                    Profile currentProfile) {
                // App code
            }
        };*/


    }

    public void newMyFriendsRequest() {
        Log.i(TAG, "newMyFriendsRequest");
        GraphRequest request = GraphRequest.newMyFriendsRequest(accessToken, new GraphRequest.GraphJSONArrayCallback() {
            @Override
            public void onCompleted(JSONArray objects, GraphResponse response) {
                Log.i(TAG, "newMyFriendsRequest:onCompleted " + objects.toString());
                newMyFriendsRequestResult(objects.toString(), response.getError() != null);
            }
        });
        request.executeAsync();
    }


    public boolean appInviteDialog(final String appLinkUrl,final String previewImageUrl) {

        //appLinkUrl = "https://www.mydomain.com/myapplink";
        //previewImageUrl = "https://www.mydomain.com/my_invite_image.jpg";

        if (AppInviteDialog.canShow()) {
            AppInviteContent content = new AppInviteContent.Builder()
                    .setApplinkUrl(appLinkUrl)
                    .setPreviewImageUrl(previewImageUrl)
                    .build();
            AppInviteDialog.show(activity, content);
            return true;
        }

        return false;
    }

    public void newMeRequest() {
        Log.i(TAG, "newMeRequest");
        GraphRequest request = GraphRequest.newMeRequest(
                accessToken,
                new GraphRequest.GraphJSONObjectCallback() {
                    @Override
                    public void onCompleted(JSONObject object, GraphResponse response) {
                    	try
                    	{
	                        userData = object;
	                        newMeRequestResult(object.toString(), response.getError() != null);
                    	}
                    	catch (Exception e) 
                    	{	      
                    		newMeRequestResult("", true);  
	    				}
                    }
                });
        Bundle parameters = new Bundle();
        parameters.putString("fields", "id,name,link");
        request.setParameters(parameters);
        request.executeAsync();
    }

    public String getUserID()
    {
        if (accessToken == null)
            return "";
        
        return accessToken.getUserId();
    }

    public void sendGameRequest(final String title, final String text, final String[] dest, final String objectID, final String userData)
    {
        _activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {


                GameRequestContent.Builder builder = new GameRequestContent.Builder();

                if (text != null)
                    builder.setMessage(text);

                if (title != null)
                    builder.setTitle(title);

                if (dest != null)
                {
                    List<String> rec = new ArrayList<>(Arrays.asList(dest));
                    builder.setRecipients(rec);
                }

                if (objectID != null)
                    builder.setObjectId(objectID);

                if (userData != null)
                    builder.setData(userData);

                //builder.setActionType(GameRequestContent.ActionType.SEND);

                requestDialog.show(builder.build());
            }
        });
    }

    public String getAppID()
    {        
        return FacebookSdk.getApplicationId();
    }

    public String getAccessToken()
    {
        if (accessToken == null)
            return "";

        if (accessToken.isExpired())
        {
            Log.i(TAG, "getAccessToken::expired " + accessToken.getToken());
            return "";
        }

        return accessToken.getToken();
    }

    public boolean isLoggedIn() {
        Log.i(TAG, "isLoggedIn");
        AccessToken accessToken = AccessToken.getCurrentAccessToken();
        return accessToken != null && !accessToken.isExpired();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        callbackManager.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void onDestroy() {
        if (accessTokenTracker != null)
            accessTokenTracker.stopTracking();
        if (profileTracker != null)
            profileTracker.stopTracking();
    }

    @Override
    public void onResume() {
        // Logs 'install' and 'app activate' App Events.
        AppEventsLogger.activateApp(activity);
    }

    @Override
    public void onPause() {
        // Logs 'app deactivate' App Event.
        AppEventsLogger.deactivateApp(activity);
    }
}