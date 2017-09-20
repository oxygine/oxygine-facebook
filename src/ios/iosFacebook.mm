//
//  iosFacebook.m
//  HelloWorldFacebook_ios
//
//  Created by Denis on 17/03/16.
//  Copyright © 2016 Mac. All rights reserved.
//
#include "iosFacebook.h"
#import <Foundation/Foundation.h>
#import <FBSDKCoreKit/FBSDKCoreKit.h>
#import <FBSDKLoginKit/FBSDKLoginKit.h>
#import <FBSDKShareKit/FBSDKShareKit.h>
#include "facebook.h"




@interface FacebookRequests:NSObject<FBSDKGameRequestDialogDelegate>
{
}
@end

@implementation FacebookRequests

- (id)init
{
    if (!(self = [super init]))
        return nil;
    
    return self;
}


#pragma mark - FBSDKGameRequestDialogDelegate


/**
 Sent to the delegate when the game request completes without error.
 - Parameter gameRequestDialog: The FBSDKGameRequestDialog that completed.
 - Parameter results: The results from the dialog.  This may be nil or empty.
 */
- (void)gameRequestDialog:(FBSDKGameRequestDialog *)gameRequestDialog didCompleteWithResults:(NSDictionary *)results
{
    NSError *error2;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:results
                                                       options:NSJSONWritingPrettyPrinted // Pass 0 if you don't care about the readability of the generated string
                                                         error:&error2];
    
    NSString *jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    
    NSString *request = [results objectForKey:@"request"];
    if (request)
        facebook::internal::gameRequestResult([jsonString UTF8String], false);
    else
        facebook::internal::gameRequestResult("", true);
}

/**
 Sent to the delegate when the game request encounters an error.
 - Parameter gameRequestDialog: The FBSDKGameRequestDialog that completed.
 - Parameter error: The error.
 */
- (void)gameRequestDialog:(FBSDKGameRequestDialog *)gameRequestDialog didFailWithError:(NSError *)error
{
    facebook::internal::gameRequestResult("", true);
}

/**
 Sent to the delegate when the game request dialog is cancelled.
 - Parameter gameRequestDialog: The FBSDKGameRequestDialog that completed.
 */
- (void)gameRequestDialogDidCancel:(FBSDKGameRequestDialog *)gameRequestDialog
{
    facebook::internal::gameRequestResult("", true);
}
#pragma mark -

@end

FacebookRequests* requests = 0;



UIViewController * getViewcontrollerForFB(void)
{
    @try {
        UIWindow *window = [[UIApplication sharedApplication] keyWindow];
        if ( window == nullptr )
            return nullptr;
    
        UIView* view = [window.subviews objectAtIndex:0];
        if(!view)
            return nullptr;
        id nextResponder = [view nextResponder];
        if(!nextResponder)
            return nullptr;
        if( [nextResponder isKindOfClass:[UIViewController class]] )
            return (UIViewController *)nextResponder;
    }
    @catch(NSException *e) {
    }
    
    return nullptr;
}

void iosFacebookLogin(const vector<string> &permissions)
{
    //FBSDKAccessToken *t = [FBSDKAccessToken currentAccessToken];
    
    NSMutableArray *perm = [NSMutableArray array];
    
    for (const string &item:permissions)
    {
        [perm addObject:[NSString stringWithUTF8String:item.c_str()]];
    }
    
    
    FBSDKLoginManager *login = [[FBSDKLoginManager alloc] init];
    [login
     logInWithReadPermissions:perm
     fromViewController:nil
     handler:^(FBSDKLoginManagerLoginResult *result, NSError *error) {
         
         if (error) {
             facebook::internal::loginResult(false);
         } else if (result.isCancelled) {
             facebook::internal::loginResult(false);
         } else {
             NSLog(@"Logged in");
             facebook::internal::loginResult(true);
         }
     }];
}

void iosFacebookLogout()
{
    FBSDKLoginManager *loginManager = [[FBSDKLoginManager alloc] init];
    [loginManager logOut];
}

std::string iosFacebookGetAccessToken()
{
    if ([FBSDKAccessToken currentAccessToken])
        return [[FBSDKAccessToken currentAccessToken ].tokenString UTF8String];
    return "";
}

std::string iosFacebookGetUserID()
{
    if ([FBSDKAccessToken currentAccessToken])
        return [[FBSDKAccessToken currentAccessToken].userID UTF8String];
    return "";
}

std::string iosFacebookGetAppID()
{
    return [[FBSDKSettings appID] UTF8String];
}

