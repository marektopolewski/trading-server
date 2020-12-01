#include "../server/orderstore.hpp"

#include <chrono>
#include <gtest/gtest.h>

class Fixture : public OrderStore
{
public:
    static const int MAX_BUY = 20;
    static const int MAX_SELL = 15;
    Fixture() : OrderStore(MAX_BUY, MAX_SELL) {}

    Message makeNewOrder(uint64_t listingId, uint64_t orderId, uint64_t orderQuantity, uint64_t orderPrice, char side) {
        auto message = Message{};
        message.header = makeHeader();
        message.header.payloadSize = sizeof(Messages::NewOrder);
        message.payload = Messages::NewOrder{ Messages::NewOrder::MESSAGE_TYPE, listingId, orderId, orderQuantity,
                                              orderPrice, side };
        return message;
    }

    Message makeDeleteOrder(uint64_t orderId) {
        auto message = Message{};
        message.header = makeHeader();
        message.header.payloadSize = sizeof(Messages::DeleteOrder);
        message.payload = Messages::DeleteOrder{ Messages::DeleteOrder::MESSAGE_TYPE, orderId };
        return message;
    }

    Message makeModifyOrder(uint64_t orderId, uint64_t newQuantity) {
        auto message = Message{};
        message.header = makeHeader();
        message.header.payloadSize = sizeof(Messages::ModifyOrderQuantity);
        message.payload = Messages::ModifyOrderQuantity{ Messages::ModifyOrderQuantity::MESSAGE_TYPE,
                                                         orderId, newQuantity };
        return message;
    }

    Message makeTradeOrder(uint64_t listingId, uint64_t tradeId, uint64_t tradeQuantity, uint64_t tradePrice) {
        auto message = Message{};
        message.header = makeHeader();
        message.header.payloadSize = sizeof(Messages::Trade);
        message.payload = Messages::Trade{ Messages::Trade::MESSAGE_TYPE, listingId, tradeId,
                                           tradeQuantity, tradePrice };
        return message;
    }

    Messages::Header makeHeader() {
        using namespace std::chrono;
        auto ts = duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
        return { proto_,  0, sequence_++, static_cast<uint64_t>(ts) };
    }

    IntrumentMap & instruments() { return test_instruments(); };

private:
    uint16_t proto_ = 1;
    uint32_t sequence_ = 0;
};

using namespace testing;
using OrderStatus = Messages::OrderResponse::Status;

TEST(orderstore, sanity_check)
{
    auto store = Fixture();
    ASSERT_TRUE(store.instruments().empty());
}

TEST(orderstore, new_order)
{
    auto store = Fixture();
    uint64_t listingId = 1;
    uint64_t orderId = 12;
    uint64_t orderQuantity = 10;
    uint64_t orderPrice = 120000;
    char side = 'B';

    auto response = store.consume(store.makeNewOrder(listingId, orderId, orderQuantity, orderPrice, side));
    ASSERT_EQ(response.status, OrderStatus::ACCEPTED);

    auto instrument = store.instruments().find(listingId);
    ASSERT_NE(instrument, store.instruments().end()) << "Instrument present in the store";
    ASSERT_TRUE(instrument->second.sells().empty());
    ASSERT_TRUE(instrument->second.trades().empty());

    auto buy_order = instrument->second.buys().find(orderId);
    ASSERT_NE(buy_order, instrument->second.buys().end()) << "Order present within instrument";
    ASSERT_EQ(buy_order->second.id, orderId);
    ASSERT_EQ(buy_order->second.quantity, orderQuantity);
    ASSERT_EQ(buy_order->second.price, orderPrice);
}

TEST(orderstore, new_order_max_exceeded)
{
    auto store = Fixture();
    uint64_t listingId = 4;
    uint64_t orderId = 9;
    uint64_t orderQuantity = Fixture::MAX_SELL;
    uint64_t orderPrice = 90999;
    char side = 'S';

    auto response = store.consume(store.makeNewOrder(listingId, orderId, orderQuantity, orderPrice, side));
    ASSERT_EQ(response.status, OrderStatus::REJECTED);
    ASSERT_TRUE(store.instruments().find(listingId)->second.sells().empty());
}

