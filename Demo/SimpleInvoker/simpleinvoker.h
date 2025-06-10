#ifndef SIMPLEINVOKER_H
#define SIMPLEINVOKER_H

#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>

// ------------------- 核心Invoker模板 -------------------
template<int EventID, typename... Args>
class SimpleInvoker {
public:
    using Callback = std::function<void(Args...)>;

    // 无序注册
    static void reg(Callback cb) {
        defaultList().emplace_back(std::move(cb));
    }

    // 有序注册，order越小越先执行
    static void regOrdered(int order, Callback cb) {
        orderedList().emplace_back(OrderedCb{order, std::move(cb)});
        std::sort(orderedList().begin(), orderedList().end(),
                  [](const OrderedCb& a, const OrderedCb& b) { return a.order < b.order; });
    }

    // 调用所有回调（先无序，后有序）
    static void invoke(Args... args) {
        for (auto& cb : defaultList()) cb(args...);
        for (auto& pair : orderedList()) pair.cb(args...);
    }

    // 仅调用有序
    static void invokeOrdered(Args... args) {
        for (auto& pair : orderedList()) pair.cb(args...);
    }
    // 仅调用无序
    static void invokeUnordered(Args... args) {
        for (auto& cb : defaultList()) cb(args...);
    }

private:
    struct OrderedCb { int order; Callback cb; };
    static std::vector<Callback>& defaultList() {
        static std::vector<Callback> s;
        return s;
    }
    static std::vector<OrderedCb>& orderedList() {
        static std::vector<OrderedCb> s;
        return s;
    }
};

// ------------------- 自动注册器（放在外部） -------------------

// 无序自动注册器
template<int EventID, typename... Args>
struct SimpleAutoReg {
    SimpleAutoReg(std::function<void(Args...)> cb) {
        SimpleInvoker<EventID, Args...>::reg(std::move(cb));
    }
};

// 有序自动注册器
template<int EventID, typename... Args>
struct SimpleAutoRegOrdered {
    SimpleAutoRegOrdered(int order, std::function<void(Args...)> cb) {
        SimpleInvoker<EventID, Args...>::regOrdered(order, std::move(cb));
    }
};


// #define SIMPLE_INVOKER_TEST
#ifdef SIMPLE_INVOKER_TEST
// ------------------- 用法示例 -------------------

// 普通函数（无序）
void onIntEvent(int x) {
    std::cout << "普通函数收到: " << x << std::endl;
}

// 全局静态注册（main之前即自动注册，无序）
static SimpleAutoReg<1, int> reg1(onIntEvent);
static SimpleAutoReg<1, int> reg2([](int x){
    std::cout << "lambda收到: " << x << std::endl;
});

// 全局静态注册（有序，order越小越先）
static SimpleAutoRegOrdered<1, int> reg3(5, [](int x){
    std::cout << "[order=5] 有序收到: " << x << std::endl;
});
static SimpleAutoRegOrdered<1, int> reg4(1, [](int x){
    std::cout << "[order=1] 有序收到: " << x << std::endl;
});
static SimpleAutoRegOrdered<1, int> reg5(10, [](int x){
    std::cout << "[order=10] 有序收到: " << x << std::endl;
});

int main() {
    std::cout << "--- invoke（所有无序+有序都调用） ---\n";
    SimpleInvoker<1, int>::invoke(100);

    std::cout << "--- invokeOrdered（只调用有序） ---\n";
    SimpleInvoker<1, int>::invokeOrdered(200);

    std::cout << "--- invokeUnordered（只调用无序） ---\n";
    SimpleInvoker<1, int>::invokeUnordered(300);

    return 0;
}
#endif


#endif // SIMPLEINVOKER_H
