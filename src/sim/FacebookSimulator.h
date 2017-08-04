#pragma once
#include <string>
#include <vector>
using namespace std;


void facebookSimulatorInit();
void facebookSimulatorLogin(const vector<string>&);
void facebookSimulatorLogout();
void facebookSimulatorGetFriends();
bool facebookSimulatorIsLoggedIn();
void facebookSimulatorNewMeRequest();
bool facebookSimulatorAppInviteDialog(const string& appLinkUrl, const string& previewImageUrl);
void facebookSimulatorGameRequest(const std::string& title, const std::string& text, const std::vector<std::string>& dest, const std::string& objectID, const std::string& userData);

std::string facebookSimulatorGetAccessToken();
std::string facebookSimulatorGetUserID();
std::string facebookSimulatorGetAppID();