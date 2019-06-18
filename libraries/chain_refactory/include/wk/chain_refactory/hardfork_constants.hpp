#pragma once

#include <fc/time.hpp>

namespace wk { namespace chain {

#define STEEMIT_NUM_HARDFORKS 22

#ifndef STEEMIT_HARDFORK_0_1
#define STEEMIT_HARDFORK_0_1 1
#define STEEMIT_HARDFORK_0_1_TIME 1461605400
#endif

#ifndef STEEMIT_HARDFORK_0_10
#define STEEMIT_HARDFORK_0_10 10
#define STEEMIT_MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10 fc::seconds(60*30) /// 30 min
#define STEEMIT_HARDFORK_0_10_TIME 1468584000  //2016-07-15T12:00:00 UTC
#endif

#ifndef STEEMIT_HARDFORK_0_11
#define STEEMIT_HARDFORK_0_11 11
#define STEEMIT_HARDFORK_0_11_TIME 1468767600 // 2016-07-17T15:00:00 UTC (11:00:00 EDT)
#endif

#ifndef STEEMIT_HARDFORK_0_12
#define STEEMIT_HARDFORK_0_12 12
#define STEEMIT_HARDFORK_0_12_TIME 1469545200 // 2016-07-26T15:00:00 UTC (11:00:00 EDT)
#endif


#ifndef STEEMIT_HARDFORK_0_13
#define STEEMIT_HARDFORK_0_13 13
#define STEEMIT_HARDFORK_0_13_TIME  1471269600 // Mon, 15 Aug 2016 14:00:00 GMT
#endif

#ifndef STEEMIT_HARDFORK_0_14
#define STEEMIT_HARDFORK_0_14 14
#define STEEMIT_HARDFORK_0_14_TIME  1474383600 // Tue, 20 Sep 2016 15:00:00 GM
#endif

#ifndef STEEMIT_HARDFORK_0_15
#define STEEMIT_HARDFORK_0_15 15
#define STEEMIT_HARDFORK_0_15_TIME  1478620800 // Tue, 8 Nov 2016 16:00:00 UTC (11:00:00 EST)
#endif

#ifndef STEEMIT_HARDFORK_0_16
#define STEEMIT_HARDFORK_0_16 16
#define STEEMIT_HARDFORK_0_16_TIME  1481040000 // Tue, 6 Dec 2016 16:00:00 UTC (11:00:00 EST)
#endif

#ifndef STEEMIT_HARDFORK_0_17
#define STEEMIT_HARDFORK_0_17 17
#define STEEMIT_HARDFORK_0_17_TIME  1490886000 // Thu, 30 March 2017 15:00:00 UTC (11:00:00 EDT)
#define STEEMIT_HF_17_NUM_POSTS     (49357)
#define STEEMIT_HF_17_NUM_REPLIES   (242051)
#define STEEMIT_HF_17_RECENT_CLAIMS (fc::uint128_t(808638359297ull,13744269167557038121ull)) // 14916744862149894120447332012073
#endif

#ifndef STEEMIT_HARDFORK_0_18
#define STEEMIT_HARDFORK_0_18 18
#define STEEMIT_HARDFORK_0_18_TIME  1490886000 // Thu, 30 March 2017 15:00:00 UTC (11:00:00 EDT)
#endif

#ifndef STEEMIT_HARDFORK_0_19
#define STEEMIT_HARDFORK_0_19 19
#define STEEMIT_HARDFORK_0_19_TIME 1497970800 // Tue, 20 Jun 2017 15:00:00 UTC (11:00:00 EDT) 
#define STEEMIT_HF_19_RECENT_CLAIMS (fc::uint128_t(uint64_t(140797515942543623ull)))
#define STEEMIT_HARDFORK_0_19_ACTUAL_TIME (fc::time_point_sec(1497970809)) // Tue, 20 Jun 2017 15:00:09 UTC
#define STEEMIT_HF_19_SQRT_PRE_CALC (fc::time_point_sec( STEEMIT_HARDFORK_0_19_ACTUAL_TIME - STEEMIT_CASHOUT_WINDOW_SECONDS ))
#endif

#ifndef STEEMIT_HARDFORK_0_2
#define STEEMIT_HARDFORK_0_2 2
// Tue, 26 Apr 2016 18:00:00 GMT
#define STEEMIT_HARDFORK_0_2_TIME 1461693600
#endif

#ifndef STEEMIT_HARDFORK_0_20
#define STEEMIT_HARDFORK_0_20 20
#define STEEMIT_HARDFORK_0_20_TIME 1538989200 // Monday, October 8, 2018 9:00:00 AM UTC
#endif

#ifndef STEEMIT_HARDFORK_0_21
#define STEEMIT_HARDFORK_0_21 21
#define STEEMIT_HARDFORK_0_21_TIME 1556637564 // Tuesday, April 30, 2019 3:19:24 PM Block 9585623
#endif

#ifndef STEEMIT_HARDFORK_0_22
#define STEEMIT_HARDFORK_0_22 22
#define STEEMIT_HARDFORK_0_22_TIME 1560592800 // Saturday, June 15, 2019 10:00:00 AM 
#endif

#ifndef STEEMIT_HARDFORK_0_3
#define STEEMIT_HARDFORK_0_3 3
// 4/27/2016 13:00:00 GMT
#define STEEMIT_HARDFORK_0_3_TIME 1461762000
#endif

#ifndef STEEMIT_HARDFORK_0_4
#define STEEMIT_HARDFORK_0_4 4
#define STEEMIT_HARDFORK_0_4_TIME 1462028400
#endif

#ifndef STEEMIT_HARDFORK_0_5
#define STEEMIT_HARDFORK_0_5 5
// May 31 2016 17:00:00 UTC (1:00 PM EDT)
#define STEEMIT_HARDFORK_0_5_TIME 1464714000
#endif

#ifndef STEEMIT_HARDFORK_0_6
#define STEEMIT_HARDFORK_0_6 6
// Thu Jun 30 14:00:00 UTC 2016
// Thu Jun 30 10:00:00 EDT 2016
#define STEEMIT_HARDFORK_0_6_TIME 1467295200
// Fri Jun 24 14:00:00 UTC 2016
// Fri Jun 24 10:00:00 EDT 2016
#ifdef IS_TEST_NET
#define STEEMIT_HARDFORK_0_6_REVERSE_AUCTION_TIME (0)
#else
#define STEEMIT_HARDFORK_0_6_REVERSE_AUCTION_TIME (1467295200-(60*60*24*6))
#endif
#endif

#ifndef STEEMIT_HARDFORK_0_7
#define STEEMIT_HARDFORK_0_7 7
#define STEEMIT_FIRST_CASHOUT_TIME STEEMIT_HARDFORK_0_7
// Mon Jul  4 00:00:00 UTC 2016
// Sun Jul  3 20:00:00 EDT 2016
#define STEEMIT_HARDFORK_0_7_TIME 1467590400  //July 4th
#endif

#ifndef STEEMIT_HARDFORK_0_8
#define STEEMIT_HARDFORK_0_8 8
// Mon Jul  4 01:00:00 UTC 2016
// Sun Jul  3 21:00:00 EDT 2016
#define STEEMIT_HARDFORK_0_8_TIME 1467594000
#endif

#ifndef STEEMIT_HARDFORK_0_9
#define STEEMIT_HARDFORK_0_9 9
#define STEEMIT_HARDFORK_0_9_TIME 1468454400 // 2016-07-14T00:00:00 UTC
#endif

}}