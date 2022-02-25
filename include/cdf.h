#pragma once

#include "common.h"

#include <vector>

class cdf
{
public:
	cdf(int count)
	{
		m_cdf.reserve(count + 1);
		m_cdf.push_back(0.0);
	}

	/// Access an entry by its index
	inline fType operator[](size_t entry) const 
	{
		return m_cdf[entry + 1] - m_cdf[entry];
	}

	inline void append(fType value) 
	{
		m_cdf.push_back(m_cdf[m_cdf.size() - 1] + value);
	}

	inline fType normalize() 
	{
		fType sum = m_cdf[m_cdf.size() - 1];
		if (sum > 1e-6)
		{
			m_normalization = 1.0 / sum;
			for (size_t i = 1; i < m_cdf.size(); i++)
				m_cdf[i] *= m_normalization;

			m_cdf[m_cdf.size() - 1] = 1.0;
		}
		else
			m_normalization = 0.0;

		return sum;
	}

	inline size_t sample(fType sampleValue) const 
	{
		std::vector<fType>::const_iterator entry = std::lower_bound(m_cdf.begin(), m_cdf.end(), sampleValue);
		size_t index = std::min(m_cdf.size() - 2, (size_t)std::max((ptrdiff_t)0, entry - m_cdf.begin() - 1));

		/* Handle a rare corner-case where a entry has probability 0
		   but is sampled nonetheless */
		while (operator[](index) == 0 && index < m_cdf.size() - 1)
			++index;

		return index;
	}

	inline size_t sample(fType sampleValue, fType& pdf) const 
	{
		size_t index = sample(sampleValue);
		pdf = operator[](index);
		return index;
	}

	inline fType getNormalization() const 
	{
		return m_normalization;
	}

private:
	fType m_normalization;
	std::vector<fType> m_cdf;
};