std::vector<std::string> iosFacebookGetPermissions()
{
    std::vector<std::string> permissions;
    
    FBSDKAccessToken *token = [FBSDKAccessToken currentAccessToken];
    if (token)
    {
        NSArray* perm = [[token permissions] allObjects];
        for (int i = 0; i < [perm count]; ++i)
            permissions.push_back([[perm objectAtIndex: i] UTF8String]);
        //return [[FBSDKAccessToken currentAccessToken].userID UTF8String];
    }
    
    return permissions;
}

void invFriendsRequest(NSDictionary *params)
{
    FBSDKGraphRequest *request = [[FBSDKGraphRequest alloc]
                                  initWithGraphPath:@"/me/invitable_friends"
                                  parameters:params
                                  HTTPMethod:@"GET"];
    [request startWithCompletionHandler:^(FBSDKGraphRequestConnection *connection,
                                          id result,
                                          NSError *error) {
        
        facebook::InvitableFriendsEvent ev;
        
        if (error)
        {
            ev.status = -2;
            
            facebook::dispatcher()->dispatchEvent(&ev);
            
            return;
        }
        
        
        NSError *error2;
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:result
                                                           options:NSJSONWritingPrettyPrinted // Pass 0 if you don't care about the readability of the generated string
                                                             error:&error2];

        NSString *jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    
        ev.data = [jsonString UTF8String];
        
        
        // Handle the result
        NSString* next  = result[@"paging"][@"cursors"][@"after"];
        //NSString* next = [t objectForKey:@"after"];
        if (next)
        {
            NSDictionary *params = @{@"fields":@"id,name,picture", @"after":next};
            invFriendsRequest(params);
            ev.status = 0;
        }
        else
            ev.status = -1;
        
        
        facebook::dispatcher()->dispatchEvent(&ev);

    }];
}

void iosFacebookRequestInvitableFriends(const vector<string> &exclude_ids)
{
#if 1
    string exc;
    for (const string &fid:exclude_ids)
        exc += "'" + fid + "',";
    
    if (!exc.empty())
        exc.pop_back();
    
    exc = "[" + exc  + "]";
    NSDictionary *params = @{@"fields":@"id,name,picture", @"limit":@5000, @"excluded_ids": [NSString stringWithUTF8String:exc.c_str()] };
#else
    
    NSMutableDictionary *params = [NSMutableDictionary dictionary];
    [params setValue:@"id,name,picture" forKey:@"fields"];
    if (!exclude_ids.empty())
    {
        NSMutableArray *exc = [NSMutableArray array];
        for (const string &fid:exclude_ids)
        {
            [exc addObject:[NSString stringWithUTF8String:fid.c_str()] ];
        }
        [params setValue:exc forKey:@"excluded_ids"];
    }
    
#endif
    
    invFriendsRequest(params);
}

void iosFacebookRequestMe()
{
    if ([FBSDKAccessToken currentAccessToken])
    {
        [[[FBSDKGraphRequest alloc] initWithGraphPath:@"me" parameters:nil]
         startWithCompletionHandler:^(FBSDKGraphRequestConnection *connection, id result, NSError *error) {
             if (error)
             {
                 facebook::internal::newMeRequestResult("", true);
             }
             else
             {
                 string data;
                 
                 NSError *error1;
                 NSData *jsonData = [NSJSONSerialization dataWithJSONObject:result
                                                                    options:0
                                                                      error:&error1];
                 
                 if (jsonData)
                 {
                     NSString *str = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
                     facebook::internal::newMeRequestResult([str UTF8String], false);
                 }
                 else
                 {
                     facebook::internal::newMeRequestResult("", true);
                 }
                 
                 NSLog(@"fetched user:%@", result);
             }
         }];
    }
}

void iosFacebookInit()
{
    requests = [[FacebookRequests alloc] init];
    
}

void iosFacebookFree()
{
    requests = Nil;
}

void iosFacebookGameRequest(const string &title, const string &text, const vector<string>& dest, const string &objectID, const std::string &userData)
{
    FBSDKGameRequestContent *request = [[FBSDKGameRequestContent alloc] init];
    
    request.message = [NSString stringWithUTF8String:text.c_str()];
    request.title = [NSString stringWithUTF8String:title.c_str()];

    
    //request.actionType = FBSDKGameRequestActionTypeSend;
    request.data = [NSString stringWithUTF8String:userData.c_str()];

    
    //request.objectID = [NSString stringWithUTF8String:objectID.c_str()];


    
    NSMutableArray *rec = [NSMutableArray array];
    for (const string &id:dest)
    {
        [rec addObject:[NSString stringWithUTF8String:id.c_str()] ];
    }
    request.recipients = rec;
    
    
    FBSDKGameRequestDialog *dialog = [[FBSDKGameRequestDialog alloc] init];
    dialog.content = request;
    dialog.frictionlessRequestsEnabled = true;
    dialog.delegate = requests;
    
    [dialog show];
}
