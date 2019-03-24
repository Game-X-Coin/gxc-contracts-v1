# gxc.game

Handles actions related to game account.

## Actions

### setgame

``` c++
void setgame(name name, bool activated);
```

Register account as game account.

**Required Authorization:** `gxc.game`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|name|name||the account name to be registered as game|
|activated|bool||whether given account is game|

### seturi

``` c++
void seturi(name name, std::string uri);
```

Set uri which contains game information

**Required Authorization:** `gxc.game`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|name|name||the name of game account|
|uri|string||the uri providing game information|