TEST(orderstore, delete_order)
{
    auto store = Fixture();
    uint64_t listingId = 2;
    uint64_t order_id_1 = 11;
    uint64_t orderQuantity = 6;
    uint64_t orderPrice = 12000;
    char side = 'S';
    store.consume(store.makeNewOrder(listingId, order_id_1, orderQuantity, orderPrice, side));

    auto order_id_2 = 12;
    orderQuantity = 7;
    side = 'B';
    store.consume(store.makeNewOrder(listingId, order_id_2, orderQuantity, orderPrice, side));

    auto instrument = store.instruments().find(listingId);
    ASSERT_EQ(instrument->second.buys().size(), 1);
    ASSERT_EQ(instrument->second.sells().size(), 1);

    // check if the correct one is persisted
    auto response = store.consume(store.makeDeleteOrder(order_id_2));
    ASSERT_EQ(response.status, OrderStatus::ACCEPTED);
    instrument = store.instruments().find(listingId);
    ASSERT_FALSE(instrument->second.sells().empty());

    // check that all orders were removed
    response = store.consume(store.makeDeleteOrder(order_id_1));
    ASSERT_EQ(response.status, OrderStatus::ACCEPTED);
    instrument = store.instruments().find(listingId);
    ASSERT_TRUE(instrument->second.sells().empty());
}

TEST(orderstore, delete_nonexisitent_order)
{
    auto store = Fixture();
    uint64_t listingId = 1;
    uint64_t orderId = 12;
    uint64_t orderQuantity = 10;
    uint64_t orderPrice = 120000;
    char side = 'B';

    store.consume(store.makeNewOrder(listingId, orderId, orderQuantity, orderPrice, side));
    auto response = store.consume(store.makeDeleteOrder(orderId + 1));
    ASSERT_EQ(response.status, OrderStatus::REJECTED);

    auto instrument = store.instruments().find(listingId);
    ASSERT_FALSE(instrument->second.buys().empty());
}

TEST(orderstore, modify_order)
{
    auto store = Fixture();
    uint64_t listingId = 2;
    uint64_t order_id_1 = 11;
    uint64_t orderQuantity = static_cast<int>(Fixture::MAX_SELL / 2);
    uint64_t orderPrice = 12000;
    char side = 'S';
    store.consume(store.makeNewOrder(listingId, order_id_1, orderQuantity, orderPrice, side));

    auto order_id_2 = 12;
    orderQuantity = 1;
    store.consume(store.makeNewOrder(listingId, order_id_2, orderQuantity, orderPrice, side));

    // check that correct value takes effect
    auto order_qty_1 = static_cast<int>(Fixture::MAX_SELL / 2) - 1;
    auto response = store.consume(store.makeModifyOrder(order_id_2, order_qty_1));
    auto current_qty = store.instruments().find(listingId)->second.sells().find(order_id_2)->second.quantity;
    ASSERT_EQ(response.status, OrderStatus::ACCEPTED);
    ASSERT_EQ(current_qty, order_qty_1);

    // check that exceeding threshold does not affect the store state
    auto order_qty_2 = static_cast<int>(Fixture::MAX_SELL / 2) + 1;
    response = store.consume(store.makeModifyOrder(order_id_2, order_qty_2));
    current_qty = store.instruments().find(listingId)->second.sells().find(order_id_2)->second.quantity;
    ASSERT_EQ(response.status, OrderStatus::REJECTED);
    ASSERT_EQ(current_qty, order_qty_1);
}

TEST(orderstore, trade_long) {
    auto store = Fixture();
    uint64_t listingId = 1;
    uint64_t orderId = 12;
    uint64_t orderQuantity = 5;
    uint64_t orderPrice = 120000;
    char side = 'S';
    store.consume(store.makeNewOrder(listingId, orderId, orderQuantity, orderPrice, side));

    auto trade_id = orderId;
    auto response = store.consume(store.makeTradeOrder(listingId, trade_id, orderQuantity, orderPrice));
    ASSERT_EQ(response.status, OrderStatus::ACCEPTED);
    auto trade = store.instruments().find(listingId)->second.trades().find(trade_id)->second;
    ASSERT_EQ(trade.id, trade_id);
    ASSERT_EQ(trade.quantity, orderQuantity);
    ASSERT_EQ(trade.price, orderPrice);
}

