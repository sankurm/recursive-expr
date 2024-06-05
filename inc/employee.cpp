#include <algorithm>
#include <array>
#include <chrono>
#include <concepts>
#include <cstdint>
#include <functional>
#include <iostream>
#include <span>
#include <sstream>
#include <variant>
#include <vector>

template<typename F>
concept a_field = requires(F f) {
    typename F::value_type;
    { F::n } -> std::convertible_to<int>;
};

template<typename Field, typename Data>
concept field_applies_to_data = a_field<Field> && requires(Data d, Field f) {
    get_impl<Data, typename Field::value_type, Field::n>(d);
};

//template<typename D, typename Fn>

template<typename T, int occurrence = 0>
struct field
{
    using value_type = T;
    static const int n = occurrence;
    T val;

    template<typename Callable>
    //requires std::invocable<Callable, field>
    requires std::is_invocable_v<Callable, field>
    auto apply(Callable&& fn) -> std::invoke_result_t<Callable, field> {
        return std::invoke(std::forward<Callable>(fn), *this);
    }
};
template<typename T, int occ>
std::ostream& operator<<(std::ostream& out, const field<T, occ>& jl) {
    return out << jl.val;
}

template<typename T>
struct value
{
    T val;
    auto operator<=>(const value<T>&) const = default;
};
template<typename T>
std::ostream& operator<<(std::ostream& out, const value<T>& v) {
    return out << v.val;
}

using job_level = value<int>;
/*struct job_level
{
    int val;
    auto operator<=>(const job_level&) const = default;
};
std::ostream& operator<<(std::ostream& out, const job_level& jl) {
    return out << jl.val;
}*/

using joining_date = value<std::chrono::system_clock::time_point>;
/*struct joining_date
{
    std::chrono::system_clock::time_point val;
    auto operator<=>(const joining_date&) const = default;
};*/

struct wfh_percent
{
    int val;
    auto operator<=>(const wfh_percent&) const = default;
};

enum class initiative { csr, volunteer, referral };

static_assert(a_field<field<job_level, 0>>, "field<job_level, 0> does not model the concept `a_field`");

struct Employee
{
    std::chrono::system_clock::time_point doj;
    int jl;
    int wfh_pct;
    initiative initiatives_taken;
    std::chrono::system_clock::time_point doe;
};

template<typename Data, typename T, int occurrence>
auto get_impl(const Data& d); /*{
    static_assert(false, "Unsupported field occurrence");
}*/

/*template<typename T, int occurrence>
const auto get(const Employee& emp);*/ /*{
    static_assert(false, "Unsupported field occurrence");
}*/

/*template<int occurrence>
const auto get(const Employee& emp) {
    return emp.doj;
}*/

template<typename Data, a_field F>
auto get(const Data& d) {
    return get_impl<Data, typename F::value_type, F::n>(d);
}

template<>
auto get_impl<Employee, job_level, 0>(const Employee& emp) {
    return job_level{emp.jl};
}

template<>
auto get_impl<Employee, wfh_percent, 0>(const Employee& emp) {
    return wfh_percent{emp.wfh_pct};
}

template<>
auto get_impl<Employee, initiative, 0>(const Employee& emp) {
    return emp.initiatives_taken;
}

template<>
auto get_impl<Employee, joining_date, 0>(const Employee& emp) {
    return emp.doj;
}

template<>
auto get_impl<Employee, std::chrono::system_clock::time_point, 0>(const Employee& emp) {
    return emp.doj;
}

template<>
auto get_impl<Employee, std::chrono::system_clock::time_point, 1>(const Employee& emp) {
    return emp.doe;
}

template<typename Data, typename T>
requires (not a_field<T>)
auto get(const Data& d) {
    return get_impl<Data, T, 0>(d);
}

using ip = uint32_t;

struct ip_hdr
{
    uint8_t ihl;
    uint8_t flags;
    uint16_t len;
    ip src_ip;
    ip dst_ip;
};

template<>
auto get_impl<std::span<const uint8_t>, ip, 0>(const std::span<const uint8_t>& ip_header) {
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

std::string to_string(const std::chrono::system_clock::time_point& tp) {
    std::ostringstream out;
    out << std::chrono::floor<std::chrono::microseconds>(tp.time_since_epoch());
    return std::move(out).str();
}

//auto print = []<typename T, int occ>(const field<T, occ>& f) { std::cout << f.val << '\n'; };
auto print = []<a_field F>(const F& f) {
    if constexpr (requires { std::cout << f.val; }) {
        std::cout << f.val << '\n';
    } else {
        std::cout << to_string(f.val) << '\n';
    }
};
auto hex_print = []<a_field F>(const F& f) {
    if constexpr (requires { std::cout << f.val; }) {
        std::cout << std::hex << f.val << '\n';
    } else {
        std::cout << std::hex << to_string(f.val) << '\n';
    }
};


int main(int, char**) {
    Employee e1;

    //Work with a field
    job_level_fld jlf{10};

    //Print it
    jlf.apply(print);

    //Match it
    auto equal = [&e1]<a_field F>(const F& fld) -> bool {
		const auto actual_val = get<Employee, F>(e1);
		return actual_val == fld.val;
	};
    jlf.apply(equal);

    //Fields with multiple occurrences
    auto doj = field<std::chrono::system_clock::time_point, 0>{};
    auto doe = field<std::chrono::system_clock::time_point, 1>{};
    doj.apply(print);
    doe.apply(print);

    auto extract = [&e1]<a_field F>(const F& fld) -> typename F::value_type {
        return get<Employee, F>(e1);
    };
    //std::cout << "Extracted DOJ: " << extract(doj) << '\n';
    //std::cout << "Extracted DOE: " << extract(doe) << '\n';

    //Work with binary data
    field<ip, 0> src_ip{0xa0b0c0d0};
    field<ip, 1> dst_ip{0xa0b0c0d0};
    constexpr std::array<uint8_t, 20> ipheader = {
        0x04, 0x00, 0x08, 0x00, 0x67, 0x6f, 0x70, 0x61,  0x63, 0x6b, 0x65, 0x74, 0x02, 0x00, 0x05, 0x00,
        0x61, 0x6d, 0x64, 0x36
    };
    const std::span<const uint8_t> iph{ipheader};

    src_ip.apply(hex_print);
    dst_ip.apply(hex_print);
    
    //auto doj = get_impl<joining_date, 0>(e1);
    //auto doe = get_impl<joining_date, 1>(e1);
    //auto unexp = get_impl<joining_date, 2>(e1);
}
