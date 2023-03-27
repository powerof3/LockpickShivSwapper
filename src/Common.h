#pragma once

struct string_hash
{
	using is_transparent = void;  // enable heterogeneous overloads
	using is_avalanching = void;  // mark class as high quality avalanching hash

	[[nodiscard]] std::uint64_t operator()(std::string_view str) const noexcept
	{
		return ankerl::unordered_dense::hash<std::string_view>{}(str);
	}
};

template <class D>
using StringMap = ankerl::unordered_dense::map<std::string, D, string_hash, std::equal_to<>>;
using StringSet = ankerl::unordered_dense::set<std::string, string_hash, std::equal_to<>>;

template <class K, class D>
using Map = ankerl::unordered_dense::map<K, D>;

// bidirectional map
template <class K, class V>
class BiMap
{
public:
	BiMap(std::initializer_list<std::pair<K, V>> a_list)
	{
		for (const auto& [key, value] : a_list) {
			_forward.emplace(key, value);
			_reverse.emplace(value, key);
		}
	}

	// update both keys and values
    bool assign(const K& key, const V& value)
	{
		if (auto fIt = _forward.find(key); fIt != _forward.end()) {
			if (auto rIt = _reverse.find(fIt->second); rIt != _reverse.end()) {
				_forward[key] = value;

				_reverse.erase(rIt);
				_reverse.emplace(value, key);

				return true;
			}
		}
		return false;
	}

	// get value for key
    const V& value(const K& key) const
	{
		return _forward.at(key);
	}

	// get key for a value
    std::optional<K> key(const V& value) const
	{
		auto it = _reverse.find(value);
		if (it != _reverse.end()) {
			return it->second;
		} else {
			return std::nullopt;
		}
	}

private:
	Map<K, V> _forward;
	Map<V, K> _reverse;
};
