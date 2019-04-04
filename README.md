# gxc.contracts

## Version : 1.6.3

The GXC, the shortening of GameXCoin, is the EOSIO-based blockchain aiming at integrating __GAMES__ into crypto platform. To accomplish this purpose, GXC has different policies on operations such as account creation, privileged accounts, resource management etc. This repository contains examples of smart contracts which would be deployed to GXC mainnet. They are provided for reference purposes and can be changed anytime to enhance their features or fix errors.

   * [gxc.system](https://github.com/Game-X-Coin/gxc.contracts/tree/master/contracts/gxc.system)
   * [gxc.token](https://github.com/Game-X-Coin/gxc.contracts/tree/master/contracts/gxc.token)
   * [gxc.reserve](https://github.com/Game-X-Coin/gxc.contracts/tree/master/contracts/gxc.reserve)
   * [gxc.user](https://github.com/Game-X-Coin/gxc.contracts/tree/master/contracts/gxc.user)
   * [gxc.game](https://github.com/Game-X-Coin/gxc.contracts/tree/master/contracts/gxc.game)

The following contracts are not migrated yet, but can be used in boot step.

   * [eosio.bios](https://github.com/Game-X-Coin/gxc.contracts/tree/master/contracts/eosio.bios)
   * [eosio.msig](https://github.com/Game-X-Coin/gxc.contracts/tree/master/contracts/eosio.msig)
   * [eosio.wrap](https://github.com/Game-X-Coin/gxc.contracts/tree/master/contracts/eosio.wrap)

`libraries` contains common files are used in several contracts, but their APIs are not stable. Be careful when including them to your own contract.

Dependencies:
* [gxc v1.7.x](https://github.com/Game-X-Coin/gxc/releases/tag/gxc-1.7.0)
* [eosio.cdt v1.6.x](https://github.com/EOSIO/eosio.cdt/releases/tag/v1.6.1)

To build the contracts and the unit tests:
* First, ensure that your __gxc__ is compiled to the core symbol for the GXC blockchain that intend to deploy to.
* Second, make sure that you have ```sudo make install```ed __gxc__.
* Then just run the ```build.sh``` in the top directory to build all the contracts and the unit tests for these contracts.

After build (Automated tests are not supported yet):
* The unit tests executable is placed in the _build/tests_ and is named __unit_test__.
* The contracts are built into a _bin/\<contract name\>_ folder in their respective directories.
* Finally, simply use __gxcli__ to _set contract_ by pointing to the previously mentioned directory.
