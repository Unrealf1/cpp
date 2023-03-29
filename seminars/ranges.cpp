#include <ranges>
#include <algorithm>
#include <list>
#include <vector>
#include <string_view>

#include "util.hpp"

namespace rng = std::ranges;
namespace views = std::views;


struct Point {
    float x;
    float y;
};

struct Point3 {
    float x;
    float y;
    float z;
};

std::istream& operator>>(std::istream& istr, Point& point) {
    return istr >> point.x >> point.y;
}

std::ostream& operator<<(std::ostream& ostr, const Point3& point) {
    return ostr << point.x << " " << point.y << " " << point.z;
}

int main() {
    // Power of ranges
    auto points = views::istream<Point>(std::cin)
        | views::filter([](Point value) {
            return value.x < 20.0f;
        })
        | views::transform([](Point value) {
            return Point3(value.x, value.x * value.y, value.y);
        });
    // `points` has a very complex type, I do not recommend to
    // type it by hand. Better to use auto
    print("points type", type_of(points));
    print("");
    // views remember the type of the container and inner views! (unlike string_view)
    // this is how we can get a copy of a previous view:
    print("points base.base", type_of(points.base().base()));
    print("");
    // Until now _no_ points were read from std::cin.

    // similar to std::copy, but can work with ranges and support
    // projections (we'll talk about them later)
    rng::copy(points | views::drop(1), std::ostream_iterator<Point3>(std::cout, "\n"));
    // Notice that we don't need ANY container and dynamic memory! Only ranges and views

    // If iterated again, new values will be read from input, no caching is happening
    // However we can cache the values by hand:
    std::vector<Point3> cached_points;
    rng::copy(points, std::back_inserter(cached_points));

    print("type of points.begin()", type_of(points.begin())); // std::ranges::transform_view<...>::iterator
    print("");
    print("type of points.end()", type_of(points.end())); // std::ranges::transform_view<...>::sentinel
    print("");
    // begin() and end() have different types, this means, that
    // constructing vector like this is not possible
    // std::vector<Point3> cached_points(points.begin(), points.end());

    // However, we can do this, with c++23
    // auto cached_points2 = points | rng::to<std::vector>();

    // another example of ranges
    for (auto point : views::iota(0)
            | views::filter([](int v) {return v % 1 == 0;}) 
            | views::transform([](int v) {return v * v;})
            | views::drop(2)
            | views::take(20)) {
        print("item", point);
    }

    // Now, this should lead to UB, because vector will be destroyed by 
    // the time we get a pointer out of it
    auto ub_iter = rng::find(std::vector{1, 2, 3, 4}, 3);
    // However, if rvalue range is passed to such algorithm, library
    // will return a special type "std::ranges::dangling" it's an empty
    // type that isn't really an iterator, so you will get a _compile_ error,
    // NOT runtime UB!
    print("ub_iter", type_of(ub_iter)); // std::ranges::dangling
    // it's hard to achieve something similar with std::find, as you'll need
    // to materialize temporary vector to call two different methods on it

    // That being said, sometimes we DO want to pass an rvalue to the
    // fund algorithm and expect it to work correctly.
    std::vector<int> vvv{1, 2, 3};
    // "vvv | reverse" is same as "reverse(vvv)" 
    auto legit_iter = rng::find(vvv | views::reverse | views::drop(1), 3);
    // Here reverse view is constructed in place, but it has no _ownership_
    // of the elements and iterators of reverse and dtop are stil valid even after
    // death of the original view. 
    
    // string view is another example of case where rvalue should
    // be allowed to be passed to rng::find 
    auto string_view_iter = rng::find(std::string_view("abacaba"), 'c');
    
    // And library works correctly here, it returns a valid iterator that
    // can be dereferenced:
    print("legit", *legit_iter);
    print("string_view_iter", *string_view_iter);

    // About projections
    // cached_points is a vector of Point3, and I want to sort it by z coordinate
    // the old way is to write
    std::sort(cached_points.begin(), cached_points.end(), [](const Point3& a, const Point3& b) {
        return a.z < b.z;
    });
    // the new way is to use rng::sort with projections:
    rng::sort(cached_points, {}, [](Point3 value) {return value.z;});
    // this is cool and all, but we can do even better:
    rng::sort(cached_points, {}, &Point3::z);
    // member pointers aren't useless after all

    // Minor mindblower near the end of the file, not related
    // to the ranges
    // everyone knows, that you have to capture variables
    // in the lambda capture-list in order to use them inside:
    const int b = 0;
    auto lambda = [b](int a) {return a + b;};
    // But what if I told you, that sometimes, if everything 
    // is set up just right, you can do this and it still compiles:
    auto lambda2 = [](int a) {return a + b;};

    // lifehack to make code go faster:
    // if you are 100% sure, that item IS in the collection
    // but you need to find it's position, you can use
    // rng::find(begin(), std::unreachable_sentinel);
    // unreachable sentinel will compare false with everything
    // thus compiler have enough information to remove this check
    // from the code completely
}
