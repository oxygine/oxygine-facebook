#pragma once

#include <string>
#include <vector>

using namespace std;

void jniFacebookInit();
void jniFacebookFree();
void jniFacebookGameRequest(const string& title, const string& text, const vector<string>& dest, const string& objectID, const std::string& userData);
bool jniFacebookIsLoggedIn();
void jniFacebookGetFriends();
void jniFacebookLogin(const std::vector<std::string>& permissions);
void jniFacebookLogout();
void jniFacebookNewMeRequest();
bool jniFacebookAppInviteDialog(const string& appLinkUrl, const string& previewImageUrl);

string jniFacebookGetAccessToken();
vector<string> jniFacebookGetAccessTokenPermissions();
string jniFacebookGetUserID();
string jniFacebookGetAppID();