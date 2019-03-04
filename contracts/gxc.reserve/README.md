# gxc.escrow

Creates token based on escrow deposit. Users can claim with their earning game tokens.

## deposit (issuer, derv, base)

Deposits system token (base asset) to create game token (derivative asset)

**Required Authorization:** `issuer`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|issuer|name||the name of issuer|
|derv|asset||the maximum supply of derivative asset|
|base|asset||the quantity of base asset (system token)|

## claim (account_name, quantity, issuer)

Claims system token with game token

**Required Authorization:** `account_name`

|Param|Type|Default|Description|
|-----|----|-------|-----------|
|account_name|name||the name of account|
|quantity|asset||the amount of game token|
|issuer|name$||the name of issuer|

