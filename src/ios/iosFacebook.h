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


void iosFacebookLogin();
void iosFacebookLogout();
void iosFacebookGameRequest(const std::string &title, const std::string &text, const std::vector<std::string>& dest, const std::string &objectID, const std::string &userData);

std::string iosFacebookGetAccessToken();
std::string iosFacebookGetUserID();
std::string iosFacebookGetAppID();
void iosFacebookRequestMe();
#endif /* Header_h */
