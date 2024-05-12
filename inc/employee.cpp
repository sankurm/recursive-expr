#include <iostream>
#include <algorithm>
#include <chrono>
#include <span>
#include <cstdint>
#include <variant>

template<typename T, int occurrence = 0>
struct field
{
    using value_type = T;
    const int n = occurrence;
    T val;
};

enum class initiative { csr, volunteer, referral };

struct job_level
{
    int val;
    auto operator<=>(const job_level&) const = default;
};

struct joining_date
{
    std::chrono::system_clock::time_point val;
};

struct wfh_percent
{
    int val;
    auto operator<=>(const wfh_percent&) const = default;
};

struct Employee
{
    std::chrono::system_clock::time_point doj;
    int jl;
    int wfh_pct;
    initiative initiatives_taken;
    std::chrono::system_clock::time_point doe;
};

template<typename T, int occurrence>
const auto get(const Employee& emp) {
    static_assert(false, "Unsupported field occurrence");
}

template<int occurrence>
const auto get(const Employee& emp) {
    return emp.doj;
}

template<>
const auto get<std::chrono::system_clock::time_point, 0>(const Employee& emp) {
    return emp.doj;
}

template<>
const auto get<std::chrono::system_clock::time_point, 1>(const Employee& emp) {
    return emp.doe;
}

template<typename T>
const auto get(const Employee& emp) {
    return get<T, 0>(emp);
}

template<typename Data, typename T, int occurrence>
const auto get(const Data& emp) {
    static_assert(false, "Unsupported field occurrence");
}

template<>
const auto get<std::span<uint8_t>, ip, 0>(const std::span<uint8_t>& ip_header) {
    ip ret;
    std::copy_n(ip_header.data() + offsetof(ip_hdr, src_ip), sizeof(ip), &ret);
    return ret;
}

int const any = 0xffffffff;

using job_level_fld = field<job_level>;
using wfh_percent_fld = field<wfh_percent>;
using joining_date_fld = field<joining_date>;

using expr_value = std::variant<job_level_fld, wfh_percent_fld, joining_date_fld>;

template<typename T, typename U>
struct or_t
{
    T lhs;
    U rhs;
};



int main(int, char**) {
    Employee e1;

    job_level_fld jlf{10};
    auto print = [](auto val) { std::cout << val; };
    jlf.apply(print);

    auto equal = [&e1](int val) -> bool { return get<typename job_level_fld::value_type, job_level_fld::n>(e1) == val; };
    jlf.apply(equal);


    
    auto doj = get<std::chrono::system_clock::time_point, 0>(e1);
    auto doe = get<std::chrono::system_clock::time_point, 1>(e1);

    field<ip, any> host;
    
    //auto doj = get<joining_date, 0>(e1);
    //auto doe = get<joining_date, 1>(e1);
    //auto unexp = get<joining_date, 2>(e1);
}