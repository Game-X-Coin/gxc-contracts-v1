# gxc.token

Handles actions related to token like create, issue or transfer. The action pushed to this contract is forwarded to sub token contract according to token type.

## create (issuer, maximum_supply, type)

Creates new token, supported token type {"token.sys", "token.std"}

**Required Authorization:** `gxc.token`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|issuer|name||the name of issuer|
|maximum_supply|asset||the maximum supply of the token|
|type|name||the type of token (the name of the contract that handles token)|

## issue (to, quantity, memo, issuer)

Issues token (need to create token first)

**Required Authorization:** `issuer`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|to|name||the name of recipient|
|quantity|asset||the amount of token|
|memo|string||the description|
|issuer|name$||the name of issuer|

## retire (quantity, memo, issuer)

Retires token

**Required Authorization:** `issuer`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|quantity|asset||the amount of token|
|memo|string||the description|
|issuer|name$||the name of issuer|

## transfer (from, to, quantity, memo, issuer)

Transfer token from sender to recipient

**Required Authorization:** `from` (or `issuer` : allowed to handle deposit balance only)

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|from|name||the name of sender|
|to|name||the name of recipient|
|quantity|asset||the amount of token|
|memo|string||the description|
|issuer|name$||the name of issuer|

