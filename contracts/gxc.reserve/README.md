# gxc.reserve

Creates token based on reserved deposit. Users can claim with their earning game tokens.

## How to create game token

Before starting the process of token creation, token issuer account should be set as game account.
Game account can be set `setgame` by `gxc.game`.

1. Transfer system token to be used as underlying asset to token issuer balance
2. Approve the amount of system token to be transferred to `gxc.reserve`
3. Create token by `mint` with the maximum supply of game token and underlying system token

## Actions

### mint

``` c++
void mint(extended_asset derivative, extended_asset underlying, std::vector<key_value> opts);
```

Create game token (derivative asset) based on system token (underlying asset)
Issuer of derivative asset (`derivative.contract`) should be game account set by `gxc.game`.

**Required Authorization:** `derivative.contract`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|dervative|extended_asset||the maximum supply of derivative asset|
|underlying|extended_asset||the quantity of base asset (system token)|
|opts|key_value[]||token options (withdraw_min_amount, withdraw_delay_sec)|

### claim

``` c++
void claim(name owner, extended_asset value);
```

Claims system token by burning game token

**Required Authorization:** `owner`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|owner|name||the name of account|
|value|extended_asset||the amount of game token|

