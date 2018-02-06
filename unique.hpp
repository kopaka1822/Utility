#pragma once
#include <algorithm>

namespace util
{
	// struct that ensures that a value can only be moved ond not copied
	// if the move constructor is called, the value will be initialized to the provided default value
	template<class T, T defaultValue = T(0.0)>
	struct Unique
	{
		T value;
		Unique() : value(defaultValue) {}
		explicit Unique(const T& v) : value(v) {}
		Unique(const Unique&) = delete;
		Unique& operator=(const Unique&) = delete;
		Unique(Unique&& m) noexcept
			: value(m.value)
		{
			m.value = defaultValue;
		}
		Unique& operator=(Unique&& m) noexcept
		{
			std::swap(value, m.value);
			return *this;
		}
	};
}