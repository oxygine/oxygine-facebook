#pragma once

#include <string>

using namespace std;

void jniFacebookInit();
void jniFacebookFree();
void jniFacebookGameRequest(const string& title, const string& text, const vector<string>& dest, const string& objectID, const std::string& userData);
bool jniFacebookIsLoggedIn();
void jniFacebookGetFriends();
void jniFacebookLogin();
void jniFacebookLogout();
void jniFacebookNewMeRequest();
bool jniFacebookAppInviteDialog(const string& appLinkUrl, const string& previewImageUrl);

string jniFacebookGetAccessToken();
string jniFacebookGetUserID();
string jniFacebookGetAppID();