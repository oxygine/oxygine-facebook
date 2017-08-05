package org.oxygine.facebook;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import com.facebook.*;
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
import java.util.*;

import android.util.Base64;

public class FacebookAdapter extends ActivityObserver
{
    private static String TAG = "FacebookAdapter";

    ProfileTracker profileTracker;
    //AccessToken accessToken;
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
    public native void nativeResponseInvitableFriends(String data, int page);

    public void logout()
    {
        Log.i(TAG, "logout");
        userData = null;
        LoginManager.getInstance().logOut();
    }

    public void login(String[] permissions)
    {
        Log.i(TAG, "login");
        LoginManager.getInstance().logInWithReadPermissions(activity, Arrays.asList(permissions));
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
                        //accessToken = loginResult.getAccessToken();
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
                //accessToken = currentAccessToken;
                if (currentAccessToken != null)
                	newToken(currentAccessToken.getToken());
            }
        };
    }

    public void newMyFriendsRequest() {
        Log.i(TAG, "newMyFriendsRequest");
        GraphRequest request = GraphRequest.newMyFriendsRequest(AccessToken.getCurrentAccessToken(), new GraphRequest.GraphJSONArrayCallback() {
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
                AccessToken.getCurrentAccessToken(),
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
        if (AccessToken.getCurrentAccessToken() == null)
            return "";
        
        return AccessToken.getCurrentAccessToken().getUserId();
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
        AccessToken token = AccessToken.getCurrentAccessToken();
        if (token == null)
            return "";

        if (token.isExpired())
        {
            Log.i(TAG, "getAccessToken::expired " + token.getToken());
            return "";
        }

        return token.getToken();
    }

    public String[] getAccessTokenPermissions()
    {
        AccessToken token = AccessToken.getCurrentAccessToken();
        if (token == null)
            return null;

        Set<String> perm = token.getPermissions();
        return perm.toArray(new String[perm.size()]);
    }

    public void _requestInvitableFriends2(GraphRequest r)
    {
        r.setCallback(new GraphRequest.Callback() {
            public void onCompleted(GraphResponse response) {


                if (response.getError() != null){
                    nativeResponseInvitableFriends(response.getRawResponse(), -2);
                    return;
                }

                GraphRequest next = response.getRequestForPagedResults(GraphResponse.PagingDirection.NEXT);
                int page = 0;
                if (next != null) {

                    page = -1;
                    _requestInvitableFriends2(next);
                }


                try {
                    String obj = response.getJSONObject().getJSONObject("paging").getString("next");
                    Log.d("a", "ad");
                } catch (JSONException exc){
                    Log.d("a", "adasd");
                }


            nativeResponseInvitableFriends(response.getRawResponse(), page);
        }});

        r.executeAsync();
    }

    public void _requestInvitableFriends(Bundle params)
    {
        if (params == null)
            params = new Bundle();
        params.putString("fields", "id,name,picture");

        GraphRequest r = new GraphRequest(
                AccessToken.getCurrentAccessToken(),
                "/me/invitable_friends",
                params,
                HttpMethod.GET);

        _requestInvitableFriends2(r);
    }

    public void requestInvitableFriends()
    {
        _requestInvitableFriends(null);
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

    }

    @Override
    public void onPause() {
    }
}