TEST(orderstore, trade_long_max_exceeded)
{
    auto store = Fixture();
    uint64_t listingId = 1;
    uint64_t orderId = 12;
    uint64_t orderQuantity = static_cast<int>(Fixture::MAX_BUY / 2) + 1;
    uint64_t orderPrice = 120000;
    char side = 'S';
    store.consume(store.makeNewOrder(listingId, orderId, orderQuantity, orderPrice, side));

    side = 'B';
    store.consume(store.makeNewOrder(listingId, orderId + 1, orderQuantity, orderPrice, side));

    auto trade_id = orderId;
    auto response = store.consume(store.makeTradeOrder(listingId, trade_id, orderQuantity, orderPrice));
    ASSERT_EQ(response.status, OrderStatus::REJECTED);
    ASSERT_TRUE(store.instruments().find(listingId)->second.trades().empty());
}

TEST(orderstore, trade_short)
{
    auto store = Fixture();
    uint64_t listingId = 1;
    uint64_t orderId = 12;
    uint64_t orderQuantity = 5;
    uint64_t orderPrice = 120000;
    char side = 'B';
    store.consume(store.makeNewOrder(listingId, orderId, orderQuantity, orderPrice, side));

    auto trade_id = orderId;
    auto response = store.consume(store.makeTradeOrder(listingId, trade_id, orderQuantity, orderPrice));
    ASSERT_EQ(response.status, OrderStatus::ACCEPTED);
    auto trade = store.instruments().find(listingId)->second.trades().find(trade_id)->second;
    ASSERT_EQ(trade.id, trade_id);
    ASSERT_EQ(trade.quantity, -orderQuantity);
    ASSERT_EQ(trade.price, orderPrice);
}

TEST(orderstore, trade_short_max_exceeded)
{
    auto store = Fixture();
    uint64_t listingId = 1;
    uint64_t orderId = 12;
    uint64_t orderQuantity = static_cast<int>(Fixture::MAX_SELL / 2) + 1;
    uint64_t orderPrice = 120000;
    char side = 'B';
    store.consume(store.makeNewOrder(listingId, orderId, orderQuantity, orderPrice, side));

    side = 'S';
    store.consume(store.makeNewOrder(listingId, orderId + 1, orderQuantity, orderPrice, side));

    auto trade_id = orderId;
    auto response = store.consume(store.makeTradeOrder(listingId, trade_id, orderQuantity, orderPrice));
    ASSERT_EQ(response.status, OrderStatus::REJECTED);
    ASSERT_TRUE(store.instruments().find(listingId)->second.trades().empty());
}

TEST(orderstore, trade_mismatch)
{
    auto store = Fixture();
    uint64_t listingId = 1;
    uint64_t orderId = 12;
    uint64_t orderQuantity = 5;
    uint64_t orderPrice = 120000;
    char side = 'S';
    store.consume(store.makeNewOrder(listingId, orderId, orderQuantity, orderPrice, side));

    // trade that doesnt match quantity
    auto trade_id_1 = orderId;
    auto response = store.consume(store.makeTradeOrder(listingId, trade_id_1, orderQuantity - 1, orderPrice));
    ASSERT_EQ(response.status, OrderStatus::REJECTED);
    ASSERT_TRUE(store.instruments().find(listingId)->second.trades().empty());

    // trade that doesnt match price
    auto trade_id_2 = orderId;
    response = store.consume(store.makeTradeOrder(listingId, trade_id_2, orderQuantity, orderPrice + 1));
    ASSERT_EQ(response.status, OrderStatus::REJECTED);
    ASSERT_TRUE(store.instruments().find(listingId)->second.trades().empty());

    // trade that doesnt match order id
    auto trade_id_3 = orderId + 1;
    response = store.consume(store.makeTradeOrder(listingId, trade_id_3, orderQuantity, orderPrice));
    ASSERT_EQ(response.status, OrderStatus::REJECTED);
    ASSERT_TRUE(store.instruments().find(listingId)->second.trades().empty());
}