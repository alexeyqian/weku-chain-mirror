#include <weku/account_statistics/account_statistics_api.hpp>

namespace weku { namespace account_statistics {

namespace detail
{
   class account_statistics_api_impl
   {
      public:
         account_statistics_api_impl( weku::app::application& app )
            :_app( app ) {}

         weku::app::application& _app;
   };
} // detail

account_statistics_api::account_statistics_api( const weku::app::api_context& ctx )
{
   _my= std::make_shared< detail::account_statistics_api_impl >( ctx.app );
}

void account_statistics_api::on_api_startup() {}

} } // weku::account_statistics