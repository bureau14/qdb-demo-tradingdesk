#pragma once

#include "itch_status.hpp"
#include <boost/endian/conversion.hpp>
#include <brigand/algorithms/transform.hpp>
#include <brigand/algorithms/wrap.hpp>
#include <brigand/functions/lambda.hpp>
#include <brigand/sequences/list.hpp>
#include <brigand/sequences/map.hpp>
#include <brigand/types/integer.hpp>
#include <array>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <system_error>
#include <variant>

namespace itch
{

namespace messages
{

inline void unchecked_decode_char(const std::uint8_t *& p, size_t & l, char & c) noexcept
{
    c = *reinterpret_cast<const char *>(p);

    ++p;
    --l;
}

inline void unchecked_decode_string_to_lower(const std::uint8_t *& p, size_t & l, char * dest, size_t expected) noexcept
{
    for (size_t i = 0; i < expected; ++i)
    {
        dest[i] = std::tolower(static_cast<char>(p[i]));
    }

    p += expected;
    l -= expected;
}

template <typename Integer>
inline void unchecked_decode_integer(const std::uint8_t *& p, size_t & l, Integer & res) noexcept
{
    static_assert(std::is_integral<Integer>::value, "need to be an integer");

    res = boost::endian::big_to_native(*reinterpret_cast<const Integer *>(p));

    p += sizeof(Integer);
    l -= sizeof(Integer);
}

// Prices are integer fields, supplied with an associated precision.When converted to a decimal format, prices are in fixed point format,
// where the precision defines the number of decimal places.For example, a field flagged as Price (4) has an implied 4 decimal places.The
// maximum value of price(4) in TotalView --­ ITCH is 200, 000.0000 (decimal 77359400 hex).
template <size_t Decimals>
struct nasdaq_price
{
    static constexpr double compute_den(double base, size_t power) noexcept
    {
        return (power <= 1) ? base : base * compute_den(base, power - 1);
    }

    static constexpr size_t stored_size = 4;

    void unchecked_decode(const std::uint8_t *& p, size_t & l) noexcept
    {
        static_assert(sizeof(std::uint32_t) == stored_size, "invalid price size");
        static_assert(Decimals > 0, "Decimals can't be zero");

        std::uint32_t v;
        unchecked_decode_integer(p, l, v);
        value = static_cast<double>(v) / (compute_den(10.0, Decimals));
    }

    double value;
};

struct nasdaq_timestamp
{
    // the nasdaq timestamp is the number of nanoseconds since Midnight
    static constexpr size_t stored_size = 6;

    void unchecked_decode(const std::uint8_t *& p, size_t & l) noexcept
    {
        static_assert(sizeof(std::uint64_t) >= stored_size, "invalid buffer size");

        std::uint64_t v = 0;

        memcpy(&v, p, stored_size);
        count = std::chrono::nanoseconds{boost::endian::big_to_native(v << 16)};

        p += stored_size;
        l -= stored_size;
    }

    std::chrono::nanoseconds count;
};

static_assert(sizeof(char) == 1, "expects a size of 1 byte for char");

// The system event message type is used to signal a market or data feed handler event.
struct system_event
{
    static constexpr char message_code = 'S';

    static const char * name() noexcept
    {
        return "system event";
    }

    static constexpr size_t message_size = 1 + 2 + 2 + 6 + 1;
};

// At the start of each trading day, Nasdaq disseminates stock directory messages for all active symbols in the Nasdaq
//    execution system.
//    Market data redistributors should process this message to populate the Financial Status Indicator(required display
//        field) and the Market Category(recommended display field) for Nasdaq - ­ listed issues.
struct stock_directory
{
    static constexpr char message_code = 'R';

    static const char * name() noexcept
    {
        return "stock directory";
    }

    // Locate code identifying the security
    std::uint16_t stock_locate;

    // Nasdaq internal tracking number
    std::uint16_t tracking_number;

    // Nanoseconds since midnight.
    nasdaq_timestamp nanoseconds;

    // Stock symbol, right padded with spaces
    std::array<char, 8> stock;

    // Indicates Listing market or listing market tier for the issue
    char market_category;

    // For Nasdaq ­ listed issues, this field indicates when a firm is not in compliance with Nasdaq continued listing requirements
    char financial_status;

    // Denotes the number of shares that represent a round lot for the issue
    std::uint32_t round_lot_size;

    // Indicates if Nasdaq system limits order entry for issue
    char round_lots_only;

    // Identifies the security class for the issue as assigned by Nasdaq.
    char issue_classification;

    // dentifies the security sub --­ type for the issue as assigned by Nasdaq.
    std::array<char, 2> issue_sub_type;

