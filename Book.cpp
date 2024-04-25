#include "Book.hpp"
#include "Order.hpp"
#include "Limit.hpp"
#include <iostream>
#include <algorithm>

Book::Book() : buyTree(nullptr), sellTree(nullptr), lowestSell(nullptr), highestBuy(nullptr){}

// When deleting the book need to ensure all used memory is freed
Book::~Book()
{
    for (auto& [id, order] : orderMap) {
        // order->print();
        delete order;
    }
    orderMap.clear();

    for (auto& [limitPrice, limit] : limitBuyMap) {
        delete limit;
    }
    limitBuyMap.clear();

    for (auto& [limitPrice, limit] : limitSellMap) {
        delete limit;
    }
    limitSellMap.clear();
}

Limit* Book::getBuyTree() const
{
    return buyTree;
}

Limit* Book::getSellTree() const
{
    return sellTree;
}

// Add a new order to the book
void Book::addOrder(int orderId, bool buyOrSell, int shares, int limitPrice)
{
    Order* newOrder = new Order(orderId, buyOrSell, shares, limitPrice);
    orderMap.emplace(orderId, newOrder);

    auto& limitMap = buyOrSell ? limitBuyMap : limitSellMap;

    if (limitMap.find(limitPrice) == limitMap.end())
    {
        addLimit(limitPrice, newOrder->getBuyOrSell());
    }
    limitMap.at(limitPrice)->append(newOrder);
}

// Add a new limit to the book
void Book::addLimit(int limitPrice, bool buyOrSell)
{
    auto& limitMap = buyOrSell ? limitBuyMap : limitSellMap;
    auto& tree = buyOrSell ? buyTree : sellTree;
    auto& bookEdge = buyOrSell ? highestBuy : lowestSell;

    Limit* newLimit = new Limit(limitPrice, buyOrSell);
    limitMap.emplace(limitPrice, newLimit);

    if (tree == nullptr)
    {
        tree = newLimit;
        bookEdge = newLimit;
    } else
    {
        Limit* root = insert(tree, newLimit);
        updateBookEdgeInsert(newLimit);
    }
}

// Insert a limit into its binary search tree
Limit* Book::insert(Limit* root, Limit* limit, Limit* parent)
{
    if (root == nullptr)
    {
        limit->setParent(parent);
        updateLimitHeightsOnInsert(limit);
        return limit;
    }
    if (limit->getLimitPrice() < root->getLimitPrice())
    {
        root->setLeftChild(insert(root->getLeftChild(), limit, root));
    } else if (limit->getLimitPrice() > root->getLimitPrice()) 
    {
        root->setRightChild(insert(root->getRightChild(), limit, root));
    }

    return root;
}

// Update the edge of the book if new limit is on edge of the book
void Book::updateBookEdgeInsert(Limit* newLimit)
{
    if (newLimit->getBuyOrSell())
    {
        if (newLimit->getLimitPrice() > highestBuy->getLimitPrice())
        {
            highestBuy = newLimit;
        }
    } else
    {
        if (newLimit->getLimitPrice() < lowestSell->getLimitPrice())
        {
            lowestSell = newLimit;
        }
    }
}

// Update the edge of the book if current edge of the book is emptied
void Book::updateBookEdgeRemove(Limit* limit)
{
    auto& bookEdge = limit->getBuyOrSell() ? highestBuy : lowestSell;
    if (limit == bookEdge)
    {
        // Need to use a balanced tree for this to work (AVL Tree)
        bookEdge = bookEdge->getParent();
    }
}

// Delete an order from the book
void Book::cancelOrder(int orderId)
{
    Order* order = searchOrderMap(orderId);
    bool buyOrSell = order->getBuyOrSell();

    if (order != nullptr)
    {
        order->cancel();
            if (order->getParentLimit()->getSize() == 0)
            {
                updateBookEdgeRemove(order->getParentLimit());

                deleteFromLimitMaps(order->getParentLimit()->getLimitPrice(), buyOrSell);
                delete order->getParentLimit();
            }
        deleteFromOrderMap(orderId);
        delete order;
    }
}

// Search the order map to find an order
Order* Book::searchOrderMap(int orderId) const
{
    auto it = orderMap.find(orderId);
    if (it != orderMap.end())
    {
        return it->second;
    } else
    {
        std::cout << "No order number " << orderId << std::endl;
        return nullptr;
    }
}

void Book::deleteFromOrderMap(int orderId)
{
    orderMap.erase(orderId);
}

