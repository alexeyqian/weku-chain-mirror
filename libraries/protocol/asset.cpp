#include <weku/protocol/asset.hpp>
#include <boost/rational.hpp>
#include <boost/multiprecision/cpp_int.hpp>

/*

The bounds on asset serialization are as follows:

index : field
0     : decimals
1..6  : symbol
   7  : \0
*/

namespace weku { namespace protocol {
      typedef boost::multiprecision::int128_t  int128_t;

      uint8_t asset::decimals()const
      {
         auto a = (const char*)&symbol;
         uint8_t result = uint8_t( a[0] );
         FC_ASSERT( result < 15 );
         return result;
      }

      void asset::set_decimals(uint8_t d)
      {
         FC_ASSERT( d < 15 );
         auto a = (char*)&symbol;
         a[0] = d;
      }

      std::string asset::symbol_name()const
      {
         auto a = (const char*)&symbol;
         FC_ASSERT( a[7] == 0 );
         return &a[1];
      }

      int64_t asset::precision()const
      {
         static int64_t table[] = {
                           1, 10, 100, 1000, 10000,
                           100000, 1000000, 10000000, 100000000ll,
                           1000000000ll, 10000000000ll,
                           100000000000ll, 1000000000000ll,
                           10000000000000ll, 100000000000000ll
                         };
         uint8_t d = decimals();
         return table[ d ];
      }

      string asset::to_string()const
      {
         int64_t prec = precision();
         string result = fc::to_string(amount.value / prec);
         if( prec > 1 )
         {
            auto fract = amount.value % prec;
            // prec is a power of ten, so for example when working with
            // 7.005 we have fract = 5, prec = 1000.  So prec+fract=1005
            // has the correct number of zeros and we can simply trim the
            // leading 1.
            result += "." + fc::to_string(prec + fract).erase(0,1);
         }
         return result + " " + symbol_name();
      }

      asset asset::from_string( const string& from )
      {
         try
         {
            string s = fc::trim( from );
            auto space_pos = s.find( " " );
            auto dot_pos = s.find( "." );

            FC_ASSERT( space_pos != std::string::npos );

            asset result;
            result.symbol = uint64_t(0);
            auto sy = (char*)&result.symbol;

            if( dot_pos != std::string::npos )
            {
               FC_ASSERT( space_pos > dot_pos );

               auto intpart = s.substr( 0, dot_pos );
               auto fractpart = "1" + s.substr( dot_pos + 1, space_pos - dot_pos - 1 );
               result.set_decimals( fractpart.size() - 1 );

               result.amount = fc::to_int64( intpart );
               result.amount.value *= result.precision();
               result.amount.value += fc::to_int64( fractpart );
               result.amount.value -= result.precision();
            }
            else
            {
               auto intpart = s.substr( 0, space_pos );
               result.amount = fc::to_int64( intpart );
               result.set_decimals( 0 );
            }
            auto symbol = s.substr( space_pos + 1 );
            size_t symbol_size = symbol.size();

            if( symbol_size > 0 )
            {
               FC_ASSERT( symbol_size <= 6 );
               memcpy( sy+1, symbol.c_str(), symbol_size );
            }

            return result;
         }
         FC_CAPTURE_AND_RETHROW( (from) )
      }

      bool operator == ( const price& a, const price& b )
      {
         if( std::tie( a.base.symbol, a.quote.symbol ) != std::tie( b.base.symbol, b.quote.symbol ) )
             return false;

         const auto amult = uint128_t( b.quote.amount.value ) * a.base.amount.value;
         const auto bmult = uint128_t( a.quote.amount.value ) * b.base.amount.value;

         return amult == bmult;
      }

      bool operator < ( const price& a, const price& b )
      {
         if( a.base.symbol < b.base.symbol ) return true;
         if( a.base.symbol > b.base.symbol ) return false;
         if( a.quote.symbol < b.quote.symbol ) return true;
         if( a.quote.symbol > b.quote.symbol ) return false;

         const auto amult = uint128_t( b.quote.amount.value ) * a.base.amount.value;
         const auto bmult = uint128_t( a.quote.amount.value ) * b.base.amount.value;

         return amult < bmult;
      }

      bool operator <= ( const price& a, const price& b )
      {
         return (a == b) || (a < b);
      }

      bool operator != ( const price& a, const price& b )
      {
         return !(a == b);
      }

      bool operator > ( const price& a, const price& b )
      {
         return !(a <= b);
      }

      bool operator >= ( const price& a, const price& b )
      {
         return !(a < b);
      }

      asset operator * ( const asset& a, const price& b )
      {
         if( a.symbol_name() == b.base.symbol_name() )
         {
            FC_ASSERT( b.base.amount.value > 0 );
            uint128_t result = (uint128_t(a.amount.value) * b.quote.amount.value)/b.base.amount.value;
            FC_ASSERT( result.hi == 0 );
            return asset( result.to_uint64(), b.quote.symbol );
         }
         else if( a.symbol_name() == b.quote.symbol_name() )
         {
            FC_ASSERT( b.quote.amount.value > 0 );
            uint128_t result = (uint128_t(a.amount.value) * b.base.amount.value)/b.quote.amount.value;
            FC_ASSERT( result.hi == 0 );
            return asset( result.to_uint64(), b.base.symbol );
         }
         FC_THROW_EXCEPTION( fc::assert_exception, "invalid asset * price", ("asset",a)("price",b) );
      }

      price operator / ( const asset& base, const asset& quote )
      { try {
         FC_ASSERT( base.symbol_name() != quote.symbol_name() );
         return price{ base, quote };
      } FC_CAPTURE_AND_RETHROW( (base)(quote) ) }

      price price::max( asset_symbol_type base, asset_symbol_type quote ) { return asset( share_type(STEEMIT_MAX_SHARE_SUPPLY), base ) / asset( share_type(1), quote); }
      price price::min( asset_symbol_type base, asset_symbol_type quote ) { return asset( 1, base ) / asset( STEEMIT_MAX_SHARE_SUPPLY, quote); }

      bool price::is_null() const { return *this == price(); }

      void price::validate() const
      { try {
         FC_ASSERT( base.amount > share_type(0) );
         FC_ASSERT( quote.amount > share_type(0) );
         FC_ASSERT( base.symbol_name() != quote.symbol_name() );
      } FC_CAPTURE_AND_RETHROW( (base)(quote) ) }


} } // weku::protocol