    // Denotes if an issue or quoting participant record is set --­ up in Nasdaq systems in a live / production, test, or demo state. Please
    // note that firms should only show live issues and quoting participants on public quotation displays.
    char authenticity;

    // Indicates if a security is subject to mandatory close --­ out of short sales under SEC Rule 203(b)(3).
    char short_sale_threshold_indicator;

    // Indicates if the Nasdaq security is set up for IPO release.This field is intended to help Nasdaq market participant firms comply with
    // FINRA Rule 5131(b)
    char ipo_flag;

    // Indicates which Limit Up / Limit Down price band calculation parameter is to be used for the instrument.
    char luld_reference_price_tier;

    // Indicates whether the security is an exchange traded product (ETP)
    char etp_flag;

    // Tracks the integral relationship of the ETP to the underlying index.
    std::uint32_t etp_leverage_factor;

    // Indicates the directional relationship between the ETP and Underlying index.
    char inverse_indicator;

    bool decode(const std::uint8_t *& p, size_t & l) noexcept
    {
        if (l < message_size) return false;

        ++p;
        --l;

        // we have already eaten the message code because we have been dispatched here
        unchecked_decode_integer(p, l, stock_locate);
        unchecked_decode_integer(p, l, tracking_number);
        nanoseconds.unchecked_decode(p, l);
        unchecked_decode_string_to_lower(p, l, stock.data(), stock.size());
        unchecked_decode_char(p, l, market_category);
        unchecked_decode_char(p, l, financial_status);
        unchecked_decode_integer(p, l, round_lot_size);
        unchecked_decode_char(p, l, round_lots_only);
        unchecked_decode_char(p, l, issue_classification);
        unchecked_decode_string_to_lower(p, l, issue_sub_type.data(), issue_sub_type.size());
        unchecked_decode_char(p, l, authenticity);
        unchecked_decode_char(p, l, short_sale_threshold_indicator);
        unchecked_decode_char(p, l, ipo_flag);
        unchecked_decode_char(p, l, luld_reference_price_tier);
        unchecked_decode_char(p, l, etp_flag);
        unchecked_decode_integer(p, l, etp_leverage_factor);
        unchecked_decode_char(p, l, inverse_indicator);

        return true;
    }

    static constexpr size_t message_size =
        sizeof(message_code) + sizeof(stock_locate) + sizeof(tracking_number) + nasdaq_timestamp::stored_size + sizeof(stock)
        + sizeof(market_category) + sizeof(financial_status) + sizeof(round_lot_size) + sizeof(round_lots_only)
        + sizeof(issue_classification) + sizeof(issue_sub_type) + sizeof(authenticity) + sizeof(short_sale_threshold_indicator)
        + sizeof(ipo_flag) + sizeof(luld_reference_price_tier) + sizeof(etp_flag) + sizeof(etp_leverage_factor) + sizeof(inverse_indicator);
};

// Nasdaq uses this administrative message to indicate the current trading status of a security to the trading
//    community.
//    Prior to the start of system hours, Nasdaq will send out a Trading Action spin.In the spin, Nasdaq will send out a
//    Stock Trading Action message with the "T"(Trading Resumption) for all Nasdaq - ­­ and other exchange --­ listed
//    securities that are eligible for trading at the start of the system hours.If a security is absent from the pre --­
//    opening Trading Action spin, firms should assume that the security is being treated as halted in the Nasdaq
//    platform at the start of the system hours.Please note that securities may be halted in the Nasdaq system for
//    regulatory or operational reasons.
//    After the start of system hours, Nasdaq will use the Trading Action message to relay changes in trading status for an
//    individual security.
// Messages will be sent when a stock is :
//     * Halted
//     * Paused
//     * Released for quotation
//     * Released for trading
struct stock_trading_action
{
    static constexpr char message_code = 'H';

    static const char * name() noexcept
    {
        return "stock trading action";
    }

    static constexpr size_t message_size = 1 + 2 + 2 + 6 + 8 + 1 + 1 + 4;
};

// In February 2011, the Securitiesand Exchange Commission(SEC) implemented changes to Rule 201 of the
//    Regulation SHO(Reg SHO).For details, please refer to SEC Release Number 34 - 61595.In association with
//    the Reg SHO rule change, Nasdaq will introduce the following Reg SHO Short Sale Price Test Restricted
//    Indicator message format.
struct reg_sho_sale_price_test
{
    static constexpr char message_code = 'Y';

    static const char * name() noexcept
    {
        return "reg SHO short sale price test";
    }

