#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eoslib/crypto.hpp>

using namespace eosio;
using std::string;

namespace gxc {

class [[eosio::contract("gxc.htlc")]] htlc_contract : public contract {
public:
   using contract::contract;

   struct [[eosio::table]] htlc {
      string contract_name;
      std::variant<name, checksum160> recipient;
      extended_asset value;
      checksum256 hashlock;
      time_point_sec timelock;

      static uint64_t hash(string s) { return fasthash64(s.data(), s.size()); }
      uint64_t primary_key()const { return htlc::hash(contract_name); }

      EOSLIB_SERIALIZE(htlc, (contract_name)(recipient)(value)(hashlock)(timelock))
   };

   typedef multi_index<"htlc"_n, htlc> htlcs;

   void transfer(name from, name to, extended_asset value, string memo) {}
   typedef action_wrapper<"transfer"_n, &htlc_contract::transfer> transfer_action;

   [[eosio::action]]
   void newcontract(name owner, string contract_name, std::variant<name, checksum160> recipient, extended_asset value, checksum256 hashlock, time_point_sec timelock);

   [[eosio::action]]
   void withdraw(name owner, string contract_name, checksum256 preimage);

   [[eosio::action]]
   void cancel(name owner, string contract_name);
};

}
