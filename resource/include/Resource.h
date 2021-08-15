//
// Created by ltc on 2021/7/19.
//

#ifndef RESOURCE_RESOURCE_H
#define RESOURCE_RESOURCE_H

#include <memory>

class Resource : public std::enable_shared_from_this<Resource> {
public:
    virtual void increase();
    virtual void decrease();
    virtual void checkOut();
};


#endif //RESOURCE_RESOURCE_H