    static constexpr size_t message_size = 1 + 2 + 2 + 6 + 1;
};

// At the start of each trading day, Nasdaq disseminates a spin of market participant position messages. The message provides the Primary
// Market Maker status, Market Maker modeand Market Participant state for each Nasdaq market participant firm registered in an issue.Market
// participant firms may use these fields to comply with certain marketplace rules.
// Throughout the day, Nasdaq will send out this message only if Nasdaq Operations changes the status of a market participant firm in an
// issue.
struct market_participant_position
{
    static constexpr char message_code = 'L';

    static const char * name() noexcept
    {
        return "market participant position";
    }

    static constexpr size_t message_size = 1 + 2 + 2 + 6 + 4 + 8 + 1 + 1 + 1;
};

// Informs data recipients what the daily MWCB breach points are set to for the current trading day.
struct mwcb_decline_level
{
    static constexpr char message_code = 'V';

    static const char * name() noexcept
    {
        return "MWCB decline level";
    }

    static constexpr size_t message_size = 1 + 2 + 2 + 6 + 8 + 8 + 8;
};

// Informs data recipients when a MWCB has breached one of the established levels
struct mwcb_status
{
    static constexpr char message_code = 'W';

    static const char * name() noexcept
    {
        return "MWCB status";
    }

    static constexpr size_t message_size = 1 + 2 + 2 + 6 + 1;
};

// Indicates the anticipated IPO quotation release time of a security.
struct ipo_quoting_period_update
{
    static constexpr char message_code = 'K';

    static const char * name() noexcept
    {
        return "IPO quoting period update";
    }

    static constexpr size_t message_size = 1 + 2 + 2 + 6 + 8 + 4 + 1 + 4;
};

// Indicates the auction collar thresholds within which a paused security can reopen following a LULD Trading Pause
struct luld_auction_collar
{
    static constexpr char message_code = 'J';

    static const char * name() noexcept
    {
        return "LULD auction collar";
    }

    static constexpr size_t message_size = 1 + 2 + 2 + 6 + 8 + 4 + 4 + 4 + 4;
};

// The Exchange uses this message to indicate the current Operational Status of a security to the trading
// community.An Operational Halt means that there has been an interruption of service on the identified
// security impacting only the designated Market Center.These Halts differ from the "Stock Trading
// Action" message types since an Operational Halt is specific to the exchange for which it is declared, and
// does not interrupt the ability of the trading community to trade the identified instrument on any other
// market place .
// Nasdaq uses this administrative message to indicate the current trading status of the three market centers
// operated by Nasdaq.
struct operational_halt
{
    static constexpr char message_code = 'h';

    static const char * name() noexcept
    {
        return "Operational halt";
    }

    static constexpr size_t message_size = 2 + 2 + 6 + 8 + 1 + 1;
};

// An Add Order Message indicates that a new order has been accepted by the Nasdaq systemand was added to the
// displayable book.The message includes a day --­ unique Order Reference Number used by Nasdaq to track the order.
// This message will be generated for unattributed orders accepted by the Nasdaq system. (Note: If a firm wants to
//    display a MPID for unattributed orders, Nasdaq recommends that it use the MPID of "NSDQ".)
struct add_order_without_attribution
{
    static constexpr char message_code = 'A';

    static const char * name() noexcept
    {
        return "Add order without attribution";
    }

    // Locate code identifying the security
    std::uint16_t stock_locate;

    // Nasdaq internal tracking number
    std::uint16_t tracking_number;

    // Nanoseconds since midnight.
    nasdaq_timestamp nanoseconds;

    // The unique reference number assigned to the new order at the time of receipt.
    std::uint64_t reference_number;

    // The type of order being added. "B" = Buy Order. "S" = Sell Order.
    char buy_sell;

    // The total number of shares associated with the order being added to the book.
    std::uint32_t shares;

    // Stock symbol, right padded with spaces
    std::array<char, 8> stock;

    // The display price of the new order.Refer to Data Types for field processing notes
    nasdaq_price<4> price;

    bool decode(const std::uint8_t *& p, size_t & l) noexcept
    {
        if (l < message_size) return false;

        ++p;
        --l;

        // we have already eaten the message code because we have been dispatched here
        unchecked_decode_integer(p, l, stock_locate);
        unchecked_decode_integer(p, l, tracking_number);
        nanoseconds.unchecked_decode(p, l);
        unchecked_decode_integer(p, l, reference_number);
        unchecked_decode_char(p, l, buy_sell);
        unchecked_decode_integer(p, l, shares);
        unchecked_decode_string_to_lower(p, l, stock.data(), stock.size());
        price.unchecked_decode(p, l);

        return true;
    }

    static constexpr size_t message_size = sizeof(message_code) + sizeof(stock_locate) + sizeof(tracking_number)
                                           + nasdaq_timestamp::stored_size + sizeof(reference_number) + sizeof(buy_sell) + sizeof(shares)
                                           + sizeof(stock) + nasdaq_price<4>::stored_size;
};

// This message will be generated for attributed orders and quotations accepted by the Nasdaq system.
struct add_order_with_attribution
{
    static constexpr char message_code = 'F';

