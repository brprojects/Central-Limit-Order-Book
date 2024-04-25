#ifndef ORDER_HPP
#define ORDER_HPP

class Limit;

class Order {
private:
    int idNumber;
    bool buyOrSell;
    int shares;
    int limit;
    Order *nextOrder;
    Order *prevOrder;
    Limit *parentLimit;

    friend class Limit;
public:
    Order(int _idNumber, bool _buyOrSell, int _shares, int _limit);

    int getOrderSize() const;
    int getOrderId() const;
    bool getBuyOrSell() const;
    Limit* getParentLimit() const;

    void cancel();

    void print() const;
};

#endif