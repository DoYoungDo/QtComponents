#ifndef SIMPLEINVOKER_H
#define SIMPLEINVOKER_H

#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>

// ------------------- 核心Invoker模板 -------------------
template<int EventID, typename R, typename... Args>
class SimpleInvoker {
public:
    using Callback = std::function<R(Args...)>;
    using StopCond = std::function<bool(R)>;

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

    // 调用有序，遇到某返回值即中断
    static bool invokeUntil(StopCond stopCond, Args... args) {
        for (auto& cb : defaultList()) {
            R ret = cb(args...);
            bool stop = stopCond(ret);
            if (stop) return stop;
        }
        for (auto& pair : orderedList()) {
            R ret = pair.cb(args...);
            bool stop = stopCond(ret);
            if (stop) return stop;
        }
        return false;
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

template<int EventID, typename... Args>
class SimpleInvoker<EventID, void, Args...> {
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
template<int EventID, typename R = void, typename... Args>
struct SimpleAutoReg {
    SimpleAutoReg(SimpleInvoker<EventID, R, Args...>::Callback cb) {
        SimpleInvoker<EventID, R, Args...>::reg(std::move(cb));
    }
};

// 有序自动注册器
template<int EventID, typename R = void, typename... Args>
struct SimpleAutoRegOrdered {
    SimpleAutoRegOrdered(int order, SimpleInvoker<EventID, R, Args...>::Callback cb) {
        SimpleInvoker<EventID, R, Args...>::regOrdered(order, std::move(cb));
    }
};


#endif // SIMPLEINVOKER_H