    static const char * name() noexcept
    {
        return "Add order with attribution";
    }

    // Locate code identifying the security
    std::uint16_t stock_locate;

    // Nasdaq internal tracking number
    std::uint16_t tracking_number;

    // Nanoseconds since midnight.
    nasdaq_timestamp nanoseconds;

    // The unique reference number assigned to the new order at the time of receipt.
    std::uint64_t reference_number;

    // The type of order being added. "B" = Buy Order. "S" = Sell Order.
    char buy_sell;

    // The total number of shares associated with the order being added to the book.
    std::uint32_t shares;

    // Stock symbol, right padded with spaces
    std::array<char, 8> stock;

    // The display price of the new order.Refer to Data Types for field processing notes
    nasdaq_price<4> price;

    // Nasdaq Market participant identifier associated with the entered order
    std::array<char, 4> attribution;

    bool decode(const std::uint8_t *& p, size_t & l) noexcept
    {
        if (l < message_size) return false;

        ++p;
        --l;

        // we have already eaten the message code because we have been dispatched here
        unchecked_decode_integer(p, l, stock_locate);
        unchecked_decode_integer(p, l, tracking_number);
        nanoseconds.unchecked_decode(p, l);
        unchecked_decode_integer(p, l, reference_number);
        unchecked_decode_char(p, l, buy_sell);
        unchecked_decode_integer(p, l, shares);
        unchecked_decode_string_to_lower(p, l, stock.data(), stock.size());
        price.unchecked_decode(p, l);
        unchecked_decode_string_to_lower(p, l, attribution.data(), attribution.size());

        return true;
    }

    static constexpr size_t message_size = sizeof(message_code) + sizeof(stock_locate) + sizeof(tracking_number)
                                           + nasdaq_timestamp::stored_size + sizeof(reference_number) + sizeof(buy_sell) + sizeof(shares)
                                           + sizeof(stock) + nasdaq_price<4>::stored_size + sizeof(attribution);
};

// Modify Order messages always include the Order Reference Number of the Add Order to which the update
// applies.To determine the current display shares for an order, ITCH subscribers must deduct the number of shares
// stated in the Modify message from the original number of shares stated in the Add Order message with the same
// reference number.Nasdaq may send multiple Modify Order messages for the same order reference numberand
// the effects are cumulative.When the number of display shares for an order reaches zero, the order is deadand
// should be removed from the book
//
// This message is sent whenever an order on the book is executed in whole or in part.It is possible to receive several
// Order Executed Messages for the same order reference number if that order is executed in several parts.The
// multiple Order Executed Messages on the same order are cumulative.
// By combining the executions from both types of Order Executed Messagesand the Trade Message, it is possible to
// build a complete view of all non --­ cross executions that happen on Nasdaq.Cross execution information is available
// in one bulk print per symbol via the Cross Trade Message.
struct order_executed
{
    static constexpr char message_code = 'E';

    static const char * name() noexcept
    {
        return "Order executed";
    }

    // Locate code identifying the security
    std::uint16_t stock_locate;

    // Nasdaq internal tracking number
    std::uint16_t tracking_number;

    // Nanoseconds since midnight.
    nasdaq_timestamp nanoseconds;

    // The unique reference number assigned to the new order at the time of receipt.
    std::uint64_t reference_number;

    // The number of shares executed
    std::uint32_t executed_shares;

    // The Nasdaq generated day unique Match Number of this execution.The Match Number is also referenced in the Trade Break Message
    std::uint64_t match_number;

    bool decode(const std::uint8_t *& p, size_t & l) noexcept
    {
        if (l < message_size) return false;

        ++p;
        --l;

        // we have already eaten the message code because we have been dispatched here
        unchecked_decode_integer(p, l, stock_locate);
        unchecked_decode_integer(p, l, tracking_number);
        nanoseconds.unchecked_decode(p, l);
        unchecked_decode_integer(p, l, reference_number);
        unchecked_decode_integer(p, l, executed_shares);
        unchecked_decode_integer(p, l, match_number);

        return true;
    }

    static constexpr size_t message_size = sizeof(message_code) + sizeof(stock_locate) + sizeof(tracking_number)
                                           + nasdaq_timestamp::stored_size + sizeof(reference_number) + sizeof(executed_shares)
                                           + sizeof(match_number);
};

// This message is sent whenever an order on the book is executed in whole or in part at a price different from the
// initial display price.Since the execution price is different than the display price of the original Add Order, Nasdaq
// includes a price field within this execution message.
// It is possible to receive multiple Order Executedand Order Executed With Price messages for the same order if that
// order is executed in several parts.The multiple Order Executed messages on the same order are cumulative.
// These executions may be marked as non --­ printable.If the execution is marked as non --­ printed, it means that the
// shares will be included into a later bulk print(e.g., in the case of cross executions).If a firm is looking to use the data
// in time --­and --­ sales displays or volume calculations, Nasdaq recommends that firms ignore messages marked as non -
//­­ printable to prevent double counting.
struct order_executed_with_price
{
    static constexpr char message_code = 'C';

