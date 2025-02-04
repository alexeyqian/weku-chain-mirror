
#include <iostream>
#include <string>

#include <boost/algorithm/string.hpp>

#include <fc/io/json.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/variant.hpp>

#include <graphene/utilities/key_conversion.hpp>

#include <weku/protocol/transaction.hpp>
#include <weku/protocol/types.hpp>

struct tx_signing_request
{
   weku::protocol::transaction     tx;
   std::string                        wif;
};

struct tx_signing_result
{
   weku::protocol::transaction     tx;
   fc::sha256                         digest;
   fc::sha256                         sig_digest;
   weku::protocol::public_key_type key;
   weku::protocol::signature_type  sig;
};

FC_REFLECT( tx_signing_request, (tx)(wif) )
FC_REFLECT( tx_signing_result, (digest)(sig_digest)(key)(sig) )

int main(int argc, char** argv, char** envp)
{
   // hash key pairs on stdin
   std::string chain_id, hash, wif;
   while( std::cin )
   {
      std::string line;
      std::getline( std::cin, line );
      boost::trim(line);
      if( line == "" )
         continue;

      fc::variant v = fc::json::from_string( line, fc::json::strict_parser );
      tx_signing_request sreq;
      fc::from_variant( v, sreq );
      tx_signing_result sres;
      sres.tx = sreq.tx;
      sres.digest = sreq.tx.digest();
      sres.sig_digest = sreq.tx.sig_digest(STEEMIT_CHAIN_ID);

      fc::ecc::private_key priv_key = *graphene::utilities::wif_to_key( sreq.wif );
      sres.sig = priv_key.sign_compact( sres.sig_digest );
      sres.key = weku::protocol::public_key_type( priv_key.get_public_key() );
      std::cout << fc::json::to_string( sres ) << std::endl;
   }
   return 0;
}
