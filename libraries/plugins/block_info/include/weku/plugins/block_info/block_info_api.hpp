
#pragma once

#include <fc/api.hpp>

#include <weku/plugins/block_info/block_info.hpp>

namespace weku { namespace app {
   struct api_context;
} }

namespace weku { namespace plugin { namespace block_info {

namespace detail {
class block_info_api_impl;
}

struct get_block_info_args
{
   uint32_t start_block_num = 0;
   uint32_t count           = 1000;
};

class block_info_api
{
   public:
      block_info_api( const weku::app::api_context& ctx );

      void on_api_startup();

      std::vector< block_info > get_block_info( get_block_info_args args );
      std::vector< block_with_info > get_blocks_with_info( get_block_info_args args );

   private:
      std::shared_ptr< detail::block_info_api_impl > my;
};

} } }

FC_REFLECT( weku::plugin::block_info::get_block_info_args,
   (start_block_num)
   (count)
   )

FC_API( weku::plugin::block_info::block_info_api,
   (get_block_info)
   (get_blocks_with_info)
   )