    static const char * name() noexcept
    {
        return "Order executed with price";
    }

    // Locate code identifying the security
    std::uint16_t stock_locate;

    // Nasdaq internal tracking number
    std::uint16_t tracking_number;

    // Nanoseconds since midnight.
    nasdaq_timestamp nanoseconds;

    // The unique reference number assigned to the new order at the time of receipt.
    std::uint64_t reference_number;

    // The number of shares executed
    std::uint32_t executed_shares;

    // The Nasdaq generated day unique Match Number of this execution.The Match Number is also referenced in the Trade Break Message
    std::uint64_t match_number;

    // Indicates if the execution should be reflected on time and sales displaysand volume calculations "N" = Non - Printable "Y" =
    // Printable
    char printable;

    // The Price at which the order execution occurred. Refer to Data Types for field processing notes
    nasdaq_price<4> execution_price;

    bool decode(const std::uint8_t *& p, size_t & l) noexcept
    {
        if (l < message_size) return false;

        ++p;
        --l;

        // we have already eaten the message code because we have been dispatched here
        unchecked_decode_integer(p, l, stock_locate);
        unchecked_decode_integer(p, l, tracking_number);
        nanoseconds.unchecked_decode(p, l);
        unchecked_decode_integer(p, l, reference_number);
        unchecked_decode_integer(p, l, executed_shares);
        unchecked_decode_integer(p, l, match_number);
        unchecked_decode_char(p, l, printable);
        execution_price.unchecked_decode(p, l);

        return true;
    }

    static constexpr size_t message_size = sizeof(message_code) + sizeof(stock_locate) + sizeof(tracking_number)
                                           + nasdaq_timestamp::stored_size + sizeof(reference_number) + sizeof(executed_shares)
                                           + sizeof(match_number) + sizeof(printable) + nasdaq_price<4>::stored_size;
};

// This message is sent whenever an order on the book is modified as a result of a partial cancellation.
struct order_cancel
{
    static constexpr char message_code = 'X';

    static const char * name() noexcept
    {
        return "Order cancel";
    }

    // Locate code identifying the security
    std::uint16_t stock_locate;

    // Nasdaq internal tracking number
    std::uint16_t tracking_number;

    // Nanoseconds since midnight.
    nasdaq_timestamp nanoseconds;

    // The reference number of the order being canceled
    std::uint64_t reference_number;

    // The number of shares being removed from the display size of the order as a result of a cancellation
    std::uint32_t cancelled_shares;

    bool decode(const std::uint8_t *& p, size_t & l) noexcept
    {
        if (l < message_size) return false;

        ++p;
        --l;

        // we have already eaten the message code because we have been dispatched here
        unchecked_decode_integer(p, l, stock_locate);
        unchecked_decode_integer(p, l, tracking_number);
        nanoseconds.unchecked_decode(p, l);
        unchecked_decode_integer(p, l, reference_number);
        unchecked_decode_integer(p, l, cancelled_shares);

        return true;
    }

    static constexpr size_t message_size = sizeof(message_code) + sizeof(stock_locate) + sizeof(tracking_number)
                                           + nasdaq_timestamp::stored_size + sizeof(reference_number) + sizeof(cancelled_shares);
};

// This message is sent whenever an order on the book is being cancelled.All remaining shares are no longer
// accessible so the order must be removed from the book.
struct order_delete
{
    static constexpr char message_code = 'D';

    static const char * name() noexcept
    {
        return "Order delete";
    }

    // Locate code identifying the security
    std::uint16_t stock_locate;

    // Nasdaq internal tracking number
    std::uint16_t tracking_number;

    // Nanoseconds since midnight.
    nasdaq_timestamp nanoseconds;

    // The reference number of the order being canceled
    std::uint64_t reference_number;

    bool decode(const std::uint8_t *& p, size_t & l) noexcept
    {
        if (l < message_size) return false;

        ++p;
        --l;

        // we have already eaten the message code because we have been dispatched here
        unchecked_decode_integer(p, l, stock_locate);
        unchecked_decode_integer(p, l, tracking_number);
        nanoseconds.unchecked_decode(p, l);
        unchecked_decode_integer(p, l, reference_number);

        return true;
    }

