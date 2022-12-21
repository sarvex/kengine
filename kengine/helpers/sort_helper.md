# [sort_helper](sort_helper.hpp)

Helper functions to sort sets of entities.

## Members

### get_sorted_entities

```cpp
template<typename ... Comps, typename Registry, typename Pred>
auto get_sorted_entities(Registry && r, Pred && pred) noexcept;

template<size_t MaxCount, typename ... Comps, typename Registry, typename Pred>
auto get_sorted_entities(Registry && r, Pred && pred) noexcept;
```

Returns a list of all entities with the `Comps` components, sorted according to `pred`.

These are returned under the form of a container of `tuple<entt::entity, Comps * ...>` instead of the usual `tuple<entt::entity, Comps & ...>` returned by `r.view<Comps>().each()`, so you need to dereference the component pointers.

`Pred` is a functor with the `bool(tuple<entt::entity, Comps * ...> lhs, tuple<entt::entity, Comps * ...> rhs)` signature, which returns `true` if `lhs` should appear before `rhs`.

If `MaxCount` is provided, the function returns a fixed-size `putils::vector` instead of `std::vector`, avoiding the heap allocation.

#### Example

```cpp
const auto sorted = sort_helper::get_sorted_entities<64, data::name, data::transform>(
    r,
    [](const auto & lhs, const auto & rhs) {
        // lhs and rhs are std::tuple<Entity, data::name *, data::transform *>;
        // std::get<1> returns a data::name *
        return strcmp(std::get<1>(lhs)->name, std::get<1>(rhs)->name) < 0;
    }
);

for (const auto & [e, name, transform] : sorted) {
    std::cout << name->name << std::endl;
    std::cout << transform->bounding_box << std::endl;
}
```

### get_name_sorted_entities

```cpp
template<typename ... Comps, typename Registry>
auto get_name_sorted_entities(Registry && r) noexcept;

template<size_t MaxCount, typename ... Comps, typename Registry>
auto get_name_sorted_entities(Registry && r) noexcept;
```

Returns a list of all entities with the `Comps` components and a [name](../data/name.md), sorted in alphabetical order according to their name.

These are returned under the form of a container of `tuple<Entity, data::name *, Comps * ...>`.

If `MaxCount` is provided, the function returns a fixed-size `putils::vector` instead of `std::vector`, avoiding the heap allocation.

#### Example

```cpp
const auto sorted = sort_helper::get_name_sorted_entities<64, data::transform>(r);

for (const auto & [e, name, transform] : sorted) {
    std::cout << name->name << std::endl;
    std::cout << transform->bounding_box << std::endl;
}
```