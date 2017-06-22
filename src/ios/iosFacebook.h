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

void iosFacebookLogin();
void iosFacebookLogout();

std::string iosFacebookGetAccessToken();
std::string iosFacebookGetUserID();
std::string iosFacebookGetAppID();
void iosFacebookRequestMe();
#endif /* Header_h */
