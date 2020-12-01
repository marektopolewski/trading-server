#include "../orderstore.hpp"

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
    ASSERT_EQ(response, OrderStore::OrderResponse::ACCEPT);

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
    ASSERT_EQ(response, OrderStore::OrderResponse::REJECT);
    ASSERT_TRUE(store.instruments().empty());
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
    ASSERT_EQ(response, OrderStore::OrderResponse::ACCEPT);
    instrument = store.instruments().find(listingId);
    ASSERT_FALSE(instrument->second.sells().empty());

    // check that all orders were removed
    response = store.consume(store.makeDeleteOrder(order_id_1));
    ASSERT_EQ(response, OrderStore::OrderResponse::ACCEPT);
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
    ASSERT_EQ(response, OrderStore::OrderResponse::REJECT);

    auto instrument = store.instruments().find(listingId);
    ASSERT_FALSE(instrument->second.buys().empty());
}

TEST(orderstore, modify_order)
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

    // check that exceeding threshold does not affect the store state
    uint64_t new_orderQuantity = Fixture::MAX_BUY + 2;
    auto response = store.consume(store.makeModifyOrder(order_id_2, new_orderQuantity));
    auto current_qty = store.instruments().find(listingId)->second.buys().find(order_id_2)->second.quantity;
    ASSERT_EQ(response, OrderStore::OrderResponse::REJECT);
    ASSERT_EQ(current_qty, orderQuantity);

    // check that correct value takes effect
    new_orderQuantity = Fixture::MAX_BUY - 2;
    response = store.consume(store.makeModifyOrder(order_id_2, new_orderQuantity));
    current_qty = store.instruments().find(listingId)->second.buys().find(order_id_2)->second.quantity;
    ASSERT_EQ(response, OrderStore::OrderResponse::ACCEPT);
    ASSERT_EQ(current_qty, new_orderQuantity);
}
