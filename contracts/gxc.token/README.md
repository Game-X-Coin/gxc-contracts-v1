# gxc.token

Handles actions related to token like create, issue or transfer.

## create (max_supply, opts)

Creates new token with options

**Required Authorization:** `gxc.token`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|max_supply|extended_asset||maximum supply of token|
|opts|key_value[]||configurable options for token|

**Available options**

|Name|Type|Default|Description|
|----|----|-------|-----------|
|can_recall|bool|true|whether issuer can transfer user's token from deposit|
|can_freeze|bool|true|whether issuer can freeze token or account|
|can_whitelist|bool|false|whether issuer can make whitelisted users only available to transfer token|
|is_frozen|bool|false|whether token is frozen|
|enforce_whitelist|bool|false|whether whitelist feature is turned on|
|withdraw_min_amount|asset||withdrawable minimum amount|
|withdraw_delay_sec|uint32||duration in seconds required to withdraw|

## transfer (from, to, quantity, memo)

Transfer token from sender to recipient

**Required Authorization:** `from` (or `issuer` : allowed to handle deposit balance only)

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|from|name||the name of sender|
|to|name||the name of recipient|
|quantity|extended_asset||the amount of token|
|memo|string||the description|

## burn (quantity, memo)

Burn token

(differs from sending token to null account `gxc.null`, decrease supply and max_supply at the same time)

**Required Authorization:** `issuer`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|quantity|extended_asset||the amount of token|
|memo|string||the description|

## setopts (issuer, symbol, opts)

Change options for token

**Required Authorization:** `issuer`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|issuer|name||the name of token issuer|
|symbol|symbol||the symbol of token to be changed|
|opts|key_value[]||a set of option and its value|

**Available options**

|Name|Type|Default|Description|
|----|----|-------|-----------|
|is_frozen|bool|false|whether token is frozen|
|enforce_whitelist|bool|false|whether whitelist feature is turned on|

## setacntopts (owner, issuer, symbol, opts)

Change options for account

**Required Authorization:** `issuer`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|owner|name||the name of account owner|
|issuer|name||the name of token issuer|
|symbol|symbol||the symbol of token to be changed|
|opts|key_value[]||a set of option and its value|

**Available options**

|Name|Type|Default|Description|
|----|----|-------|-----------|
|frozen|bool|false|whether account is frozen|
|whitelist|bool|false|whether account is whitelisted|

## open (owner, issuer, symbol, opts)

Open account balance manually (only for token which can whitelist)

**Required Authorization:** `issuer`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|owner|name||the name of account owner|
|issuer|name||the name of token issuer|
|symbol|symbol||the symbol of token to be changed|
|opts|key_value[]||a set of option and its value|

Available options are same to those of `setacntopts`.

## close (owner, issuer, symbol)

Close account balance manually (only for token which can whitelist)

**Required Authorization:** `owner`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|owner|name||the name of account owner|
|issuer|name||the name of token issuer|
|symbol|symbol||the symbol of token to be changed|

## deposit (owner, quantity)

Deposit token to be withdrawable by issuer (for in-game use)

**Required Authorization:** `owner` (or `issuer` : allowed to handle withdrawal requested amount only)

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|owner|name||the name of account owner|
|quantity|extended_asset||the amount of token|

## withdraw (owner, quantity)

Withdraw token (for trading or escrow claim), need to wait for a specified duration

**Required Authorization:** `owner`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|owner|name||the name of account owner|
|quantity|extended_asset||the amount of token|