    static constexpr size_t message_size =
        sizeof(message_code) + sizeof(stock_locate) + sizeof(tracking_number) + nasdaq_timestamp::stored_size + sizeof(reference_number);
};

// This message is sent whenever an order on the book has been cancel --­ replaced. All remaining shares from the original order are no
// longer accessible, and must be removed.The new order details are provided for the replacement, along with a new order reference number
// which will be used henceforth.Since the side, stock symbol and attribution(if any) cannot be changed by an Order Replace event, these
// fields are not included in the Order Cancel Message Order Delete Message message. Firms should retain the side, stock symbol and MPID
// from the original Add Order message.
struct order_replace
{
    static constexpr char message_code = 'U';

    static const char * name() noexcept
    {
        return "Order replace";
    }

    // Locate code identifying the security
    std::uint16_t stock_locate;

    // Nasdaq internal tracking number
    std::uint16_t tracking_number;

    // Nanoseconds since midnight.
    nasdaq_timestamp nanoseconds;

    // The original order reference number of the order being replaced
    std::uint64_t original_reference_number;

    // The new reference number for this order at time of replacement
    // Please note that the Nasdaq system will use this new order reference number for all subsequent updates
    std::uint64_t new_reference_number;

    // The total number of shares associated with the order being added to the book.
    std::uint32_t shares;

    // The display price of the new order.Refer to Data Types for field processing notes
    nasdaq_price<4> price;

    bool decode(const std::uint8_t *& p, size_t & l) noexcept
    {
        if (l < message_size) return false;

        ++p;
        --l;

        // we have already eaten the message code because we have been dispatched here
        unchecked_decode_integer(p, l, stock_locate);
        unchecked_decode_integer(p, l, tracking_number);
        nanoseconds.unchecked_decode(p, l);
        unchecked_decode_integer(p, l, original_reference_number);
        unchecked_decode_integer(p, l, new_reference_number);
        unchecked_decode_integer(p, l, shares);
        price.unchecked_decode(p, l);

        return true;
    }

    static constexpr size_t message_size = sizeof(message_code) + sizeof(stock_locate) + sizeof(tracking_number)
                                           + nasdaq_timestamp::stored_size + sizeof(original_reference_number)
                                           + sizeof(new_reference_number) + sizeof(shares) + nasdaq_price<4>::stored_size;
};

// The Trade Message is designed to provide execution details for normal match events involving non --­ displayable
// order types. (Note: There is a separate message for Nasdaq cross events.)
// Since no Add Order Message is generated when a non --­ displayed order is initially received, Nasdaq cannot use the
// Order Executed messages for all matches.Therefore this message indicates when a match occurs between non - ­­
// displayable order types.A Trade Message is transmitted each time a non --­ displayable order is executed in whole
// or in part.It is possible to receive multiple Trade Messages for the same order if that order is executed in several
// parts.Trade Messages for the same order are cumulative.
// Trade Messages should be included in Nasdaq time --­and --­ sales displays as well as volumeand other market
// statistics.Since Trade Messages do not affect the book, however, they may be ignored by firms just looking to build
// and track the Nasdaq execution system display.
struct trade_non_cross
{
    static constexpr char message_code = 'P';

    static const char * name() noexcept
    {
        return "Trade non cross";
    }

    // Locate code identifying the security
    std::uint16_t stock_locate;

    // Nasdaq internal tracking number
    std::uint16_t tracking_number;

    // Nanoseconds since midnight.
    nasdaq_timestamp nanoseconds;

    // The original order reference number of the order being replaced
    std::uint64_t order_reference_number;

    // The type of order being added. "B" = Buy Order. "S" = Sell Order.
    char buy_sell;

    // The total number of shares associated with the order being added to the book.
    std::uint32_t shares;

    // Stock symbol, right padded with spaces
    std::array<char, 8> stock;

    // The display price of the new order.Refer to Data Types for field processing notes
    nasdaq_price<4> price;

    // The Nasdaq generated day unique Match Number of this execution.The Match Number is also referenced in the Trade Break Message
    std::uint64_t match_number;

    bool decode(const std::uint8_t *& p, size_t & l) noexcept
    {
        if (l < message_size) return false;

        ++p;
        --l;

        // we have already eaten the message code because we have been dispatched here
        unchecked_decode_integer(p, l, stock_locate);
        unchecked_decode_integer(p, l, tracking_number);
        nanoseconds.unchecked_decode(p, l);
        unchecked_decode_integer(p, l, order_reference_number);
        unchecked_decode_char(p, l, buy_sell);
        unchecked_decode_integer(p, l, shares);
        unchecked_decode_string_to_lower(p, l, stock.data(), stock.size());
        price.unchecked_decode(p, l);
        unchecked_decode_integer(p, l, match_number);

        return true;
    }

