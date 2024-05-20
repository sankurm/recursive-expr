#include <iostream>
#include <algorithm>
#include <chrono>
#include <span>
#include <cstdint>
#include <variant>
#include <concepts>
#include <functional>

template<typename F>
concept a_field = requires(F f) {
    typename F::value_type;
    { F::n } -> std::convertible_to<int>;
};

template<typename F, typename D>
concept field_applies_to_data = requires(D d, F f) {
    true;
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

enum class initiative { csr, volunteer, referral };

struct job_level
{
    int val;
    auto operator<=>(const job_level&) const = default;
};
std::ostream& operator<<(std::ostream& out, const job_level& jl) {
    return out << jl.val;
}

struct joining_date
{
    std::chrono::system_clock::time_point val;
};

struct wfh_percent
{
    int val;
    auto operator<=>(const wfh_percent&) const = default;
};

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
auto get_impl(const Data& emp); /*{
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
auto get(const Data& emp) {
    return get_impl<Data, typename F::value_type, F::n>(emp);
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
auto get(const Data& emp) {
    return get_impl<Data, T, 0>(emp);
}

using ip = uint32_t;

struct ip_hdr
{
    ip src_ip;
    ip dst_ip;
};

template<>
auto get_impl<std::span<uint8_t>, ip, 0>(const std::span<uint8_t>& ip_header) {
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
    //auto print = []<typename T, int occ>(const field<T, occ>& f) { std::cout << f.val << '\n'; };
    auto print = []<a_field F>(const F& f) { std::cout << f.val << '\n'; };
    jlf.apply(print);

    auto equal = [&e1]<a_field F>(const F& fld) -> bool {
		const auto actual_val = get<Employee, F>(e1);
		return actual_val == fld.val;
		//return get<typename F::value_type, F::n>(e1) == fld.val;
	};
    jlf.apply(equal);


    
    auto doj = get_impl<Employee, std::chrono::system_clock::time_point, 0>(e1);
    auto doe = get_impl<Employee, std::chrono::system_clock::time_point, 1>(e1);

    //field<ip, any> host;
    
    //auto doj = get_impl<joining_date, 0>(e1);
    //auto doe = get_impl<joining_date, 1>(e1);
    //auto unexp = get_impl<joining_date, 2>(e1);
}
