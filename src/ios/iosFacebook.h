//
//  Header.h
//  HelloWorldFacebook_ios
//
//  Created by Denis on 17/03/16.
//  Copyright © 2016 Mac. All rights reserved.
//

#ifndef Header_h
#define Header_h
#include <string>
#include <vector>

void iosFacebookInit();
void iosFacebookFree();


void iosFacebookLogin(const std::vector<std::string>& permissions);
void iosFacebookLogout();
void iosFacebookGameRequest(const std::string& title, const std::string& text, const std::vector<std::string>& dest, const std::string& objectID, const std::string& userData);

std::string iosFacebookGetAccessToken();
std::string iosFacebookGetUserID();
std::string iosFacebookGetAppID();
std::vector<std::string> iosFacebookGetPermissions();
void iosFacebookRequestInvitableFriends(const std::vector<std::string>&);
void iosFacebookRequestMe();
void iosFacebookShareLink(const std::string &url, const std::string &quote);

#endif /* Header_h */
