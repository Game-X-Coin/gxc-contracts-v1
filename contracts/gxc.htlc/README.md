# EOS Hashed Timelock Contract

This contract implements hashed timelock contract which allows user to defer token transfer and process it only when valid preimage is provided.

## Build

It requires [eosio.cdt](https://github.com/EOSIO/eosio.cdt) v1.6.1 or higher. Compiled wasm and abi will be generated under `./build`.

``` console
$ ./build.sh
```

## Deploy

``` console
$ cleos set account permission eoshtlc active --add-code
$ cleos set contract eoshtlc build/eoshtlc
```

## Usage

It requires deployed [eosio.token](https://github.com/EOSIO/eosio.contracts/tree/master/contracts/eosio.token) contract or other contracts which owns `transfer` action compatible with `eosio.token`.

``` console
$ cleos push action eoshtlc newcontract '["alice", "sendeos2bob", "bob", {"quantity": "100.0000 EOS", "contract": "eosio.token"}, "32_BYTES_HEX_STRING_WHICH_YOU_WANT_TO_USE_AS_HASHLOCK", "2020-01-01T00:00:00"]' -p alice@active
$ cleos push action eosio.token transfer '["alice, "eoshtlc", "100.0000 EOS", "sendeos2bob"]' -p alice@active
```

You can check generated hashed timelock by the next command.

``` console
$ cleos get table eoshtlc alice htlc
```

`bob` can withdraw token by providing valid preimage. (requires `hashlock == sha256(preimage)`)

``` console
$ cleos push action eoshtlc withdraw '["alice", "sendeos2bob", "32_BYTES_HEX_STRING_PREIMAGE_WHOSE_HASH_VALUE_IS_SAME_TO_HASHLOCK"]' -p bob@active
```

When hashed timelock contract is expired (it passes timelock point), contract owner can cancel it.
If hashed timelock contract was activated by proper token transfer, not processed token amount will be returned to contract owner

``` console
$ cleos push action eoshtlc cancel '["alice", "sendeos2bob"]' -p alice@active
```
