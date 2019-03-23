# gxc.token

Handles actions related to token like create, issue or transfer.

## Typedefs

### key_value

`key_value` is a wrapper type to inject option value to token.
You can access key with `first` and value with `second` field.
`std::vector<int8_t>` is interpreted as `bytes` in ABI.

``` c++
using key_value = std::pair<std::string, std::vector<int8_t>>;
```

## Actions

### mint

``` c++
void mint(extended_asset value, std::vector<key_value> opts);
```

Create new token or increase maximum supply

**Required Authorization:** `gxc.token`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|value|extended_asset||maximum supply of token|
|opts|key_value[]||configurable options for token|

**Available options**

|Name|Type|Default|Description|
|----|----|-------|-----------|
|mintable|bool|true|whether issuer can mint additional amount (increase max supply)|
|recallable|bool|true|whether issuer can transfer user's token from deposit|
|freezable|bool|true|whether issuer can freeze account by security issue|
|pausable|bool|false|whether issuer can pause transactions among users (inactivate token)|
|paused|bool|false|whether transactions are paused|
|whitelistable|bool|false|whether issuer can make whitelisted users only available to transfer token|
|whitelist_on|bool|false|whether whitelist feature is turned on|
|withdraw_min_amount|int64_t||withdrawable minimum amount|
|withdraw_delay_sec|uint32||duration in seconds required to withdraw|

**Note**

* `paused` can be set during token creation, even though not set `pausable`
* `withdraw_min_amount` should be represented as `int64_t` including decimal
  (ex) 100.0000 GXC -> 1000000

### transfer

``` c++
void transfer(name from, name to, extended_asset value, std::string memo);
```

Transfer token from sender to recipient

**Required Authorization:** `from` (or `value.contract` : allowed to handle deposit balance only)

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|from|name||the name of sender|
|to|name||the name of recipient|
|value|extended_asset||the amount of token|
|memo|string||the description|

### burn

``` c++
void burn(extended_asset value, std::string memo);
```

Burn token

(differs from sending token to null account `gxc.null`, decrease supply and max supply at the same time)

**Required Authorization:** `value.contract`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|value|extended_asset||the amount of token|
|memo|string||the description|

### setopts

``` c++
void setopts(name issuer, symbol_code symbol, std::vector<key_value> opts);
```

Change options for token

**Required Authorization:** `issuer`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|issuer|name||the name of token issuer|
|symbol|symbol_code||the symbol of token to be changed|
|opts|key_value[]||a set of option and its value|

**Available options**

|Name|Type|Default|Description|
|----|----|-------|-----------|
|paused|bool|false|whether token is paused|
|whitelist_on|bool|false|whether whitelist feature is turned on|

### setacntopts

``` c++
void setacntopts(name account, name issuer, symbol_code symbol, std::vector<key_value> opts);
```

Change options for account

**Required Authorization:** `issuer`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|account|name||the name of account owner|
|issuer|name||the name of token issuer|
|symbol|symbol_code||the symbol of token to be changed|
|opts|key_value[]||a set of option and its value|

**Available options**

|Name|Type|Default|Description|
|----|----|-------|-----------|
|frozen|bool|false|whether account is frozen|
|whitelist|bool|false|whether account is whitelisted|

### open

``` c++
void open(name owner, name issuer, symbol_code symbol, std::vector<key_value> opts);
```

Open account balance manually (only for token which can whitelist)

**Required Authorization:** `issuer`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|owner|name||the name of account owner|
|issuer|name||the name of token issuer|
|symbol|symbol_code||the symbol of token to be changed|
|opts|key_value[]||a set of option and its value|

Available options are same to those of `setacntopts`.

### close

``` c++
void close(name owner, name isuser, symbol_code symbol);
```

Close account balance manually (only for token which can whitelist)

**Required Authorization:** `owner`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|owner|name||the name of account owner|
|issuer|name||the name of token issuer|
|symbol|symbol_code||the symbol of token to be changed|

### deposit

``` c++
void deposit(name owner, extended_asset value);
```

Deposit token to be withdrawable by issuer (for in-game use)

**Required Authorization:** `owner`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|owner|name||the name of account owner|
|value|extended_asset||the amount of token|

### pushwithdraw

``` c++
void pushwithdraw(name owner, extended_asset value);
```

Withdraw token (for trading or claiming reserve), need to wait for a specified duration

**Required Authorization:** `owner`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|owner|name||the name of account owner|
|value|extended_asset||the amount of token|

### popwithdraw

``` c++
void popwithdraw(name owner, name issuer, symbol_code symbol);
```

Cancel withdrawal request

**Required Authorization:** `owner`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|owner|name||the name of account owner|
|issuer|name||the name of token issuer|
|symbol|symbol_code||the symbol of token|

### approve

``` c++
void approve(name owner, name spender, extended_asset value);
```

Approve the available amount to be transferred to specified account by its own permission

**Required Authorization:** `owner`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|owner|name||the name of account owner|
|spender|name||the name of account who will be permitted to transfer token|
|value|extended_asset||the amount of token|