// Search the limit maps to find a limit
Limit* Book::searchLimitMaps(int limitPrice, bool buyOrSell) const
{
    auto& limitMap = buyOrSell ? limitBuyMap : limitSellMap;

    auto it = limitMap.find(limitPrice);
    if (it != limitMap.end())
    {
        return it->second;
    } else
    {
        std::cout << "No "<< (buyOrSell ? "buy " : "sell ") << " limit at " << limitPrice << std::endl;
        return nullptr;
    }
}

void Book::deleteFromLimitMaps(int limitPrice, bool buyOrSell)
{
    auto& limitMap = buyOrSell ? limitBuyMap : limitSellMap;
    limitMap.erase(limitPrice);
}

void Book::updateLimitHeightsOnInsert(Limit* limit)
{
    int i = 2;
    while (limit->getParent() != nullptr && limit->getParent()->getHeight() < i)
    {
        limit->getParent()->setHeight(i);
        i++;
        limit = limit->getParent();
    }
}

void Book::printLimit(int limitPrice, bool buyOrSell) const
{
    searchLimitMaps(limitPrice, buyOrSell)->print();
}

void Book::printOrder(int orderId) const
{
    searchOrderMap(orderId)->print();
}

void Book::printBookEdges() const
{
    std::cout << "Buy edge: " << highestBuy->getLimitPrice() 
    << "Sell edge: " << lowestSell->getLimitPrice() << std::endl;
}

// In order traversal of the binary search tree
std::vector<int> Book::inOrderTreeTraversal(Limit* root)
{
    std::vector<int> result;
    if (root == nullptr)
        return result;

    std::vector<int> leftSubtree = inOrderTreeTraversal(root->getLeftChild());
    result.insert(result.end(), leftSubtree.begin(), leftSubtree.end());
    
    result.push_back(root->getLimitPrice());

    std::vector<int> rightSubtree = inOrderTreeTraversal(root->getRightChild());
    result.insert(result.end(), rightSubtree.begin(), rightSubtree.end());

    return result;
}

// Pre order traversal of the binary search tree
std::vector<int> Book::preOrderTreeTraversal(Limit* root)
{
    std::vector<int> result;
    if (root == nullptr)
        return result;

    result.push_back(root->getLimitPrice());

    std::vector<int> leftSubtree = preOrderTreeTraversal(root->getLeftChild());
    result.insert(result.end(), leftSubtree.begin(), leftSubtree.end());
    
    std::vector<int> rightSubtree = preOrderTreeTraversal(root->getRightChild());
    result.insert(result.end(), rightSubtree.begin(), rightSubtree.end());

    return result;
}

// Post order traversal of the binary search tree
std::vector<int> Book::postOrderTreeTraversal(Limit* root)
{
    std::vector<int> result;
    if (root == nullptr)
        return result;

    std::vector<int> leftSubtree = postOrderTreeTraversal(root->getLeftChild());
    result.insert(result.end(), leftSubtree.begin(), leftSubtree.end());
    
    std::vector<int> rightSubtree = postOrderTreeTraversal(root->getRightChild());
    result.insert(result.end(), rightSubtree.begin(), rightSubtree.end());
    
    result.push_back(root->getLimitPrice());

    return result;
}

int Book::limitHeightDifference(Limit *t) {
    int l_height = t->getLeftChild()->getHeight();
    int r_height = t->getRightChild()->getHeight();
    int b_factor = l_height - r_height;
    return b_factor;
}

void Book::removeLimit(Limit* limit){

    auto leftOrRightChild = (limit->getLimitPrice() < limit->getParent()->getLimitPrice()) ? limit->getParent()->getLeftChild() : limit->getParent()->getRightChild();

    // Case 1: Node with only one child or no child
    if (limit->getLeftChild() == nullptr) {
        leftOrRightChild = limit->getRightChild();
        limit->getRightChild()->setParent(limit->getParent());
        delete limit;
        return;
    } else if (limit->getRightChild() == nullptr) {
        leftOrRightChild = limit->getLeftChild();
        limit->getLeftChild()->setParent(limit->getParent());
        delete limit;
        return;
    }

    // Case 2: Node with two children
    Limit* temp = limit->getRightChild();
    while (temp->getLeftChild() != nullptr) {
        temp = temp->getLeftChild();
    }
    temp->getParent()->setLeftChild(nullptr);
    temp->setParent(limit->getParent());
    temp->setLeftChild(limit->getLeftChild());
    temp->getLeftChild()->setParent(temp);
    temp->setRightChild(limit->getRightChild());
    temp->getRightChild()->setParent(temp);

    delete limit;
}