    static constexpr size_t message_size = sizeof(message_code) + sizeof(stock_locate) + sizeof(tracking_number)
                                           + nasdaq_timestamp::stored_size + sizeof(order_reference_number) + sizeof(buy_sell)
                                           + sizeof(shares) + sizeof(stock) + nasdaq_price<4>::stored_size + sizeof(match_number);
};

// Cross Trade message indicates that Nasdaqhas completed its cross process for a specific security.Nasdaq sends out
// a Cross Trade message for all active issues in the system following the Opening, Closingand EMC cross events.Firms
// may use the Cross Trade message to determine when the cross for each security has been completed. (Note: For
//    the halted / paused securities, firms should use the Trading Action message to determine when an issue has been
//    released for trading.)
//    For most issues, the Cross Trade message will indicate the bulk volume associated with the cross event.If the order
//    interest is insufficient to conduct a cross in a particular issue, however, the Cross Trade message may show the
//    shares as zero.
//    To avoid double counting of cross volume, firms should not include transactions marked as non --­ printable in time - ­­
//    and --­ sales displays or market statistic calculations.
struct trade_cross
{
    static constexpr char message_code = 'Q';

    static const char * name() noexcept
    {
        return "Trade cross";
    }

    // Locate code identifying the security
    std::uint16_t stock_locate;

    // Nasdaq internal tracking number
    std::uint16_t tracking_number;

    // Nanoseconds since midnight.
    nasdaq_timestamp nanoseconds;

    // The number of shares matched in the Nasdaq Cross.
    std::uint64_t shares;

    // Stock symbol, right padded with spaces
    std::array<char, 8> stock;

    // The price at which the cross occurred. Refer to Data Types for field processing notes.
    nasdaq_price<4> cross_price;

    // The Nasdaq generated day unique Match Number of this execution.The Match Number is also referenced in the Trade Break Message
    std::uint64_t match_number;

    // The Nasdaq cross session for which the message is being generated.
    char cross_type;

    bool decode(const std::uint8_t *& p, size_t & l) noexcept
    {
        if (l < message_size) return false;

        ++p;
        --l;

        // we have already eaten the message code because we have been dispatched here
        unchecked_decode_integer(p, l, stock_locate);
        unchecked_decode_integer(p, l, tracking_number);
        nanoseconds.unchecked_decode(p, l);
        unchecked_decode_integer(p, l, shares);
        unchecked_decode_string_to_lower(p, l, stock.data(), stock.size());
        cross_price.unchecked_decode(p, l);
        unchecked_decode_integer(p, l, match_number);
        unchecked_decode_char(p, l, cross_type);

        return true;
    }

    static constexpr size_t message_size = sizeof(message_code) + sizeof(stock_locate) + sizeof(tracking_number)
                                           + nasdaq_timestamp::stored_size + sizeof(shares) + sizeof(stock) + nasdaq_price<4>::stored_size
                                           + sizeof(match_number) + sizeof(cross_type);
};

// The Broken Trade Message is sent whenever an execution on Nasdaq is broken.An execution may be broken if it is
// found to be "clearly erroneous" pursuant to Nasdaq’s Clearly Erroneous Policy.A trade break is final; once a trade is
// broken, it cannot be reinstated.
// Firms that use the ITCH feed to create time - ­­ and -­­ sales displays or calculate market statistics should be prepared
// to process the broken trade message.If a firm is only using the ITCH feed to build a book, however, it may ignore
// these messages as they have no impact on the current book.
struct broken_trade_order
{
    static constexpr char message_code = 'B';

    static const char * name() noexcept
    {
        return "Broken trade order";
    }

    // Locate code identifying the security
    std::uint16_t stock_locate;

    // Nasdaq internal tracking number
    std::uint16_t tracking_number;

    // Nanoseconds since midnight.
    nasdaq_timestamp nanoseconds;

    // The Nasdaq Match Number of the execution that was broken. This refers to a Match Number from a previously transmitted Order Executed
    // Message, Order Executed With Price Message, or Trade Message.
    std::uint64_t match_number;

    bool decode(const std::uint8_t *& p, size_t & l) noexcept
    {
        if (l < message_size) return false;

        ++p;
        --l;

        // we have already eaten the message code because we have been dispatched here
        unchecked_decode_integer(p, l, stock_locate);
        unchecked_decode_integer(p, l, tracking_number);
        nanoseconds.unchecked_decode(p, l);
        unchecked_decode_integer(p, l, match_number);

        return true;
    }

