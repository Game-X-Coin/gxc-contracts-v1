# gxc.user

Handles actions related to user accounts such as login or nickname register.

## login (account_name, game_name, login_token)

Validate user login to game

**Required Authorization:** `account_name`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|account_name|name||the name of user account|
|game_name|name||the name of game account|
|login_token|string||access token to validate login|

## setnick (account_name, nickname)

Set user nickname. Nickname allows alphabet (lower, upper), numbers and Korean characters, and has length limitations of 6-16 (alphanum) or 3-8 (Korean).

**Required Authorization:** `account_name`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|account_name|name||the name of user account|
|nickname|string||the string to be set as nickname|

