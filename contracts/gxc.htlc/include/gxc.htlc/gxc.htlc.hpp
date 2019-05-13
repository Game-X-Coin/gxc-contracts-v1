#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace eosio;
using std::string;

namespace gxc {

class [[eosio::contract("gxc.htlc")]] htlc_contract : public contract {
public:
   using contract::contract;

   struct [[eosio::table]] htlc {
      name contract_name;
      std::variant<name, checksum160> recipient;
      extended_asset value;
      checksum256 hashlock;
      time_point_sec timelock;

      uint64_t primary_key()const { return contract_name.value; }

      EOSLIB_SERIALIZE(htlc, (contract_name)(recipient)(value)(hashlock)(timelock))
   };

   typedef multi_index<"htlc"_n, htlc> htlcs;

   void transfer(name from, name to, extended_asset value, string memo) {}
   typedef action_wrapper<"transfer"_n, &htlc_contract::transfer> transfer_action;

   [[eosio::action]]
   void newcontract(name owner, name contract_name, std::variant<name, checksum160> recipient, extended_asset value, checksum256 hashlock, time_point_sec timelock);

   [[eosio::action]]
   void withdraw(name owner, name contract_name, checksum256 preimage);

   [[eosio::action]]
   void cancel(name owner, name contract_name);
};

}