    static constexpr size_t message_size =
        sizeof(message_code) + sizeof(stock_locate) + sizeof(tracking_number) + nasdaq_timestamp::stored_size + sizeof(match_number);
};

// NASDAQ disseminates NOII data at 5 --­ second intervals in the minutes leading up to the Nasdaq Opening Cross and
// Nasdaq Closing Cross.
// For the Nasdaq Opening Cross, NOII messages will be disseminated during the two minutes leading up to
// the start of market hours.
// For the Nasdaq IPO, Haltand Imbalance Crosses, NOII messages will be disseminated during the quote only
// period.The first NOII message will be disseminated approximately five seconds after the Stock Trading
// Action message with the "Q" or "R" action value is disseminated.
// For the Nasdaq Closing Cross, NOII messages will be disseminated during the ten minutes leading up to the
// end of market hours.
struct noii
{
    static constexpr char message_code = 'I';

    static const char * name() noexcept
    {
        return "NOII";
    }

    static constexpr size_t message_size = 1 + 2 + 2 + 6 + 1;
};

// Identifies a retail interest indication of the Bid, Ask or both the Bid and Ask for Nasdaq --­ listed securities.
struct rpii
{
    static constexpr char message_code = 'N';

    static const char * name() noexcept
    {
        return "RPII";
    }

    static constexpr size_t message_size = 1 + 2 + 2 + 6 + 1;
};

struct eot
{
    static constexpr char message_code = '_';

    static const char * name() noexcept
    {
        return "End of transmission";
    }

    static constexpr size_t message_size = 0;
};

using complete_list = brigand::list<system_event,
    stock_directory,
    stock_trading_action,
    reg_sho_sale_price_test,
    market_participant_position,
    mwcb_decline_level,
    mwcb_status,
    ipo_quoting_period_update,
    luld_auction_collar,
    operational_halt,
    add_order_without_attribution,
    add_order_with_attribution,
    order_executed,
    order_executed_with_price,
    order_cancel,
    order_delete,
    order_replace,
    trade_non_cross,
    trade_cross,
    broken_trade_order,
    noii,
    rpii,
    eot>;

template <typename Array>
std::string_view view_on_nasdaq_str(const Array & a) noexcept
{
    auto it = std::find(a.cbegin(), a.cend(), ' ');
    return std::string_view{a.data(), static_cast<std::string_view::size_type>(std::distance(a.cbegin(), it))};
}

namespace detail
{
template <typename Message>
struct make_pair
{
    using code_type = brigand::uint8_t<Message::message_code>;
    using type      = brigand::pair<code_type, Message>;
};

template <typename L>
struct the_map_maker
{
    using temp_code_mapping = brigand::transform<L, make_pair<brigand::_1>>;

    // template <typename... T>
    // using map_wrapper = typename brigand::map<T...>;

    // template <typename L>
    // using as_map = brigand::wrap<L, map_wrapper>;

    using type = temp_code_mapping;
};

template <typename... T>
using std_variant_wrapper = typename std::variant<T...>;

using messages_variant_type = brigand::wrap<complete_list, std_variant_wrapper>;

} // namespace detail

using code_to_message_map = typename detail::the_map_maker<complete_list>::type;

using message_type = detail::messages_variant_type;

template <std::uint8_t Code>
struct message_type_builder
{
    using code_type = brigand::uint8_t<Code>;
    using type      = typename brigand::lookup<code_to_message_map, code_type>::type;
};

template <std::uint8_t Code>
using message_type_by_code = typename message_type_builder<Code>::type;

#define MESSAGE_READ(x)                \
    case x ::message_code: {           \
        message_type m;                \
        m.emplace<x>().decode(p, l);   \
        while (!q.try_push(m))         \
            status.stall++;            \
        status.loaded += message_size; \
        status.read++;                 \
    }                                  \
    break

template <typename Queue>
bool read_next_message(const std::uint8_t *& p, size_t & l, read_status::messages_status & status, Queue & q)
{
    if (l < 2u) return false;

    std::uint16_t message_size;
    unchecked_decode_integer(p, l, message_size);

    if (l < message_size) return false;

    const auto code = *reinterpret_cast<const char *>(p);

    switch (code)
    {
        MESSAGE_READ(stock_directory);
        MESSAGE_READ(add_order_without_attribution);
        MESSAGE_READ(add_order_with_attribution);
        MESSAGE_READ(order_executed);
        MESSAGE_READ(order_executed_with_price);
        MESSAGE_READ(order_cancel);
        MESSAGE_READ(order_delete);
        MESSAGE_READ(order_replace);
        MESSAGE_READ(trade_cross);
        MESSAGE_READ(trade_non_cross);
        MESSAGE_READ(broken_trade_order);

    default: {
        // skip the message
        p += message_size;
        l -= message_size;

        status.skipped++;
        break;
    }
    }

    return true;
}

#undef MESSAGE_READ

} // namespace messages
} // namespace itch